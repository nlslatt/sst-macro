/*
 *  This file is part of SST/macroscale:
 *               The macroscale architecture simulator from the SST suite.
 *  Copyright (c) 2009 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the LICENSE file in the top
 *  SST/macroscale directory.
 */

#include <sumi-mpi/mpi_comm/mpi_comm_factory.h>
#include <sumi-mpi/mpi_comm/mpi_comm.h>
#include <sumi-mpi/mpi_comm/mpi_comm_cart.h>
#include <sumi-mpi/mpi_api.h>
#include <sumi-mpi/sstmac_mpi_integers.h>
#include <sumi-mpi/mpi_types.h>
#include <sstmac/common/thread_lock.h>
#include <sstmac/common/thread_info.h>
#include <sprockit/errors.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <stdint.h>
#include <iterator>

namespace sumi {


//
// Build comm_world using information retrieved from the environment.
//
mpi_comm_factory::mpi_comm_factory(app_id aid, mpi_api* parent) :
  parent_(parent),
  aid_(aid),
  mpirun_np_(0),
  next_id_(1)
{
}

//
// Goodbye.
//
mpi_comm_factory::~mpi_comm_factory()
{
  delete worldcomm_;
  delete selfcomm_;
}

//
// Initialize.
//
void
mpi_comm_factory::init(app_manager* env, int rank)
{
  next_id_ = 1;

  mpirun_np_ = env->nproc();

  MPI_Comm cid(0);

  mpi_group* g = new mpi_group(mpirun_np_);

  worldcomm_ = new mpi_comm(MPI_COMM_WORLD, rank, g, env, aid_);

  std::vector<task_id> selfp;
  selfp.push_back(task_id(rank));

  mpi_group* g2 = new mpi_group(selfp);
  selfcomm_ = new mpi_comm(MPI_COMM_SELF, int(0), g2, env, aid_);
}

void
mpi_comm_factory::finalize()
{
  parent_ = 0;
  worldcomm_ = 0;
  selfcomm_ = 0;
}

//
// Duplicate a communicator.
//
mpi_comm*
mpi_comm_factory::comm_dup(mpi_comm* caller)
{
  mpi_comm* ret = this->comm_create(caller, caller->group_);
  ret->dup_keyvals(caller);
  return ret;
}

//
// Make the given mpiid refer to a newly created communicator.
//
mpi_comm*
mpi_comm_factory::comm_create(mpi_comm* caller, mpi_group* group)
{
  int inputID = next_id_;
  int outputID = 0;
  parent_->allreduce(&inputID, &outputID, 1, MPI_INT, MPI_MAX, caller->id());

  MPI_Comm cid = outputID;

  std::pair<app_id, int> index = std::make_pair(aid_, cid);

  //now find my rank
  int newrank = group->rank_of_task(caller->my_task());

  next_id_ = cid + 1;

  if (newrank >= 0) {
    return new mpi_comm(cid, newrank, group, worldcomm_->env_, aid_);
  }
  else {
    return mpi_comm::comm_null;
  }

}

typedef std::map<int, std::list<int> > key_to_ranks_map;
#if !SSTMAC_DISTRIBUTED_MEMORY || SSTMAC_MMAP_COLLECTIVES

static sstmac::thread_lock split_lock;
typedef std::map<int, key_to_ranks_map> color_to_key_map;
//comm id, comm root task id, tag

struct comm_split_entry {
  int* buf;
  int refcount;
  comm_split_entry() : buf(0), refcount(0) {}
};
static std::map<int, std::map<int, std::map<int, comm_split_entry> > > comm_split_entries;
#endif


//
// MPI_Comm_split.
//
mpi_comm*
mpi_comm_factory::comm_split(mpi_comm* caller, int my_color, int my_key)
{
  key_to_ranks_map key_map;
  int mydata[3];
  mydata[0] = next_id_;
  mydata[1] = my_color;
  mydata[2] = my_key;
#if SSTMAC_DISTRIBUTED_MEMORY && !SSTMAC_MMAP_COLLECTIVES
  if (my_color < 0){ //I'm not part of this!
    return mpi_comm::comm_null;
  }
  int* result = new int[3*caller->size()];
  parent_->allgather(&mydata, 3, MPI_INT,
                     result, 3, MPI_INT,
                     caller->id());
#else
  split_lock.lock();
  int root = caller->peer_task(int(0));
  int tag = caller->next_collective_tag();
  comm_split_entry& entry = comm_split_entries[int(caller->id())][root][tag];
  char fname[256];
  size_t len = 3*caller->size()*sizeof(int);
  entry.refcount++;
  if (entry.buf == 0){
#if SSTMAC_MMAP_COLLECTIVES
    sprintf(fname, "%d.%d.%d", int(caller->id()), root, tag);
    int fd = shm_open(fname, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
    if (fd < 0){ //oops, someone else already created it
      fd = shm_open(fname, O_RDWR, S_IRWXU);
    }
    if (fd < 0){
      spkt_throw_printf(sprockit::value_error,
        "invalid fd %d shm_open on %s: error=%d",
        fd, fname, errno);
    }
    ftruncate(fd, len);
    void* bufptr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (bufptr == ((void*)-1)){
      spkt_throw_printf(sprockit::value_error,
        "bad mmap on shm_open %s:%d: error=%d",
        fname, fd, errno);
    }
    entry.buf = (int*)bufptr;
#else
    entry.buf = new int[3*caller->size()];
#endif
  }
  split_lock.unlock();

  //just model the allgather
  parent_->allgather(NULL, 3, MPI_INT,
                     NULL, 3, MPI_INT,
                     caller->id());
  int* result = entry.buf;
#endif

  if (my_color < 0){ //I'm not part of this!
    return mpi_comm::comm_null;
  }

  int cid = -1;
  int ninput_ranks = caller->size();
  int new_comm_size = 0;
  for (unsigned rank = 0; rank < ninput_ranks; rank++) {
    int* thisdata = result + 3*rank;

    int comm_id = thisdata[0];
    int color = thisdata[1];
    int key = thisdata[2];

    if (color >= 0 && color == my_color){
      key_map[key].push_back(rank);
      ++new_comm_size;
    }

    if (comm_id > cid) {
      cid = comm_id;
    }
  }


  //the next id I use needs to be greater than this
  next_id_ = cid + 1;

  std::vector<task_id> task_list(new_comm_size);

  key_to_ranks_map::iterator it, end = key_map.end();
  //iterate map in sorted order
  int next_rank = 0;
  int my_new_rank = -1;
  for (it=key_map.begin(); it != end; ++it){
    std::list<int>& ranks = it->second;
    ranks.sort();
    std::list<int>::iterator rit, rend = ranks.end();
    for (rit=ranks.begin(); rit != rend; ++rit, ++next_rank){
      int comm_rank = *rit;
      task_id tid = caller->peer_task(int(comm_rank));
      task_list[next_rank] = tid;
      if (comm_rank == caller->rank()){
        my_new_rank = next_rank;
      }
    }
  }
#if !SSTMAC_DISTRIBUTED_MEMORY || SSTMAC_MMAP_COLLECTIVES
  entry.refcount--;
  if (entry.refcount == 0){
#if SSTMAC_MMAP_COLLECTIVES
    munmap(entry.buf, len);
    shm_unlink(fname);
#else
    delete[] result;
#endif
  }
#endif
  return new mpi_comm(cid, my_new_rank, new mpi_group(task_list), worldcomm_->env_, aid_);
}

mpi_comm*
mpi_comm_factory::create_cart(mpi_comm* caller, int ndims,
                              const int *dims, const int *periods, int reorder)
{
  int inputID = next_id_;
  int outputID = 0;
  parent_->allreduce(&inputID, &outputID, 1, MPI_INT, MPI_MAX, caller->id());

  MPI_Comm cid = outputID;


  //now find my rank
  int newrank = caller->group_->rank_of_task(caller->my_task());
  next_id_ = cid + 1;

  if (newrank >= 0) {
    return new mpi_comm_cart(cid, newrank, caller->group_,
                     worldcomm_->env_, aid_, ndims, dims, periods, reorder);
  }
  else {
    return mpi_comm::comm_null;
  }
}


}


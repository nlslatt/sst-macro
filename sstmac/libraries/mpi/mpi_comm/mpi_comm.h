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

#ifndef SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_COMM_MPICOMM_H_INCLUDED
#define SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_COMM_MPICOMM_H_INCLUDED

#include <sstmac/libraries/mpi/mpi_types/mpi_id.h>
#include <sstmac/software/process/task_id.h>
#include <sstmac/software/process/app_id.h>
#include <sstmac/software/process/app_manager.h>
#include <sstmac/libraries/mpi/mpi_comm/mpi_comm_id.h>
#include <sstmac/libraries/mpi/mpi_types/mpi_tag.h>
#include <sstmac/libraries/mpi/mpi_comm/keyval_fwd.h>
#include <sstmac/libraries/mpi/mpi_comm/mpi_group.h>

#include <sstmac/common/node_address.h>

#include <iosfwd>

namespace sstmac {
namespace sw {

/**
 * An MPI communicator handle.
 */
class mpi_comm  {
 public:

  enum topotypes {
    TOPO_NONE, TOPO_GRAPH, TOPO_CART
  };

  static const int proc_null = -1;

 public:
  mpi_comm();

  /// Hello.
  mpi_comm(
    mpi_comm_id id,
    mpi_id rank,
    mpi_group* peers,
    app_manager*env,
    app_id aid);

  /// Goodbye.
  virtual
  ~mpi_comm() {
  }

  void
  set_name(std::string name) {
    name_ = name;
  }

  std::string
  name() const {
    return name_;
  }

  topotypes
  topo_type() const {
    return topotype_;
  }

  mpi_group*
  group() {
    return group_;
  }

  void
  dup_keyvals(mpi_comm* m);

  /// This is the null communicator.
  static mpi_comm* comm_null;

  mpi_comm_id id_;
  mpi_id rank_;

  virtual std::string
  to_string() const;

  /// The rank of this peer in the communicator.
  mpi_id
  rank() const {
    return rank_;
  }

  /// The size of the communicator.
  mpi_id
  size() const;

  /// The identifier for this communicator.
  /// To be used to tag messages to/from this communicator.
  mpi_comm_id
  id() const {
    return id_;
  }

  void
  set_keyval(keyval* k, void* val);

  void
  get_keyval(keyval* k, void* val, int* flag);

  app_id
  app() const {
    return aid_;
  }

  //
  // Get a unique tag for a collective operation.
  //
  mpi_tag
  next_collective_tag() const;

  /// The task index of the caller.
  task_id
  my_task() const;

  /// The task index of the given peer.
  task_id
  peer_task(mpi_id rank) const;

  node_id
  my_node() const;

  /// The list of nodes involved in this communicator.
  /// Indexing is done by mpiid::rank().id.
  node_id
  node_at(mpi_id rank) const;

  /// Equality comparison.
  inline bool
  operator==(mpi_comm* other) const {
    return ((rank_ == other->rank_) && (id_ == other->id_));
  }

  /// Inequality comparison.
  inline bool
  operator!=(mpi_comm* other) const {
    return !this->operator==(other);
  }

 protected:
  friend std::ostream&
  operator<<(std::ostream &os, mpi_comm* comm);

  void
  validate(const char* fxn) const;

 protected:
  friend class mpi_comm_factory;

  app_manager* env_;

  /// The tasks participating in this communicator.  This is only used for an mpicomm* which is NOT WORLD_COMM.
  mpi_group* group_;

  mutable mpi_tag next_collective_tag_;

  spkt_unordered_map<int, keyval*> keyvals_;

  app_id aid_;

  topotypes topotype_;

  std::string name_;


};

/// Fairly self-explanatory.
std::ostream&
operator<<(std::ostream &os, mpi_comm* comm);

}
} //end of namespace sstmac

#endif


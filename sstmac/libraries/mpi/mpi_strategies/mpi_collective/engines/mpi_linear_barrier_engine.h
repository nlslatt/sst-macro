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

#ifndef SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_STRATEGIES_MPI_COLLECTIVE_ENGINES_MPILINEARBARRIERENGINE_H_INCLUDED
#define SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_STRATEGIES_MPI_COLLECTIVE_ENGINES_MPILINEARBARRIERENGINE_H_INCLUDED

#include <sstmac/libraries/mpi/mpi_strategies/mpi_collective/mpi_collective.h>
#include <sstmac/libraries/mpi/mpi_comm/mpi_comm.h>
#include <sstmac/libraries/mpi/mpi_types/mpi_tag.h>
#include <sstmac/libraries/mpi/mpi_strategies/mpi_iterator/topology_iterator.h>
#include <sstmac/libraries/mpi/mpi_status.h>

namespace sstmac {
namespace sw {

// Important types.




/**
 * A basic mpikernel to provide an MPI_Barrier.
 */
class mpi_linear_barrier_engine : public mpi_collective
{
 public:
  std::string
  to_string() const {
    return sprockit::printf("mpilinearbarrier(tag=%d)", int(tag_));
  }

  mpi_linear_barrier_engine(topology_iterator* iter,
                            mpi_request* the_key,
                            mpi_queue* queue,
                            mpi_tag tag,
                            mpi_comm* comm,const mpi_id& root,
                            event_handler* completion);

  /// Goodbye.
  virtual ~mpi_linear_barrier_engine() throw();

  /// Start this kernel.
  virtual void start();

 protected:
  /// Callback method to indicate that a send operation has completed.
  virtual void
  send_complete(const mpi_message::ptr& msg);

  /// Callback method to indicate that a receive operation has completed.
  virtual void
  recv_complete(const mpi_message::ptr& msg);

  /// Fire off the next set of send and receive operations.
  void sendrecv(bool firsttime);

 protected:
  /// The iterator to whom we entrust decisions on where to send and receive.
  topology_iterator* iter_;

  /// The number of pending sends.
  int pending_sends_;
  /// The number of pending receives.
  int pending_recvs_;

  mpi_id root_;


};

}
} // end of namespace sstmac

#endif


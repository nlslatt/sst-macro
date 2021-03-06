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

#ifndef SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_QUEUE_MPIQUEUE_SENDREQUEST_H_INCLUDED
#define SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_QUEUE_MPIQUEUE_SENDREQUEST_H_INCLUDED

#include <sstmac/libraries/mpi/mpi_message.h>
#include <sstmac/libraries/mpi/mpi_queue/mpi_queue_fwd.h>
#include <sstmac/libraries/mpi/mpi_request_fwd.h>

namespace sstmac {
namespace sw {

/**
 * A nested type to handle individual send operations.
 */
class mpi_queue_send_request  {
 public:

 protected:
  friend class mpi_queue;

 public:
  virtual std::string
  to_string() const {
    return "mpi queue send request";
  }

  mpi_queue_send_request(const mpi_message::ptr& mess,
              mpi_request* key, mpi_queue* queue,
              event_handler* completion);

  /// Goodbye.
  virtual ~mpi_queue_send_request() throw();

  bool matches(const mpi_message::ptr& send_ack) const;

  /// Eventhandler completion.
  void complete(const mpi_message::ptr& msg);

  void wait_for_buffer();

 protected:
  /// The queue.
  mpi_queue* queue_;

  mpi_tag tag_;

  /// The object that gets notified when we're done.
  event_handler* completion_;

  mpi_request* key_;

  int seqnum_;
  mpi_id dest_;
  mpi_id src_;


};

}
} // end of namespace sstmac

#endif


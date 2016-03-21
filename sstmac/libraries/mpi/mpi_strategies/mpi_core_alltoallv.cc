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

#include <sstmac/libraries/mpi/mpi_strategies/mpi_core_alltoallv.h>
#include <sstmac/libraries/mpi/mpi_strategies/mpi_collective/engines/mpi_direct_alltoall.h>
#include <sstmac/libraries/mpi/mpi_queue/mpi_queue.h>

namespace sstmac {
namespace sw {

//
// Must be virtual.
//
mpi_core_alltoallv::~mpi_core_alltoallv() throw()
{
}

//
// Build a kernel.
//
mpi_collective*
mpi_core_alltoallv::execute(mpi_request* req,
                            mpi_queue* queue,
                            const std::vector<int> &sendcnts,
                            mpi_type_id sendtype,
                            const std::vector<int> &recvcnts,
                            mpi_type_id recvtype,
                            mpi_tag tag, mpi_comm* comm,
                            const std::vector<payload::const_ptr >& content,
                            operating_system* os
                           ) const
{
  event_handler* completion = queue->progress_done_handler(os, req);
  mpi_direct_alltoall* engine = new mpi_direct_alltoall(
                                      req, queue, sendcnts, sendtype, recvcnts, recvtype, tag, comm,
                                      content, completion);

  engine->start();
  return engine;
}

}
} // end of namespace sstmac


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

#include <sstmac/hardware/interconnect/interconnect.h>
#include <sstmac/hardware/topology/topology.h>
#include <sstmac/software/launch/index_strategy.h>

ImplementFactory(sstmac::sw::index_strategy);
RegisterDebugSlot(indexing);

namespace sstmac {
namespace sw {

index_strategy::~index_strategy() throw()
{
}

void
index_strategy::init_factory_params(sprockit::sim_parameters *params)
{
  STATIC_INIT_TOPOLOGY(params)
}

int
index_strategy::validate_nproc(int ppn, int num_nodes, int nproc,
                               const char *name) const
{
  if(nproc < 0) {
    nproc = num_nodes * ppn;
  }

  if(num_nodes == 0) {
    spkt_throw_printf(sprockit::value_error,
                     "%s::allocate:  empty node vector",
                     name);
  }

  if(ppn <= 0) {
    spkt_throw_printf(sprockit::value_error,
                     "%s::allocate:  ppn must be > 0. "
                     "If you're running DUMPI or another tracer, launch_indexing parameter "
                     "needs to be hostname or dumpi. Alternatively you can specify a launch_app1_cmd.",
                     name);
  }

  int max_nproc = ppn * num_nodes;
  if (max_nproc < nproc) {
    spkt_throw_printf(sprockit::value_error,
                     "%s::allocate: require ppn * num_nodes >= nproc: ppn=%d num_nodes=%d nproc=%d",
                     name, ppn, num_nodes, nproc);
  }

  return nproc;
}

}
}


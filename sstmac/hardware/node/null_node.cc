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

#include <sstmac/hardware/node/null_node.h>
#include <sstmac/hardware/nic/nic.h>
#include <sstmac/hardware/memory/memory_model.h>
#include <sstmac/software/process/operating_system.h>
#include <sstmac/software/launch/machine_descriptor.h>
#include <sstmac/software/launch/launcher.h>
#include <sstmac/common/messages/timed_message.h>

#include <sprockit/errors.h>
#include <sprockit/util.h>
#include <sprockit/output.h>
#include <sprockit/sim_parameters.h>

#include <iostream>


namespace sstmac {
namespace hw {

#if !SSTMAC_INTEGRATED_SST_CORE
SpktRegister("null | nullnode", node, null_node,
            "Node which implements interface, but does no compute modeling");
#endif

using namespace sstmac::sw;

null_node::~null_node()
{
}

void
null_node::null_warning(sprockit::sim_parameters* params)
{
  //this function should really only ever be invoked once for the template node
  //should be safe to print warnings here
}

void
null_node::init_factory_params(sprockit::sim_parameters *params)
{
  null_warning(params);
  params->add_param_override("compute_scheduler", "null");
  params->add_param_override("memory.model", "null");
  params->add_param_override("proc.frequency", "100MHz");
  simple_node::init_factory_params(params);
}

}
} // end of namespace sstmac.



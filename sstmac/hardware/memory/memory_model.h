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

#ifndef MEMORYMODEL_H_
#define MEMORYMODEL_H_

#include <sstmac/hardware/common/connection.h>
#include <sprockit/factories/factory.h>
#include <sprockit/debug.h>

#include <sstmac/hardware/node/node_fwd.h>
#include <sstmac/hardware/memory/memory_id.h>

DeclareDebugSlot(memory)
#define mem_debug(...) \
    debug_printf(sprockit::dbg::memory, "Memory on Node %d: %s", int(nodeid_), sprockit::printf(__VA_ARGS__).c_str())

namespace sstmac {
namespace hw {

class memory_model :
  public event_subscheduler,
  public sprockit::factory_type
{

 public:
  static void
  delete_statics();

  virtual void
  init_param1(node* parent_node);

  virtual void
  init_factory_params(sprockit::sim_parameters* params);

  virtual ~memory_model();

  virtual void
  access(const sst_message::ptr& msg) = 0;

  virtual std::string
  to_string() const {
    return "memory model";
  }

  virtual double
  max_single_bw() const = 0;

 protected:
  memory_model();

 protected:
  node_id nodeid_;
  node* parent_node_;
  event_handler* done_;

};

DeclareFactory1InitParam(memory_model,node*);

}
} /* namespace sstmac */
#endif /* MEMORYMODEL_H_ */


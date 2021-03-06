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

#ifndef SSTMAC_HARDWARE_PROCESSOR_SIMPLEPROCESSOR_H_INCLUDED
#define SSTMAC_HARDWARE_PROCESSOR_SIMPLEPROCESSOR_H_INCLUDED

#include <sstmac/hardware/processor/processor.h>
#include <sstmac/common/messages/timed_message.h>
#include <sstmac/common/timestamp.h>

namespace sstmac {
namespace hw {


/**
 * A very simple processor model
 */
class simple_processor :
  public processor
{
 public:
  /// Goodbye.
  virtual ~simple_processor() {}

  virtual void
  finalize_init();

  void
  compute(const sst_message::ptr& msg);

  virtual std::string
  to_string() const {
    return "simple_processor";
  }

 protected:
  void
  do_compute(timed_interface* msg, long thread_id);

};

}
} // end of namespace sstmac

#endif


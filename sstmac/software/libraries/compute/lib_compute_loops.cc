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

#include <sstmac/software/libraries/compute/lib_compute_loops.h>
#include <sstmac/software/process/operating_system.h>
#include <sstmac/software/process/key.h>
#include <sstmac/software/process/backtrace.h>
#include <sstmac/common/sstmac_env.h>
#include <sprockit/sim_parameters.h>
#include <stdint.h>
#include <math.h>

namespace sstmac {
namespace sw {

int lib_compute_loops::mem_op_size_ = 16;

double lib_compute_loops::mem_line_ratio_ = -1;
double lib_compute_loops::flop_line_ratio_ = -1;
bool lib_compute_loops::do_loops_ = true;

lib_compute_loops::lib_compute_loops(software_id id) :
  lib_compute_memmove(id)
{
  libname_ = "computelibloops" + id.to_string();
  key_cat_ = lib_compute::key_category;
}

lib_compute_loops::lib_compute_loops(const std::string& id) :
  lib_compute_memmove(id)
{
  libname_ = id;
  key_cat_ = lib_compute::key_category;
}

void
lib_compute_loops::consume_params(sprockit::sim_parameters* params)
{
  mem_line_ratio_ = params->get_optional_double_param(
                        "lib_compute_loops_mem_ratio", 0.8);
  flop_line_ratio_ = params->get_optional_double_param(
                         "lib_compute_loops_flop_ratio", 0.8);
  do_loops_ = params->get_optional_bool_param(
                  "lib_compute_loops_enable", true);
  lib_compute_memmove::consume_params(params);
}

void
lib_compute_loops::compute_loop_work(long long loop, double numlines)
{
  SSTMACBacktrace("Compute Loops");
  if (loop < 0) {
    loop = 0;
  }

  if (numlines < 0) {
    numlines = 0;
  }


  doing_memory_ = true;
  long long bytes = std::max((long long) 1,
                             (long long) (loop * mem_line_ratio_ * numlines * mem_op_size_));
  lib_compute_memmove::read(bytes);
  doing_memory_ = false;

  compute_message::ptr inst = new compute_message;
  long long flops = std::max((long long) 1,
                             (long long) (loop * flop_line_ratio_ * numlines));
  inst->set_event_value(compute_message::flop, flops);
  lib_compute_inst::compute_inst(inst);
}



void
lib_compute_loops::compute_fft()
{
  if (do_loops_) {
    compute_message::ptr inst = new compute_message;
    inst->set_event_value(compute_message::flop, 500);
    lib_compute_inst::compute_inst(inst);
  }
}

}
} //end of namespace sstmac


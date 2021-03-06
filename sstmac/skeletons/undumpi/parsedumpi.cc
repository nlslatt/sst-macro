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


#include <sstmac/software/process/operating_system.h>
#include <sstmac/common/event_manager.h>
#include <sstmac/skeletons/undumpi/parsedumpi.h>
#include <sstmac/skeletons/undumpi/parsedumpi_callbacks.h>
#include <sstmac/dumpi_util/dumpi_meta.h>
#include <sstmac/dumpi_util/dumpi_util.h>
#include <sstmac/libraries/mpi/mpi_api.h>
#include <sprockit/errors.h>
#include <sprockit/sim_parameters.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <fstream>
#include <algorithm>

namespace sstmac {
namespace sw {

SpktRegister("parsedumpi", app, parsedumpi,
            "application for parsing and simulating dumpi traces");

using namespace sstmac::hw;


void
parsedumpi::consume_params(sprockit::sim_parameters* params)
{

  fileroot_ = params->reread_param("launch_dumpi_metaname");

  timescaling_ = params->get_optional_double_param("parsedumpi_timescale", 1);

  print_progress_ = params->get_optional_bool_param("parsedumpi_print_progress", true);

  percent_terminate_ = params->get_optional_double_param("parsedumpi_terminate_percent", -1);
}

//
// Wait!  That's not good news at all!
//
parsedumpi::~parsedumpi() throw()
{
}

//
// Parse the file please (this needs an update, there is no reason
// for it to return a TRACEREC (which should not be uppercase).
// Need to figure out how to give the caller access to header/footer recs.
//
void parsedumpi::skeleton_main()
{
  mpi()->init();

  int rank = mpi()->comm_world()->rank();

  dumpi_meta* meta = new dumpi_meta(fileroot_);
  parsedumpi_callbacks cbacks(this);
  std::string fname = dumpi_file_name(rank, meta->dirplusfileprefix_);
  // Ready to go.

  //only rank 0 should print progress
  bool print_my_progress = rank == 0 && print_progress_;
  //only rank 0 should cause termination
  double my_percent_terminate = rank == 0 ? percent_terminate_ : -1;

  cbacks.parse_stream(fname.c_str(), print_my_progress, my_percent_terminate);

  if (rank == 0) {
    std::cout << "Parsedumpi finalized on rank 0.  Trace run successful!" <<
              std::endl;
  }

#if !SSTMAC_INTEGRATED_SST_CORE
  // TODO make this work with @integrated_core
  if (percent_terminate_ >= 0){
    //we must stop the event manager now
    mpi()->os()->parent()->event_mgr()->stop();
  }
#endif

}


}
} // end of namespace sstmac



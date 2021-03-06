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

#include <sstmac/common/sstmac_env.h>

#include <sstmac/backends/common/sim_partition.h>
#include <sstmac/backends/native/node_list.h>
#include <sstmac/backends/native/manager.h>
#include <sstmac/backends/native/event_map.h>
#include <sstmac/backends/native/clock_cycle_parallel/clock_cycle_event_container.h>
#include <sstmac/backends/native/skeleton_app_manager.h>

#include <sstmac/common/runtime.h>
#include <sstmac/common/vis/vis.h>
#include <sstmac/common/logger.h>

#include <sstmac/dumpi_util/dumpi_meta.h>

#include <sstmac/software/launch/launch_message.h>
#include <sstmac/software/process/app_manager.h>

#include <sprockit/driver_util.h>
#include <sprockit/keyword_registration.h>
#include <sprockit/output.h>
#include <sprockit/errors.h>
#include <sprockit/util.h>
#include <sprockit/sim_parameters.h>

#include <iostream>
#include <iterator>
#include <cstdlib>


namespace sstmac {
namespace native {

using namespace sstmac::sw;
using namespace sstmac::hw;

const char* keywords[] = { "sst_rank", "sst_nproc" };
sprockit::StaticKeywordRegister reg_keywords(2, keywords);

class timestamp_prefix_fxn :
  public sprockit::debug_prefix_fxn
{
 public:
  timestamp_prefix_fxn(event_manager* mgr) : mgr_(mgr){}

  std::string
  str() {
    double t_ms = mgr_->now().msec();
    return sprockit::printf("T=%12.8e ms:", t_ms);
  }

 private:
  event_manager* mgr_;
};

//
// Default constructor.
//
manager::manager() :
  next_ppid_(0),
  interconnect_(0),
  rt_(0)
{
}

//
// Define a network.
//
void
manager::init_factory_params(sprockit::sim_parameters* params)
{
  build_apps(params);
}

int
manager::compute_max_nproc(int appnum, sprockit::sim_parameters* params)
{
  int max_nproc = 0;
  /** Do a bunch of dumpi stuff */
  static const char* dmeta = "launch_dumpi_metaname";
  std::string launch_param_name = sprockit::printf("launch_app%d", appnum);
  if (params->get_param(launch_param_name) == "parsedumpi"){
    std::string dumpi_meta_filename;
    if (!params->has_param(dmeta)){
      FILE *fp = popen("ls *.meta", "r");
      char buf[1024];
      char* ret = fgets(buf, 1024, fp);
      int len = ::strlen(buf);
      if (ret){
        char& lastchar = buf[len-1];
        if (lastchar == '\n'){
          lastchar = '\0';
        }
        cout0 << "Using dumpi meta " << buf << std::endl;
        params->add_param(dmeta, buf);
      } else {
        spkt_throw(sprockit::input_error,
         "no dumpi file found in folder or specified with launch_dumpi_metaname");
      }
      dumpi_meta_filename = buf;
    } else {
      dumpi_meta_filename = params->get_param(dmeta);
    }
    if (!params->has_param("launch_app1_cmd")){
      sw::dumpi_meta* meta = new sw::dumpi_meta(dumpi_meta_filename);
      int nproc = meta->num_procs();
      std::string cmd = sprockit::printf("aprun -n %d -N 1", nproc);
      params->add_param("launch_app1_cmd", cmd);
      max_nproc = std::max(max_nproc, nproc);
      delete meta;
    }
  }
  else {
    //regular application
    int nproc, procs_per_node;
    std::vector<int> ignore;
    skeleton_app_manager::parse_launch_cmd(launch_param_name,
        params, nproc, procs_per_node, ignore);
    max_nproc = std::max(nproc, max_nproc);
  }
  return max_nproc;
}

int
manager::compute_max_nproc(sprockit::sim_parameters* params)
{
  int appnum = 1;
  int max_nproc = 0;
  while (true) {
    std::string launch_prefix = sprockit::printf("launch_app%d", appnum);
    if (!params->has_param(launch_prefix)) {
      break;
    }
    int nproc = compute_max_nproc(appnum, params);
    max_nproc = std::max(nproc, max_nproc);
    ++appnum;
  }
  return max_nproc;
}

void
manager::build_app(int appnum, const std::string& launch_prefix,
    sprockit::sim_parameters* params)
{
  std::string param_name = launch_prefix + "_type";
  timestamp start = params->get_optional_time_param(
                      launch_prefix + "_start", 0);

  sstmac::sw::app_id aid(appnum);
  app_manager* appman = app_manager_factory::get_optional_param(param_name,
                            "skeleton", params, aid, rt_);
  appman->set_interconnect(interconnect_);

  app_managers_[appnum] = appman;
  app_starts_[appnum] = start;
  sstmac_runtime::register_app_manager(aid, appman);
}

void
manager::build_apps(sprockit::sim_parameters* params)
{
  int appnum = 1;
  while (true) {
    std::string launch_prefix = sprockit::printf("launch_app%d", appnum);
    if (!params->has_param(launch_prefix)) {
      break;
    }
    build_app(appnum, launch_prefix, params);
    ++appnum;
  }
}

manager::~manager() throw ()
{
  if (interconnect_) delete interconnect_;
}

#if SSTMAC_INTEGRATED_SST_CORE
void
sst_manager::init_factory_params(sprockit::sim_parameters* params)
{
  //these are not used
  parallel_runtime* rt = 0;
  partition* part = 0;
  const char* ic_param = params->has_param("network_name") ? "network_name" : "interconnect";
  interconnect_ = interconnect_factory::get_param(ic_param, params, part, rt);
}
#else
void
macro_manager::init_factory_params(sprockit::sim_parameters* params)
{
  event_manager_ = event_manager_factory::get_optional_param(
                       "event_manager", SSTMAC_DEFAULT_EVENT_MANAGER_STRING, params, rt_);
  event_manager::global = event_manager_;

  if (sprockit::debug::slot_active(sprockit::dbg::timestamp)){
    sprockit::debug_prefix_fxn* fxn = new timestamp_prefix_fxn(event_manager_);
    sprockit::debug::prefix_fxn = fxn;
  }

  bool debug_startup = params->get_optional_bool_param("debug_startup", true);
  if (!debug_startup){
    sprockit::debug::turn_off();
  }

  /** sstkeyword {
        docstring = Specify the general type of network congestion model that will be used
                    for the interconnect;
        gui = train;
  } */
  const char* ic_param = params->has_param("network_name") ? "network_name" : "interconnect";
  interconnect_ = interconnect_factory::get_param(ic_param, params, event_manager_->topology_partition(), rt_);

  event_manager_->set_interconnect(interconnect_);
  interconnect_->set_event_manager(event_manager_);

  logger::timer_ = event_manager_;

  has_vis_engine_ = params->has_param("vis_engine");

  //this should definitely be called last
  manager::init_factory_params(params);
}

void
macro_manager::start()
{
  launch_apps();

  // Setup the simulation before starting
  if (has_vis_engine_) {
    vis::vis_display* visd = test_cast(vis::vis_display, interconnect_);
    if (visd) {
      visd->vis_start(true);
      std::cout << "--- Starting visualizer.\n";
    }
  }
}

//
// Start the simulation.
//
timestamp
macro_manager::run(timestamp until)
{
  start();

  running_ = true;

  if (until.sec() > 0) {
    event_manager_->schedule_stop(until);
  }

  event_manager_->run();

  running_ = false;
  // Now call done routine to end simulation and print Stats.
  stop();

  return event_manager_->now();
}

void
macro_manager::stop()
{
  if (has_vis_engine_){
    vis::vis_display* visd = test_cast(vis::vis_display, interconnect_);
    if (visd){
      std::cout << "--- Closing vsisualizer.\n";
      visd->vis_complete();
    }
  }

  event_manager::global = 0;

  sstmac_runtime::finish();
}

macro_manager::macro_manager(parallel_runtime* rt) :
  running_(false),
  event_manager_(0)
{
  rt_ = rt;
}

void
macro_manager::finish()
{
  //interconnect_->deadlock_check();
  event_manager_->finish_stats();
  event_manager::global = 0;
  logger::timer_ = 0;
}

void
macro_manager::launch_app(int appnum, timestamp start, sw::app_manager* appman)
{
  appman->allocate_and_index_jobs();
  launch_info* linfo = appman->launch_info();
  sstmac::sw::app_id aid(appnum);
  for (int i=0; i < appman->nproc(); ++i) {
    node_id dst_nid = appman->node_assignment(i);
    sstmac_runtime::register_node(aid, task_id(i), dst_nid);

    hw::node* dst_node = interconnect_->node_at(dst_nid);
    if (!dst_node) {
      // mpiparallel, this node belongs to someone else
      continue;
    }

    sw::launch_message::ptr lmsg = new launch_message(linfo, sw::launch_message::ARRIVE, task_id(i));

    dst_node->launch(start, lmsg);


    //int dstthread = dst_node->thread_id();
    //event_manager_->ev_man_for_thread(dstthread)->schedule(start, new handler_event(lmsg, dst_node));
  }
}

void
macro_manager::launch_apps()
{
  std::map<int, app_manager*>::iterator it, end = app_managers_.end();
  for (it=app_managers_.begin(); it != end; ++it){
    int appnum = it->first;
    app_manager* appman = it->second;
    timestamp start = app_starts_[appnum];
    launch_app(appnum, start, appman);
  }
}


//
// Goodbye.
//
macro_manager::~macro_manager() throw ()
{
  if (this->running_) {
    cerrn << "FATAL:  manager going out of scope while still running.\n";
    abort();
  }
  if (event_manager_) delete event_manager_;
}
#endif

}
} // end of namespace sstmac



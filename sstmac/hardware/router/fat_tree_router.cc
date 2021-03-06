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

#include <sstmac/hardware/router/fat_tree_router.h>
#include <sstmac/hardware/switch/network_switch.h>
#include <sstmac/hardware/topology/fat_tree.h>
#include <sprockit/util.h>
#include <sprockit/sim_parameters.h>
#include <cmath>

#define ftree_rter_debug(...) \
  rter_debug("fat tree: %s", sprockit::printf(__VA_ARGS__).c_str())

namespace sstmac {
namespace hw {

SpktRegister("fattree | ftree", router, fat_tree_router);

void
fat_tree_router::build_rng()
{
  long seed = 0;
  if (seed_ == -1){
    seed = time(NULL);
  } else {
    seed = seed_;
  }

  std::vector<RNG::rngint_t> seeds;
  seeds.push_back(seed);
  rng_ = RNG::Combo::construct(seeds);
}

void
fat_tree_router::init_factory_params(sprockit::sim_parameters *params)
{
  structured_router::init_factory_params(params);
  seed_ = params->get_optional_long_param("router_seed", -1);
  build_rng();

  k_ = params->get_int_param("radix");
  l_ = params->get_int_param("num_levels");
}

void
fat_tree_router::route(const sst_message::ptr& msg)
{
  minimal_route_to_node(msg->toaddr(),
    msg->interface<routable>()->rinfo().current_path());
}

void
fat_tree_router::productive_paths_to_switch(
  switch_id dst, routing_info::path_set &paths)
{
  routing_info::path tmp_path;
  minimal_route_to_switch(dst, tmp_path);
  int dim = tmp_path.outport / k_;
  int dir = tmp_path.outport % k_;
  if (is_top_level()) {
    paths.resize(2);
    //we have two possible down connections
    //these ports are offset by k_, though
    int path_group = dir % k_;
    int path0dir = path_group;
    int path1dir = path_group + k_;
    //paths[0].dim = paths[1].dim = fat_tree::down_dimension;
    paths[0].outport = regtop_->convert_to_port(fat_tree::down_dimension, path0dir);
    paths[1].outport = regtop_->convert_to_port(fat_tree::down_dimension, path1dir);
    paths[0].vc = paths[1].vc = 1;
    return;
  }
  else if (dim == fat_tree::down_dimension) {
    //we have no choice - only one path down is correct
    paths.resize(1);
    paths[0] = tmp_path;
  }
  else {
    // we have k productive paths up
    paths.resize(k_);
    for (int i=0; i < k_; ++i) {
      //paths[i].dim = fat_tree::up_dimension;
      //paths[i].dir = i;
      paths[i].vc = 0;
      paths[i].outport = convert_to_port(fat_tree::up_dimension, i);
    }
  }
}

void
fat_tree_router::minimal_route_to_switch(
  switch_id ej_addr,
  routing_info::path& path)
{

  rter_debug("routing from switch %d:%s -> %d:%s on fat tree router",
    int(my_addr_), top_->switch_coords(my_addr_).to_string().c_str(),
    int(ej_addr), top_->switch_coords(ej_addr).to_string().c_str());

  int pathDim, pathDir;
  long ej_id = ej_addr;
  if (myL_ == 0) {
    // if we're on the first level and we're not there yet, go up
    pathDim = fat_tree::up_dimension;
    pathDir = choose_up_path();
    path.vc = 0;
    rter_debug("fat_tree: routing up from first level");
  }
  else if (is_top_level()) {
    pathDim = fat_tree::down_dimension;
    path.vc = 1;
    //in an l=3,k=2 train, e.g.
    // switch 8 performs the "role" of switches 8,9
    // switch 9 performs the "role" of switches 10,11
    // choose one of the roles to perform
    int my_virtual_role = choose_down_path();
    long relative_ej_id = ej_id - min_reachable_leaf_id_;
    // the leaf ids are divided into groups of size num_leaf_switches_per_path
    // based on which leaf nodes are reachable following a given path
    long path_group = relative_ej_id / num_leaf_switches_per_path_;
    // each node can play "two" roles giving two degenerate paths down
    pathDir = path_group + my_virtual_role * k_;
    // Because
    ftree_rter_debug("routing down with dir %d: eject-id=%ld rel-eject-id=%ld num_leaf_switches_per_path-%ld my_virtual_role=%d",
          pathDir, ej_id, relative_ej_id, num_leaf_switches_per_path_, my_virtual_role);
  }
  else if (ej_id >= min_reachable_leaf_id_ && ej_id < max_reachable_leaf_id_) {
    pathDim = fat_tree::down_dimension;
    path.vc = 1;
    long relative_ej_id = ej_id - min_reachable_leaf_id_;
    pathDir = relative_ej_id / num_leaf_switches_per_path_;
    ftree_rter_debug("routing down with dir %d: eject-id=%ld rel-eject-id=%ld",
        pathDir, ej_id, relative_ej_id);
  }
  else {
    //route up
    pathDim = fat_tree::up_dimension;
    pathDir = choose_up_path();
    path.vc = 0;
    ftree_rter_debug("routing up with dir %d", pathDir);
  }
  path.outport = regtop_->convert_to_port(pathDim, pathDir);
}

int
fat_tree_router::choose_up_path()
{
  int ret = numpicked_;
  numpicked_ = (numpicked_ + 1) % k_;
  return ret;
}

void
fat_tree_router::finalize_init()
{
  //fat_tree* ft_top = safe_cast(fat_tree, top_);
  //k_ = ft_top->k();
  //l_ = ft_top->l();

  int switchesperlevel = pow(k_, l_ - 1);
  myL_ = my_addr_ / switchesperlevel;
  logicalid_ = (my_addr_ - (myL_ * switchesperlevel)) / pow(k_,
               myL_);

  num_leaf_switches_reachable_ = pow(k_, myL_);
  num_leaf_switches_per_path_ = num_leaf_switches_reachable_ / k_;
  level_relative_id_ = my_addr_ - myL_ * switchesperlevel;
  if (is_top_level()) {
    level_relative_id_ *= 2;
  }

  int my_leaf_group = level_relative_id_ / num_leaf_switches_reachable_;
  min_reachable_leaf_id_ = my_leaf_group * num_leaf_switches_reachable_;
  max_reachable_leaf_id_ = min_reachable_leaf_id_ + num_leaf_switches_reachable_;

  pickstart_ = rng_->value() % k_;
  numpicked_ = 0;

  numpicktop_ = 0;

  structured_router::finalize_init();
}

int
fat_tree_router::choose_down_path()
{
  int ret = numpicktop_;
  int two_down_connections_per_top_switch = 2;
  numpicktop_ = (numpicktop_ + 1) % two_down_connections_per_top_switch;
  return ret;
}

int
fat_tree_router::number_paths(const sst_message::ptr& msg) const
{
  switch_id ej_addr = top_->endpoint_to_ejection_switch(msg->toaddr());
  long ej_id = ej_addr;
  if (ej_addr == my_addr_) {
    return 1;
  }
  else if (myL_ == 0) {
    return k_;
  }
  else if (is_top_level()) {
    return 2;
  }
  else if (ej_id >= min_reachable_leaf_id_ && ej_id < max_reachable_leaf_id_) {
    return 1;
  }
  else {
    //route up
    return k_;
  }
}


}
}


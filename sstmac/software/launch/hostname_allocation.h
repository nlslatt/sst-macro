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

#ifndef SSTMAC_BACKENDS_NATIVE_LAUNCH_hostname_allocation_H_INCLUDED
#define SSTMAC_BACKENDS_NATIVE_LAUNCH_hostname_allocation_H_INCLUDED

#include <sstmac/software/launch/allocation_strategy.h>
#include <sstmac/dumpi_util/dumpi_meta.h>

#include <sstmac/hardware/topology/structured_topology.h>

namespace sstmac {

namespace sw {
class hostname_allocation : public allocation_strategy
{

 public:
  virtual void
  set_topology(hw::topology *top);

  virtual void
  init_factory_params(sprockit::sim_parameters* params);

  /**
   * This function reads in a map file for mapping host names to network coordinates.
   * @param here the function making the call, used for more informative exceptions
   * @param mapfile name of the map file
   * @param hostmap map in which host mappings are placed
   */
  static void
  read_map_file(parallel_runtime* rt,
                const char* here,
                const std::string &mapfile,
                std::map<std::string,std::vector<int> >& hostmap);

  virtual void
  allocate(int nnode_requested,
           node_set &allocation);

  virtual
  ~hostname_allocation() throw () {
  }

  typedef spkt_unordered_map<std::string, node_id> nodemap_t;
  static nodemap_t hostnamemap_;

  static std::map<long, std::string> nodenum_to_host_map_;

 protected:
  std::string mapfile_;
  sstmac::hw::structured_topology* regtop_;

};

}
}

#endif /* hostname_allocation_H_ */


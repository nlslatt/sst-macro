#include <sstmac/software/launch/node_id_allocation.h>

#include <sstmac/hardware/interconnect/interconnect.h>
#include <sstmac/software/launch/coordinate_allocation.h>
#include <sprockit/fileio.h>
#include <sprockit/sim_parameters.h>

namespace sstmac {
namespace sw {

SpktRegister("node_id", allocation_strategy, node_id_allocation);

void
node_id_allocation::init_factory_params(sprockit::sim_parameters* params)
{
  allocation_strategy::init_factory_params(params);
  coord_file_ = params->get_param("launch_node_id_file");
}

void
node_id_allocation::read_coordinate_file(
    const std::string &file,
    std::vector<node_id> &node_list,
    hw::topology* top)
{
  std::ifstream in;
  sprockit::SpktFileIO::open_file(in, file);

  if (!in.is_open()) {
    spkt_throw_printf(sprockit::input_error,
     "node_id_allocation: could not find node id file %s in current folder or configuration include path",
     file.c_str());
  }

  int num_nodes;
  in >> num_nodes;

  int max_node_id = top->num_nodes();

  node_list.resize(num_nodes);
  for (int nid=0; nid < num_nodes; ++nid){
    int next;
    in >> next;
    if (next >= max_node_id){
      spkt_throw_printf(sprockit::value_error,
        "node_id_allocation: invalid node id %d in file %s - max id is %d",
        next, file.c_str(), max_node_id);
    }
    node_list[nid] = node_id(next);
  }
}

void
node_id_allocation::allocate(int nnode_requested,
                              node_set &allocation)
{
  std::vector<node_id> node_list;
  read_coordinate_file(coord_file_, node_list, topology_);

  if (node_list.size() < nnode_requested){
    spkt_throw_printf(sprockit::value_error,
        "node_id_allocation::allocation: requested %d, but only have %d nodes",
        nnode_requested, int(node_list.size()) );
  }

  for (int i=0; i < nnode_requested; ++i){
    node_id nid = node_list[i];
    debug_printf(sprockit::dbg::allocation,
        "adding node %d to allocation",
        int(nid));
    allocation.insert(nid);
  }
}

}
}

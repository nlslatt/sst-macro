#ifndef sstmac_hardware_network_topology_routing_VALIANT_ROUTER_H
#define sstmac_hardware_network_topology_routing_VALIANT_ROUTER_H

#include <sstmac/hardware/router/minimal_routing.h>

namespace sstmac {
namespace hw {

class valiant_router : public minimal_router
{
 public:
  virtual
  ~valiant_router() { }

  virtual void
  route(const sst_message::ptr& msg);

  virtual void
  route(const sst_message::ptr& msg, routing_info::path_set& paths);

  virtual std::string
  to_string() const {
    return "valiant";
  }

  virtual void
  finalize_init();

  /**
    The topology object specifies a virtual based purely on geometry.
    However, the final virtual channel on both geometry and
    routing algorithm.  In this case, we need a separate set of
    virtual channels depending on whether we are in the first
    stage, routing to the intermediate switch, or the second stage,
    routing to the final switch.
    @param topology_vc The geometry-specific virtual channel
    @return The first stage virtual channel
  */
  virtual int
  first_stage_vc(int topology_vc) {
    return 2*topology_vc;
  }

  /**
    The topology object specifies a virtual based purely on geometry.
    However, the final virtual channel on both geometry and
    routing algorithm.  In this case, we need a separate set of
    virtual channels depending on whether we are in the first
    stage, routing to the intermediate switch, or the second stage,
    routing to the final switch.
    @param topology_vc The geometry-specific virtual channel
    @return The second stage virtual channel
  */
  virtual int
  second_stage_vc(int topology_vc) {
    return 2*topology_vc + 1;
  }

  int
  num_stages() const {
    return 2;
  }

 protected:
  typedef enum {
    minimal,
    intermediate_switch,
    final_node
  } next_action_t;

  next_action_t
  intermediate_step(routing_info& rinfo,
                    const sst_message::ptr& msg);

  /**
   Different for pure valiant and UGAL.
   */
  virtual next_action_t
  initial_step(routing_info& rinfo,
               const sst_message::ptr& msg);

  next_action_t
  next_routing_stage(const sst_message::ptr& msg);

  void
  configure_intermediate_path(routing_info::path& path);

  void
  configure_final_path(routing_info::path& path);

 protected:
  void route_valiant(const sst_message::ptr& msg);

};

}
}

#endif // VALIANT_ROUTER_H


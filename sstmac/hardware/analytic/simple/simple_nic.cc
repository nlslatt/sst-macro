#include <sstmac/hardware/analytic/simple/simple_nic.h>
#include <sprockit/util.h>
#include <sprockit/sim_parameters.h>

#if !SSTMAC_INTEGRATED_SST_CORE

namespace sstmac {
namespace hw {

SpktRegister("simple", nic, simple_nic,
            "implements a nic that models messages via a simple latency/bandwidth injection delay");

simple_nic::simple_nic() :
  next_free_(0)
{
}

void
simple_nic::init_factory_params(sprockit::sim_parameters *params)
{
  nic::init_factory_params(params);
  double inj_bw = params->get_bandwidth_param("injection_bandwidth");
  inj_bw_inverse_ = 1.0/inj_bw;
  inj_lat_ = params->get_time_param("injection_latency");
}

void
simple_nic::do_send(const network_message::ptr& msg)
{
  long num_bytes = msg->byte_length();
  timestamp now_ = now();
  timestamp start_send = now_ > next_free_ ? now_ : next_free_;
  timestamp time_to_inject = inj_lat_ + timestamp(inj_bw_inverse_ * num_bytes);
  //leave the injection latency term to the interconnect
  send_to_injector(start_send, msg);

  next_free_ = start_send + time_to_inject;
  if (msg->needs_ack()) {
    //do whatever you need to do so that this msg decouples all pointers
    network_message::ptr acker = msg->clone_injection_ack();
    send_to_node(next_free_, acker);
  }
}

void
simple_nic::connect(
    int src_outport,
    int dst_inport,
    connection_type_t ty,
    connectable* mod)
{
  if (ty == connectable::output){
    injector_ = safe_cast(event_handler, mod);
  }
}

void
simple_nic::finalize_init()
{
}

void
simple_nic::send_to_injector(timestamp t, const network_message::ptr& msg)
{
  interconn_->immediate_send(parent(), msg, t);
}

void
simple_nic::send_to_node(timestamp t, const network_message::ptr& msg){
  SCHEDULE(t, parent_, msg);
}

}
}

#endif


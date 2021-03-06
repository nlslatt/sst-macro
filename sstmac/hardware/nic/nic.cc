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

#include <sstmac/hardware/nic/nic.h>
#include <sstmac/hardware/interconnect/interconnect.h>
#include <sstmac/software/process/operating_system.h>
#include <sstmac/common/stats/stat_spyplot.h>
#include <sstmac/common/stats/stat_histogram.h>
#include <sstmac/common/stats/stat_local_int.h>
#include <sstmac/common/stats/stat_global_int.h>
#include <sstmac/common/event_manager.h>
#include <sprockit/statics.h>
#include <sprockit/sim_parameters.h>
#include <sprockit/keyword_registration.h>
#include <sprockit/util.h>

ImplementFactory(sstmac::hw::nic);
RegisterDebugSlot(nic);

RegisterNamespaces("nic", "message_sizes", "traffic_matrix",
                   "message_size_histogram");


#if SSTMAC_INTEGRATED_SST_CORE
#define DEFAULT_NEGLIGIBLE_SIZE 0
#else
#define DEFAULT_NEGLIGIBLE_SIZE 256
#endif

namespace sstmac {
namespace hw {

static sprockit::need_delete_statics<nic> del_statics;

nic::nic() :
  spy_num_messages_(0),
  spy_bytes_(0),
  hist_msg_size_(0),
  local_bytes_sent_(0),
  global_bytes_sent_(0)
{
}

nic::~nic()
{
}

void
nic::init_factory_params(sprockit::sim_parameters *params)
{
  my_addr_ = node_id(params->get_int_param("id"));
  init_loc_id(event_loc_id(my_addr_));

  negligible_size_ = params->get_optional_int_param("negligible_size", DEFAULT_NEGLIGIBLE_SIZE);

  if (params->has_namespace("traffic_matrix")){
    sprockit::sim_parameters* traffic_params = params->get_namespace("traffic_matrix");
    spy_num_messages_ = test_cast(stat_spyplot, stat_collector_factory::get_optional_param("type", "spyplot_png", traffic_params));
    spy_bytes_ = test_cast(stat_spyplot, stat_collector_factory::get_optional_param("type", "spyplot_png", traffic_params));

    if (!spy_num_messages_ || !spy_bytes_){
      spkt_throw_printf(sprockit::value_error,
        "NIC traffic_matrix must be spyplot or spyplot_png, %s given",
        traffic_params->get_param("type").c_str());
    }

    spy_num_messages_->add_suffix("num_messages");
    spy_bytes_->add_suffix("bytes");
  }

  if (params->has_namespace("local_bytes_sent")) {
    sprockit::sim_parameters* traffic_params = params->get_namespace("local_bytes_sent");
    local_bytes_sent_ = test_cast(stat_local_int, stat_collector_factory::get_optional_param("type", "local_int", traffic_params));
    local_bytes_sent_->set_id(my_addr_);
  }

  if (params->has_namespace("global_bytes_sent")) {
    sprockit::sim_parameters* traffic_params = params->get_namespace("global_bytes_sent");
    global_bytes_sent_ = test_cast(stat_global_int, stat_collector_factory::get_optional_param("type", "global_int", traffic_params));
    global_bytes_sent_->set_label("NIC Total Bytes Sent");
  }

  if (params->has_namespace("message_size_histogram")){
    sprockit::sim_parameters* size_params = params->get_namespace("message_size_histogram");
    hist_msg_size_ = test_cast(stat_histogram, stat_collector_factory::get_optional_param("type", "histogram", size_params));

    if (!hist_msg_size_){
      spkt_throw_printf(sprockit::value_error,
        "NIC message size tracker must be histogram, %s given",
        size_params->get_param("type").c_str());
    }
  }
}

void
nic::recv_chunk(const sst_message::ptr& chunk)
{
  spkt_throw_printf(sprockit::unimplemented_error,
            "nic::recv_chunk: not valid for %s receiving %s",
            to_string().c_str(), chunk->to_string().c_str());
}

void
nic::recv_credit(const sst_message::ptr &msg)
{
  //do nothing
}

void
nic::delete_statics()
{
}

void
nic::finish_recv_ack(const sst_message::ptr& msg)
{
}

void
nic::finish_recv_req(const sst_message::ptr& msg)
{
}

void
nic::recv_message(const sst_message::ptr& msg)
{
  network_message::ptr netmsg = ptr_safe_cast(network_message, msg);

  nic_debug("handling message %s:%lu of type %s from node %d while running",
    netmsg->to_string().c_str(),
    uint64_t(netmsg->net_id()),
    network_message::tostr(netmsg->type()),
    int(netmsg->fromaddr()));

  switch (netmsg->type()) {
    case network_message::rdma_get_request: {
      netmsg->nic_reverse(network_message::rdma_get_payload);
      netmsg->put_on_wire();
      internode_send(netmsg);
      finish_recv_req(msg);
      break;
    }
    case network_message::nvram_get_request: {
      netmsg->nic_reverse(network_message::nvram_get_payload);
      internode_send(netmsg);
      break;
    }
    case network_message::rdma_get_sent_ack:
    case network_message::payload_sent_ack:
    case network_message::rdma_put_sent_ack: {
      parent_->handle(netmsg);
      finish_recv_ack(msg);
      break;
    }
    case network_message::failure_notification: {
      parent_->handle(netmsg);
      break;
    }
    case network_message::rdma_get_nack:
    case network_message::rdma_get_payload:
    case network_message::rdma_put_payload:
    case network_message::nvram_get_payload:
    case network_message::payload: {
      send_to_node(netmsg);
      break;
    }
    default: {
      spkt_throw_printf(sprockit::value_error,
        "nic::handle: invalid message type %s",
        network_message::tostr(netmsg->type()));
    }
  }
}

void
nic::ack_send(const network_message::ptr& payload)
{
  if (payload->needs_ack()){
    network_message::ptr ack = payload->clone_injection_ack();
    nic_debug("acking payload %p:%s with ack %p",
      payload.get(), payload->to_string().c_str(), ack.get());
    send_to_node(ack);
  }
}

void
nic::intranode_send(const network_message::ptr& payload)
{
  record_message(payload);
  switch(payload->type())
  {
  case network_message::nvram_get_request:
    payload->nic_reverse(network_message::nvram_get_payload);
    break;
  case network_message::rdma_get_request:
    payload->nic_reverse(network_message::rdma_get_payload);
    break;
  default:
    break; //nothing to do
  }

  send_to_node(payload);
  ack_send(payload);
}

void
nic::handle(const sst_message::ptr& msg)
{
  if (parent_->failed()){
    return;
  }

  nic_debug("handling message %p:%s going %d->%d",
    msg.get(),
    msg->to_string().c_str(),
    int(msg->fromaddr()),
    int(msg->toaddr()));

  if (msg->is_credit()){
    recv_credit(msg);
  } else if (msg->is_chunk()){
    recv_chunk(msg);
  } else {
    recv_message(msg);
  }

}

void
nic::record_message(const network_message::ptr& netmsg)
{
  nic_debug("sending message %lu of size %ld of type %s to node %d: "
      "netid=%lu for %s",
      uint64_t(netmsg->net_id()),
      netmsg->byte_length(),
      network_message::tostr(netmsg->type()),
      int(netmsg->toaddr()),
      netmsg->unique_id(), netmsg->to_string().c_str());

  if (netmsg->type() == network_message::null_netmsg_type){
    //assume this is a simple payload
    netmsg->set_type(network_message::payload);
  }

  if (spy_num_messages_) {
    spy_num_messages_->add_one(netmsg->fromaddr(),
                  netmsg->toaddr());
  }

  if (spy_bytes_) {
    spy_bytes_->add(netmsg->fromaddr(),
                    netmsg->toaddr(), netmsg->byte_length());
  }

  if (hist_msg_size_) {
    hist_msg_size_->collect(netmsg->byte_length());
  }

  if (local_bytes_sent_) {
    local_bytes_sent_->collect(netmsg->byte_length());
  }

  if (global_bytes_sent_) {
    global_bytes_sent_->collect(netmsg->byte_length());
  }
}

void
nic::internode_send(const network_message::ptr& netmsg)
{
  record_message(netmsg);
  if (negligible_size(netmsg->byte_length())){
    send_to_interconn(netmsg);
    ack_send(netmsg);
  } else {
    do_send(netmsg);
  }
}

void
nic::finalize_init()
{
  //this is just a template nic
  if (my_addr_ == node_id()) {
    return;
  }
}

void
nic::send_to_node(const network_message::ptr& payload)
{
  SCHEDULE_NOW(parent_, payload);
}

void
nic::init_param1(sprockit::factory_type *interconn)
{
  if (interconn) interconn_ = safe_cast(interconnect, interconn);
}

void
nic::send_to_interconn(const network_message::ptr &netmsg)
{
#if SSTMAC_INTEGRATED_SST_CORE
  spkt_throw(sprockit::unimplemented_error,
       "nic::send_to_interconn: integrated core");
#else
  safe_cast(interconnect, interconn_)->immediate_send(parent(), netmsg, now());
#endif
}

void
nic::set_event_parent(event_scheduler* m)
{
  connectable_subcomponent::set_event_parent(m);
#if !SSTMAC_INTEGRATED_SST_CORE
  if (spy_num_messages_) m->register_stat(spy_num_messages_);
  if (spy_bytes_) m->register_stat(spy_bytes_);
  if (hist_msg_size_) m->register_stat(hist_msg_size_);
  if (local_bytes_sent_) m->register_stat(local_bytes_sent_);
  if (global_bytes_sent_) m->register_stat(global_bytes_sent_);
#endif

}

}
} // end of namespace sstmac.


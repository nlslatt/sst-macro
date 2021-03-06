#include <sstmac/hardware/network/network_message.h>
#include <sstmac/software/process/operating_system.h>
#include <sprockit/serializer.h>

DeclareSerializable(sstmac::hw::network_message);

#define enumcase(x) case x: return #x;

namespace sstmac {
namespace hw {

RegisterEnum(sst_message::message_type_t, network_message::NETWORK);

network_message::network_message()
  : needs_ack_(true),
    type_(null_netmsg_type),
    route_algo_(routing::deflt),
    bytes_(0)
{
  msgtype_ = NETWORK;
}

network_message::network_message(long payload_bytes) :
  bytes_(payload_bytes),
  needs_ack_(true),
  type_(null_netmsg_type),
  route_algo_(routing::deflt)
{
  msgtype_ = NETWORK;
}

network_message::network_message(
  node_id to, node_id from,
  sw::task_id src, sw::task_id dst,
  long bytes)
  : needs_ack_(true),
    route_algo_(routing::deflt),
    toaddr_(to),
    fromaddr_(from),
    src_task_(src),
    dest_task_(dst),
    bytes_(bytes),
    type_(null_netmsg_type)
{
  msgtype_ = NETWORK;
}

bool
network_message::is_nic_ack() const
{
  switch(type_)
  {
  case payload_sent_ack:
  case rdma_put_sent_ack:
  case rdma_get_sent_ack:
    return true;
  default:
    return false;
  }
}

void
network_message::convert_to_ack()
{
  reverse();
  switch(type_)
  {
    case rdma_get_payload:
      type_ = rdma_get_sent_ack;
      break;
    case rdma_put_payload:
      type_ = rdma_put_sent_ack;
      break;
    case payload:
      type_ = payload_sent_ack;
      break;
    default:
      spkt_throw_printf(sprockit::value_error,
        "network_message::clone_injection_ack: cannot ack msg type %s",
        tostr(type_));
      break;
  }
}

void
network_message::reverse()
{
  //also flip the addresses
  node_id dst = fromaddr_;
  node_id src = toaddr_;
  toaddr_ = dst;
  fromaddr_ = src;

  sw::task_id srcid = src_task_;
  sw::task_id dstid = dest_task_;
  src_task_ = dstid;
  dest_task_ = srcid;
}

const char*
network_message::tostr(nic_event_t mut)
{
  switch (mut) {
      enumcase(RDMA_GET_REQ_TO_RSP);
      enumcase(RDMA_GET_FAILED);
      enumcase(NVRAM_GET_REQ_TO_RSP);
  }
  spkt_throw_printf(sprockit::value_error,
       "network_message: invalid nic event %d", mut);
}

const char*
network_message::tostr(type_t ty)
{
  switch(ty)
  {
      enumcase(payload);
      enumcase(payload_sent_ack);
      enumcase(rdma_get_request);
      enumcase(rdma_get_payload);
      enumcase(rdma_get_sent_ack);
      enumcase(rdma_get_nack);
      enumcase(rdma_put_payload);
      enumcase(rdma_put_sent_ack);
      enumcase(rdma_put_nack);
      enumcase(null_netmsg_type);
      enumcase(nvram_get_request);
      enumcase(nvram_get_payload);
      enumcase(failure_notification);
  }
  spkt_throw_printf(sprockit::value_error,
    "network_message::tostr: unknown type_t %d",
    ty);
}

long
network_message::source_thread(sw::operating_system*os) const
{
  return os->task_threadid(src_task_);
}

long
network_message::dest_thread(sw::operating_system*os) const
{
  return os->task_threadid(dest_task_);
}

void
network_message::serialize_order(sprockit::serializer& ser)
{
  ser & needs_ack_;
  ser & route_algo_;
  ser & toaddr_;
  ser & fromaddr_;
  ser & src_task_;
  ser & dest_task_;
  ser & net_id_;
  ser & bytes_;
  ser & type_;
  sst_message::serialize_order(ser);
}

bool
network_message::is_metadata() const
{
  switch(type_)
  {
    case nvram_get_request:
    case rdma_get_request:
    case rdma_get_sent_ack:
    case rdma_put_sent_ack:
    case payload_sent_ack:
    case rdma_get_nack:
    case rdma_put_nack:
    case failure_notification:
    case null_netmsg_type:
      return true;
    case payload:
    case rdma_get_payload:
    case nvram_get_payload:
    case rdma_put_payload:
      return false;
  }
}

void
network_message::nic_reverse(type_t newtype)
{
  reverse();
  type_ = newtype;
}

long
network_message::byte_length() const
{
  switch (type_)
  {
    case rdma_get_request:
    case rdma_get_sent_ack:
    case rdma_put_sent_ack:
    case payload_sent_ack:
    case rdma_get_nack:
    case rdma_put_nack:
      return 32; //hack for now CHANGE
    default:
      //never return less than 8 bytes - every message has to carry somethng
      return std::max(8L, bytes_);
  }
}

void
network_message::clone_into(const network_message::ptr& cln) const
{
  cln->needs_ack_ = needs_ack_;
  cln->route_algo_ = route_algo_;
  cln->toaddr_ = toaddr_;
  cln->fromaddr_ = fromaddr_;
  cln->src_task_ = src_task_;
  cln->dest_task_ = dest_task_;
  cln->net_id_ = net_id_;
  cln->bytes_ = bytes_;
  cln->type_ = type_;
}

void
network_message::put_on_wire()
{
 //do nothing by default
}

}
}




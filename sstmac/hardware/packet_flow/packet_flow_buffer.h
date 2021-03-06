#ifndef BUFFER_H
#define BUFFER_H

#include <sstmac/hardware/packet_flow/packet_flow_handler.h>
#include <sstmac/hardware/packet_flow/packet_flow_crossbar.h>
#include <sstmac/hardware/packet_flow/packet_flow_sender.h>

namespace sstmac {
namespace hw {


class packet_flow_buffer :
  public packet_flow_sender
{

 public:
  virtual void
  set_output(int this_outport, int dst_inport,
    event_handler* output);

  virtual int
  get_queue_length() const {
    return 0;
  }

  event_loc_id
  output_location() const {
    return output_.handler->event_location();
  }

  event_loc_id
  input_location() const {
    return input_.handler->event_location();
  }

 protected:
  packet_flow_buffer(
    double out_bw,
    const timestamp& send_lat,
    const timestamp& credit_lat,
    packet_flow_bandwidth_arbitrator* arb);

  packet_flow_buffer() : bytes_delayed_(0) {}

  std::string
  buffer_string(const char* name) const;

 protected:
  packet_flow_bandwidth_arbitrator* arb_;
  packet_flow_input input_;
  packet_flow_output output_;
  long bytes_delayed_;

  static const int my_outport = 0;
  static const int my_inport = 0;
};

class packet_flow_finite_buffer :
  public packet_flow_buffer
{
 public:
  virtual void
  set_input(int this_inport, int src_outport,
            event_handler* input);

  long
  size_bytes() const {
    return size_bytes_;
  }

  int
  num_initial_credits() const {
    return size_bytes_;
  }

 protected:
  long size_bytes_;

  packet_flow_finite_buffer(
    double out_bw,
    const timestamp& send_lat,
    const timestamp& credit_lat,
    int max_num_bytes,
    packet_flow_bandwidth_arbitrator* arb) :
      size_bytes_(max_num_bytes),
      packet_flow_buffer(out_bw, send_lat, credit_lat, arb)
  {
  }

  packet_flow_finite_buffer(){}

};

class packet_flow_infinite_buffer :
  public packet_flow_buffer
{
 protected:
  packet_flow_infinite_buffer(
    double out_bw,
    const timestamp& send_lat,
    packet_flow_bandwidth_arbitrator* arb):
   packet_flow_buffer(out_bw, send_lat, timestamp(0), arb)
  {
  }

  packet_flow_infinite_buffer(){}

  void //no-op, I don't need to send credits to an input, I'm infinite
  set_input(int my_inport, int dst_outport, event_handler *input){}

  int
  num_initial_credits() const;

};

class packet_flow_network_buffer :
  public packet_flow_finite_buffer
{
 public:
  packet_flow_network_buffer() {}

  packet_flow_network_buffer(
    double out_bw,
    const timestamp& send_lat,
    const timestamp& credit_lat,
    int max_num_bytes,
    int num_vc,
    packet_flow_bandwidth_arbitrator* arb);

  int
  queue_length() const;

  void
  init_credits(int port, int num_credits);

  virtual void
  start(const sst_message::ptr& msg);

  void
  handle_credit(const packet_flow_credit::ptr& msg);

  void
  do_handle_payload(const packet_flow_payload::ptr& msg);

  std::string
  packet_flow_name() const {
    return "network buffer";
  }

  void
  set_sanity_params(bool queue_depth_reporting, int queue_depth_delta) {
    queue_depth_reporting_ = queue_depth_reporting;
    queue_depth_delta_ = queue_depth_delta;
  }

  void deadlock_check();

  void deadlock_check(const sst_message::ptr &msg);

 protected:
  int num_vc_;
  std::vector<payload_queue> queues_;
  std::vector<int> credits_;

 private:
  void build_blocked_messages();

 private:
  std::set<int> deadlocked_channels_;
  std::map<int, std::list<packet_flow_payload::ptr> > blocked_messages_;
  bool queue_depth_reporting_;
  int queue_depth_delta_;
};

class packet_flow_eject_buffer :
  public packet_flow_finite_buffer
{
 public:
  packet_flow_eject_buffer(
    double out_bw,
    const timestamp& send_lat,
    const timestamp& credit_lat,
    int max_num_bytes,
    packet_flow_bandwidth_arbitrator* arb);

  packet_flow_eject_buffer() {}

  void
  handle_credit(const packet_flow_credit::ptr& msg);

  void
  return_credit(const message_chunk::ptr& msg);

  void
  start(const sst_message::ptr& msg);

  void
  do_handle_payload(const packet_flow_payload::ptr& msg);

  std::string
  packet_flow_name() const {
    return "eject buffer";
  }

  void
  init_credits(int port, int num_credits);

};

class packet_flow_injection_buffer :
  public packet_flow_infinite_buffer
{
 public:
  packet_flow_injection_buffer(
    double out_bw,
    const timestamp& out_lat,
    packet_flow_bandwidth_arbitrator* arb);

  virtual void
  start(const sst_message::ptr& msg);

  int
  get_queue_length() const;

  void
  init_credits(int port, int num_credits);

  void
  handle_credit(const packet_flow_credit::ptr& msg);

  void
  do_handle_payload(const packet_flow_payload::ptr& msg);

  std::string
  packet_flow_name() const {
    return "inject buffer";
  }

 protected:
  struct pending_send{
    sst_message::ptr msg;
    long bytes_left;
    long offset;
  };

  std::list<pending_send> pending_;
  long credits_;

  packet_flow_injection_buffer() {}

  void
  send_what_you_can();

};

}
}


#endif // BUFFER_H


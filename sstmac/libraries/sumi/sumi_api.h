#ifndef SSTMAC_SIMPMSG_API_H
#define SSTMAC_SIMPMSG_API_H

#include <sstmac/libraries/sumi/message.h>
#include <sstmac/software/process/pmi.h>
#include <sstmac/software/process/app_manager.h>
#include <sstmac/software/libraries/service.h>
#include <sstmac/software/api/api.h>
#include <sstmac/hardware/network/network_message.h>

/**
 * SUMI = Simulator unified messagine interface
 * It is also the name for a solid ink in Japanese -
 * i.e. the substrate for sending messages!
 */

namespace sstmac {

class sumi_queue
{

 public:
  std::string
  to_string() const {
    return "SUMI message queue";
  }

  sumi_queue(sstmac::sw::operating_system* os);

  sumi_queue();

  transport_message::ptr
  poll_until_message();

  transport_message::ptr
  poll_until_message(timestamp timeout);

  void
  put_message(const transport_message::ptr& message);

  bool
  blocked() const {
    return !blocked_keys_.empty();
  }

 private:
  std::list<transport_message::ptr> pending_messages_;

  std::list<sstmac::sw::key*> blocked_keys_;

  sstmac::sw::operating_system* os_;

};

class sumi_api :
  public sstmac::sw::api,
  public sstmac::sw::process_manager
{

 public:
  sumi_api();

  virtual void
  init();

  virtual void
  finalize();

  virtual ~sumi_api(){}

  virtual void
  init_param1(const sstmac::sw::software_id& sid);

  virtual void
  init_os(sstmac::sw::operating_system* os);

  virtual void
  init_factory_params(sprockit::sim_parameters* params);

  transport_message::payload_ptr
  poll_until_notification();

  transport_message::payload_ptr
  poll_until_notification(timestamp timeout);

  virtual transport_message::payload_ptr
  handle(const transport_message::ptr& msg) = 0;
  
  void
  incoming_message(const transport_message::ptr& msg);

  void
  transport_send(
    long byte_length,
    const transport_message::payload_ptr& msg,
    sstmac::hw::network_message::type_t ty,
    int dst,
    bool needs_ack,
    void* buffer = 0);
  
  bool
  blocked() const {
    return queue_->blocked();
  }

 private:
  std::string server_libname_;

  sstmac::sw::software_id sid_;

  sstmac::sw::app_manager* env_;

  /**
   * @brief queue_
   * Manages incoming/outgoing messages
   */
  sumi_queue* queue_;
  
 protected:
  event_loc_id loc_;

  int rank_;

  int nproc_;

};


class sumi_server :
  public sstmac::sw::service
{

 public:
  sumi_server(int appid);

  void
  incoming_message(const sstmac::sst_message::ptr& msg);

  void
  register_proc(int rank, sumi_api* proc);

 private:
  int appid_;

  spkt_unordered_map<int, sumi_api*> procs_;

  sumi_api*
  get_proc(int rank) const;

};

}

#endif // SSTMAC_SIMPMSG_API_H


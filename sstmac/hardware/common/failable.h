#ifndef FAILABLE_H
#define FAILABLE_H

#include <sstmac/common/messages/sst_message.h>
#include <sstmac/hardware/common/connection.h>

namespace sstmac {
  namespace hw {

class failable :
  public connectable_component
{

 public:
  std::string
  to_string() const {
    return "failable";
  }

  virtual ~failable(){}

  void
  handle(const sst_message::ptr& msg);

  bool
  failed() const {
    return failed_;
  }

  void
  fail(const sst_message::ptr& msg);

#if SSTMAC_INTEGRATED_SST_CORE
 protected:
  failable(SST::ComponentId_t id, SST::Params& params) : 
    connectable_component(id, params),
    failed_(false)
  {
  }
#else
 public:
  int
  thread_id() const {
    return event_scheduler::thread_id();
  }

 protected:
  failable() :
   failed_(false)
  {
  }
#endif

 protected:
  virtual void
  do_failure(const sst_message::ptr& msg){}

  virtual void
  handle_while_failed(const sst_message::ptr& msg) = 0;

  virtual void
  handle_while_running(const sst_message::ptr& msg) = 0;

 protected:
  bool failed_;

};

  }
}

#endif // FAILABLE_H

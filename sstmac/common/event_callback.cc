#include <sstmac/common/event_callback.h>

namespace sstmac {


event_callback::~event_callback()
{
}

void
event_callback::handle(const sst_message::ptr& msg)
{
  callback(msg);
}

std::string
event_callback::to_string() const
{
  return "event callback";
}

}



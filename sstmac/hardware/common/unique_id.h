#ifndef UNIQUE_ID_H
#define UNIQUE_ID_H

#include <sstream>
#include <stdint.h>

#include <sprockit/spkt_config.h>

namespace sstmac {
namespace hw {

struct unique_msg_id {
  uint32_t src_node;
  uint32_t msg_num;

  unique_msg_id(uint32_t src, uint32_t num) :
    src_node(src), msg_num(num) {
  }

  unique_msg_id() :
    src_node(-1), msg_num(0) {
  }

  operator uint64_t() const {
    //map onto single 64 byte number
    uint64_t lo = msg_num;
    uint64_t hi = (((uint64_t)src_node)<<32);
    return lo | hi;
  }

  std::string
  to_string() const {
    std::stringstream sstr;
    sstr << "unique_id(" << src_node << "," << msg_num << ")";
    return sstr.str();
  }

  void
  set_src_node(uint32_t src) {
    src_node = src;
  }

  void
  set_seed(uint32_t seed) {
    msg_num = seed;
  }

  unique_msg_id& operator++() {
    ++msg_num;
    return *this;
  }

  unique_msg_id operator++(int) {
    unique_msg_id other(*this);
    ++msg_num;
    return other;
  }
};


}
}


#if SPKT_HAVE_CPP11
namespace std {
template <>
struct hash<sstmac::hw::unique_msg_id>
  : public std::hash<uint64_t>
{ };
}
#else
namespace sstmac { namespace hw {
inline std::size_t
hash_value(const unique_msg_id& id){
  return uint64_t(id);
}
}}
#endif



#endif // UNIQUE_ID_H

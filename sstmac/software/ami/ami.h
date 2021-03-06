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


#ifndef SSTMAC_SOFTWARE_AMI_AMI_H_INCLUDED
#define SSTMAC_SOFTWARE_AMI_AMI_H_INCLUDED

#include <sstmac/common/sstmac_config.h>
#include <sstmac/common/timestamp.h>
#include <sstmac/common/messages/sst_message_fwd.h>

#include <sprockit/spkt_config.h>

namespace sstmac {
namespace ami {

  enum CAT {
    COMPUTE = 0,
    COMM = 1,
    DISP = 2,
    QUERY = 3,
    HW = 4
  };

  enum COMM_FUNC {
    COMM_SEND = 10101, //pretty much have to implement this one
    COMM_BCAST,
    COMM_BARRIER
  };

  enum COMP_FUNC {
    COMP_TIME = 67, //the basic compute-for-some-time
    COMP_MEM,
    COMP_INSTR,
    COMP_EIGER,
    COMP_DISKACCESS,
    COMP_STOCH,
    COMP_SLEEP,
    COMP_REGISTER_THREAD,
    GPU_MEMCPY,
    GPU_EX_ASYNC
  };

  const char*
  tostr(COMP_FUNC func);

  const char*
  tostr(COMM_FUNC func);

  const char*
  tostr(CAT cat);

}
} //end of namespace sstmac

#if SPKT_HAVE_CPP11
namespace std {
template <>
struct hash<sstmac::ami::CAT> : public hash<int> { };
}
#endif

#endif


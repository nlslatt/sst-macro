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

#ifndef SSTMAC_SOFTWARE_PROCESS_GLOBAL_BASE_H_INCLUDED
#define SSTMAC_SOFTWARE_PROCESS_GLOBAL_BASE_H_INCLUDED

#include <sstmac/software/process/process_context.h>

namespace sstmac {
namespace sw {

class sstmac_global
{
 protected:
  process_context
  current_context() const;

};


}
}
#endif


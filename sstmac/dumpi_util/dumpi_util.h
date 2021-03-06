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

#ifndef SSTMAC_SOFTWARE_SKELETONS_UNDUMPI_DUMPIUTIL_H_INCLUDED
#define SSTMAC_SOFTWARE_SKELETONS_UNDUMPI_DUMPIUTIL_H_INCLUDED

#include <sstmac/dumpi_util/dumpi_meta.h>

namespace sstmac {
namespace sw {

std::string
dumpi_file_name(int rank, std::string fileroot_);

int
getnumprocs(dumpi_meta* dmeta_);

}
}

#endif


#
#   This file is part of SST/macroscale: 
#                The macroscale architecture simulator from the SST suite.
#   Copyright (c) 2009 Sandia Corporation.
#   This software is distributed under the BSD License.
#   Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#   the U.S. Government retains certain rights in this software.
#   For more information, see the LICENSE file in the top 
#   SST/macroscale directory.
#

# This order is necessary to satisfy dependencies
SUBDIRS = \
machines \
blas \
sumi \
sockets \
pthread \
uq

if WITH_DEFAULT_MPI
SUBDIRS += mpi
endif

library_includedir=$(includedir)/sstmac/libraries
library_include_HEADERS =


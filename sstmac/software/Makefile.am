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

include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libsstmac_sw.la
libsstmac_sw_la_LDFLAGS = 

libsstmac_sw_la_LIBADD = 

AM_CPPFLAGS += -I$(top_builddir)/pth -I$(top_srcdir)/pth -I$(top_srcdir)/dumpi -I$(top_builddir)/dumpi

AM_FCFLAGS = -I$(top_builddir) -I$(top_srcdir) -m32 -m64

libsstmac_sw_la_SOURCES = \
  ami/ami.cc \
  api/api.cc \
  process/compute_scheduler.cc \
  process/cpuset_compute_scheduler.cc \
  process/simple_compute_scheduler.cc \
  process/pmi.cc \
  process/key.cc \
  process/global.cc \
  process/graphviz.cc \
  process/ftq.cc \
  process/app_manager.cc \
  process/operating_system.cc \
  process/api.cc \
  process/thread.cc \
  process/app.cc \
  process/user_app.cc \
  process/time.cc \
  process/user_app_cxx.cc \
  process/user_app_c.cc \
  process/user_app_f90.cc \
  threading/context_util.cc \
  threading/stack_alloc_chunk.cc \
  threading/stack_alloc.cc \
  threading/threading_pthread.cc \
  threading/threading_pth.cc \
  libraries/library.cc \
  libraries/compute/compute_api.cc \
  libraries/compute/compute_message.cc \
  libraries/compute/lib_compute.cc \
  libraries/compute/lib_compute_inst.cc \
  libraries/compute/lib_compute_matrix.cc \
  libraries/compute/lib_compute_memmove.cc \
  libraries/compute/lib_compute_time.cc \
  libraries/compute/lib_sleep.cc \
  libraries/compute/lib_compute_loops.cc \
  libraries/compute/sstmac_compute.cc \
  libraries/unblock_handler.cc \
  libraries/unblock_event.cc \
  libraries/service.cc \
  launch/allocation_strategy.cc \
  launch/cart_allocation.cc \
  launch/coordinate_allocation.cc \
  launch/random_allocation.cc \
  launch/node_id_allocation.cc \
  launch/hostname_allocation.cc \
  launch/dumpi_allocation.cc \
  launch/first_available_allocation.cc \
  launch/index_strategy.cc \
  launch/block_indexing.cc \
  launch/coordinate_indexing.cc \
  launch/dumpi_indexer.cc \
  launch/hostname_indexer.cc \
  launch/random_indexing.cc \
  launch/round_robin_indexing.cc \
  launch/node_id_indexing.cc \
  launch/launcher.cc \
  launch/launch_info.cc \
  launch/messages.cc


if FC_ENABLE
  libsstmac_sw_la_SOURCES += \
   libraries/compute/sstmac_compute_f.cc
endif

library_includedir=$(includedir)/sstmac/software

nobase_library_include_HEADERS = \
  ami/ami.h \
  api/api.h \
  api/api_fwd.h \
  process/app_manager.h \
  process/app_manager_fwd.h \
  process/pmi.h \
  process/simple_compute_scheduler.h \
  process/cpuset_compute_scheduler.h \
  process/compute_scheduler.h \
  process/compute_scheduler_fwd.h \
  process/backtrace.h \
  process/graphviz.h \
  process/graphviz_fwd.h \
  process/ftq.h \
  process/ftq_fwd.h \
  process/app.h \
  process/app_fwd.h \
  process/user_app.h \
  process/api.h \
  process/app_id.h \
  process/key.h \
  process/key_fwd.h \
  process/operating_system.h \
  process/operating_system_fwd.h \
  process/software_id.h \
  process/task_id.h \
  process/time.h \
  process/thread_data.h \
  process/thread.h \
  process/thread_fwd.h \
  process/thread_id.h \
  process/localize_global.h \
  process/global.h \
  process/global_base.h \
  process/global_builtin.h \
  process/global_builtin_array.h \
  process/global_ptr.h \
  process/global_ptr_array.h \
  process/global_struct.h \
  process/process_context.h \
  process/user_app_cxx.h \
  process/user_app_c.h \
  process/user_app_f90.h \
  threading/context_util.h \
  threading/stack_alloc_chunk.h \
  threading/stack_alloc.h \
  threading/threading_interface.h \
  threading/threading_interface_fwd.h \
  threading/threading_pth.h \
  threading/threading_pthread.h \
  libraries/compute/compute_api.h \
  libraries/compute/compute_message.h \
  libraries/compute/compute_message_fwd.h \
  libraries/compute/lib_compute_inst.h \
  libraries/compute/lib_compute_inst_fwd.h \
  libraries/compute/lib_compute_memmove_fwd.h \
  libraries/compute/lib_compute_matrix.h \
  libraries/compute/lib_compute_memmove.h \
  libraries/compute/lib_compute_time.h \
  libraries/compute/lib_compute.h \
  libraries/compute/lib_sleep.h \
  libraries/compute/lib_sleep_fwd.h \
  libraries/compute/lib_compute_loops.h \
  libraries/compute/sstmac_compute.h \
  libraries/library.h \
  libraries/library_fwd.h \
  libraries/unblock_handler.h \
  libraries/unblock_event.h \
  libraries/service.h \
  libraries/service_fwd.h \
  launch/allocation_strategy.h \
  launch/cart_allocation.h  \
  launch/coordinate_allocation.h \
  launch/node_id_allocation.h \
  launch/random_allocation.h \
  launch/hostname_allocation.h \
  launch/dumpi_allocation.h \
  launch/first_available_allocation.h \
  launch/index_strategy.h \
  launch/block_indexing.h \
  launch/coordinate_indexing.h \
  launch/dumpi_indexer.h \
  launch/hostname_indexer.h \
  launch/random_indexing.h \
  launch/node_id_indexing.h \
  launch/round_robin_indexing.h  \
  launch/launcher.h \
  launch/launcher_fwd.h \
  launch/launch_handler.h \
  launch/launch_info.h  \
  launch/launch_info_fwd.h  \
  launch/arrive_message.h  \
  launch/complete_message.h  \
  launch/kill_message.h  \
  launch/launch_message.h  \
  launch/launch_message_fwd.h  \
  launch/restart_message.h  \
  launch/start_message.h  \
  launch/machine_descriptor.h 

if HAVE_UCONTEXT
  libsstmac_sw_la_SOURCES += \
     threading/threading_ucontext.cc 

  nobase_library_include_HEADERS += \
    threading/threading_ucontext.h 
endif

if FC_ENABLE
  nobase_library_include_HEADERS += \
    libraries/compute/sstmac_compute_f.h
endif

if VTK_ENABLE
 AM_CPPFLAGS += $(VTK_IFLAGS)
 AM_LDFLAGS = $(VTK_LDFLAGS)
 AM_LIBS = $(VTK_LIBS)
endif


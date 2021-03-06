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

noinst_LTLIBRARIES = libsstmac_common.la

distclean-local:
	rm -f sstmac_config.h

libsstmac_common_la_SOURCES = \
  runtime.cc \
  cartgrid.cc \
  event_handler.cc \
  event_scheduler.cc \
  event_callback.cc \
  logger.cc \
  mersenne_twister.cc \
  rng.cc \
  timestamp.cc \
  event_manager.cc \
  c_params.cc \
  sst_event.cc \
  lodepng.cc \
  sstmac_env.cc \
  thread_info.cc \
  vis/vis.cc \
  thread_lock.cc \
  messages/message_chunk.cc \
  messages/message_buffer_cache.cc \
  messages/payload.cc \
  messages/library_message.cc \
  messages/sleep_message.cc \
  messages/timed_message.cc \
  messages/sst_message.cc \
  messages/value_payload.cc \
  messages/wrapper_payload.cc \
  messages/vector_payload.cc \
  stats/stat_collector.cc \
  stats/stat_spyplot.cc \
  stats/stat_histogram.cc \
  stats/event_trace.cc \
  stats/location_trace.cc \
  stats/stat_local_int.cc \
  stats/stat_local_double.cc \
  stats/stat_global_int.cc

if VTK_ENABLE
 libsstmac_common_la_SOURCES += vis/vtk_engine.cc

 AM_CPPFLAGS += $(VTK_IFLAGS)
 AM_LDFLAGS = $(VTK_LDFLAGS)
 AM_LIBS = $(VTK_LIBS)
endif

nodist_library_include_HEADERS = sstmac_config.h config.h

library_includedir=$(includedir)/sstmac/common

nobase_library_include_HEADERS = \
  cartgrid.h \
  c_params.h \
  event_callback.h \
  event_callback_fwd.h \
  event_handler.h \
  event_handler_fwd.h \
  event_location.h \
  event_manager.h \
  event_manager_fwd.h \
  event_scheduler.h \
  event_scheduler_fwd.h \
  logger.h \
  mersenne_twister.h \
  node_address.h \
  rng.h	\
  runtime.h \
  timestamp.h \
  timestamp_fwd.h \
  sst_event.h \
  sst_event_fwd.h \
  thread_info.h \
  vis/vis.h \
  lodepng.h \
  sstmac_env.h \
  thread_safe_int.h \
  thread_lock.h \
  sim_thread_lock.h \
  messages/payload.h \
  messages/message_buffer_cache.h \
  messages/sst_message.h \
  messages/sst_message_fwd.h \
  messages/timed_message.h \
  messages/library_message.h \
  messages/sleep_message.h \
  messages/callback_message.h \
  messages/message_chunk.h \
  messages/value_payload.h \
  messages/pair_payload.h \
  messages/vector_payload.h \
  messages/wrapper_payload.h \
  stats/stat_spyplot.h \
  stats/stat_spyplot_fwd.h \
  stats/stat_collector.h \
  stats/stat_collector_fwd.h \
  stats/stat_histogram.h \
  stats/stat_histogram_fwd.h \
  stats/event_trace.h \
  stats/location_trace.h \
  stats/stat_local_int.h \
  stats/stat_local_int_fwd.h \
  stats/stat_local_double.h \
  stats/stat_local_double_fwd.h \
  stats/stat_global_int.h \
  stats/stat_global_int_fwd.h

if VTK_ENABLE
 nobase_library_include_HEADERS += vis/vtk_engine.h
endif



# -*- Mode: Makefile; -*-
# vim: set ft=automake :
#
# (C) 2011 by Argonne National Laboratory.
#     See COPYRIGHT in top-level directory.
#

include $(top_srcdir)/tests/mpi/Makefile.mtest

EXTRA_DIST = testlist

## for all programs that are just built from the single corresponding source
## file, we don't need per-target _SOURCES rules, automake will infer them
## correctly
noinst_PROGRAMS =  \
    anyall         \
    bottom         \
    bsend1         \
    bsend2         \
    bsend3         \
    bsend4         \
    bsend5         \
    bsendalign     \
    cancelrecv     \
    isendself      \
    sendself       \
    eagerdt        \
    isendselfprobe \
    bsendfrag      \
    bsendpending   \
    icsend         \
    rqstatus       \
    rqfreeb        \
    greq1          \
    scancel        \
    scancel2       \
    rcancel        \
    pscancel       \
    sendflood      \
    sendrecv1      \
    sendrecv2      \
    sendrecv3      \
    waitany-null   \
    pingping       \
    probe-unexp    \
    probenull      \
    inactivereq    \
    waittestnull   \
    sendall        \
    large_message  \
    mprobe


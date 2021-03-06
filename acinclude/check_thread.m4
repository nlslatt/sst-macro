
AC_DEFUN([CHECK_THREAD_PARALLEL], [
AC_ARG_ENABLE([multithread],
  [AS_HELP_STRING([--enable-multithread],
    [enable thread-parallel PDES simulation])],
  [with_multithread=$enableval],
  [with_multithread=no])
if test "X$with_multithread" = "Xyes"; then
 AC_DEFINE_UNQUOTED([USE_MULTITHREAD], 1, "Whether to enable multithreading")
 AC_SUBST([mt_cmdline_args],["-p sst_nthread=2 -c '0 2'"])
else
 AC_SUBST([mt_cmdline_args],[""])
fi

AC_ARG_ENABLE([spinlock],
  [AS_HELP_STRING([--enable-spinlock],
    [enable spin locks for more efficient thread barriers])],
  [with_spinlock=$enableval],
  [with_spinlock=no])
if test "X$with_spinlock" = "Xyes"; then
  AC_DEFINE_UNQUOTED([USE_SPINLOCK], 1, "Whether to use spin locks for more efficient thread barriers")
fi

# pthread_setaffinity_np checking: from https://github.com/landonf/haiku-buildtools/blob/master/gcc/libgomp/configure.ac
# Check for pthread_{,attr_}[sg]etaffinity_np.
AH_TEMPLATE([HAVE_PTHREAD_AFFINITY_NP],
            [Define if pthread_{,attr_}{g,s}etaffinity_np is supported.])
AC_MSG_CHECKING([whether pthread_setaffinity_np is usable])
AC_LINK_IFELSE(
 [AC_LANG_PROGRAM(
  [#define _GNU_SOURCE
   #include <pthread.h>],
  [cpu_set_t cpuset;
   pthread_attr_t attr;
   pthread_getaffinity_np (pthread_self (), sizeof (cpu_set_t), &cpuset);
   if (CPU_ISSET (0, &cpuset))
     CPU_SET (1, &cpuset);
   else
     CPU_ZERO (&cpuset);
   pthread_setaffinity_np (pthread_self (), sizeof (cpu_set_t), &cpuset);
   pthread_attr_init (&attr);
   pthread_attr_getaffinity_np (&attr, sizeof (cpu_set_t), &cpuset);
   pthread_attr_setaffinity_np (&attr, sizeof (cpu_set_t), &cpuset);])],
  [
AC_MSG_RESULT([yes])
AC_DEFINE(HAVE_PTHREAD_AFFINITY_NP, 1,
[   Define if pthread_{,attr_}{g,s}etaffinity_np is supported.]
)
], [ AC_MSG_RESULT([no]) ]
)

AC_ARG_ENABLE([cpu-affinity],
  [AS_HELP_STRING([--enable-cpu-affinity],
    [enable strict cpu affinity for multithreading])],
  [with_cpu_affinity=$enableval],
  [with_cpu_affinity=no])
if test "X$with_cpu_affinity" = "Xyes"; then
  AC_DEFINE_UNQUOTED([USE_CPU_AFFINITY], 1, "Whether to enable strict CPU affinity")
else
  AC_DEFINE_UNQUOTED([USE_CPU_AFFINITY], 0, "Whether to enable strict CPU affinity")
fi
])

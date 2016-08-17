#include <sstmac/util.h>
#include <sstmac/variable.h>
#include <sstmac/replacements/mpi.h>

#define run_fxn(fxn, t_expected, ...) \
{  \
  double t_start, t_stop, t_total; \
  t_start = MPI_Wtime(); \
  fxn(__VA_ARGS__); \
  t_stop = MPI_Wtime(); \
  t_total = t_stop - t_start; \
  if (int(t_expected-t_total) != 0) \
    std::cerr << "FAILED: Function " #fxn " had unexpected number of flops.\n"; \
  else \
    std::cout << "SUCCESS: Function " #fxn " had expected number of flops.\n"; \
}

extern "C" int ubuntu_cant_name_mangle() { return 0; }

#define sstmac_app_name "test_variable"

void op(){ Double a = 0, b = 0; a += b; }

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  run_fxn(op, 1);

  MPI_Finalize();

  return 0;
}

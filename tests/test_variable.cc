#include <sstmac/util.h>
#include <sstmac/variable.h>
#include <sstmac/replacements/mpi.h>
#include <cmath>

#define sstmac_app_name "test_variable"

extern "C" int ubuntu_cant_name_mangle() { return 0; }

// THIS TEST ONLY WORKS WITH PROCESSOR SPEED OF 1 HZ

static int n_failures = 0;

template <class op>
bool
verify_nops(const std::string &name, int nops_expected){
  op o;
  double t_start = MPI_Wtime();
  o();
  double t_stop = MPI_Wtime();
  int nops_actual = round(t_stop - t_start);
  bool success = (nops_expected == nops_actual);

  if (success){
    std::cout << "SUCCESS: " << name << " had expected number of operations.\n";
  } else {
    std::cerr << "FAILED: " << name << " recorded " << nops_actual
              << " operation(s) when " << nops_expected
              << " was expected.\n";
    n_failures++;
  }
  return success;
}

#define DEF_UNARY_POSTFIX_OP(OP,SYM) \
  template <typename T> \
  struct unary_##SYM { void operator()(){ T a = 0; a OP; } };

#define DEF_UNARY_PREFIX_OP(OP,SYM) \
  template <typename T> \
  struct unary_##SYM { void operator()(){ T a = 0; OP a; } };

#define DEF_FUNC1(FUNC) \
  template <typename T> \
  struct func_##FUNC { void operator()(){ T a = 0; FUNC(a); } };

#define DEF_BINARY_OP(OP,SYM) \
  template <typename T, typename U> \
  struct binary_##SYM { void operator()(){ T a = 0; U b = 0; a OP b; } };

DEF_UNARY_POSTFIX_OP(++,ppr);
DEF_UNARY_POSTFIX_OP(--,mmr);

DEF_UNARY_PREFIX_OP(++,ppl);
DEF_UNARY_PREFIX_OP(--,mml);
DEF_UNARY_PREFIX_OP(-,ml);

DEF_FUNC1(sqrt);
DEF_FUNC1(cbrt);
DEF_FUNC1(fabs);

DEF_BINARY_OP(+,p);
DEF_BINARY_OP(-,m);
DEF_BINARY_OP(*,t);
DEF_BINARY_OP(/,d);

DEF_BINARY_OP(+=,pe);
DEF_BINARY_OP(-=,me);
DEF_BINARY_OP(*=,te);
DEF_BINARY_OP(/=,de);

DEF_BINARY_OP(&,a);
DEF_BINARY_OP(|,o);

DEF_BINARY_OP(&=,ae);
DEF_BINARY_OP(|=,oe);

template <typename T, typename N>
struct func_memset {
  void operator()(){ T a(10); memset(a, 0, 10*sizeof(N)); }
};

template <typename T, typename U, typename N>
struct func_memcpy1 {
  void operator()(){ T a(10); U b(10); memcpy(a, b, 10*sizeof(N)); }
};

template <typename T, typename U>
struct func_memcpy2 {
  void operator()(){ T a(10); U b[10]; memcpy(a, b, 10*sizeof(U)); }
};

template <typename T, typename U>
struct func_memcpy3 {
  void operator()(){ T a(10); U b[10]; const U *c = b; memcpy(a, c, 10*sizeof(U)); }
};

#define TEST_UNARY_POSTFIX_OP_NONCONST_DBL(OP,SYM) \
  verify_nops<unary_##SYM<Double>>("D" #OP,1);

#define TEST_UNARY_PREFIX_OP_NONCONST_DBL(OP,SYM) \
  verify_nops<unary_##SYM<Double>>(#OP "D",1);

#define TEST_FUNC1_CONST_DBL(FUNC) \
  verify_nops<func_##FUNC<Double>>(#FUNC "(D)",1); \
  verify_nops<func_##FUNC<const Double>>(#FUNC "(cD)",1);

#define TEST_BINARY_OP_NONCONST_DBL(OP,SYM) \
  verify_nops<binary_##SYM<Double,Double>>("D" #OP "D",1); \
  verify_nops<binary_##SYM<Double,double>>("D" #OP "d",1); \
  verify_nops<binary_##SYM<double,Double>>("d" #OP "D",1); \
  verify_nops<binary_##SYM<Double,const Double>>("D" #OP "cD",1); \
  verify_nops<binary_##SYM<Double,const double>>("D" #OP "cd",1); \
  verify_nops<binary_##SYM<double,const Double>>("d" #OP "cD",1);

#define TEST_BINARY_OP_CONST_DBL(OP,SYM) \
  verify_nops<binary_##SYM<Double,Double>>("D" #OP "D",1); \
  verify_nops<binary_##SYM<Double,double>>("D" #OP "d",1); \
  verify_nops<binary_##SYM<double,Double>>("d" #OP "D",1); \
  verify_nops<binary_##SYM<const Double,Double>>("cD" #OP "D",1); \
  verify_nops<binary_##SYM<const Double,double>>("cD" #OP "d",1); \
  verify_nops<binary_##SYM<const double,Double>>("cd" #OP "D",1); \
  verify_nops<binary_##SYM<Double,const Double>>("D" #OP "cD",1); \
  verify_nops<binary_##SYM<Double,const double>>("D" #OP "cd",1); \
  verify_nops<binary_##SYM<double,const Double>>("d" #OP "cD",1); \
  verify_nops<binary_##SYM<const Double,const Double>>("cD" #OP "cD",1); \
  verify_nops<binary_##SYM<const Double,const double>>("cD" #OP "cd",1); \
  verify_nops<binary_##SYM<const double,const Double>>("cd" #OP "cD",1);

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  TEST_UNARY_POSTFIX_OP_NONCONST_DBL(++,ppr);
  TEST_UNARY_POSTFIX_OP_NONCONST_DBL(--,mmr);

  TEST_UNARY_PREFIX_OP_NONCONST_DBL(++,ppl);
  TEST_UNARY_PREFIX_OP_NONCONST_DBL(--,mml);
  TEST_UNARY_PREFIX_OP_NONCONST_DBL(-,ml);

  TEST_FUNC1_CONST_DBL(sqrt);
  TEST_FUNC1_CONST_DBL(cbrt);
  TEST_FUNC1_CONST_DBL(fabs);

  TEST_BINARY_OP_CONST_DBL(+,p);
  TEST_BINARY_OP_CONST_DBL(-,m);
  TEST_BINARY_OP_CONST_DBL(*,t);
  TEST_BINARY_OP_CONST_DBL(/,d);

  TEST_BINARY_OP_NONCONST_DBL(+=,pe);
  TEST_BINARY_OP_NONCONST_DBL(-=,me);
  TEST_BINARY_OP_NONCONST_DBL(*=,te);
  TEST_BINARY_OP_NONCONST_DBL(/=,de);

  TEST_BINARY_OP_CONST_DBL(&,a);
  TEST_BINARY_OP_CONST_DBL(|,o);

  TEST_BINARY_OP_NONCONST_DBL(&=,ae);
  TEST_BINARY_OP_NONCONST_DBL(|=,oe);

  verify_nops<func_memset<DoublePtr,double>>("memset(D)",10);
  verify_nops<func_memcpy1<DoublePtr,DoublePtr,double>>("memcpy(D,D)",10);
  verify_nops<func_memcpy1<DoublePtr,const DoublePtr,double>>("memcpy(D,cD)",10);
  verify_nops<func_memcpy2<DoublePtr,double>>("memcpy(D,d)",10);
  verify_nops<func_memcpy3<DoublePtr,double>>("memcpy(D,cd)",10);

  MPI_Finalize();

  if (n_failures > 0){
    std::cerr << "Some tests failed!\n";
    return 1;
  } else {
    std::cerr << "All tests passed.\n";
    return 0;
  }
}

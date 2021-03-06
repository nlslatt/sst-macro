#ifndef sstmac_software_libraries_socketapiS_H
#define sstmac_software_libraries_socketapiS_H

#include <sstmac/software/api/api.h>
#include <sstmac/software/libraries/compute/lib_compute_inst_fwd.h>
#include <sstmac/software/libraries/compute/compute_message.h>
#include <sstmac/software/process/software_id.h>
#include <sstmac/software/process/operating_system_fwd.h>

DeclareDebugSlot(blas);

namespace sstmac {
namespace sw {

class blas_kernel :
  public sprockit::factory_type
{
 public:
  virtual std::string
  to_string() const = 0;

  virtual compute_message::ptr
  op_3d(int m, int k, int n);

  virtual compute_message::ptr
  op_2d(int m, int n);

  virtual compute_message::ptr
  op_1d(int n);

};
DeclareFactory(blas_kernel);


class blas_api :
  public api
{

 public:
  blas_api();

  virtual ~blas_api();

  virtual void
  init_os(operating_system* os);

  virtual void
  init_factory_params(sprockit::sim_parameters* params);

  virtual void
  finalize_init();

  virtual void
  init_param1(const software_id &sid);

  /**
   A(m,n) * B(n,k) = C(m,k)
  */
  void
  dgemm(int m, int n, int k);

  /**
   A(m,n) * X(n) = B(m)
  */
  void
  dgemv(int m, int n);

  void
  daxpy(int n);

  void
  ddot(int n);

 protected:
  void init_kernels(sprockit::sim_parameters* params);

 protected:
  lib_compute_inst* lib_compute_;

  software_id id_;

  static blas_kernel* ddot_kernel_;
  static blas_kernel* dgemm_kernel_;
  static blas_kernel* dgemv_kernel_;
  static blas_kernel* daxpy_kernel_;

};


}
} //end of namespace sstmac

#endif // socketapiS_H


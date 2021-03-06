#include <sstmac/libraries/blas/blas_api.h>
#include <sstmac/software/process/operating_system.h>
#include <sstmac/software/process/api.h>
#include <sstmac/software/libraries/compute/lib_compute_inst.h>

#define enumcase(x) case x: return #x;

ImplementAPI(sstmac::sw, blas_api, "blas");
RegisterDebugSlot(blas);

namespace sstmac {
namespace sw {

blas_kernel* blas_api::dgemm_kernel_;
blas_kernel* blas_api::dgemv_kernel_;
blas_kernel* blas_api::daxpy_kernel_;
blas_kernel* blas_api::ddot_kernel_;

SpktRegister("blas", api, blas_api, "Create bindings for BLAS");


blas_api::blas_api()
  : api(key::general)
{
}

blas_api::~blas_api()
{
}

void
blas_api::init_os(operating_system* os)
{
  library::init_os(os);

  os_->register_lib(this, lib_compute_);
}

void
blas_api::finalize_init()
{
}

void
blas_api::init_kernels(sprockit::sim_parameters* params)
{
  dgemm_kernel_ = blas_kernel_factory::get_optional_param("dgemm", "default_dgemm", params);
  dgemv_kernel_ = blas_kernel_factory::get_optional_param("dgemv", "default_dgemv", params);
  daxpy_kernel_ = blas_kernel_factory::get_optional_param("daxpy", "default_daxpy", params);
  ddot_kernel_ = blas_kernel_factory::get_optional_param("ddot", "default_ddot", params);
}

void
blas_api::init_factory_params(sprockit::sim_parameters* params)
{
  api::init_factory_params(params);
  if (!dgemm_kernel_){
    init_kernels(params);
  }
}

void
blas_api::ddot(int n)
{
  compute_message::ptr msg = ddot_kernel_->op_1d(n);
  lib_compute_->compute_inst(msg);
}

void
blas_api::dgemm(int m, int n, int k)
{
  compute_message::ptr msg = dgemm_kernel_->op_3d(m, n, k);
  lib_compute_->compute_inst(msg);
}

void
blas_api::dgemv(int m, int n)
{
  compute_message::ptr msg = dgemv_kernel_->op_2d(m,n);
  lib_compute_->compute_inst(msg);
}

void
blas_api::daxpy(int n)
{
  compute_message::ptr msg = daxpy_kernel_->op_1d(n);
  lib_compute_->compute_inst(msg);
}

void
blas_api::init_param1(const software_id &sid)
{
  api::init_param1(sid);
  id_ = sid;
  libname_ = "libblas-" + sid.to_string();

  std::string comp_lib_name = sprockit::printf("%s-lib-compute", libname_.c_str());
  lib_compute_ = new lib_compute_inst(comp_lib_name);
}

compute_message::ptr
blas_kernel::op_3d(int m, int k, int n)
{
  spkt_throw_printf(sprockit::unimplemented_error,
    "blas_kernel::mult_op: %s does not implement 3D ops",
    to_string().c_str());
}

compute_message::ptr
blas_kernel::op_2d(int m, int n)
{
  spkt_throw_printf(sprockit::unimplemented_error,
    "blas_kernel::mult_op: %s does not implement 2D ops",
    to_string().c_str());
}

compute_message::ptr
blas_kernel::op_1d(int n)
{
  spkt_throw_printf(sprockit::unimplemented_error,
    "blas_kernel::mult_op: %s does not implement 1D ops",
    to_string().c_str());
}


}
}


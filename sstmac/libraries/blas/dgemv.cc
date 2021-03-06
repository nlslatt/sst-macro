#include <sstmac/libraries/blas/blas_api.h>
#include <sprockit/sim_parameters.h>
#include <algorithm>

namespace sstmac {
namespace sw {

class default_dgemv :
  public blas_kernel
{
 public:
  std::string
  to_string() const {
    return "default dgemv";
  }

  compute_message::ptr
  op_2d(int m, int n);

  virtual void
  init_factory_params(sprockit::sim_parameters* params);

 protected:
  double loop_unroll_;
  double pipeline_;

};
SpktRegister("default_dgemv", blas_kernel, default_dgemv);

void
default_dgemv::init_factory_params(sprockit::sim_parameters* params)
{
  loop_unroll_ = params->get_optional_double_param("dgemv_loop_unroll", 4);
  pipeline_ = params->get_optional_double_param("dgemv_pipeline_efficiency", 2);
}

compute_message::ptr
default_dgemv::op_2d(int m, int n)
{
  long nops = long(m) * long(n);
  long nflops = nops / long(pipeline_);
  long loop_ops = nops / long(loop_unroll_) / long(pipeline_);

  long total_bytes = long(m)*long(n)*sizeof(double);

  compute_message::ptr msg = new compute_message();
  msg->set_event_value(compute_message::flop, nflops);
  msg->set_event_value(compute_message::intop, loop_ops);
  msg->set_event_value(compute_message::mem_sequential, total_bytes);
  return msg;
}

}
}




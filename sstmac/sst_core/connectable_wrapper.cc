#include <sstmac/sst_core/connectable_wrapper.h>
#include <Python.h>

namespace sstmac {

PyObject* connectable_proxy_component::sst = NULL;
connectable_proxy_component::switch_map connectable_proxy_component::registered_switches;
connectable_proxy_component::endpoint_map connectable_proxy_component::registered_endpoints;

connectable_proxy_component::~connectable_proxy_component()
{
  if(component_proxy) {
    Py_DECREF(component_proxy);
  }
}

void
connectable_proxy_component::connect_weighted(
  int src_port,
  int dst_port,
  connection_type_t ty,
  connectable* mod,
  double weight,
  int red
)
{
  connectable_proxy_component* other = safe_cast(connectable_proxy_component, mod);
  std::string link_name = sprockit::printf(
      "__auto_link_%s__(%s)_%d_to_(%s)_%d",
      str(ty),
      component_name.c_str(), src_port,
      other->component_name.c_str(), dst_port
  );
  std::string port_name_prefix = sprockit::printf(
      "__auto_port_%s_(%s)_%d_to_(%s)_%d_%f_%d",
      str(ty),
      component_name.c_str(), src_port,
      other->component_name.c_str(), dst_port,
      weight, red
  );
  std::string my_port_name = sprockit::printf("%s_src", port_name_prefix.c_str());
  std::string other_port_name = sprockit::printf("%s_dest", port_name_prefix.c_str());

  std::string latency_str;
  if (other->category == Switch && this->category == Switch){
    latency_str = hop_latency;
  } else if (other->category == Switch && this->category == Endpoint){
    latency_str = injection_latency;
  } else if (other->category == Endpoint && this->category == Switch){
    latency_str = injection_latency;
  } else {
    latency_str = "0 ms";
    //spkt_throw(sprockit::value_error,
    //  "connectable_proxy_component: connecting two endpoints");
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
  PyObject* link = PyObject_CallMethod(
      sst, "Link", "s", link_name.c_str()
  );
  PyObject* __discarded_result = PyObject_CallMethod(link, "connect", "(Oss)(Oss)",
      component_proxy, my_port_name.c_str(), latency_str.c_str(),
      other->component_proxy, other_port_name.c_str(), latency_str.c_str()
  );
#pragma GCC diagnostic pop

  Py_DECREF(__discarded_result);

  Py_DecRef(link);
}

}


/*
 *  This file is part of SST/macroscale:
 *               The macroscale architecture simulator from the SST suite.
 *  Copyright (c) 2009 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the LICENSE file in the top
 *  SST/macroscale directory.
 */

#ifndef SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_TYPES_MPITYPE_H_INCLUDED
#define SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_TYPES_MPITYPE_H_INCLUDED

#include <sstmac/libraries/mpi/type_operator.h>
#include <sprockit/spkt_config.h>
#include <sprockit/serializable.h>
#include <sprockit/opaque_typedef.h>

#include <iosfwd>
#include <map>

namespace sstmac {
namespace sw {

class pairdata;
class vecdata;
struct inddata;

typedef_opaque_int(mpi_type_id,int);
implement_opaque_int(mpi_type_id);

}}

#if SPKT_HAVE_CPP11
namespace std {
template <>
struct hash<sstmac::sw::mpi_type_id>
  : public std::hash<sprockit::opaque_type<int>>
{ };
}
#else
namespace sstmac { namespace sw {
inline std::size_t
hash_value(mpi_type_id id){
  return id.id_;
}
}}
#endif

namespace sstmac {
namespace sw {

/// MPI datatypes.
class mpi_type
{
 public:
  enum TYPE_TYPE {
    PRIM, PAIR, VEC, IND, NONE
  };

  mpi_type();

  /** Annoying memory leak avoider */
  void
  init_primitive(const char* label, const int sizeit, int align,
           int comb, mpi_type_operator* o);

  void
  init_primitive(const char* label, mpi_type*b1, mpi_type*b2,
           int size, int comb, mpi_type_operator* o);

  void
  init_primitive(const std::string& labelit, const int sizeit, int align,
           int comb, mpi_type_operator* o);

  //pair of primitives datatype
  void
  init_primitive(const std::string& labelit, mpi_type*b1, mpi_type*b2,
           int size, int comb, mpi_type_operator* o);

  void
  init_vector(const std::string &labelit, mpi_type*base, int count,
           int block, int str, bool in_elem, int comb);

  void
  init_indexed(const std::string &labelit,
           inddata* dat,
           int sz, int ext, int numint, int numdtype, int comb);

  // id gets assigned automatically by the constructor.
  mpi_type_id id;
  std::string label;

  static void
  delete_statics();

 public:
  void
  serialize_order(sprockit::serializer& ser);

  operator mpi_type_id() const {
    return id;
  }

  virtual ~mpi_type();

  TYPE_TYPE
  type() const {
    return type_;
  }

  int
  comb() const {
    return combiner_;
  }

  void
  set_comb(int c) {
    combiner_ = c;
  }

  int
  align() const {
    return alignment_;
  }

  int
  bytes_to_elements(size_t bytes) const;

  int
  packed_size() const;

  //returns the marker if available, true lb if not
  int
  lb() const;

  //disregards the marker, returns lowest displacement
  int
  true_lb() const;

  //returns the marker if available, true ub if not
  int
  ub() const;

  //returns the difference between markers if available, true extent if not
  int
  extent() const;

  //returns the actual difference between min and max displacements (ignore markers)
  int
  true_extent() const;

  void
  envelope(int* numint, int* numaddr, int* numdtype, int* comb);

  void
  contents(int* ints, int* addr, long* dtypes);

  virtual void
  pack(const void *inbuf, void *outbuf, int bytes) const;

  virtual void
  unpack(const void *inbuf, void *outbuf, int bytes) const;

  virtual std::string
  to_string() const;

  void
  set_committed(bool flag){
    committed_ = flag;
  }

  bool
  committed() const {
    return committed_;
  }

  // some implementations have other built-in types
  // DUMPI stores them by size
  // this just creates a list of types by size
  // this is a hack since these types cannot be operated on by a reduce
  static std::map<int, mpi_type> builtins;
  static mpi_type* mpi_null;
  static mpi_type* mpi_char;
  static mpi_type* mpi_unsigned_char;
  static mpi_type* mpi_signed_char;
  static mpi_type* mpi_wchar;
  static mpi_type* mpi_unsigned_long_long;
  static mpi_type* mpi_lb;
  static mpi_type* mpi_ub;
  static mpi_type* mpi_byte;
  static mpi_type* mpi_short;
  static mpi_type* mpi_unsigned_short;
  static mpi_type* mpi_int;
  static mpi_type* mpi_unsigned;
  static mpi_type* mpi_long;
  static mpi_type* mpi_unsigned_long;
  static mpi_type* mpi_float;
  static mpi_type* mpi_double;
  static mpi_type* mpi_long_double;
  static mpi_type* mpi_long_long_int;
  static mpi_type* mpi_long_long;
  static mpi_type* mpi_packed;
  static mpi_type* mpi_float_int;
  static mpi_type* mpi_double_int;
  static mpi_type* mpi_long_int;
  static mpi_type* mpi_short_int;
  static mpi_type* mpi_2int;
  static mpi_type* mpi_long_double_int;
  static mpi_type* mpi_complex;
  static mpi_type* mpi_complex8;
  static mpi_type* mpi_complex16;
  static mpi_type* mpi_complex32;
  static mpi_type* mpi_double_complex;
  static mpi_type* mpi_logical;
  static mpi_type* mpi_real;
  static mpi_type* mpi_real4;
  static mpi_type* mpi_real8;
  static mpi_type* mpi_real16;
  static mpi_type* mpi_double_precision;
  static mpi_type* mpi_integer;
  static mpi_type* mpi_integer1;
  static mpi_type* mpi_integer2;
  static mpi_type* mpi_integer4;
  static mpi_type* mpi_integer8;
  static mpi_type* mpi_2integer;
  static mpi_type* mpi_2complex;
  static mpi_type* mpi_2double_complex;
  static mpi_type* mpi_2real;
  static mpi_type* mpi_2double_precision;
  static mpi_type* mpi_character;

  mpi_type_operator*
  op() const {
    return op_;
  }

 protected:
  virtual void
  pack_action(void* packed_buf, void* unpacked_buf, int bytes, bool pack) const;

  virtual void
  pack_action_primitive(void* packed_buf, void* unpacked_buf, int bytes, bool pack) const;

  virtual void
  pack_action_pair(void* packed_buf, void* unpacked_buf, int bytes, bool pack) const;

  virtual void
  pack_action_vector(void* packed_buf, void* unpacked_buf, int bytes, bool pack) const;

  virtual void
  pack_action_indexed(void* packed_buf, void* unpacked_buf, int bytes, bool pack) const;


 protected:
  TYPE_TYPE type_;

  bool committed_;

  int alignment_; //required for primitive types

  pairdata* pdata_;
  vecdata* vdata_;
  inddata* idata_;

  int size_; //this is the packed size !!!
  size_t extent_; //holds the extent, as defined by the MPI standard

  //stuff for get_envelope
  int combiner_;
  int numints_;
  int numaddr_;
  int numdtype_;

  long parent_;

  mpi_type_operator* op_;


};

class pairdata :
  public sprockit::serializable,
  public sprockit::serializable_type<pairdata>
{
  ImplementSerializable(pairdata)

 public:
  virtual
  ~pairdata() {
  }

  virtual std::string
  to_string() const {
    return "pairdata";
  }

  mpi_type* base1_;
  mpi_type* base2_;

  void
  serialize_order(sprockit::serializer& ser);

};

class vecdata :
  public sprockit::serializable,
  public sprockit::serializable_type<vecdata>
{
  ImplementSerializable(vecdata)

 public:
  virtual
  ~vecdata() {
  }

  virtual std::string
  to_string() const {
    return "vecdata";
  }

  mpi_type* base_;
  int count_;
  int blocklen_;
  int stride_; //always in bytes!!!!

  void
  serialize_order(sprockit::serializer& ser);

};

struct ind_block {
  mpi_type* base;
  int disp; ///always in bytes!!!!
  int num;
};

struct inddata :
    public sprockit::serializable,
    public sprockit::serializable_type<inddata> {

  ImplementSerializable(inddata)

 public:
  virtual
  ~inddata() {
  }

  virtual std::string
  to_string() const {
    return "inddata";
  }

  int mindisp_; //in bytes, true lb
  int maxbyte_; //top byte, true ub
  int ub_;
  int lb_;
  spkt_unordered_map<int, ind_block> blocks;

  void
  serialize_order(sprockit::serializer& ser);

};

std::ostream&
operator<<(std::ostream &os, mpi_type* type);

}

} // end of namespace sstmac

#endif


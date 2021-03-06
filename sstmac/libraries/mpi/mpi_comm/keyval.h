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

#ifndef SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_COMM_KEYVAL_H_INCLUDED
#define SSTMAC_SOFTWARE_LIBRARIES_MPI_MPI_COMM_KEYVAL_H_INCLUDED

#include <sstmac/common/thread_safe_int.h>
#include <sstmac/libraries/mpi/sstmac_mpi_integers.h>
#include <string>

namespace sstmac {
namespace sw {

class keyval  {

 public:
  keyval(int k, MPI_Copy_function* c, MPI_Delete_function* d, void* e) :
    key_(k), copy_(c), del_(d), extra_(e) {
  }

  static int
  get_new_key() {
    return keycount++;
  }

  virtual
  ~keyval() {
  }

  virtual std::string
  to_string() const {
    return "keyval";
  }

  int
  key() const {
    return key_;
  }

  keyval*
  clone(int k) const;

  void
  set_val(void* val) {
    val_ = val;
  }

  void*
  val() const {
    return val_;
  }

 protected:
  int key_;
  MPI_Copy_function* copy_;
  MPI_Delete_function* del_;
  void* extra_;


  void* val_;

  static thread_safe_int keycount;

};

}
}

#endif /* KEYVAL_H_ */


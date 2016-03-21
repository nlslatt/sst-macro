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

#ifndef SSTMAC_COMMON_MESSAGES_SST_MESSAGE_H_INCLUDED
#define SSTMAC_COMMON_MESSAGES_SST_MESSAGE_H_INCLUDED


#include <sprockit/ser_ptr_type.h>
#include <sprockit/serializable.h>
#include <sstmac/common/node_address.h>

#include <sprockit/expandable_enum.h>
#include <sprockit/metadata_bits.h>

#include <sstmac/common/sst_event.h>
#include <sstmac/common/event_callback_fwd.h>
#include <sstmac/common/event_handler_fwd.h>
#include <sstmac/software/process/operating_system_fwd.h>
#include <sstmac/software/process/key_fwd.h>

namespace sstmac {

/**
 * A class describing an event.
 */
class message :
  public event
{

 public:
  declare_expandable_enum(field);

 public:
  virtual ~message() {}

  virtual std::string
  to_string() const {
    return "sst_message";
  }

  // --------------------------------------//

 public:
  message();

  /**
   * Virtual function to return size. Child classes should impement this
   * if they want any size tracked / modeled.
   * @return Zero size, meant to be implemented by children.
   */
  virtual long
  byte_length() const = 0;

  /**
   * Serialize this message during parallel simulation.
   * @param ser The serializer to use
   */
  virtual void
  serialize_order(sprockit::serializer& ser);

  virtual node_id
  toaddr() const = 0;

  virtual node_id
  fromaddr() const = 0;

  virtual uint64_t
  unique_id() const = 0;

  template <class T>
  T&
  get_field(field name){
    uint64_t* ptr = &fields_[name];
    return *reinterpret_cast<T*>(ptr);
  }

 protected:
  sw::key* key_;
  std::map<field, uint64_t> fields_;

};

implement_enum_functions(message::field)




} // end of namespace sstmac
#endif


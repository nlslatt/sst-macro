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

#include <sstmac/common/thread_safe_int.h>
#include <sstmac/software/process/thread.h>
#include <sstmac/software/process/operating_system.h>
#include <sstmac/software/process/key.h>
#include <sstmac/software/process/app.h>
#include <sstmac/software/libraries/library.h>
#include <sstmac/software/api/api.h>
#include <sstmac/common/sst_event.h>
#include <sprockit/errors.h>
#include <sprockit/output.h>
#include <iostream>
#include <exception>
#include <unistd.h>  // getpagesize()
#include <sys/mman.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstmac/software/process/localize_global.h>



namespace sstmac {
namespace sw {

static thread_safe_long THREAD_ID_CNT(0);
const app_id thread::main_thread_aid(-1);
const task_id thread::main_thread_tid(-1);

//
// Private method that gets called by the scheduler.
//
void
thread::init_thread(int physical_thread_id, threading_interface* threadcopy, void *stack,
                    int stacksize, operating_system* os, threading_interface *yield_to)
{
  stack_ = stack;
  stacksize_ = stacksize;
  os_ = os;
  int size = global_variables::context_size;
  if (size >= 0){
    global_variables_ = new char[size];
    ::memcpy(global_variables_, global_variables::global_initer, size);
  } else {
    global_variables_ = 0;
  }


  init_id();

  state_ = INITIALIZED;

  //there were libs that wanted to be registered
  if (!pending_libs_.empty()) {
    std::list<library*>::iterator it;
    for (it = pending_libs_.begin(); it != pending_libs_.end(); it++) {
      os_->register_lib(this, *it);
    }
  }

  pending_libs_.clear();

  init_os(os);

  context_ = threadcopy->copy();

  threadinfo* info = new threadinfo();
  info->thethread = this;

  context_->start_context(physical_thread_id, stack, stacksize, run_routine, info, yield_to);
}

thread*
thread::current()
{
  return operating_system::current_thread();
}

api*
thread::build_api(int aid, const std::string &name)
{
  if (parent_app_) {
    api* a = parent_app_->build_api(aid, name);
    register_lib(a);
    return a;
  }
  else {
    spkt_throw_printf(sprockit::illformed_error,
                     "thread::build_api: thread has no parent thread");
    return 0;
  }
}

void
thread::init_os(operating_system* os)
{
  //nothing by default
}

void
thread::clear_subthread_from_parent_app()
{
  //if this is canceled, the parent app might already be dead
  if (parent_app_){
    parent_app_->remove_subthread(this);
  }
}

/**
 * This can get called by anyone to have a thread exit, including during normal app termination
 */
void
thread::kill()
{
  // We are done, ask the scheduler to remove this task from the
  state_ = DONE;

  clear_subthread_from_parent_app();

  unregister_all_libs();

  // This is a little bit weird - kill is happening on a non-DES thread stack
  os_->complete_thread(true);

  //we will never actually arrive here, instead the os context switches out
}

void
thread::cleanup()
{
  if (pthread_map_){
    //don't delete here
    //someone will come along and try to join this thread
    //we have to leave to the a null pointer so the joiner
    //knows that the thread has finished
    (*pthread_map_)[thread_id_] = 0;
  }
}

class delete_thread_event :
  public event
{
 public:
  delete_thread_event(thread* thr) :
    thr_(thr),
    event(thr->os()->event_location(), thr->os()->event_location())
  {
  }

  std::string
  to_string() const {
    return "delete thread event";
  }

  void
  execute(){
    thr_->cleanup();
    delete thr_;
  }

 protected:
  thread* thr_;
};

//
// Run routine that defines the initial context for this task.
//
void
thread::run_routine(void* threadptr)
{
  threadinfo* info = (threadinfo*) threadptr;
  thread* self = info->thethread;
  delete info;

  // Go.
  if (self->is_initialized()) {
    self->state_ = ACTIVE;
    bool success = false;
    try {
      self->run();
      success = true;
      //JJW 11/6/2014 This here is weird
      //The thread has run out of work and is terminating
      //However, because of weird thread swapping the DES thread
      //might still operate on the thread... we need to delay the delete
      //until the DES thread has completely finished processing its current event
      START_VALID_SCHEDULE(self->os())
      self->os()->schedule_now(new delete_thread_event(self));
      STOP_VALID_SCHEDULE(self->os())
      //this doesn't so much kill the thread as context switch it out
      //it is up to the above delete thread event to actually to deletion/cleanup
      //all of this is happening ON THE THREAD - it kills itself
      //this is not the DES thread killing it
      self->kill();
    }
    catch (const std::exception &ex) {
      cerrn << "thread terminated with exception: " << ex.what()
                << "\n";
      // should forward the exception to the main thread,
      // but for now will abort
      cerrn << "aborting" << std::endl;
      std::cout.flush();
      std::cerr.flush();
      abort();
    }
    catch (const std::string& str) {
      cerrn << "thread terminated with string exception: " << str << "\n";
      cerrn << "aborting" << std::endl;
      std::cout.flush();
      std::cerr.flush();
      abort();
    }
  }
  else {
    spkt_throw(sprockit::os_error,
              "thread::run_routine: task has not been initialized");
  }
}

key::category schedule_delay("CPU_Sched Delay");

thread::thread() :
  state_(PENDING),
  isInit(false),
  backtrace_(0),
  bt_nfxn_(0),
  last_bt_collect_nfxn_(0),
  thread_id_(thread::main_thread),
  schedule_key_(0),
  p_txt_(process_context::none),
  stack_(0),
  cpumask_(0),
  pthread_map_(0),
  parent_app_(0)
{
  //make all cores possible active
  cpumask_ = ~(cpumask_);
  schedule_key_ = key::construct(schedule_delay);
}

long
thread::init_id()
{
  //thread id not yet initialized
  if (thread_id_ == thread::main_thread)
    thread_id_ = THREAD_ID_CNT++;
  //I have not yet been assigned a process context (address space)
  //make my own, for now
  if (p_txt_ == process_context::none)
    p_txt_ = thread_id_;
  return thread_id_;
}

void*
thread::get_tls_value(long thekey) const
{
  std::map<long,void*>::const_iterator it = tls_values_.find(thekey);
  if (it == tls_values_.end())
    return NULL;
  return it->second;
}

void
thread::set_tls_value(long thekey, void *ptr)
{
  tls_values_[thekey] = ptr;
}

void
thread::set_sid(const software_id& sid)
{
  aid_ = sid.app_;
  tid_ = sid.task_;
}

void
thread::append_backtrace(void* fxn)
{
  backtrace_[bt_nfxn_] = fxn;
  bt_nfxn_++;
}

void
thread::pop_backtrace()
{
  --bt_nfxn_;
  last_bt_collect_nfxn_ = std::min(last_bt_collect_nfxn_, bt_nfxn_);
}

void
thread::collect_backtrace(int nfxn)
{
  last_bt_collect_nfxn_ = nfxn;
}

void
thread::register_lib(library* lib)
{
  if (is_initialized()) {
    os_->register_lib(this, lib);
  }
  else {
    pending_libs_.push_back(lib);
  }
}

void
thread::spawn(thread* thr)
{
  thr->parent_app_ = parent_app();
  os_->start_thread(thr);
}

void
thread::unregister_all_libs()
{
  os_->unregister_all_libs(this);
}

timestamp
thread::now()
{
  return os_->now();
}

node_id
thread::physical_address()
{
  return os_->my_addr();
}

//
// Goodbye.
//
thread::~thread()
{
  if (backtrace_){
    graph_viz::delete_trace(backtrace_);
  }

  if (stack_){
    os_->free_thread_stack(stack_);
  }

  //all my apis should have been deleted
  //since they are libraries
  //i just need to clear the map
  apis_.clear();
}

//
// Start a new thread.
//
void
thread::start_thread(thread* thr)
{
  thr->p_txt_ = p_txt_;
  os_->start_thread(thr);
}

//
// Join threads.
//
void
thread::join()
{
  if (!this->is_initialized()) {
    // We can't context switch the caller out without first being initialized
    spkt_throw_printf(sprockit::illformed_error,
                     "thread::join: target thread has not been initialized.");
  }
  os_->join_thread(this);
}

}
} // end of namespace sstmac


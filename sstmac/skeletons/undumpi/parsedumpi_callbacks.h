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

#ifndef SSTMAC_SOFTWARE_SKELETONS_UNDUMPI_PARSEDUMPI_CALLBACKS_H_INCLUDED
#define SSTMAC_SOFTWARE_SKELETONS_UNDUMPI_PARSEDUMPI_CALLBACKS_H_INCLUDED

#include <sstmac/skeletons/undumpi/parsedumpi.h>
#include <sstmac/libraries/mpi/mpi_request.h>
#include <sstmac/libraries/mpi/mpi_status.h>
#include <sstmac/libraries/mpi/mpi_types/mpi_op.h>
#include <dumpi/libundumpi/libundumpi.h>
#include <stdint.h>


namespace sstmac {
namespace sw {

/// Populate C-style callbacks for a libundumpi parser.
class parsedumpi_callbacks
{
  friend class parsedumpi_unit_test;

 public:
  /// Accepted actions to be taken for unimplemented functions.
  /// These could be function pointers, but wrapping those in python
  /// leads to incredibly hard-to-read code.
  enum if_unimplemented { IGNORE, PRINT, ABORT };

 private:
  /// The parent for this callback-driven parser.
  /// Can be safely held by raw pointer, since the parent holds this
  /// object by value.
  parsedumpi *parent_;

  /// What do we do with an unimplemented function?
  if_unimplemented action_;

  /// Do we remap requests?
  bool pessimistic_;

  /// The callback struct we are using.
  /// This is pretty big (2.4 K), but at least it can be shared.
  static libundumpi_callbacks *cbacks_;

  /// The dumpi timestamp at which we finished the most recent MPI call.
  dumpi_clock trace_compute_start_;

  /// The state of perfcounters when the most recent MPI call was finished.
  std::vector<int64_t> perfctr_compute_start_;

  /// Map dumpi request identifiers to objects.
  typedef std::map<dumpi_request, mpi_request*> request_map_t;
  request_map_t request_;

  /// MPI types.  Gets pre-populated with all the standard datatypes.
  typedef std::map<dumpi_datatype, mpi_type_id> mpitype_map_t;
  mpitype_map_t mpitype_;

  /// Dumpi also (as of version 0.6 or so) contains datatype sizes.
  dumpi_sizeof datatype_sizes_;

  /// MPI communicators.
  typedef std::map<dumpi_comm, mpi_comm*> mpicomm_map_t;
  mpicomm_map_t mpicomm_;

  typedef std::map<dumpi_group, mpi_group*> mpigroup_map_t;
  mpigroup_map_t mpigroups_;

  /// MPI operations.
  typedef std::map<dumpi_op, mpi_op*> mpiop_map_t;
  mpiop_map_t mpiop_;

 public:
  /// Populate callbacks.
  parsedumpi_callbacks(parsedumpi *parent,
            if_unimplemented action = IGNORE,
            bool pessimistic = true);

  /// Start parsing.
  /**
   * @brief parse_stream
   * @param filename
   * @param print_progress Whether to print progress - usually only true for rank 0
   * @param percent_terminate -1 indicates run to end, otherwise terminate at some percent - only rank 0 shoudl cause termination
   */
  void parse_stream(const std::string &filename,
    bool print_progress,
    double percent_terminate);

 private:
  /// Initialize maps (datatypes etc.).  Called at constrution.
  void init_maps();

  /// Indicate that we are starting an MPI call.
  void start_mpi(const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf);

  /// Indicate that we have completed an MPI call.
  void end_mpi(const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf);

  /// Access the mpiapi.
  mpi_api* getmpi();

  bool
  pessimistic() const {
    return pessimistic_;
  }

  /// Store a single request handle.
  void store_request(dumpi_request id, mpi_request* request);

  /// Get a single request handle.
  mpi_request*& get_request(dumpi_request id);

  /// Get a group of request handles.
  void get_requests(int count, const dumpi_request *dumpireq,
                               std::vector<mpi_request*> &simreq);

  /// Remove a request from the map.
  void complete_request(dumpi_request id);

  /// Complete multiple requests.
  template <typename Iter>
  void complete_requests(Iter begin, Iter end);

  void nullify_request(dumpi_request rid);

  void nullify_requests(const dumpi_request* trace_requests,
    const std::vector<mpi_request*>& sst_reqs);

  void remap_request(dumpi_request in, dumpi_request out);

  void remap_request(int total_num_requests,
                    int dumpi_req_idx, int sst_req_idx,
                    const dumpi_request* tracefile_requests);

  /// The simulator and tracefile each completed some number of requests,
  /// but these requests may not be the same. This is compatiable with null requests.
  void remap_requests(int total_num_requests,
                      int num_done_tracefile,
                      const dumpi_request* dumpi_indices_completed,
                      const std::vector<int> &done_simulator,
                      const dumpi_request *trace_requests);

  /// Get an mpiid.
  /// Special handling for MPI_ROOT and MPI_ANY_SOURCE.
  mpi_id get_mpiid(dumpi_source id);

  /// Get an mpi tag.
  /// Special handling for MPI_ANY_TAG.
  mpi_tag get_mpitag(dumpi_tag tag);

  /// Add a new mpi type.
  void add_mpitype(dumpi_datatype id, mpi_type_id mpit);

  /// Erase the mapping for an mpi type.  Does not erase built-in mpi types.
  void erase_mpitype(dumpi_datatype id);

  /// Access mpi type.
  /// \throw sprockit::value_error if no mapping exists for this datatype.
  mpi_type_id get_mpitype(int count, dumpi_datatype id);

  /// Add a new mpi comm.
  void add_mpicomm(dumpi_comm id, mpi_comm* comm);

  /// Erase the mapping for an mpi comm.  Does not erase built-in comms.
  void erase_mpicomm(dumpi_comm id);

  /// Access an mpi communicator.
  mpi_comm* get_mpicomm(dumpi_comm id);

  /// Add a new mpi group.
  void add_mpigroup(dumpi_group id, mpi_group*comm);

  /// Erase the mapping for an mpi group.  Does not erase built-in group.
  void erase_mpigroup(dumpi_group id);

  /// Access an mpi communicator.
  mpi_group* get_mpigroup(dumpi_group id);

  /// Add a new mpi op.
  void add_mpiop(dumpi_op id, mpi_op* op);

  /// Erase the mapping for an mpi op.  Does not erase built-in operations.
  void erase_mpiop(dumpi_op id);

  /// Access an mpi opunicator.
  mpi_op* get_mpiop(dumpi_op id);

  /// Define all callback routines.
  void set_callbacks();

  /// This is what happens during unimplemented functions.
  int handle_unimplemented(const std::string &funcname);
  int handle_unimplemented(const char *funcname);

  /// Prototypes for callback actions.
  /// If the underlying MPI function is not implemented in the simulator,
  /// return handle_unimplemented("MPI_Function_name").

  static int
  on_MPI_Send(const dumpi_send *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Recv(const dumpi_recv *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Get_count(const dumpi_get_count *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Bsend(const dumpi_bsend *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Ssend(const dumpi_ssend *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Rsend(const dumpi_rsend *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Buffer_attach(const dumpi_buffer_attach *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Buffer_detach(const dumpi_buffer_detach *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Isend(const dumpi_isend *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Ibsend(const dumpi_ibsend *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Issend(const dumpi_issend *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Irsend(const dumpi_irsend *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Irecv(const dumpi_irecv *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Wait(const dumpi_wait *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Test(const dumpi_test *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Request_free(const dumpi_request_free *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Waitany(const dumpi_waitany *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);

  /// Variant implementation of MPI_Waitany:  Simulator waits for any
  /// request to complete and rewrites request indices to ensure that
  /// future calls to MPI_Waitany make sense (optimistic wait).
  static int
  waitany_optimistic(const dumpi_waitany *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);

  /// Variant implemenation of MPI_Waitany:  Simulator waits for exactly
  /// the request matched by the trace file (pessimistic wait).
  static int
  waitany_pessimistic(const dumpi_waitany *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);

  static int
  on_MPI_Testany(const dumpi_testany *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);

  /// Variant implementation of MPI_Testany:  Simulator matches any
  /// completed request and rewrites request indices to ensure that
  /// future calls to MPI_Testany make sense (optimistic test).
  static int
  testany_optimistic(const dumpi_testany *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);

  /// Variant implementation of MPI_Testany:  Simulator waits for exactly
  /// the request matched by the trace file (pessimistic test).
  static int
  testany_pessimistic(const dumpi_testany *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);

  static int
  on_MPI_Waitall(const dumpi_waitall *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Testall(const dumpi_testall *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Waitsome(const dumpi_waitsome *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);

  /*
  /// Variant implementation of MPI_Waitsome:  Simulator waits for any
  /// requests to complete and rewrites request indices to ensure that
  /// future calls to MPI_Waitsome make sense (optimistic wait).
  static int
  waitsome_optimistic(const dumpi_waitsome *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);

  /// Variant implemenation of MPI_Waitsome:  Simulator waits for exactly
  /// the requests matched by the trace file (pessimistic wait).
  static int
  waitsome_pessimistic(const dumpi_waitsome *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  */

  static int
  on_MPI_Testsome(const dumpi_testsome *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Iprobe(const dumpi_iprobe *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Probe(const dumpi_probe *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cancel(const dumpi_cancel *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Test_cancelled(const dumpi_test_cancelled *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Send_init(const dumpi_send_init *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Bsend_init(const dumpi_bsend_init *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Ssend_init(const dumpi_ssend_init *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Rsend_init(const dumpi_rsend_init *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Recv_init(const dumpi_recv_init *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Start(const dumpi_start *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Startall(const dumpi_startall *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Sendrecv(const dumpi_sendrecv *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Sendrecv_replace(const dumpi_sendrecv_replace *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_contiguous(const dumpi_type_contiguous *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_vector(const dumpi_type_vector *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_hvector(const dumpi_type_hvector *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_indexed(const dumpi_type_indexed *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_hindexed(const dumpi_type_hindexed *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_struct(const dumpi_type_struct *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Address(const dumpi_address *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_extent(const dumpi_type_extent *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_size(const dumpi_type_size *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_lb(const dumpi_type_lb *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_ub(const dumpi_type_ub *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_commit(const dumpi_type_commit *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_free(const dumpi_type_free *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Get_elements(const dumpi_get_elements *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Pack(const dumpi_pack *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Unpack(const dumpi_unpack *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Pack_size(const dumpi_pack_size *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Barrier(const dumpi_barrier *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Bcast(const dumpi_bcast *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Gather(const dumpi_gather *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Gatherv(const dumpi_gatherv *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Scatter(const dumpi_scatter *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Scatterv(const dumpi_scatterv *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Allgather(const dumpi_allgather *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Allgatherv(const dumpi_allgatherv *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Alltoall(const dumpi_alltoall *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Alltoallv(const dumpi_alltoallv *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Reduce(const dumpi_reduce *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Op_create(const dumpi_op_create *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Op_free(const dumpi_op_free *prm, uint16_t thread,
                 const dumpi_time *cpu, const dumpi_time *wall,
                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Allreduce(const dumpi_allreduce *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Reduce_scatter(const dumpi_reduce_scatter *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Scan(const dumpi_scan *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_size(const dumpi_group_size *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_rank(const dumpi_group_rank *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_translate_ranks(const dumpi_group_translate_ranks *prm,
                               uint16_t thread,
                               const dumpi_time *cpu, const dumpi_time *wall,
                               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_compare(const dumpi_group_compare *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_group(const dumpi_comm_group *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_union(const dumpi_group_union *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_intersection(const dumpi_group_intersection *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_difference(const dumpi_group_difference *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_incl(const dumpi_group_incl *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_excl(const dumpi_group_excl *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_range_incl(const dumpi_group_range_incl *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_range_excl(const dumpi_group_range_excl *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Group_free(const dumpi_group_free *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_size(const dumpi_comm_size *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_rank(const dumpi_comm_rank *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_compare(const dumpi_comm_compare *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_dup(const dumpi_comm_dup *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_create(const dumpi_comm_create *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_split(const dumpi_comm_split *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_free(const dumpi_comm_free *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_test_inter(const dumpi_comm_test_inter *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_remote_size(const dumpi_comm_remote_size *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_remote_group(const dumpi_comm_remote_group *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Intercomm_create(const dumpi_intercomm_create *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Intercomm_merge(const dumpi_intercomm_merge *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Keyval_create(const dumpi_keyval_create *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Keyval_free(const dumpi_keyval_free *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Attr_put(const dumpi_attr_put *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Attr_get(const dumpi_attr_get *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Attr_delete(const dumpi_attr_delete *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Topo_test(const dumpi_topo_test *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_create(const dumpi_cart_create *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Dims_create(const dumpi_dims_create *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Graph_create(const dumpi_graph_create *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Graphdims_get(const dumpi_graphdims_get *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Graph_get(const dumpi_graph_get *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cartdim_get(const dumpi_cartdim_get *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_get(const dumpi_cart_get *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_rank(const dumpi_cart_rank *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_coords(const dumpi_cart_coords *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Graph_neighbors_count(const dumpi_graph_neighbors_count *prm,
                               uint16_t thread,
                               const dumpi_time *cpu, const dumpi_time *wall,
                               const dumpi_perfinfo *perf,void *uarg);
  static int
  on_MPI_Graph_neighbors(const dumpi_graph_neighbors *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_shift(const dumpi_cart_shift *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_sub(const dumpi_cart_sub *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Cart_map(const dumpi_cart_map *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Graph_map(const dumpi_graph_map *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Get_processor_name(const dumpi_get_processor_name *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Get_version(const dumpi_get_version *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Errhandler_create(const dumpi_errhandler_create *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Errhandler_set(const dumpi_errhandler_set *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Errhandler_get(const dumpi_errhandler_get *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Errhandler_free(const dumpi_errhandler_free *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Error_string(const dumpi_error_string *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Error_class(const dumpi_error_class *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Wtime(const dumpi_wtime *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Wtick(const dumpi_wtick *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Init(const dumpi_init *prm, uint16_t thread,
              const dumpi_time *cpu, const dumpi_time *wall,
              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Finalize(const dumpi_finalize *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Initialized(const dumpi_initialized *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Abort(const dumpi_abort *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Close_port(const dumpi_close_port *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_accept(const dumpi_comm_accept *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_connect(const dumpi_comm_connect *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_disconnect(const dumpi_comm_disconnect *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_get_parent(const dumpi_comm_get_parent *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_join(const dumpi_comm_join *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_spawn(const dumpi_comm_spawn *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_spawn_multiple(const dumpi_comm_spawn_multiple *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Lookup_name(const dumpi_lookup_name *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Open_port(const dumpi_open_port *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Publish_name(const dumpi_publish_name *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Unpublish_name(const dumpi_unpublish_name *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Accumulate(const dumpi_accumulate *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Get(const dumpi_get *prm, uint16_t thread,
             const dumpi_time *cpu, const dumpi_time *wall,
             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Put(const dumpi_put *prm, uint16_t thread,
             const dumpi_time *cpu, const dumpi_time *wall,
             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_complete(const dumpi_win_complete *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_create(const dumpi_win_create *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_fence(const dumpi_win_fence *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_free(const dumpi_win_free *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_get_group(const dumpi_win_get_group *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_lock(const dumpi_win_lock *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_post(const dumpi_win_post *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_start(const dumpi_win_start *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_test(const dumpi_win_test *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_unlock(const dumpi_win_unlock *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_wait(const dumpi_win_wait *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Alltoallw(const dumpi_alltoallw *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Exscan(const dumpi_exscan *prm, uint16_t thread,
                const dumpi_time *cpu, const dumpi_time *wall,
                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Add_error_class(const dumpi_add_error_class *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Add_error_code(const dumpi_add_error_code *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Add_error_string(const dumpi_add_error_string *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_call_errhandler(const dumpi_comm_call_errhandler *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_create_keyval(const dumpi_comm_create_keyval *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_delete_attr(const dumpi_comm_delete_attr *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_free_keyval(const dumpi_comm_free_keyval *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_get_attr(const dumpi_comm_get_attr *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_get_name(const dumpi_comm_get_name *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_set_attr(const dumpi_comm_set_attr *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_set_name(const dumpi_comm_set_name *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_call_errhandler(const dumpi_file_call_errhandler *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Grequest_complete(const dumpi_grequest_complete *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Grequest_start(const dumpi_grequest_start *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Init_thread(const dumpi_init_thread *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Is_thread_main(const dumpi_is_thread_main *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Query_thread(const dumpi_query_thread *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Status_set_cancelled(const dumpi_status_set_cancelled *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Status_set_elements(const dumpi_status_set_elements *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_keyval(const dumpi_type_create_keyval *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_delete_attr(const dumpi_type_delete_attr *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_dup(const dumpi_type_dup *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_free_keyval(const dumpi_type_free_keyval *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_get_attr(const dumpi_type_get_attr *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_get_contents(const dumpi_type_get_contents *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_get_envelope(const dumpi_type_get_envelope *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_get_name(const dumpi_type_get_name *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_set_attr(const dumpi_type_set_attr *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_set_name(const dumpi_type_set_name *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_match_size(const dumpi_type_match_size *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_call_errhandler(const dumpi_win_call_errhandler *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_create_keyval(const dumpi_win_create_keyval *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_delete_attr(const dumpi_win_delete_attr *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_free_keyval(const dumpi_win_free_keyval *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_get_attr(const dumpi_win_get_attr *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_get_name(const dumpi_win_get_name *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_set_attr(const dumpi_win_set_attr *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_set_name(const dumpi_win_set_name *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Alloc_mem(const dumpi_alloc_mem *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_create_errhandler(const dumpi_comm_create_errhandler *prm,
                                uint16_t thread,
                                const dumpi_time *cpu, const dumpi_time *wall,
                                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_get_errhandler(const dumpi_comm_get_errhandler *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Comm_set_errhandler(const dumpi_comm_set_errhandler *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_create_errhandler(const dumpi_file_create_errhandler *prm,
                                uint16_t thread,
                                const dumpi_time *cpu, const dumpi_time *wall,
                                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_errhandler(const dumpi_file_get_errhandler *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_set_errhandler(const dumpi_file_set_errhandler *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Finalized(const dumpi_finalized *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Free_mem(const dumpi_free_mem *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Get_address(const dumpi_get_address *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_create(const dumpi_info_create *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_delete(const dumpi_info_delete *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_dup(const dumpi_info_dup *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_free(const dumpi_info_free *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_get(const dumpi_info_get *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_get_nkeys(const dumpi_info_get_nkeys *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_get_nthkey(const dumpi_info_get_nthkey *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_get_valuelen(const dumpi_info_get_valuelen *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Info_set(const dumpi_info_set *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Pack_external(const dumpi_pack_external *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Pack_external_size(const dumpi_pack_external_size *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Request_get_status(const dumpi_request_get_status *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_darray(const dumpi_type_create_darray *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_hindexed(const dumpi_type_create_hindexed *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_hvector(const dumpi_type_create_hvector *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_indexed_block(const dumpi_type_create_indexed_block *prm,
                                   uint16_t thread,
                                   const dumpi_time *cpu, const dumpi_time *wall,
                                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_resized(const dumpi_type_create_resized *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_struct(const dumpi_type_create_struct *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_create_subarray(const dumpi_type_create_subarray *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_get_extent(const dumpi_type_get_extent *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Type_get_true_extent(const dumpi_type_get_true_extent *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Unpack_external(const dumpi_unpack_external *prm, uint16_t thread,
                         const dumpi_time *cpu, const dumpi_time *wall,
                         const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_create_errhandler(const dumpi_win_create_errhandler *prm,
                               uint16_t thread,
                               const dumpi_time *cpu, const dumpi_time *wall,
                               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_get_errhandler(const dumpi_win_get_errhandler *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Win_set_errhandler(const dumpi_win_set_errhandler *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_open(const dumpi_file_open *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_close(const dumpi_file_close *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_delete(const dumpi_file_delete *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_set_size(const dumpi_file_set_size *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_preallocate(const dumpi_file_preallocate *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_size(const dumpi_file_get_size *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_group(const dumpi_file_get_group *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_amode(const dumpi_file_get_amode *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_set_info(const dumpi_file_set_info *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_info(const dumpi_file_get_info *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_set_view(const dumpi_file_set_view *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_view(const dumpi_file_get_view *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_at(const dumpi_file_read_at *prm, uint16_t thread,
                      const dumpi_time *cpu, const dumpi_time *wall,
                      const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_at_all(const dumpi_file_read_at_all *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_at(const dumpi_file_write_at *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_at_all(const dumpi_file_write_at_all *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_iread_at(const dumpi_file_iread_at *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_iwrite_at(const dumpi_file_iwrite_at *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read(const dumpi_file_read *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_all(const dumpi_file_read_all *prm, uint16_t thread,
                       const dumpi_time *cpu, const dumpi_time *wall,
                       const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write(const dumpi_file_write *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_all(const dumpi_file_write_all *prm, uint16_t thread,
                        const dumpi_time *cpu, const dumpi_time *wall,
                        const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_iread(const dumpi_file_iread *prm, uint16_t thread,
                    const dumpi_time *cpu, const dumpi_time *wall,
                    const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_iwrite(const dumpi_file_iwrite *prm, uint16_t thread,
                     const dumpi_time *cpu, const dumpi_time *wall,
                     const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_seek(const dumpi_file_seek *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_position(const dumpi_file_get_position *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_byte_offset(const dumpi_file_get_byte_offset *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_shared(const dumpi_file_read_shared *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_shared(const dumpi_file_write_shared *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_iread_shared(const dumpi_file_iread_shared *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_iwrite_shared(const dumpi_file_iwrite_shared *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_ordered(const dumpi_file_read_ordered *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_ordered(const dumpi_file_write_ordered *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_seek_shared(const dumpi_file_seek_shared *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_position_shared(const dumpi_file_get_position_shared *prm,
                                  uint16_t thread,
                                  const dumpi_time *cpu, const dumpi_time *wall,
                                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_at_all_begin(const dumpi_file_read_at_all_begin *prm,
                                uint16_t thread,
                                const dumpi_time *cpu, const dumpi_time *wall,
                                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_at_all_end(const dumpi_file_read_at_all_end *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_at_all_begin(const dumpi_file_write_at_all_begin *prm,
                                 uint16_t thread,
                                 const dumpi_time *cpu, const dumpi_time *wall,
                                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_at_all_end(const dumpi_file_write_at_all_end *prm,
                               uint16_t thread,
                               const dumpi_time *cpu, const dumpi_time *wall,
                               const dumpi_perfinfo *perf,void *uarg);
  static int
  on_MPI_File_read_all_begin(const dumpi_file_read_all_begin *prm,
                             uint16_t thread,
                             const dumpi_time *cpu, const dumpi_time *wall,
                             const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_all_end(const dumpi_file_read_all_end *prm, uint16_t thread,
                           const dumpi_time *cpu, const dumpi_time *wall,
                           const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_all_begin(const dumpi_file_write_all_begin *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_all_end(const dumpi_file_write_all_end *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_ordered_begin(const dumpi_file_read_ordered_begin *prm,
                                 uint16_t thread,
                                 const dumpi_time *cpu, const dumpi_time *wall,
                                 const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_read_ordered_end(const dumpi_file_read_ordered_end *prm,
                               uint16_t thread,
                               const dumpi_time *cpu, const dumpi_time *wall,
                               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_ordered_begin(const dumpi_file_write_ordered_begin *prm,
                                  uint16_t thread,
                                  const dumpi_time *cpu, const dumpi_time *wall,
                                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_write_ordered_end(const dumpi_file_write_ordered_end *prm,
                                uint16_t thread,
                                const dumpi_time *cpu, const dumpi_time *wall,
                                const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_type_extent(const dumpi_file_get_type_extent *prm,
                              uint16_t thread,
                              const dumpi_time *cpu, const dumpi_time *wall,
                              const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_Register_datarep(const dumpi_register_datarep *prm, uint16_t thread,
                          const dumpi_time *cpu, const dumpi_time *wall,
                          const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_set_atomicity(const dumpi_file_set_atomicity *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_get_atomicity(const dumpi_file_get_atomicity *prm, uint16_t thread,
                            const dumpi_time *cpu, const dumpi_time *wall,
                            const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPI_File_sync(const dumpi_file_sync *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Test(const dumpio_test *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Wait(const dumpio_wait *prm, uint16_t thread,
               const dumpi_time *cpu, const dumpi_time *wall,
               const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Testall(const dumpio_testall *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Waitall(const dumpio_waitall *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Testany(const dumpio_testany *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Waitany(const dumpio_waitany *prm, uint16_t thread,
                  const dumpi_time *cpu, const dumpi_time *wall,
                  const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Waitsome(const dumpio_waitsome *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
  static int
  on_MPIO_Testsome(const dumpio_testsome *prm, uint16_t thread,
                   const dumpi_time *cpu, const dumpi_time *wall,
                   const dumpi_perfinfo *perf, void *uarg);
};

}
} // End of namespace sstmac.

#endif /* SSTMAC_SOFTWARE_SKELETONS_UNDUMPI_PARSEDUMPI_CALLBACKS_H_INCLUDED */


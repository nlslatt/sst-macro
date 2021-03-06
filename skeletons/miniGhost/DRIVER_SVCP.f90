MODULE DRIVER_SVCP_MOD

! ************************************************************************
!
!               miniGhost: stencil computations with boundary exchange.
!                 Copyright (2011) Sandia Corporation
!
! Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
! license for use of this work by or on behalf of the U.S. Government.
!
! This library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Lesser General Public License as
! published by the Free Software Foundation; either version 2.1 of the
! License, or (at your option) any later version.
!
! This library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
! Lesser General Public License for more details.
!
! You should have received a copy of the GNU Lesser General Public
! License along with this library; if not, write to the Free Software
! Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
! USA
! Questions? Contact Richard F. Barrett (rfbarre@sandia.gov) or
!                    Michael A. Heroux (maherou@sandia.gov)
!
! ************************************************************************

CONTAINS

   SUBROUTINE DRIVER_SVCP ( IERR, mype )
   
   USE MG_CONSTANTS_MOD
   USE MG_STENCIL_MOD
   USE MG_SVCP_INIT_MOD
   USE MG_SVCP_MOD
   USE MG_PROFILING_MOD
   USE MG_SUM_GRID_MOD

   USE MG_BSPMA_MOD         ! For initializing halos.
   USE MG_BSPMA_DIAGS_MOD   ! For initializing halos.

   IMPLICIT NONE

      INTEGER, INTENT(OUT) :: &
         IERR                        ! Return status

         
         integer :: mype

      ! ------------------
      ! Local Declarations
      ! ------------------
   
      INTEGER :: &
         I,                       &  ! Counter
         IVAR,  &                        ! Counter (over variables)
         tsteptemp
 
      REAL(KIND=MG_REAL) ::       &
         ERROR_ITER,              &  ! Difference between new and old GRIDi sum.
         GSUM                        ! Global SUM across GRIDs.
   
      REAL(KIND=MG_REAL8) ::       &
         TIME_START,              &
         TIME_START_2,            &
         TIME_START_ALL

      ! ---------------------
      ! Executable Statements
      ! ---------------------
   
      IERR = 0

      ! This method presumes coherent data upon initialization,
      ! thus we exchange the boundaries here first. This is not timed.

      CALL MG_SVCP_INIT ( IERR, mype )
      CALL CHECK_ERROR ( IERR, 'DRIVER_SVCP:MG_SVCP_INIT', linfo(mype+1)%TSTEP, mype )

      ! -------------------
      ! Begin time stepping
      ! -------------------

      TIME_START_ALL = MPI_WTIME ( )
      DO tsteptemp = 1, NUM_TSTEPS
   		linfo(mype+1)%TSTEP = tsteptemp
         DO IVAR = 1, NUM_VARS
            TIME_START = MPI_WTIME ( )

            ! Self-contained: all functionality, i.e. computation and communication,
            ! contained in this function.
            CALL MG_SVCP ( IVAR, IERR, mype )
            CALL CHECK_ERROR ( IERR, 'DRIVER_SVCP:MG_SVCP', IVAR, mype )
            MG_PERF(mype+1)%TIME_SVCP_PE = MG_PERF(mype+1)%TIME_SVCP_PE + MPI_WTIME ( ) - TIME_START
  
            ! Reduction across GRID option
            IF ( linfo(mype+1)%GRIDS_TO_SUM(IVAR) ) THEN
               TIME_START_2 = MPI_WTIME()
               CALL MG_SUM_GRID ( IVAR, GSUM, IERR, mype )
               CALL CHECK_ERROR ( IERR, 'DRIVER_SVCP:MG_SUM_GRID', IVAR, mype )
               MG_PERF(mype+1)%TIME_SUMGRID_PE = MG_PERF(mype+1)%TIME_SUMGRID_PE + MPI_WTIME() - TIME_START_2

               IF ( ( linfo(mype+1)%TSTEP > NX / 2 ) .OR. ( linfo(mype+1)%TSTEP > NY / 2 ) .OR. &
               ( linfo(mype+1)%TSTEP > NZ / 2 ) ) &
                  CHECK_DIFFUSION = .FALSE.
               IF ( linfo(mype+1)%MYPE == ROOT_PE ) THEN
                  IF ( CHECK_DIFFUSION ) THEN   ! Correctness checking.
                     ERROR_ITER = ABS ( GSUM - linfo(mype+1)%GSUM_OLD(IVAR) ) / linfo(mype+1)%GSUM_OLD(IVAR)
                     IF ( ERROR_ITER > ERROR_TOLERANCE ) THEN
                        WRITE(*,99) IVAR, linfo(mype+1)%TSTEP, GSUM, linfo(mype+1)%GSUM_OLD(IVAR), ERROR_TOLERANCE
                        IERR = -1
                        CALL CHECK_ERROR ( IERR, 'DRIVER_SVCP', IVAR, mype )
                     END IF
                     linfo(mype+1)%GSUM_OLD(IVAR) = GSUM
                  END IF
               END IF
            END IF

         END DO
      END DO
      MG_PERF(mype+1)%TIME_WALL_PE = MPI_WTIME ( ) - TIME_START_ALL
   
 99   FORMAT ( ' ** Error ** : GRID', I3, ', ', I5, ' time step: GSUM(NEW/OLD) = ', 1PE12.6, ', ', 1PE12.6, &
               ' off error tolerance of ', 1PE8.2, '.' )

   END SUBROUTINE DRIVER_SVCP

END MODULE DRIVER_SVCP_MOD

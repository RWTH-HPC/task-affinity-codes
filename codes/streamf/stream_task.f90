program bsp
  use omp_lib
  implicit none

#define TASK_VERSION 1

  integer(kind=8) :: ntimes, i, ntasks_overall, step, k, j
  integer(kind=8) :: tmp_idx_start, tmp_idx_end
  integer(kind=8) :: nsize
  double precision, dimension(:), allocatable :: a,b,c
  double precision :: t, scalar

#if defined ( STREAM_ARRAY_SIZE )  
   nsize = STREAM_ARRAY_SIZE
#else
   nsize = 2**29   
#endif
  
  !allocate(a(1:nsize), b(1:nsize), c(1:nsize))
  allocate(a(nsize), b(nsize), c(nsize))

  !$omp parallel do
  do i = 1,nsize
    a(i) = 1.0
    b(i) = 2.0
    c(i) = 0.0
  end do
  !$omp end parallel do

  ntimes = 10
#if defined (T_AFF_NUM_TASK_MULTIPLICATOR)
  ntasks_overall = T_AFF_NUM_TASK_MULTIPLICATOR*omp_get_max_threads()
#else
  ntasks_overall = 16*omp_get_max_threads()	
#endif
  step = nsize / ntasks_overall;
  if (mod(nsize, step) > 0) then
    step = step + 1
  end if
  scalar = 3.0

  PRINT *, 'NSIZE= ', nsize, ' Step = ', step, ' ntasks_overall = ', ntasks_overall

  t = omp_get_wtime()
  do k = 1,ntimes
    !PRINT *, 'Running Copy ...'

    !$omp parallel
#if TASK_VERSION
    !$omp taskgroup
    !$omp master
      do j = 1, ntasks_overall
        tmp_idx_start = (j-1)*step + 1;
        tmp_idx_end = j*step;
        if(tmp_idx_end > nsize) then
          tmp_idx_end = nsize
        end if
#if defined ( TASK_AFFINITY )
        PRINT *, 'Location of data is ', loc(c(tmp_idx_start))
        call kmpc_set_task_affinity(loc(c(tmp_idx_start)))
#endif
        !$omp task firstprivate(tmp_idx_start, tmp_idx_end) private(i) shared(a,b,c,scalar)
        do i = tmp_idx_start,tmp_idx_end
          c(i) = a(i)
        end do
        !$omp end task
      end do
    !$omp end master
    !$omp end taskgroup
#else
    !$omp do private(i)
    do i = 1, nsize
      c(i) = a(i)
    end do
    !$omp end do
#endif
    !$omp end parallel

    !PRINT *, 'Running Scale ...'

    !$omp parallel
#if TASK_VERSION
    !$omp taskgroup
    !$omp master
      do j = 1, ntasks_overall
        tmp_idx_start = (j-1)*step + 1;
        tmp_idx_end = j*step;
        if(tmp_idx_end > nsize) then
          tmp_idx_end = nsize
        end if
#if defined ( TASK_AFFINITY )
        call kmpc_set_task_affinity(loc(b(tmp_idx_start)))
#endif
        !$omp task firstprivate(tmp_idx_start, tmp_idx_end) private(i) shared(a,b,c,scalar)
        do i = tmp_idx_start,tmp_idx_end
          b(i) = scalar * c(i)
        end do
        !$omp end task  
      end do
    !$omp end master
    !$omp end taskgroup
#else
    !$omp do private(i)
    do i = 1, nsize
      b(i) = scalar * c(i)
    end do
    !$omp end do
#endif
    !$omp end parallel

    !PRINT *, 'Running Add ...'

    !$omp parallel
#if TASK_VERSION
    !$omp taskgroup
    !$omp master
      do j = 1, ntasks_overall
        tmp_idx_start = (j-1)*step + 1;
        tmp_idx_end = j*step;
        if(tmp_idx_end > nsize) then
          tmp_idx_end = nsize
        end if
#if defined ( TASK_AFFINITY )
        call kmpc_set_task_affinity(loc(c(tmp_idx_start)))
#endif
        !$omp task firstprivate(tmp_idx_start, tmp_idx_end) private(i) shared(a,b,c,scalar)
        do i = tmp_idx_start,tmp_idx_end
          c(i) = a(i) + b(i)
        end do
        !$omp end task  
      end do
    !$omp end master
    !$omp end taskgroup
#else
    !$omp do private(i)
    do i = 1, nsize
      c(i) = a(i) + b(i)
    end do
    !$omp end do
#endif
    !$omp end parallel

    !PRINT *, 'Running Triad ...'

    !$omp parallel
#if TASK_VERSION
    !$omp taskgroup
    !$omp master
      do j = 1, ntasks_overall
        tmp_idx_start = (j-1)*step + 1;
        tmp_idx_end = j*step;
        if(tmp_idx_end > nsize) then
          tmp_idx_end = nsize
        end if
#if defined ( TASK_AFFINITY )
        call kmpc_set_task_affinity(loc(a(tmp_idx_start)))
#endif
        !$omp task firstprivate(tmp_idx_start, tmp_idx_end) private(i) shared(a,b,c,scalar)
        do i = tmp_idx_start,tmp_idx_end
          a(i) = b(i) + scalar * c(i)
        end do
        !$omp end task  
      end do
    !$omp end master
    !$omp end taskgroup
#else
    !$omp do private(i)
    do i = 1, nsize
      a(i) = b(i) + scalar * c(i)
    end do
    !$omp end do
#endif
    !$omp end parallel

  end do
  t = omp_get_wtime() - t
  PRINT *, 'Elapsed time for program = ', t
end program bsp

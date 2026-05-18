# Requirements Checklist

## 1. Scheduler (MLQ)
- [x] MLQ scheduler is implemented in the project.
- [x] CPU dispatch uses priority-based ready queues.
- [x] Round-robin / time-slice behavior is present in the scheduler flow.
- [x] The project builds and runs with MLQ-enabled configurations.

## 2. Memory Management with separable user/kernel space
- [x] User-facing memory/syscall wrappers use kernel context plus PID.
- [x] PCB is resolved internally in kernel mode from PID.
- [x] PCB is not passed directly through the user-facing path.
- [x] `running_list` is implemented as a list, not a queue.
- [x] `purgequeue` is removed from the scheduler path.

## 3. Multi-level paging (64-bit)
- [x] 64-bit multi-level paging is implemented.
- [x] Page-table levels PGD / P4D / PUD / PMD / PT are present.
- [x] Address decomposition helpers exist for 64-bit paging.
- [x] Paging-related runtime paths execute successfully.

## 4. Optional page replacement
- [x] FIFO victim tracking exists via `fifo_pgn`.
- [x] Victim selection logic exists.
- [x] Swap-out / swap-in code paths exist.
- [ ] Replacement is fully demonstrated on the provided stock workloads.
- [x] Paging statistics are collected and printed.
- [x] Memory access count is reported.
- [x] Page-table access count is reported.
- [x] Page-fault count is reported.
- [x] Page replacement count is reported.
- [x] Swap-in / swap-out counts are reported.
- [x] Page-table storage size is reported.

## 5. `vmap_pgd_memset` dummy allocation
- [x] `vmap_pgd_memset` uses `memset`-based zeroing.
- [x] The 64-bit paging path avoids real page-directory allocation for the dummy setup.
- [x] Syscall-driven test paths can reach the dummy allocation logic.

## 6. Validation status
- [x] `make all` succeeds.
- [x] `./os os_syscall` runs successfully.
- [x] `./os os_1_mlq_paging_small_1K` runs successfully.
- [x] `./os os_2_mlq_paging` runs successfully.
- [ ] A stock workload was not found that reliably forces nonzero replacement counters.

## 7. Notes
- The checklist above reflects the current codebase state after validation.
- The only clearly incomplete item is the lack of a stock workload that reliably demonstrates replacement counters > 0.

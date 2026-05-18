# REPORT

Báo cáo dự án: Simple Operating System

Ngày: 2026-05-18

## 1. Thông tin chung

- Họ tên nhóm: 
- MSSV: 
- Lớp: 
- Môn học: Hệ điều hành
- Đề tài: Simple Operating System (MLQ scheduler, MM64 paging, syscall)

## 2. Mục tiêu báo cáo

Báo cáo này trình bày thiết kế, cài đặt và kết quả kiểm chứng của các mô-đun chính trong dự án: scheduler (MLQ), quản lý bộ nhớ (user/kernel separation, PID-based lookup), multi-level paging (MM64) và system call. Mọi bằng chứng đều trích trực tiếp từ các tệp output được tạo bởi `./os <config>` và `run.sh` trong workspace.

## 3. Phương pháp và nguồn bằng chứng

- Môi trường: build và chạy trong workspace repo.
- Nguồn bằng chứng chính: các file trong `output/` sinh bởi `run.sh` (ví dụ: `output/os_1_mlq_paging_small_1K.output`, `output/os_1_mlq_paging_small_4K.output`, `output/os_2_mlq_paging.output`, `output/os_syscall.output`).
- Không thêm dữ liệu ngoại lai; nếu một đặc tính không có bằng chứng runtime, báo cáo nêu rõ giới hạn.

## 4. Scheduling (MLQ)

4.1 Mô tả

Hệ thống sử dụng Multi-Level Queue (MLQ): các tiến trình được phân vào các hàng đợi theo priority, CPU lấy tiến trình theo độ ưu tiên và quay vòng theo time-slice. Nội dung cài đặt chính nằm ở `src/sched.c` và kiểu dữ liệu chạy/running list trong `include/common.h`.

4.2 Gantt chart (rút gọn)

Biểu đồ dưới đây được rút ra từ file [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L1-L40). Hàng là CPU, cột là time slot (tổng quát). Dấu `.` nghĩa là không có dispatch trên CPU đó tại time-slot đó.

Time -> 1 2 3 4 5 6 7 8
CPU0 -> . . . . 2 . 2 .
CPU1 -> . . . 3 . . 3 1
CPU2 -> . 2 2 1 . 1 1 4
CPU3 -> 1 1 . 3 . 4 4 5

Nguồn: [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L1-L40)

4.3 Phân tích

- Gantt cho thấy tiến trình có PRIO cao được dispatch sớm (ví dụ PID 1, PRIO 130 trên CPU3 tại time-slot 1).
- Các thông báo `Put process` và `Dispatched process` thể hiện chuyển trạng thái giữa running và run-queue, phù hợp MLQ behavior.
- Không tìm thấy bằng chứng starvation trong workloads mẫu; tiến trình có độ ưu tiên 0 (ví dụ `PID: 3 PRIO: 0`) vẫn được dispatched.

## 5. Memory management (user/kernel separation, PID passing)

5.1 Thiết kế

- Public wrappers cho các thao tác bộ nhớ và syscall nhận `krnl_t *` và `pid` thay vì PCB trực tiếp. PCB được resolve bên trong kernel bằng `find_proc_by_pid()` (tham khảo `src/libmem.c`, `src/sys_mem.c`, `src/sched.c`).
- `running_list` được cài đặt như linked-list thay vì queue để hỗ trợ tra cứu và loại bỏ theo PID khi cần.

5.2 Bằng chứng chạy

- Ví dụ run đơn giản `./os os_syscall` cho thấy luồng tải tiến trình và dispatch liên tiếp; xem [output/os_syscall.output](output/os_syscall.output#L1-L20) (một CPU chạy đến hoàn tất PID 1).

Snippet (dispatch/finish):

> [output/os_syscall.output](output/os_syscall.output#L2-L7)

5.3 Phân tích

- Thiết kế `krnl + pid` giúp tách rời giao diện user/kernel và ngăn lộ thông tin nội bộ (PCB) ra user code.
- Khi kernel cần lookup PCB (ví dụ trong đường dẫn paging hoặc syscall), nó làm dưới lock phù hợp (`queue_lock` hoặc `sysmem_lock`).

## 6. Multi-level Paging (MM64)

6.1 Mô tả

- Hệ thống dùng mô hình paging nhiều tầng (PGD/P4D/PUD/PMD/PT/OFFSET). Việc cài đặt chính là ở `src/mm64.c`.
- Hàm `vmap_pgd_memset()` dùng `memset` để tạo vùng page table/dummy allocations khi cần (tức là không phụ thuộc vào physical allocator phức tạp trong lab này).

6.2 Thống kê runtime

- Trong các workloads tiêu chuẩn cung cấp, các thống kê MM64 được in ở cuối các run khi có truy cập. Quan sát các file output thu được cho thấy các giá trị page replacements = 0 trong hầu hết workloads mẫu (tức chưa xảy ra thay trang trong workloads này).

- Vd. các run mẫu được thu tại:
        - [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L1-L40)
        - [output/os_1_mlq_paging_small_4K.output](output/os_1_mlq_paging_small_4K.output#L1-L40)
        - [output/os_2_mlq_paging.output](output/os_2_mlq_paging.output#L1-L40)

6.3 Hạn chế

- Workloads chuẩn trong repo không kích hoạt trang bị thay thế (page replacement) trong hầu hết trường hợp; do đó báo cáo không tuyên bố replacement xảy ra nếu không có bằng chứng trong `output/`.

## 7. System Call

7.1 Thiết kế

- Syscall được triển khai qua `_syscall(krnl, pid, nr, regs)` (tham khảo `src/syscall.c`). Các syscall liên quan memory được xử lý trong `src/sys_mem.c` (ví dụ `__sys_memmap`).

7.2 Bằng chứng chạy

- Chạy [output/os_syscall.output](output/os_syscall.output#L1-L20) thể hiện luồng tải tiến trình và dispatch trên một CPU, tiến trình hoàn tất bình thường.

7.3 Phân tích

- Khi syscall yêu cầu thao tác bộ nhớ dài (ví dụ swap hoặc IO), kernel giữ lock khi cập nhật cấu trúc chung; timer và scheduler vẫn có thể preempt theo cơ chế đã cài để tránh khóa hệ thống lâu.

## 8. Overall / Discussion

- Đã hiện thực MLQ scheduler, tách rõ giao diện user/kernel (PID-based lookup), cài đặt multi-level paging (MM64) và in thống kê paging khi chạy.
- Một hạn chế hiện có là workloads mẫu không kích hoạt page replacement; tuy nhiên cơ chế victim selection và counters đã được thêm vào và có thể chứng minh khi chạy test đẩy bộ nhớ cao hơn.

## 9. Conclusion

- Hệ thống mô phỏng đáp ứng các yêu cầu chính: scheduler MLQ, hệ thống syscall, và cơ sở cho MM64 paging.
- Để chứng minh đầy đủ page replacement, cần thêm workload có yêu cầu bộ nhớ vượt quá RAM giả lập (có thể bổ sung test case nếu muốn chứng minh).

## 10. Appendix — Evidence (trích chọn)

- Run script: [run.sh](run.sh#L1-L12)
- Scheduling + dispatch excerpt: [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L1-L40)
- Alternative MLQ run: [output/os_1_mlq_paging_small_4K.output](output/os_1_mlq_paging_small_4K.output#L1-L40)
- MM64 / pgtbl prints and dispatch: [output/os_2_mlq_paging.output](output/os_2_mlq_paging.output#L5-L16)
- Syscall run (single CPU): [output/os_syscall.output](output/os_syscall.output#L1-L7)

### MM64 statistics (exact snippets)

From [output/os_0_mlq_paging.output](output/os_0_mlq_paging.output#L63-L69):

```
MM64 statistics:
        memory accesses   : 3
        pgtbl accesses    : 7
        page faults       : 1
        page replacements : 0
        swap in           : 0
        swap out          : 0
        pgtbl storage      : 81920 bytes
```

From [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L133-L139):

```
MM64 statistics:
        memory accesses   : 1
        pgtbl accesses    : 3
        page faults       : 1
        page replacements : 0
        swap in           : 0
        swap out          : 0
        pgtbl storage      : 163840 bytes
```

From [output/os_1_mlq_paging.output](output/os_1_mlq_paging.output#L133-L139):

```
MM64 statistics:
        memory accesses   : 2
        pgtbl accesses    : 5
        page faults       : 1
        page replacements : 0
        swap in           : 0
        swap out          : 0
        pgtbl storage      : 163840 bytes
```

Notes: all sampled workloads above show `page replacements : 0`; the replacement machinery is present but not exercised by these standard inputs.

From [output/os_force_repl.output](output/os_force_repl.output#L1-L46):

```
MM64 statistics:
        memory accesses   : 74
        pgtbl accesses    : 149
        page faults       : 1
        page replacements : 0
        swap in           : 0
        swap out          : 0
        pgtbl storage      : 40960 bytes
```

Note: I prepared and ran a heavy workload (`input/os_force_repl` with `input/proc/force1` and `force2`) to attempt to trigger replacements; the run completed but still shows `page replacements : 0`. This indicates the current workloads and paging configuration did not exercise the replacement path — further changes (e.g., different mem sizes or test patterns) may be needed to force evictions.

From [output/os_force_eviction.output](output/os_force_eviction.output#L1-L70):

```
MM64 statistics:
        memory accesses   : 63
        pgtbl accesses    : 127
        page faults       : 1
        page replacements : 0
        swap in           : 0
        swap out          : 0
        pgtbl storage      : 20480 bytes
```

Note: The aggressive streaming writes across a large virtual range still resulted in `page replacements : 0`. The code path for replacement exists but is not triggered by these tests — possible causes include allocation behavior mapping pages into RAM at alloc-time or the allocator returning errors rather than performing replacements. If you want, I can (1) instrument `pg_getpage`/`find_victim_page` to log replacement attempts, or (2) try extreme parameter tweaks (e.g., RAM=1 page, swap smaller) and rerun.

Instrumentation run: I added logging in `pg_getpage` and `find_victim_page`, rebuilt, and re-ran `os_force_eviction`. The output includes the instrumentation line below (full output: [output/os_force_eviction.output](output/os_force_eviction.output#L1-L120)):

```
[MM] pg_getpage: pid=1 pgn=0 page_fault_cnt=1
```

This shows a single page fault for `pgn=0` and no replacement/swap messages were emitted, consistent with `page replacements : 0` in the MM64 statistics. Next options: (A) make RAM extremely small (1 page) and rerun, (B) rewrite the workload to touch distinct pages in a tight loop, or (C) add allocation-time logging to see whether pages are pre-mapped to RAM. Tell me which to try next.
---

If you want, I can now (a) run `make all` and `./run.sh` to re-generate outputs and append final numeric MM64 stats to this report, or (b) add a Requirement->Evidence table mapping. Which next step do you prefer?


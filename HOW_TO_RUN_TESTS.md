# How to run tests (Linux / WSL)

Mục tiêu: Hướng dẫn từng bước để build, chạy test thủ công, so sánh `expected` vs `actual`, và thu bằng chứng.

## Prerequisites
- WSL (Ubuntu) hoặc Linux
- `gcc`, `make`, `bash`

Cài (WSL):
```bash
sudo apt update
sudo apt install -y build-essential
```
Kiểm tra:
```bash
gcc --version
make --version
```

## Build
Mở WSL, chuyển vào thư mục dự án và build:
```bash
cd /mnt/c/Users/stephen-work/source/repos/HCMUT/btl-os/ossim_caitoa
make all 2>&1 | tee build.log
ls -l os
```
Bằng chứng: `build.log`, file `os` tồn tại.

## Chạy single test (một Phase)
Cú pháp:
```bash
./os <config-name>
```
Ví dụ (Phase A — MLQ + paging):
```bash
./os os_1_mlq_paging > run_os_1_mlq_paging.out 2>&1
```
So sánh với expected:
```bash
diff -u output/os_1_mlq_paging.output run_os_1_mlq_paging.out | tee diff_os_1_mlq_paging.txt
```
Lấy thống kê paging (nếu có):
```bash
grep -A8 "MM64 statistics" run_os_1_mlq_paging.out > mm_stats_os_1_mlq_paging.txt || true
```
Bằng chứng lưu: `run_os_1_mlq_paging.out`, `diff_os_1_mlq_paging.txt`, `mm_stats_os_1_mlq_paging.txt`.

## Chạy tất cả test (tự động)
Script đã có: `scripts/run_all_tests.sh` (đã tạo sẵn). Chạy trong WSL:
```bash
cd /mnt/c/Users/stephen-work/source/repos/HCMUT/btl-os/ossim_caitoa
bash scripts/run_all_tests.sh
```
Output/ bằng chứng sẽ được lưu trong `evidence/`:
- `evidence/run_<config>.out` — stdout của lần chạy
- `evidence/diff_<config>.txt` — diff vs `output/<config>.output`
- `evidence/mm_stats_<config>.txt` — MM stats extract
- `evidence/summary.txt` — danh sách test OK/DIFF

## Sử dụng run.sh (regen outputs và verify)
Nếu bạn muốn sử dụng script `run.sh` có sẵn để chạy tập lệnh mẫu và so sánh kết quả, làm theo bước sau — lưu ý: `run.sh` mặc định sẽ ghi/di chuyển các file `*.output` vào thư mục `output/`, vì vậy hãy sao lưu `output/` trước khi chạy nếu bạn muốn so sánh với expected hiện tại.

Chạy (WSL):
```bash
cd /mnt/c/Users/stephen-work/source/repos/HCMUT/btl-os/ossim_caitoa
# sao lưu expected hiện tại
cp -r output output.expected.bak

# chạy run.sh (sẽ tạo/ghi file output/*.output)
bash run.sh 2>&1 | tee run_sh.log

# so sánh mọi file mới với expected đã sao lưu, lưu diff vào evidence/runsh_diffs/
mkdir -p evidence/runsh_diffs
for f in output/*.output; do
  base=$(basename "$f")
  diff -u "output.expected.bak/${base}" "output/${base}" > "evidence/runsh_diffs/diff_${base}.txt" || true
  if [ -s "evidence/runsh_diffs/diff_${base}.txt" ]; then
    echo "DIFF: ${base}"
  else
    echo "OK: ${base}"
  fi
done | tee evidence/runsh_diffs/summary.txt

# (tùy chọn) khôi phục expected gốc nếu muốn
# rm -rf output && mv output.expected.bak output
```

Kết quả:
- `run_sh.log` — log chạy `run.sh`
- `evidence/runsh_diffs/diff_*.txt` — diff từng file
- `evidence/runsh_diffs/summary.txt` — tóm tắt OK/DIFF

Ghi chú: nếu bạn muốn `run.sh` chỉ xuất kết quả vào một thư mục khác (không ghi đè `output/`), tôi có thể tạo một bản `run_local.sh` nhỏ để redirect kết quả mà không cần thay đổi `run.sh` gốc.

## Các Phase (kịch bản, lệnh, evidence)
- Phase A — Scheduler (MLQ)
  - Kịch bản: kiểm tra dispatch/put/add, priority behavior
  - Lệnh run:
    ```bash
    ./os os_1_mlq_paging > run_os_1_mlq_paging.out 2>&1
    diff -u output/os_1_mlq_paging.output run_os_1_mlq_paging.out > evidence/diff_os_1_mlq_paging.txt || true
    grep "Dispatched process" run_os_1_mlq_paging.out | tee evidence/dispatch_lines_os_1_mlq_paging.txt
    ```
  - Evidence: `evidence/diff_os_1_mlq_paging.txt`, `evidence/dispatch_lines_os_1_mlq_paging.txt`.

- Phase B — MMU & Paging basic
  - Kịch bản: test page faults, alloc, IO
  - Lệnh run:
    ```bash
    ./os os_1_mlq_paging_small_1K > run_small_1K.out 2>&1
    diff -u output/os_1_mlq_paging_small_1K.output run_small_1K.out > evidence/diff_small_1K.txt || true
    grep -A8 "MM64 statistics" run_small_1K.out > evidence/mm_stats_small_1K.txt || true
    ```
  - Evidence: `evidence/diff_small_1K.txt`, `evidence/mm_stats_small_1K.txt`.

- Phase C — Multi-level 64-bit paging & `vmap_pgd_memset`
  - Kịch bản: xác nhận `vmap_pgd_memset` được gọi (memset-based allocation) và page-dir arrays inited
  - Kiểm tra code:
    ```bash
    grep -n "vmap_pgd_memset" -R src include || true
    sed -n '1,240p' src/mm64.c | sed -n '1,200p' > evidence/mm64_snippet.txt
    ```
  - Run:
    ```bash
    ./os os_1_mlq_paging > run_os_1_mlq_paging.out 2>&1
    grep -A8 "MM64 statistics" run_os_1_mlq_paging.out > evidence/mm_stats_os_1_mlq_paging.txt || true
    ```
  - Evidence: `evidence/mm64_snippet.txt`, `evidence/mm_stats_os_1_mlq_paging.txt`.

- Phase D — Kernel/User separation
  - Kịch bản: kiểm tra syscall chỉ resolve PCB qua `find_proc_by_pid(krnl,pid)` và thao tác qua `krnl`
  - Kiểm tra static:
    ```bash
    grep -n "find_proc_by_pid" -R src include | tee evidence/find_proc_refs.txt || true
    grep -n "->krnl" -R src include | tee evidence/krnl_refs.txt || true
    ```
  - Evidence: `evidence/find_proc_refs.txt`, `evidence/krnl_refs.txt`.

- Phase E — Regression / Batch run
  - Kịch bản: chạy mọi input trong `input/` và so sánh
  - Lệnh (WSL):
    ```bash
    bash scripts/run_all_tests.sh
    less evidence/summary.txt
    ```
  - Evidence: `evidence/diff_*.txt`, `evidence/run_*.out`, `evidence/summary.txt`.

## Cách đọc diff và xử lý
- Nếu `evidence/diff_<config>.txt` rỗng → test khớp expected.
- Nếu có diff: kiểm tra mục khác biệt chủ yếu:
  - Thứ tự `CPU X`/timestamp → có thể do scheduling nondeterminism (đa thread). So sánh các sự kiện logic (dispatch/finish), không nhất thiết phải đúng CPU id.
  - Nếu thiếu phần `MM64 statistics` hoặc số liệu bằng 0 → kiểm tra cấu hình `include/os-cfg.h` (MM_PAGING, MM64).

## Ghi chú
- Chạy trên WSL để tránh thiếu `make`/công cụ trên PowerShell.
- Các diff nhỏ liên quan CPU IDs thường chấp nhận được nếu hành vi chức năng (dispatch/finish, stats) khớp.
- Nếu muốn tôi tạo một báo cáo Markdown tự động (tổng hợp `evidence/summary.txt` và đính kèm diffs), nói "tạo báo cáo".


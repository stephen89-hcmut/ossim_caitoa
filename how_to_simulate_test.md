# Hướng Dẫn Mô Phỏng Và Kiểm Thử

Tài liệu này hướng dẫn cách build, chạy và kiểm tra bài Simple Operating System trong repository hiện tại.

## 1. Bắt Đầu Nhanh

### Điều Kiện Cần Có

- Dùng môi trường Linux.
- Có sẵn `gcc`, `make` và `bash`.
- Nếu đang dùng Windows, nên chạy qua WSL Ubuntu.

### Build

Chạy:

```bash
make all
```

Lệnh này biên dịch toàn bộ mô phỏng OS thành file thực thi `./os`.

### Chạy Một Test

Chạy một file cấu hình trong `input/` như sau:

```bash
./os sched
./os sched_0
./os sched_1
./os os_1_singleCPU_mlq
```

Mỗi lần chạy sẽ in log mô phỏng ra standard output. Nếu muốn so sánh với file mẫu, hãy chuyển output ra file trước:

```bash
./os sched > /tmp/sched.actual
diff -u output/sched.output /tmp/sched.actual
```

### Output Tham Chiếu

Repository lưu các output mẫu trong thư mục `output/`. Khi test từng phase, hãy so sánh với file tương ứng trong thư mục này.

### Quy Trình Chạy Theo Repository

File `run.sh` đã mô tả sẵn luồng sinh output:

1. Chạy cấu hình bằng `./os <config>`.
2. Ghi output ra `input/<config>.output`.
3. Khi muốn cập nhật bộ output tham chiếu, chuyển các file từ `input/` sang `output/`.

Vì vậy, vòng kiểm thử chuẩn là:

```bash
./os <config> > /tmp/<config>.actual
diff -u output/<config>.output /tmp/<config>.actual
```

## 1.1 Bản Đồ Phase Nhanh

| Phase | File cấu hình | File output tương ứng | Kiểm tra chính |
| --- | --- | --- | --- |
| Scheduler | `input/sched`, `input/sched_0`, `input/sched_1`, `input/os_1_singleCPU_mlq` | `output/sched.output`, `output/sched_0.output`, `output/sched_1.output`, `output/os_1_singleCPU_mlq.output` | MLQ, round-robin, preemption, time slice, dispatch đa CPU |
| Paging / Memory | `input/os_0_mlq_paging`, `input/os_1_mlq_paging`, `input/os_1_singleCPU_mlq_paging`, `input/os_1_mlq_paging_small_1K`, `input/os_1_mlq_paging_small_4K`, `input/os_2_mlq_paging`, `input/os_2_singleCPU_mlq_paging` | Các file `.output` tương ứng trong `output/` nếu có | Boot paging, swap, ánh xạ bộ nhớ, paging 64-bit |
| Syscall | `input/os_syscall_list`, `input/os_syscall`, `input/os_sc` | Các file `.output` tương ứng trong `output/` nếu có | Bảng syscall, wrapper, đường đi handler kernel |

## 2. Phase 1: CPU Scheduling

Phase 1 kiểm tra MLQ scheduler, round-robin, preemption và dispatch đa CPU khi chưa bật paging.

### File Đầu Vào

- `input/sched`
- `input/sched_0`
- `input/sched_1`
- `input/os_1_singleCPU_mlq`

### File Tiến Trình Thường Dùng

- `input/proc/s0` đến `s4`
- `input/proc/p1s`, `input/proc/p2s`, `input/proc/p3s`

### Cần Kiểm Tra

- `sched`: dispatch đa CPU và chọn theo priority của MLQ.
- `sched_0`: round-robin cơ bản trên 1 CPU.
- `sched_1`: preemption trên 1 CPU.
- `os_1_singleCPU_mlq`: bài toán MLQ phức tạp hơn trên 1 CPU.

### Kiểu Kết Quả Hợp Lệ

Thứ tự log có thể lệch nhẹ nếu nhiều CPU hoặc nhiều luồng chạy đồng thời. Khi đối chiếu output, hãy ưu tiên tính đúng của hành vi:

- Queue có priority cao hơn phải được chọn trước.
- Process chỉ được chạy trong đúng time slice.
- Khi hết slice, process phải được đưa lại đúng queue.
- Process đã hoàn tất thì không được chạy lại.
- Không có process nào bị mất khỏi ready queue.

### Lệnh Test Phase 1

```bash
./os sched > /tmp/sched.actual
diff -u output/sched.output /tmp/sched.actual

./os sched_0 > /tmp/sched_0.actual
diff -u output/sched_0.output /tmp/sched_0.actual

./os sched_1 > /tmp/sched_1.actual
diff -u output/sched_1.output /tmp/sched_1.actual

./os os_1_singleCPU_mlq > /tmp/os_1_singleCPU_mlq.actual
diff -u output/os_1_singleCPU_mlq.output /tmp/os_1_singleCPU_mlq.actual
```

## 3. Phase 2: Memory Management

Phase 2 bật paging và kiểm tra virtual memory, swap, cũng như hỗ trợ paging 64-bit.

### 3a. Paging Chuẩn

Chạy:

- `input/os_0_mlq_paging`
- `input/os_1_mlq_paging`
- `input/os_1_singleCPU_mlq_paging`

File tiến trình thường đi cùng:

- `input/proc/m0s`
- `input/proc/m1s`
- `input/proc/p0s`

Kiểm tra hệ thống khởi động paging đúng, ánh xạ bộ nhớ đúng và process vẫn chạy sau các thao tác memory.

### 3b. RAM Nhỏ Và Swap Nhiều

Chạy:

- `input/os_1_mlq_paging_small_1K`
- `input/os_1_mlq_paging_small_4K`

Các test này ép hệ thống swap liên tục. Cần kiểm tra hệ thống vẫn tiến triển được khi page-in và page-out xảy ra nhiều lần.

### 3c. Paging 64-bit

Chạy:

- `input/os_2_mlq_paging`
- `input/os_2_singleCPU_mlq_paging`

File tiến trình tiêu biểu:

- `input/proc/m2s`

Kiểm tra cấu trúc paging 64-bit, dịch địa chỉ dài và tách biệt kernel/user.

### Lệnh Test Cụ Thể Cho Phase 2

```bash
./os os_0_mlq_paging > /tmp/os_0_mlq_paging.actual
diff -u output/os_0_mlq_paging.output /tmp/os_0_mlq_paging.actual

./os os_1_mlq_paging > /tmp/os_1_mlq_paging.actual
diff -u output/os_1_mlq_paging.output /tmp/os_1_mlq_paging.actual

./os os_1_singleCPU_mlq_paging > /tmp/os_1_singleCPU_mlq_paging.actual
diff -u output/os_1_singleCPU_mlq_paging.output /tmp/os_1_singleCPU_mlq_paging.actual

./os os_1_mlq_paging_small_1K > /tmp/os_1_mlq_paging_small_1K.actual
diff -u output/os_1_mlq_paging_small_1K.output /tmp/os_1_mlq_paging_small_1K.actual

./os os_1_mlq_paging_small_4K > /tmp/os_1_mlq_paging_small_4K.actual
diff -u output/os_1_mlq_paging_small_4K.output /tmp/os_1_mlq_paging_small_4K.actual

./os os_2_mlq_paging > /tmp/os_2_mlq_paging.actual
diff -u output/os_2_mlq_paging.output /tmp/os_2_mlq_paging.actual

./os os_2_singleCPU_mlq_paging > /tmp/os_2_singleCPU_mlq_paging.actual
diff -u output/os_2_singleCPU_mlq_paging.output /tmp/os_2_singleCPU_mlq_paging.actual
```

### Điểm Cần Để Ý Ở Phase 2

- OS phải boot được với cấu hình paging đang bật trong `include/os-cfg.h`.
- Output memory không được có truy cập sai khi thao tác hợp lệ trong user-space.
- Nếu test có swap, hệ thống vẫn phải tiếp tục chạy sau khi thay trang.

## 4. Phase 3: System Calls

Phase 3 kiểm tra giao diện syscall và khả năng gọi handler ở kernel.

### File Đầu Vào

- `input/os_syscall_list`
- `input/os_syscall`
- `input/os_sc`

### Cần Kiểm Tra

- `os_syscall_list` phải in ra danh sách syscall.
- `os_syscall` phải đi qua syscall path.
- `os_sc` kiểm tra đường đi syscall tùy chỉnh của bài.

### Lệnh Test Cụ Thể Cho Phase 3

```bash
./os os_syscall_list > /tmp/os_syscall_list.actual
diff -u output/os_syscall_list.output /tmp/os_syscall_list.actual

./os os_syscall > /tmp/os_syscall.actual
diff -u output/os_syscall.output /tmp/os_syscall.actual

./os os_sc > /tmp/os_sc.actual
diff -u output/os_sc.output /tmp/os_sc.actual
```

### Điểm Cần Để Ý Ở Phase 3

- Bảng syscall phải được sinh và link đúng.
- Wrapper và kernel handler phải thống nhất về tham số.
- Dòng thông báo cuối cùng phải đúng hành vi mong đợi khi syscall thành công.

## 5. Quy Tắc Đối Chiếu Kết Quả

Khi so sánh output thật với output mẫu, hãy dùng các quy tắc sau:

- Kiểm tra đúng tên file trước, rồi mới so log.
- Với phase đơn luồng, output nên gần như khớp hoàn toàn.
- Với phase đa CPU, cho phép khác biệt nhỏ về thứ tự log do cạnh tranh luồng.
- Không chấp nhận nếu process bị mất, bị nhân đôi hoặc kết thúc sai.
- Không chấp nhận nếu time slice bị xử lý sai rõ ràng.

## 6. Checklist

### Build Và Chạy

- [ ] `make all` biên dịch thành công.
- [ ] `./os <config>` chạy không bị crash.
- [ ] Đã dùng đúng file `input/<config>`.

### Phase Scheduler

- [ ] `sched` dispatch đa CPU đúng.
- [ ] `sched_0` chạy đúng kiểu round-robin cơ bản.
- [ ] `sched_1` thể hiện preemption khi hết time slice.
- [ ] `os_1_singleCPU_mlq` chọn process đúng theo MLQ priority.
- [ ] Không process nào biến mất khỏi queue.
- [ ] Process đã xong thì dừng hẳn.

### Phase Memory

- [ ] Paging được bật đúng khi chạy các test paging.
- [ ] Test swap vẫn chạy tiếp khi RAM nhỏ.
- [ ] Test paging 64-bit boot và chạy được.
- [ ] Phân tách user/kernel được giữ đúng.

### Phase Syscall

- [ ] Output danh sách syscall xuất hiện.
- [ ] Syscall thực sự đi vào kernel handler.
- [ ] Output cuối cùng khớp với hành vi mong đợi.

### Kỷ Luật Code

- [ ] Không xóa TODO comments nếu chưa được yêu cầu.
- [ ] Không hard-code output.
- [ ] Giữ thay đổi tập trung đúng module.

## 7. Thứ Tự Test Khuyến Nghị

1. Build bằng `make all`.
2. Chạy `sched`, `sched_0`, `sched_1` và `os_1_singleCPU_mlq`.
3. Chạy các test paging và swap.
4. Chạy các test syscall.
5. So sánh từng output thực tế với file tương ứng trong `output/`.
6. Nếu cần cập nhật bộ output tham chiếu, dùng cùng kiểu lệnh như `run.sh`.

# Bài tập lớn Hệ điều hành - Simple Operating System

Tài liệu này là bản tổng hợp về repo Simple OS: cách build, cách chạy, cấu trúc thư mục, vai trò từng nhóm file, và cách đối chiếu kết quả theo 3 phase của bài tập lớn CO2018.

## 1. Mục tiêu của dự án

Simple OS mô phỏng các thành phần lõi của một hệ điều hành đơn giản:

- Bộ lập lịch CPU theo Multilevel Queue (MLQ) kết hợp round-robin.
- Cơ chế đồng bộ giữa nhiều CPU ảo và timer.
- Quản lý bộ nhớ ảo, ánh xạ sang bộ nhớ vật lý, paging và swap.
- Giao tiếp user/kernel thông qua system call.

Mục tiêu thực hành là hiểu luồng phối hợp giữa scheduler, memory management, loader, CPU và syscall khi nhiều tiến trình chạy đồng thời.

## 2. Build và chạy

### 2.1 Cách build

Đường build chính của repo là:

```bash
make all
```

Lệnh này biên dịch toàn bộ mã nguồn trong [src/](src/), đồng thời tạo file trung gian [src/syscalltbl.lst](src/syscalltbl.lst) từ [src/syscall.tbl](src/syscall.tbl) thông qua [src/syscalltbl.sh](src/syscalltbl.sh) trước khi link ra binary os.

Bạn có thể build trong VS Code terminal, terminal của IDE, hoặc terminal WSL. Nếu đang dùng Windows, nên mở repo bằng VS Code Remote - WSL hoặc chạy trong WSL Ubuntu để có sẵn `make`, `gcc` và các công cụ Unix chuẩn.

Nếu máy chưa có toolchain:

```bash
sudo apt update
sudo apt install build-essential
```

### 2.2 Cách chạy tay

Chạy mô phỏng bằng cách truyền tên cấu hình trong [input/](input/):

```bash
./os <ten_cau_hinh>
```

Ví dụ:

```bash
./os input/os_syscall
```

Trong repo này, [src/os.c](src/os.c) là entry point chính và kỳ vọng nhận đúng tên file cấu hình từ thư mục [input/](input/).

### 2.3 Cách chạy batch bằng `run.sh`

Script [run.sh](run.sh) dùng để chạy hàng loạt bộ test và ghi log ra file .output. Về ý tưởng, nó sẽ:

1. Gọi ./os với từng cấu hình trong [input/](input/).
2. Chuyển stdout sang file .output tương ứng.
3. Gom các file kết quả vào thư mục [output/](output/) để so sánh với output mẫu.

Đây là cách thuận tiện nhất khi bạn muốn chạy hồi quy sau mỗi lần sửa code.

## 3. Cấu hình build hiện tại

Repo hiện đang bật các macro quan trọng trong [include/os-cfg.h](include/os-cfg.h):

- MLQ_SCHED = 1
- MAX_PRIO = 140
- MM_PAGING
- MM64 = 1

Các macro đang tắt: MM_FIXED_MEMSZ, VMDBG, MMDBG, IODUMP, PAGETBL_DUMP. Nghĩa là repo đang chạy theo chế độ MLQ + paging + MM64, không bật dump I/O hoặc page table.

## 4. Cấu trúc thư mục

```text
.
├── Makefile
├── run.sh
├── README.md
├── include/
├── input/
├── output/
└── src/
```

### 4.1 include

Đây là lớp blueprint của hệ điều hành. Các header chính gồm:

- [include/os-cfg.h](include/os-cfg.h): macro cấu hình build, bật/tắt MLQ, paging, MM64 và debug.
- [include/common.h](include/common.h): kiểu dữ liệu chung, opcode, pcb_t, krnl_t, code segment và thanh ghi.
- [include/queue.h](include/queue.h): định nghĩa queue tiến trình và API enqueue/dequeue/purge/empty.
- [include/sched.h](include/sched.h): giao diện scheduler: init, finish, add, put, get.
- [include/os-mm.h](include/os-mm.h): cấu trúc quản lý bộ nhớ ảo, VMA, vùng nhớ rỗng, mm_struct, memphy.
- [include/mm.h](include/mm.h): đặc tả paging 32-bit và giao tiếp với RAM/SWAP vật lý.
- [include/mm64.h](include/mm64.h): đặc tả paging nhiều tầng 64-bit.
- [include/mem.h](include/mem.h): giao diện memory cố định khi bật MM_FIXED_MEMSZ.
- [include/libmem.h](include/libmem.h): wrapper phía user space cho alloc/free/read/write và các syscall bộ nhớ nâng cao.
- [include/syscall.h](include/syscall.h): register frame của syscall, bảng tên syscall, và interface _syscall.
- [include/loader.h](include/loader.h): giao diện nạp chương trình từ file kịch bản tiến trình.
- [include/cpu.h](include/cpu.h): giao diện chạy một tiến trình trên CPU ảo.
- [include/timer.h](include/timer.h): đồng bộ thời gian và điều phối slot giữa các CPU ảo.

### 4.2 src

Đây là phần cài đặt thực tế của hệ điều hành mô phỏng:

- [src/os.c](src/os.c): entry point, đọc config, khởi tạo kernel, CPU, timer, loader và scheduler.
- [src/cpu.c](src/cpu.c): vòng lặp fetch-decode-execute của CPU ảo.
- [src/timer.c](src/timer.c): bộ đếm thời gian và đồng bộ đa CPU.
- [src/loader.c](src/loader.c): nạp file tiến trình thành pcb_t.
- [src/queue.c](src/queue.c): hiện thực các thao tác trên hàng đợi tiến trình.
- [src/sched.c](src/sched.c): scheduler MLQ và lựa chọn tiến trình theo priority/time slice.
- [src/mm-memphy.c](src/mm-memphy.c): quản lý khung trang vật lý, RAM/SWAP và các thao tác đọc/ghi mức thấp.
- [src/mm-vm.c](src/mm-vm.c): quản lý không gian địa chỉ ảo và vùng nhớ của từng process.
- [src/mm.c](src/mm.c): module paging 32-bit cũ, hiện phục vụ tương thích hoặc tham chiếu.
- [src/mm64.c](src/mm64.c): paging 64-bit nhiều tầng.
- [src/paging.c](src/paging.c): trình kiểm thử độc lập cho module paging.
- [src/mem.c](src/mem.c): bộ nhớ cố định cũ dùng khi bật MM_FIXED_MEMSZ.
- [src/libmem.c](src/libmem.c): wrapper bộ nhớ phía userspace.
- [src/libstd.c](src/libstd.c): wrapper syscall phía userspace.
- [src/syscall.c](src/syscall.c): trung tâm điều phối syscall và bảng tên syscall.
- [src/sys_listsyscall.c](src/sys_listsyscall.c): handler cho syscall liệt kê danh sách syscall.
- [src/sys_mem.c](src/sys_mem.c): handler syscall quản lý bộ nhớ.
- [src/syscall.tbl](src/syscall.tbl): bảng khai báo syscall.
- [src/syscalltbl.sh](src/syscalltbl.sh): script sinh file danh sách syscall cho build.

### 4.3 input

Thư mục này chứa toàn bộ dữ liệu đầu vào để chạy mô phỏng:

- `input/proc/`: các file kịch bản tiến trình, mỗi file mô tả một chương trình riêng.
- `input/sched*`: các cấu hình phục vụ Phase 1 scheduler.
- `input/os_*`: các cấu hình tích hợp scheduler + memory + syscall theo từng phase.

### 4.4 output

Thư mục này chứa các kết quả sinh ra từ [run.sh](run.sh). Các file .output được sinh tự động và đang được ignore bởi git, nhưng vẫn dùng để đối chiếu khi chạy test cục bộ.

## 5. Format dữ liệu đầu vào

### 5.1 File tiến trình trong input/proc

Mỗi file tiến trình có dạng chung:

```text
[priority] [N]
instruction 0
instruction 1
...
instruction N-1
```

Trong đó:

- `priority` là độ ưu tiên mặc định của tiến trình.
- `N` là số lượng lệnh.
- Các lệnh thường gặp gồm `CALC`, `ALLOC`, `FREE`, `READ`, `WRITE`, `KMALLOC`, `KMEM_CACHE_CREATE`, `KMEM_CACHE_ALLOC`, `COPY_FROM_USER`, `COPY_TO_USER`, `SYSCALL`.

### 5.2 File cấu hình hệ điều hành trong input

Dạng tổng quát là:

```text
[time slice] [number of CPU] [number of processes]
[time 0] [path 0] [priority 0]
...
[time M-1] [path M-1] [priority M-1]
```

Nếu cấu hình có memory paging động, file còn chứa thông tin RAM/SWAP theo format riêng của đề.

## 6. 3 Testing

### Phase 1: CPU Scheduling

Mục tiêu của phase này là kiểm tra MLQ scheduler, round-robin, preemption và tương tác đa CPU khi không dùng paging.

Các file cấu hình chính:

- [input/sched](input/sched)
- [input/sched_0](input/sched_0)
- [input/sched_1](input/sched_1)
- [input/os_1_singleCPU_mlq](input/os_1_singleCPU_mlq)

Các file tiến trình thường dùng:

- [input/proc/s0](input/proc/s0) đến [input/proc/s4](input/proc/s4)
- [input/proc/p1s](input/proc/p1s), [input/proc/p2s](input/proc/p2s), [input/proc/p3s](input/proc/p3s)

Output tương ứng là:

- [output/sched.output](output/sched.output)
- [output/sched_0.output](output/sched_0.output)
- [output/sched_1.output](output/sched_1.output)
- [output/os_1_singleCPU_mlq.output](output/os_1_singleCPU_mlq.output)

### Phase 2: Memory Management

Phase này bật paging và kiểm tra phân trang, swap, và MM64.

#### 2a. Paging 32-bit chuẩn

- [input/os_0_mlq_paging](input/os_0_mlq_paging)
- [input/os_1_mlq_paging](input/os_1_mlq_paging)
- [input/os_1_singleCPU_mlq_paging](input/os_1_singleCPU_mlq_paging)

Các file tiến trình thường đi kèm:

- [input/proc/m0s](input/proc/m0s)
- [input/proc/m1s](input/proc/m1s)
- [input/proc/p0s](input/proc/p0s)

#### 2b. RAM nhỏ và swap

- [input/os_1_mlq_paging_small_1K](input/os_1_mlq_paging_small_1K)
- [input/os_1_mlq_paging_small_4K](input/os_1_mlq_paging_small_4K)

Mục tiêu là ép hệ thống swap liên tục để kiểm tra thuật toán thay trang.

#### 2c. Paging 64-bit nhiều tầng

- [input/os_2_mlq_paging](input/os_2_mlq_paging)
- [input/os_2_singleCPU_mlq_paging](input/os_2_singleCPU_mlq_paging)

Tiến trình đặc trưng:

- [input/proc/m2s](input/proc/m2s)

Phase này kiểm tra kmalloc, cache, copy_from_user và copy_to_user qua syscall nội bộ.

### Phase 3: System Calls, Interface Kernel

Mục tiêu là kiểm tra syscall interface, bảng syscall và khả năng thêm syscall mới.

Các file cấu hình chính:

- [input/os_syscall_list](input/os_syscall_list)
- [input/os_syscall](input/os_syscall)
- [input/os_sc](input/os_sc)

Các tiến trình kiểm thử:

- [input/proc/sc1](input/proc/sc1)
- [input/proc/sc2](input/proc/sc2)
- [input/proc/sc3](input/proc/sc3)

Output đối chiếu tương ứng:

- [output/os_syscall_list.output](output/os_syscall_list.output)
- [output/os_syscall.output](output/os_syscall.output)
- [output/os_sc.output](output/os_sc.output)

## 7. Quy trình làm việc khuyến nghị

1. Sửa code trong [src/](src/) theo đúng module cần làm.
2. Chạy make all để build lại toàn bộ hệ thống.
3. Chạy `./os <config>` cho từng case cần kiểm tra nhanh.
4. Chạy `./run.sh` để sinh hoặc refresh toàn bộ output.
5. So sánh file kết quả trong `output/` với output mẫu của đề.

## 8. Ghi chú quan trọng

- Output của một số test đa CPU có thể có nhiều thứ tự hợp lệ, miễn là tuân thủ đúng luật scheduler và memory model.
- Không thay đổi macro cấu hình nếu chưa hiểu tác động tới toàn bộ hệ thống.
- Khi làm memory management, luôn phân biệt rõ user space và kernel space.

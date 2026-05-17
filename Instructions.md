# Instructions for Implementation

Tài liệu này mô tả quy ước triển khai, cách kiểm thử, và các ràng buộc kỹ thuật cần tuân thủ khi làm bài tập lớn Simple Operating System.

## 1. Mục tiêu triển khai

Khi sửa code trong repo này, mục tiêu không chỉ là làm cho chương trình chạy được, mà còn phải bảo đảm:

- Đúng với mô hình OS mà đề bài yêu cầu.
- Không phá vỡ cấu trúc module hiện có.
- Không làm mờ ranh giới user space và kernel space.
- Không làm hỏng các phase khác khi sửa một module cụ thể.

## 2. Chuẩn viết code

Áp dụng tinh thần của GNU C Coding Standards:

https://www.gnu.org/prep/standards/html_node/Writing-C.html

Quy ước nên giữ nhất quán:

- Đặt tên rõ nghĩa, ưu tiên mô tả vai trò.
- Mỗi hàm làm một việc chính, ngắn gọn và dễ kiểm thử.
- Tránh magic number; hằng số nghiệp vụ nên đưa vào macro hoặc constant có tên.
- Giữ formatting thống nhất trong toàn bộ project.
- Comment chỉ dùng khi cần giải thích quyết định thiết kế hoặc logic khó đọc.
- Ưu tiên kiểm tra lỗi sớm và trả về mã lỗi rõ ràng.

## 3. Ràng buộc kiến trúc cần giữ nguyên

### 3.1 Không phá cấu trúc module

- Không đổi vai trò của các file trong `include/` và `src/` nếu không cần thiết.
- Khi mở rộng tính năng, ưu tiên sửa đúng module chịu trách nhiệm.
- Không đổi public API nếu chưa có lý do rõ ràng.

### 3.2 Không truyền PCB trực tiếp từ user space

Đề bài yêu cầu giao tiếp qua PID và dữ liệu hợp lệ, không truyền thẳng `struct pcb_t` từ user space.

- User space chỉ gửi PID hoặc tham số hợp lệ qua syscall.
- Kernel phải truy vết từ `krnl_t` hoặc các cấu trúc quản lý nội bộ để tìm PCB tương ứng.
- Không để user code truy cập trực tiếp PCB của tiến trình khác.

### 3.3 Phân biệt rõ user space và kernel space

- User process chỉ được thao tác trong vùng địa chỉ hợp lệ của nó.
- Kernel memory phải được bảo vệ.
- Khi copy dữ liệu giữa user/kernel, dùng cơ chế copy thích hợp thay vì truy cập thẳng.

### 3.4 Giữ đúng cấu hình build hiện tại

Repo hiện đang hướng tới cấu hình:

- `MLQ_SCHED`
- `MM_PAGING`
- `MM64`
- `MAX_PRIO 140`

Khi sửa code, phải bảo đảm các macro này vẫn biên dịch được và hành vi không mâu thuẫn với README hay input/output của đề.

## 4. Phạm vi theo module

### 4.1 Scheduler

Các file chính:

- `include/queue.h`
- `include/sched.h`
- `src/queue.c`
- `src/sched.c`

Mục tiêu cần giữ đúng:

- `enqueue`, `dequeue`, `purgequeue`, `empty` phải ổn định.
- MLQ phải chọn đúng hàng đợi theo priority.
- Không làm mất process khỏi queue.
- Không rơi vào vòng lặp vô hạn khi duyệt queue.
- Time slice phải được tôn trọng.

### 4.2 Memory management

Các file chính:

- `include/os-mm.h`
- `include/mm.h`
- `include/mm64.h`
- `src/mm.c`
- `src/mm-vm.c`
- `src/mm-memphy.c`
- `src/mm64.c`
- `src/paging.c`

Mục tiêu cần giữ đúng:

- `ALLOC/FREE` cập nhật đúng vùng nhớ ảo.
- `READ/WRITE` tuân thủ giới hạn user space.
- `KMALLOC` và cache allocation không vi phạm kernel memory.
- Paging và swap phải nhất quán giữa RAM và SWAP.
- MM64 phải bóc tách địa chỉ bằng đúng các tầng index của 64-bit paging.

### 4.3 System call

Các file chính:

- `include/syscall.h`
- `src/syscall.c`
- `src/sys_listsyscall.c`
- `src/sys_mem.c`
- `src/syscall.tbl`
- `src/syscalltbl.sh`

Mục tiêu cần giữ đúng:

- Syscall phải được khai báo đúng trong bảng syscall.
- Số hiệu syscall không được trùng.
- Register frame phải nhận tham số đúng.
- Handler phải trả về trạng thái rõ ràng.
- Khi thêm syscall mới, phải thêm đúng vào `syscall.tbl` rồi để build sinh lại danh sách.

## 5. Cách build và chạy khi phát triển

### 5.1 Build

Lệnh chuẩn:

```bash
make all
```

Chỉ dùng các target khác nếu bạn đã hiểu rõ mục đích của nó. Với hầu hết trường hợp, `make all` là bước kiểm tra đầu tiên sau khi sửa code.

### 5.2 Chạy nhanh một cấu hình

```bash
./os input/<ten_cau_hinh>
```

Ví dụ:

```bash
./os input/os_0_mlq_paging
```

### 5.3 Chạy batch toàn bộ test

```bash
./run.sh
```

Mục đích của script là chạy tuần tự các cấu hình trong `input/`, ghi output ra file `.output`, rồi gom vào `output/` để đối chiếu. Đây là cách phù hợp nhất sau khi sửa scheduler, paging, hoặc syscall.

## 6. Tiêu chí chấp nhận

Một thay đổi chỉ nên được coi là đạt khi:

- Biên dịch thành công với `make all`.
- Chạy được các cấu hình trong `input/` mà không crash.
- Scheduler chọn tiến trình theo priority/MLQ hợp lý.
- Bộ nhớ ảo ánh xạ đúng với cơ chế paging đang bật.
- System call nhận và xử lý tham số đúng luồng.
- Output sinh ra giải thích được bằng mô hình của đề bài.

## 7. Cách kiểm thử theo module

### 7.1 Scheduler

- Chạy các file `sched`, `sched_0`, `sched_1`, `os_1_singleCPU_mlq`.
- Kiểm tra thứ tự dispatch, preemption, và time slice.
- So sánh kết quả với các file `.output` tương ứng trong `output/`.

### 7.2 Paging và memory

- Chạy `os_0_mlq_paging`, `os_1_mlq_paging`, `os_1_singleCPU_mlq_paging`.
- Chạy thêm `os_1_mlq_paging_small_1K`, `os_1_mlq_paging_small_4K` để kiểm tra swap.
- Chạy `os_2_mlq_paging`, `os_2_singleCPU_mlq_paging` để kiểm tra MM64.

### 7.3 System call

- Chạy `os_syscall_list`, `os_syscall`, `os_sc`.
- Kiểm tra syscall 0, syscall memmap, và syscall tùy chỉnh mới thêm.

## 8. Những điều không nên làm

- Không sửa lan man các file ngoài phạm vi module đang làm.
- Không hard-code kết quả thay vì xử lý theo dữ liệu đầu vào.
- Không bỏ qua kiểm tra biên, đặc biệt ở queue và memory.
- Không giả định mọi test đa CPU chỉ có một thứ tự output duy nhất.

## 9. Quy trình tối thiểu khi sửa code

1. Đọc file liên quan và xác định hàm chịu trách nhiệm.
2. Hiểu luồng dữ liệu đầu vào/đầu ra.
3. Sửa nhỏ gọn, tập trung vào root cause.
4. Biên dịch lại bằng `make all`.
5. Chạy test/config liên quan.
6. Chỉ mở rộng phạm vi khi đã xác nhận sửa đúng.

## 10. Môi trường khuyến nghị

- Trên Windows, nên dùng WSL Ubuntu để build và run.
- Cài toolchain bằng:

```bash
sudo apt update
sudo apt install build-essential
```

- Sau đó mở repo trong VS Code Remote - WSL hoặc đi vào thư mục repo từ terminal WSL.

## 11. Khi nào cần dừng và hỏi lại

Hãy dừng và hỏi lại nếu:

- Có thay đổi của người dùng xung đột trực tiếp với thay đổi mới.
- Đề bài hoặc input không đủ rõ để xác định hành vi mong muốn.
- Cần chỉnh sửa cấu trúc lớn ngoài phạm vi một module rõ ràng.

## 12. Mẫu chất lượng mong đợi

Code cuối cùng nên có các đặc điểm:

- Đúng chức năng.
- Dễ đọc.
- Dễ kiểm thử.
- Có đường đi lỗi rõ ràng.
- Không phụ thuộc vào giả định ngầm không được ghi trong đề.

# QUY ĐỊNH VÀ TIÊU CHUẨN TRIỂN KHAI BÀI TẬP LỚN (SIMPLE OS)

Tài liệu này quy định các tiêu chuẩn bắt buộc về phong cách lập trình, phân chia phạm vi tệp tin và các yêu cầu kỹ thuật mà toàn bộ thành viên trong nhóm phải tuân thủ nghiêm ngặt trong quá trình hoàn thiện hệ thống giả lập.

---

# 1. TIÊU CHUẨN PHONG CÁCH LẬP TRÌNH (CODING STYLE)

## Yêu cầu chung

Tất cả các đoạn mã nguồn viết mới hoặc chỉnh sửa trong các hàm hệ thống, đặc biệt là các hàm xử lý cuộc gọi hệ thống (System Calls), bắt buộc phải tuân thủ theo tiêu chuẩn viết mã C của GNU.

- Tài liệu tham khảo chính thức:  
  https://www.gnu.org/prep/standards/html_node/Writing-C.html

---

## Đặt dấu ngoặc nhọn (Formatting Braces)

Dấu ngoặc nhọn mở của thân hàm phải nằm ở một dòng riêng biệt, không viết liền sau khai báo tham số. Các khối lệnh điều kiện bên trong cũng phải xuống dòng cho dấu ngoặc.

### ĐÚNG THEO CHUẨN GNU

```c
int
sys_xxxhandler (struct krnl_t *krnl, uint32_t pid, struct sc_regs *regs)
{
  /* Thân hàm thụt lề sử dụng khoảng trắng (Spaces) */
  if (regs == NULL)
    {
      return -1;
    }
}
```

---

## Khoảng trắng hàm (Spacing)

Phải có một khoảng trắng giữa tên hàm và dấu mở ngoặc đơn `(` khi gọi hàm hoặc khai báo hàm, ngoại trừ các macro.

### Đúng

```c
enqueue (q, proc);
malloc (sizeof (struct pcb_t));
```

### Sai

```c
enqueue(q, proc);
malloc(sizeof(struct pcb_t));
```

---

## Đặt tên biến và hàm

Sử dụng chữ thường, phân tách nhau bằng dấu gạch dưới (`snake_case`).

Tên biến cần rõ ràng, mang tính gợi nhớ ý nghĩa kiến trúc hệ điều hành.

### Ví dụ

```c
vma_it
pgd_idx
free_frame_list
```

## Giữ nguyên comment và TODO hiện có

Khi chỉnh sửa mã nguồn trong các file được phép thay đổi, tuyệt đối không tự ý xoá các comment mô tả kiến trúc, comment TODO, hoặc các block chú thích do giảng viên/nhóm đã để sẵn nếu chưa có yêu cầu thay thế tương đương.

- Nếu một comment TODO liên quan trực tiếp đến luồng triển khai hiện tại, hãy giữ lại và bổ sung code bên dưới comment đó.
- Chỉ được xoá comment khi comment đó đã được thay thế bằng comment tương đương hoặc khi có yêu cầu rõ ràng từ đề bài/nhóm.
- Quy tắc này đặc biệt áp dụng cho `src/queue.c`, `src/sched.c`, `src/sys_mem.c` và các file module Phase 1/2/3 đang tiếp tục phát triển.

---

# 2. PHÂN CHIA PHẠM VI TỆP TIN (FILE PERMISSION)

Để đảm bảo tính toàn vẹn của bộ kiểm thử (testcases) gốc phục vụ cho quá trình chấm điểm tự động, cấu trúc dự án được phân chia ranh giới nghiêm ngặt như sau:

---

# 🟢 Các tệp ĐƯỢC PHÉP THAY ĐỔI VÀ THÊM MỚI

Sinh viên thực hiện toàn bộ giải thuật lập lịch, quản lý bộ nhớ và bổ sung giao tiếp hệ thống gọi hàm tại các file sau:

---

## Module Lập lịch (Phase 1)

### `src/queue.c`

Hiện thực cơ chế xếp hàng và lấy tiến trình ưu tiên.

### `src/sched.c`

Thuật toán MLQ và phân phối slot thời gian tránh bỏ đói.

---

## Module Bộ nhớ (Phase 2)

### `src/mm64.c`

Khởi tạo PTE, dịch bit địa chỉ và phân trang đa cấp 64-bit.

### `src/mm-vm.c`

Xử lý co giãn giới hạn và kiểm tra trùng lấn vùng nhớ ảo.

### `src/libmem.c`

Giải thuật tìm kiếm, cấp phát và thu hồi vùng nhớ tự do.

---

## Hệ thống gọi hàm (Phase 3)

### `src/syscall.tbl`

Đăng ký số hiệu syscall và liên kết hàm nhân.

### `src/sys_mem.c`

Sửa mã dummy để duyệt tìm tiến trình thật qua PID trong nhân.

---

## Mở rộng tính năng

Được phép tự tạo thêm các file xử lý syscall mới trong thư mục `src/`.

### Ví dụ

```text
src/sys_xxxhandler.c
```

Và cập nhật khai báo trong các file cấu hình tiêu đề tương ứng.

---

# 🔴 Các tệp TUYỆT ĐỐI KHÔNG ĐƯỢC THAY ĐỔI

Bất kỳ hành vi chỉnh sửa nào lên các tệp này làm thay đổi logic nền tảng hoặc can thiệp cấu hình testcase sẽ bị coi là vi phạm quy chế.

---

## Dữ liệu đầu vào giả lập

Toàn bộ các file kịch bản môi trường hệ thống:

```text
os_0_mlq_paging
os_1_mlq_paging
os_1_mlq_paging_small_1K
sched
os_syscall
...
```

Và toàn bộ mã lệnh tiến trình trong thư mục:

```text
input/proc/
```

### Ví dụ

```text
p0s
m0s
s1
sc2
...
```

---

## Tự động hóa hệ thống

Không được chỉnh sửa:

```text
Makefile
src/syscalltbl.sh
run.sh
```

---

## Logic phần cứng ảo cốt lõi

Không được chỉnh sửa:

```text
src/os.c
src/cpu.c
src/timer.c
src/mm-memphy.c
src/libstd.c
src/syscall.c
src/sys_listsyscall.c
```

---

# 3. CÁC YÊU CẦU BẮT BUỘC KHÔNG THỂ BỎ QUA (CRITICAL REQUIREMENTS)

Trong quá trình chấm điểm và bảo vệ, hệ thống của nhóm phải chứng minh được việc hiện thực thành công các cơ chế kiến trúc hệ điều hành sau:

---

# Cô lập không gian Nhị phân (User/Kernel Isolation)

Tuyệt đối KHÔNG truyền trực tiếp con trỏ cấu trúc Khối điều khiển tiến trình (`struct pcb_t *`) từ không gian người dùng (`Userspace - libstd.c`) xuống tầng nhân (`Kernelspace`).

Tất cả các hàm gọi hệ thống bắt buộc phải:

- Đóng gói tham số qua tập thanh ghi ảo `struct sc_regs`
- Nhân hệ điều hành phải chịu trách nhiệm:
  - duyệt danh sách luồng trong nhân
  - dùng `pid` để định danh
  - truy vết cấu trúc tiến trình thực tế

---

# Đồng bộ hóa đa lõi (Multi-core Synchronization)

Do hệ thống giả lập chạy môi trường đa xử lý với các luồng CPU hoạt động song song, mọi thao tác can thiệp vào tài nguyên dùng chung trong nhân bắt buộc phải được bảo vệ bằng cơ chế khóa Mutex (`pthread_mutex_lock`).

## Ví dụ tài nguyên dùng chung

```text
mlq_ready_queue
free_fp_list
biến thời gian toàn cục
danh sách khung trang
bộ cấp phát bộ nhớ
```

## Cấm tuyệt đối việc để xảy ra

```text
Race Condition
Deadlock
Treo terminal
Trạng thái bộ nhớ ảo không hợp lệ
```

---

# Bảo toàn cơ chế phân trang đa cấp 64-bit

Khi bật cờ cấu hình `MM64`, hệ thống phải phân tách địa chỉ dài 57-bit qua đúng lược đồ cascaded 5 tầng:

```text
PGD -> P4D -> PUD -> PMD -> PT
```

Mã nguồn phải sử dụng chính xác các hàm bóc tách trường bit và mặt nạ bit có sẵn trong:

```text
include/bitops.h
include/mm64.h
```
---

# 4. QUY ĐỊNH VỀ COMMENT VÀ TODO

Để đảm bảo khả năng đối chiếu với mã nguồn gốc, phục vụ kiểm thử và chấm điểm tự động, các comment hướng dẫn trong source code phải được xử lý theo quy tắc sau:

---

## Comment TODO BẮT BUỘC PHẢI GIỮ NGUYÊN

Tất cả comment có chứa từ khóa:

```text
TODO
```

bắt buộc:

- KHÔNG được xoá
- KHÔNG được đổi nội dung
- KHÔNG được đổi format
- KHÔNG được đổi vị trí nếu không thật sự cần thiết
- KHÔNG được sửa indentation làm sai khác cấu trúc ban đầu

Ví dụ:

```c
/* TODO: Implement page table allocation */
```

hoặc:

```c
// TODO: validate overlap vm area
```

phải được giữ nguyên đúng định dạng ban đầu trong source code.

---

## Các comment KHÔNG chứa TODO

Các comment khác không chứa từ khóa `TODO`:

- Có thể được xoá nếu không còn giá trị sử dụng
- Có thể được thay đổi nội dung để làm rõ logic xử lý
- Có thể được bổ sung thêm giải thích kỹ thuật

Tuy nhiên:

> Trước khi xoá bất kỳ comment nào không chứa TODO, phải xác nhận (confirm) lại với trưởng nhóm / người phụ trách review code.

---

## Không implement cứng theo comment gợi ý nếu chưa xác minh

Một số comment trong source gốc chỉ mang tính chất gợi ý hoặc mô tả hướng triển khai.

Sinh viên không được:

- implement máy móc chỉ dựa trên comment
- giả định comment luôn đúng tuyệt đối
- bỏ qua kiểm tra logic thực tế của hệ thống

Cần ưu tiên:

- kiểm tra testcase
- phân tích luồng thực thi
- đối chiếu với kiến trúc OS hiện tại
- xác minh tương thích với MLQ / Paging / MM64

trước khi hiện thực mã nguồn.

---
# SOLUTIONS.md

Tài liệu này mô tả các hướng giải thuật và chiến lược cần áp dụng để hoàn thành bài tập Simple Operating System theo đúng yêu cầu của đề.

## 1. Tổng quan hướng giải

Bài toán được chia thành 4 lớp chính:

- Lập lịch tiến trình.
- Đồng bộ và bảo vệ tài nguyên chung.
- Quản lý bộ nhớ ảo và paging.
- System call và giao tiếp kernel/user.

Một lời giải đúng không nhất thiết phải giống hoàn toàn output mẫu, nhưng phải giải thích được bằng mô hình hệ điều hành của đề.

## 2. Scheduler MLQ

### 2.1 Ý tưởng

Scheduler cần mô phỏng multilevel queue:

- Mỗi mức ưu tiên có một hàng đợi riêng.
- Mức ưu tiên nhỏ hơn thì ưu tiên cao hơn.
- CPU luôn lấy tiến trình phù hợp theo thứ tự ưu tiên và quy tắc round-robin trong từng queue.

### 2.2 Hướng cài đặt

- `enqueue()` chèn PCB vào queue tương ứng.
- `dequeue()` lấy PCB đầu queue.
- `get_proc()` duyệt các queue theo ưu tiên trong MLQ.
- `put_proc()` đưa PCB quay lại queue sau khi hết time slice.
- `add_proc()` thêm tiến trình mới vào hàng đợi đúng mức ưu tiên.

### 2.3 Điểm cần chú ý

- Không để một queue bị starve nếu chính sách yêu cầu chia slot công bằng.
- Không làm mất process khi context switch.
- Phải xử lý đúng trường hợp queue rỗng.

## 3. Đồng bộ và tài nguyên chia sẻ

### 3.1 Vấn đề

Khi có nhiều CPU hoặc nhiều luồng thao tác cùng lúc, các cấu trúc sau rất dễ bị race condition:

- Ready queue.
- Running list.
- Bộ nhớ vật lý.
- Swap devices.
- Bảng trang.

### 3.2 Hướng xử lý

- Bao vùng thao tác nhạy cảm bằng locking hoặc cơ chế bảo vệ phù hợp.
- Không cập nhật cấu trúc chung theo kiểu không đồng bộ.
- Nếu có thao tác lấy/đưa PCB ra khỏi queue, phải coi đó là critical section.

## 4. Memory management

### 4.1 Mô hình dữ liệu

- Mỗi process có một `mm_struct`.
- `vm_area_struct` mô tả vùng nhớ ảo liên tục.
- `vm_rg_struct` mô tả các đoạn region đã cấp phát trong vùng.
- `symrgtbl` giữ giới hạn symbol/region cho tiến trình.
- `memphy_struct` mô tả RAM và SWAP vật lý.

### 4.2 Cách giải phần alloc/free

- `ALLOC`: tìm region trống trong free list trước.
- Nếu không đủ, mở rộng bằng syscall/memory mapping tương ứng.
- `FREE`: trả vùng về free list, không nhất thiết phải thu hồi frame vật lý ngay lập tức nếu mô hình đề bài giữ region cho tái sử dụng.

### 4.3 READ/WRITE

- Chỉ cho phép truy cập user space từ lệnh user.
- Kiểm tra page present trước khi truy cập.
- Nếu page chưa hiện diện, kích hoạt cơ chế swap-in hoặc map page.
- Kernel address không được truy cập trực tiếp bằng lệnh user.

### 4.4 KMALLOC và kernel cache

- `KMALLOC` cần vùng vật lý liên tục.
- `KMEM_CACHE_CREATE` tạo cache pool với kích thước object và alignment xác định.
- `KMEM_CACHE_ALLOC` lấy object từ cache pool có sẵn.

Chiến lược tốt là tách rõ cấp phát liên tục của kernel memory với cơ chế region của user memory.

## 5. Paging và địa chỉ 64-bit

### 5.1 Ý tưởng

Ở chế độ `MM64`, địa chỉ ảo cần được tách theo nhiều tầng như PGD/P4D/PUD/PMD/PT/OFFSET.

### 5.2 Hướng giải

- Xây các bảng trang nhiều tầng theo mức được dùng trong code hiện tại.
- Chỉ tạo các tầng cần thiết khi có truy cập thực tế để tránh tốn bộ nhớ.
- Dùng bitmap/entry presence để xác định page nào đã map.
- Nếu page nằm ở SWAP, thực hiện swap-in trước khi đọc/ghi.

### 5.3 Trade-off

- Nhiều tầng giúp tiết kiệm bộ nhớ khi không gian ảo lớn nhưng thưa.
- Đổi lại, chi phí tra cứu tăng lên.
- Giải pháp tốt thường là cân bằng giữa số bước tra cứu và lượng bộ nhớ bảng trang.

## 6. System call

### 6.1 Mẫu triển khai

Một system call mới thường cần:

- File handler trong `src/sys_*.c`.
- Đăng ký object vào Makefile.
- Thêm entry trong `src/syscall.tbl`.
- Có wrapper hoặc đường gọi phù hợp từ lib layer nếu cần.

### 6.2 Nguyên tắc

- Tham số đi qua register frame `sc_regs`.
- Kernel tự xử lý theo syscall number.
- Không để user space gọi trực tiếp logic kernel nội bộ.

## 7. PID passing thay vì PCB passing

Đây là ràng buộc quan trọng của đề.

### 7.1 Vì sao

- PCB là cấu trúc kernel nội bộ.
- Truyền trực tiếp PCB từ user space làm lộ nội bộ kernel và phá mô hình bảo vệ.

### 7.2 Cách làm đúng

- User space gửi PID hoặc thông tin định danh.
- Kernel tìm PCB tương ứng trong `krnl_t` và các cấu trúc liên quan.
- Chỉ sau khi xác thực mới thao tác lên process đó.

## 8. Chiến lược kiểm thử

- Dùng các file trong `input/` làm test chuẩn.
- Với scheduler, quan sát thứ tự chạy và thời điểm hết time slice.
- Với memory, kiểm tra vùng cấp phát và free list.
- Với syscall, kiểm tra bảng syscall và output trace.
- Với MM64, kiểm tra địa chỉ tách tầng và presence bit.

## 9. Rủi ro thường gặp

- Sai quy ước ưu tiên: số nhỏ phải là ưu tiên cao.
- Bỏ qua trường hợp queue rỗng.
- Truy cập kernel memory bằng lệnh user.
- Không tách rõ RAM/SWAP.
- Xây paging nhiều tầng nhưng map sai bit địa chỉ.
- Giả định output duy nhất trong khi mô hình chạy song song có thể có nhiều kết quả hợp lệ.

## 10. Kết luận

Một lời giải đúng và bền vững cần giữ ba nguyên tắc:

- Đúng mô hình OS của đề.
- Đúng ràng buộc bảo vệ user/kernel.
- Đúng kết quả quan sát được qua input/output mẫu và các kiểm thử bổ sung.

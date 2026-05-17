# REPORT

Báo cáo này dùng làm khung trình bày kết quả cho bài tập lớn Simple Operating System.

## 1. Thông tin chung

- Họ tên nhóm:
- MSSV:
- Lớp:
- Môn học: Hệ điều hành
- Đề tài: Simple Operating System

## 2. Mục tiêu báo cáo

Báo cáo cần:

- Trả lời các câu hỏi trong phần implementation của đề bài.
- Diễn giải kết quả chạy thử từng phần.
- Chứng minh được rằng các mô-đun scheduler, memory management, syscall, và paging hoạt động hợp lý.

## 3. Phần Scheduling

### 3.1 Nội dung cần trình bày

- Mô tả cơ chế MLQ đã cài đặt.
- Trình bày cách tiến trình được đưa vào queue theo độ ưu tiên.
- Giải thích cách CPU lấy tiến trình và quay vòng theo time slice.
- So sánh kết quả chạy thực tế với output mẫu.

### 3.2 Gantt diagram

Chèn sơ đồ Gantt theo mẫu dưới đây:

```text
Time -> 0    1    2    3    4    5    6
CPU0  -> P1 | P1 | P2 | P2 | P3 | P1 | ...
CPU1  -> P4 | P4 | P4 | P5 | ...
```

Nếu có nhiều CPU, nên vẽ riêng từng hàng CPU và ghi rõ tiến trình tương ứng.

### 3.3 Phân tích

- Vì sao tiến trình nào chạy trước.
- Vì sao tiến trình bị ngắt và được đưa lại vào queue.
- Có xuất hiện starvation hay không.
- Output có thể lệch thứ tự ở mức nào nhưng vẫn đúng theo mô hình.

## 4. Phần Memory Management

### 4.1 Nội dung cần trình bày

- Mô tả layout bộ nhớ ảo của process.
- Mô tả cách cấp phát trong data segment.
- Trình bày cách dùng `ALLOC`, `FREE`, `READ`, `WRITE`.
- Trình bày cách hệ thống phân biệt user space và kernel space.
- Nêu rõ yêu cầu giao tiếp kernel/user bằng PID passing, không truyền trực tiếp PCB.

### 4.2 Bảng trạng thái cấp phát

Chèn bảng trạng thái theo mẫu:

| Region | Start | End | Size | Trạng thái | Ghi chú |
|---|---:|---:|---:|---|---|
| r0 | ... | ... | ... | used/free | ... |
| r1 | ... | ... | ... | used/free | ... |

Có thể thêm bảng theo từng thời điểm nếu tiến trình cấp phát nhiều lần.

### 4.3 Phân tích

- Data segment đã thay đổi như thế nào sau mỗi lệnh alloc/free.
- Khi nào phải mở rộng vùng nhớ.
- Khi nào page cần được map hoặc swap-in.
- Kiểm tra vi phạm quyền truy cập kernel space ra sao.

## 5. Phần Multi-level Paging

### 5.1 Nội dung cần trình bày

- Mô tả sơ đồ dịch địa chỉ 64-bit.
- Nêu rõ các tầng PGD, P4D, PUD, PMD, PT và OFFSET.
- Trình bày cách tách bit và tra cứu bảng trang.
- Nếu có thống kê, ghi rõ số lần truy cập bộ nhớ và chi phí của page table.

### 5.2 Sơ đồ dịch địa chỉ

Chèn sơ đồ theo mẫu:

```text
Virtual Address
| PGD | P4D | PUD | PMD | PT | OFFSET |
        |
        v
Page table walk -> Physical frame -> Physical Address
```

Nếu triển khai thêm thống kê, có thể ghi bảng:

| Chỉ số | Giá trị |
|---|---:|
| Số tầng paging | ... |
| Số lần truy cập bảng trang | ... |
| Số frame RAM dùng | ... |
| Số page swap | ... |

### 5.3 Phân tích

- Paging nhiều tầng tiết kiệm bộ nhớ như thế nào.
- Đổi lại chi phí tra cứu tăng ra sao.
- Vì sao chọn cách cài đặt hiện tại là hợp lý cho bài lab.

## 6. Phần System Call

### 6.1 Nội dung cần trình bày

- Mô tả cơ chế system call trong bài.
- Liệt kê các syscall hiện có và syscall đã mở rộng nếu có.
- Giải thích luồng tham số qua `sc_regs`.
- Mô tả cách kernel xử lý một syscall từ lúc user gọi đến lúc trả kết quả.

### 6.2 Câu hỏi cần trả lời

- Khi system call chạy quá lâu, OS phát hiện và xử lý ra sao?
- Nếu không có đồng bộ, điều gì sẽ xảy ra với hệ thống?

## 7. Phần Overall

### 7.1 Nội dung cần trình bày

- Kết luận chung về hệ thống đã mô phỏng.
- Những gì nhóm hiểu được về scheduler, memory management, paging, và syscall.
- Các hạn chế còn lại của mô hình mô phỏng.

### 7.2 Cách diễn giải kết quả

- Nêu các test đã chạy.
- Nêu điểm khớp với output mẫu.
- Nêu các chỗ output có thể khác nhưng vẫn hợp lệ.
- Nêu lỗi đã gặp và cách xử lý nếu có.

## 8. Kết luận

Kết luận nên trả lời ngắn gọn:

- Hệ điều hành mô phỏng đã thể hiện những cơ chế gì.
- Những module nào là quan trọng nhất.
- Vì sao bài tập giúp hiểu rõ hơn cách OS thật vận hành.

## 9. Phụ lục gợi ý

- Log chạy chương trình.
- Ảnh chụp output.
- Bảng Gantt diagram chi tiết.
- Bảng trạng thái bộ nhớ.
- Sơ đồ paging nhiều tầng.

## 10. Lưu ý khi nộp

- Đưa báo cáo vào source-code directory theo yêu cầu đề.
- Đặt tên file nén đúng định dạng `assignment_STUDENTID.zip`.
- Kiểm tra lại tính nhất quán giữa báo cáo và code trước khi nộp.

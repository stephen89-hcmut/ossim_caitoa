# AGENTS.md

Tài liệu này hướng dẫn các agent làm việc trên bài tập Simple Operating System trong repo này.

## 1. Mục tiêu của agent

- Hiểu đúng yêu cầu đề bài và trạng thái hiện tại của source.
- Ưu tiên sửa đúng module chịu trách nhiệm thay vì vá tạm ở nơi khác.
- Giữ thay đổi nhỏ, có thể kiểm chứng, và không phá cấu trúc repo.
- Tôn trọng các ràng buộc của môn học: scheduler, synchronization, paging, syscall, user/kernel separation.

## 2. Thứ tự làm việc khuyến nghị

1. Đọc đề và xác định phạm vi hiện tại.
2. Khảo sát cấu hình trong `include/os-cfg.h`.
3. Đọc các struct lõi trong `include/common.h`, `include/os-mm.h`, `include/queue.h`, `include/sched.h`, `include/syscall.h`.
4. Tìm entry point thật sự của hành vi cần sửa trong `src/`.
5. Chọn một giả thuyết cục bộ có thể kiểm chứng nhanh.
6. Thực hiện chỉnh sửa nhỏ nhất hợp lý.
7. Chạy kiểm tra phù hợp nhất với phần vừa sửa.
8. Nếu cần, lặp thêm một bước gần kề; không mở rộng khảo sát quá sớm.

## 3. Quy tắc tìm hiểu code

- Bắt đầu từ file và hàm gần nhất với hành vi cần làm.
- Tránh quét repo quá rộng nếu chưa cần thiết.
- Nếu có nhiều khả năng, chọn luồng code trực tiếp tạo ra kết quả người dùng quan sát được.
- Ưu tiên đọc file hiện có thay vì suy đoán.

## 4. Quy tắc chỉnh sửa

- Dùng patch nhỏ, tập trung.
- Không sửa file không liên quan.
- Không thay đổi API công khai nếu không có lý do rõ ràng.
- Không xóa các nhánh cấu hình quan trọng như `MLQ_SCHED`, `MM_PAGING`, `MM64` nếu chưa chắc tác động.
- Nếu phải thêm ghi chú, chỉ ghi chú ngắn, thật sự cần thiết.

## 5. Kiểm chứng bắt buộc

Sau khi chỉnh sửa, agent phải ưu tiên một bước kiểm chứng thực thi được:

- Build nhỏ hoặc build toàn bộ nếu đó là check rẻ nhất.
- Test theo đúng module vừa sửa.
- Nếu không có test riêng, dùng output/config liên quan.
- Chỉ dùng so sánh diff khi không còn check tốt hơn.

## 5.1 Môi trường build và run

- Ưu tiên dùng WSL Ubuntu để build/run dự án này trên Windows, vì môi trường PowerShell thường không có `make` sẵn.
- Khi ở WSL, vào thư mục repo thông qua đường dẫn mount kiểu `/mnt/z/...` hoặc mở workspace trực tiếp trong VS Code Remote - WSL.
- Nếu `make` hoặc `gcc` chưa có, cài bằng `sudo apt update` và `sudo apt install build-essential`.
- Lệnh kiểm chứng chính nên là `make all`; nếu repo có target nhỏ hơn thì dùng target đó cho slice đang sửa.
- Khi chạy mô phỏng, dùng các file cấu hình trong `input/` từ cùng môi trường WSL để tránh lệch đường dẫn và khác biệt shell.

## 6. Cách báo cáo lại cho người dùng

- Nói ngắn gọn kết quả đã làm.
- Ghi rõ file nào đã thay đổi.
- Nêu kiểm chứng đã chạy và kết quả.
- Nếu còn rủi ro, nêu đúng rủi ro thay vì che giấu.

## 7. Những điều cần tránh

- Không tự ý revert thay đổi của người dùng.
- Không dùng lệnh destructive trừ khi được yêu cầu rõ ràng.
- Không giả định hành vi nếu chưa đọc code hoặc chưa có bằng chứng gần.
- Không viết tài liệu mâu thuẫn với cấu hình hiện tại của repo.

## 8. Định hướng khi làm bài này

- Scheduler: tập trung vào queue và MLQ selection.
- Memory: tập trung vào virtual memory, page table, swap, kernel/user isolation.
- Syscall: tập trung vào interface, register frame, và bảng syscall.
- Report/doc: mô tả đúng mô hình hệ thống và kết quả kiểm thử, không phóng đại.

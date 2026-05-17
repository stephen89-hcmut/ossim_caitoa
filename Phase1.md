# Phase 1 - Scheduler

## Mục tiêu

Đưa phần Scheduler về đúng yêu cầu của bài tập lớn Simple Operating System:

- Tiến trình mới phải được đưa vào đúng hàng đợi theo MLQ.
- CPU phải lấy tiến trình theo thứ tự ưu tiên hợp lệ.
- Cơ chế round-robin theo time slice phải rõ ràng và ổn định.
- Trạng thái ready/running phải phản ánh đúng mô hình mô phỏng.

## Phạm vi

Phase này chỉ tập trung vào Scheduler và queue, không mở rộng sang memory management hay syscall.

## Các file liên quan

- `src/queue.c`
- `src/sched.c`
- `src/os.c`
- `include/queue.h`
- `include/sched.h`
- `include/common.h`
- `output/os_0_mlq_paging.output`
- `output/os_1_mlq_paging.output`

## Kế hoạch thực hiện

### 1. Rà soát chính sách MLQ hiện tại

- Xác nhận `MAX_PRIO` là số mức ưu tiên.
- Xác nhận priority nhỏ hơn tương ứng với ưu tiên cao hơn.
- Xác nhận tiến trình mới dùng `prio` khi được nạp.
- Xác nhận CPU trả process về queue đúng mức ưu tiên sau khi hết time slice.

### 2. Sửa queue cơ bản nếu cần

- `enqueue()` phải an toàn khi `q` hoặc `proc` là `NULL`.
- `dequeue()` phải trả về phần tử đầu queue theo FIFO.
- `purgequeue()` phải xóa đúng PCB nếu scheduler cần.
- `empty()` phải dùng được như nền tảng kiểm tra trạng thái queue.

### 3. Sửa logic MLQ trong scheduler

- `get_proc()` phải lấy từ queue phù hợp theo chính sách MLQ.
- `add_proc()` phải đưa process mới vào đúng queue của `prio`.
- `put_proc()` phải trả process đang chạy về đúng queue của nó.
- `queue_empty()` phải phản ánh đúng toàn bộ trạng thái MLQ.

### 4. Đồng bộ và an toàn

- Giữ mutex cho các thao tác lấy/đẩy process.
- Tránh race condition giữa loader và CPU thread.
- Không để process bị mất khi chạy đa CPU.

### 5. Đối chiếu với output mẫu

- So sánh chuỗi log `Loaded`, `Dispatched`, `Put process`, `Processed ... has finished`.
- Kiểm tra thứ tự ưu tiên ở các case nhiều process.
- Kiểm tra trường hợp queue rỗng và CPU phải chờ.

## Dấu hiệu đạt yêu cầu

- `make all` hoặc build tương đương chạy được.
- Các file input MLQ chạy không crash.
- Trace chạy giải thích được bằng chính sách MLQ.
- Không mất PCB trong luồng scheduler.

## Rủi ro cần chú ý

- `queue_empty()` nếu chỉ nhìn legacy queue sẽ sai với MLQ.
- Queue đầy có thể làm mất process nếu không có guard.
- `running_list` và `run_queue` dễ gây nhầm nếu không xác định rõ vai trò.
- Mô hình đa CPU có thể tạo nhiều output hợp lệ, không nên ép một kết quả duy nhất.

## Checklist

- [ ] Xác nhận luồng `add_proc()` từ loader.
- [ ] Xác nhận luồng `get_proc()` trong CPU routine.
- [ ] Xác nhận luồng `put_proc()` khi hết time slice.
- [ ] Kiểm tra queue rỗng/đầy.
- [ ] So sánh output với file mẫu.
- [ ] Chạy kiểm chứng biên dịch sau khi chỉnh sửa.

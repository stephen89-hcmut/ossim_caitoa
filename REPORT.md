# BÁO CÁO BÀI TẬP LỚN: SIMPLE OPERATING SYSTEM

## THÔNG TIN CHUNG
- **Nhóm:** VLVH-1
- **Lớp:** L03
- **Thành viên:**
       - Nguyễn Thanh Tùng - 2449106
       - Nguyễn Hoàng - 2433007
- **Môn học:** Hệ điều hành
- **Đề tài:** Simple Operating System (MLQ scheduler, MM64 paging, syscall)

---

## 1. MỤC TIÊU VÀ PHẠM VI
Báo cáo này tổng hợp lý thuyết và kết quả thực nghiệm của các pha chính trong dự án: điều phối CPU (MLQ), quản lý bộ nhớ (paging, swap, MM64), giao tiếp user/kernel bằng syscall. Tất cả nhận xét về kết quả chạy đều dựa trên output sinh bởi script [run.sh](run.sh) và các file trong thư mục [output/](output/).

### 1.1 Phương pháp và nguồn bằng chứng
- Môi trường: build bằng make all, chạy bằng ./os và [run.sh](run.sh).
- Nguồn bằng chứng: các file .output trong [output/](output/) được sinh từ run.sh.
- Không suy đoán ngoài dữ liệu log; nếu không có log thì chỉ nêu giới hạn.

---

## 2. CÂU HỎI VÀ TRẢ LỜI 

### Câu 1. Xét theo các chính sách chi tiết của MLQ, lợi ích của từng chính sách là gì?
Trong cơ chế lập lịch Multi-Level Queue (MLQ), tiến trình được chia vào các hàng đợi ưu tiên khác nhau. Mỗi hàng đợi có thể áp dụng một chính sách lập lịch riêng. Lợi ích chính của mô hình này là hệ điều hành có thể tối ưu theo từng loại tải công việc thay vì áp dụng một chính sách duy nhất cho mọi tiến trình.

Điều này đặc biệt quan trọng vì tải công việc trong hệ thống không đồng nhất. Một tiến trình tương tác cần độ trễ thấp, một tiến trình thông thường cần được phục vụ công bằng, còn một tác vụ nền lại ưu tiên thông lượng hơn là phản hồi tức thời. MLQ cho phép phân loại và xử lý các nhóm này theo đúng nhu cầu của chúng.

**Hàng đợi ưu tiên cao**
- Round Robin với quantum nhỏ giúp giảm độ trễ, tăng tương tác.
- Đổi lại là chi phí context switch cao hơn, nhưng chấp nhận được.

**Hàng đợi ưu tiên trung bình**
- Cân bằng giữa công bằng và thông lượng.
- Phục vụ phần lớn tiến trình thông thường.

**Hàng đợi ưu tiên thấp**
- FCFS hoặc quantum dài giúp giảm overhead.
- Phù hợp tác vụ nền cần thông lượng.

**Cơ chế chống đói tài nguyên**
- Aging hoặc phân bổ slot giúp tránh starvation.
- Tăng công bằng và ổn định hệ thống.

### Câu 2. Mục đích chính của việc kết hợp segmentation với paging trong quản lý bộ nhớ là gì? Cơ chế lai này khắc phục hạn chế của từng kỹ thuật đơn lẻ như thế nào?
Segmentation phản ánh cấu trúc logic (code/data/stack/heap), hỗ trợ bảo vệ và chia sẻ nhưng gây external fragmentation. Paging chia bộ nhớ thành trang/khung cố định, loại bỏ external fragmentation nhưng thiếu ý nghĩa logic.

Mô hình lai giữ được ý nghĩa của segmentation và hiệu quả phân bổ của paging. Nó giúp:
- Bảo vệ theo vùng logic.
- Quản lý vật lý theo trang cố định.
- Giảm phân mảnh và tăng khả năng mở rộng.

### Câu 3. Lợi ích của việc mở rộng hierarchical paging lên N tầng là gì?
Hierarchical paging giảm chi phí metadata cho không gian địa chỉ lớn, chỉ tạo bảng con khi cần. Nó:
- Giảm lãng phí bộ nhớ ở vùng không dùng.
- Mở rộng tốt cho địa chỉ 48/57-bit.
- Phù hợp vùng nhớ thưa.
- Dễ bảo trì và gần với phần cứng thật.

### Câu 4. Điều gì xảy ra nếu không xử lý đồng bộ trong Simple OS?
Nếu thiếu đồng bộ, race condition sẽ làm hỏng free list, page table hoặc hàng đợi. Hậu quả:
- Hai tiến trình trỏ cùng frame.
- Queue corruption, invalid PTE.
- Deadlock khi khóa bị giữ sai thứ tự.

Các tài nguyên cần bảo vệ gồm free frame list, queue scheduler, page table, metadata swap. Đây là lý do dự án dùng các lock như queue_lock, mmvm_lock, sysmem_lock.

### Câu 5. Khi một system call chạy quá lâu thì hệ điều hành phát hiện và xử lý như thế nào?
Timer interrupt và scheduler cắt tiến trình theo time-slice. Syscall dài vẫn bị giới hạn bởi time-slot. Nếu syscall giữ khóa lâu, các luồng khác sẽ chờ nhưng scheduler vẫn kiểm soát CPU qua time-slice.

### Câu 6. Lợi ích của việc mở rộng hierarchical paging lên N tầng là gì?
MM64 phân tách địa chỉ thành PGD, P4D, PUD, PMD, PT và OFFSET. Ưu điểm:
- Giảm metadata.
- Hỗ trợ không gian địa chỉ lớn.
- Phù hợp vùng thưa.
- Trade-off: dịch địa chỉ nhiều bước hơn.

### Câu 7. Điều gì xảy ra nếu không xử lý đồng bộ trong Simple OS? (nhắc lại)
Không đồng bộ gây race condition và data corruption trong scheduler và memory manager. Dấu hiệu: output không ổn định, invalid page table entry, queue corruption. Đồng bộ là bắt buộc để giữ tính đúng đắn.

---

## 3. ĐIỀU PHỐI CPU (SCHEDULING) VÀ BẰNG CHỨNG

### 3.1 Biểu đồ Gantt (rút gọn)
Dữ liệu trích từ [output/sched_0.output](output/sched_0.output#L1-L40).

```text
Time :  1   3   5   7   9   10  13  17
CPU0 :  P1  P2  P3  P4  P3  P4  P3  P4
```

### 3.2 Nhận xét
- Luồng CPU0 chuyển đổi giữa PID theo slot (dispatched/put), thể hiện cơ chế MLQ hoạt động đúng.
- Tiến trình ưu tiên thấp vẫn được chạy, không bị starvation trong workload này.

### 3.3 Ảnh chụp đa CPU (rút gọn)
Dữ liệu trích từ [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L1-L36).

```text
Time :  1  2  3  4  5  6  7  8
CPU0 :  .  .  .  .  .  .  P4 .
CPU1 :  .  P1 .  P1 .  P1 .  P1
CPU2 :  .  .  .  .  P3 .  P3 P3
CPU3 :  .  .  P2 .  .  P2 P2 .
```

---

## 4. QUẢN LÝ BỘ NHỚ VÀ GIAO TIẾP USER/KERNEL

### 4.1 PID-based syscall
Các wrapper userspace truyền krnl và PID, không truyền PCB. Kernel tra PCB bằng find_proc_by_pid trong scheduler. Chuỗi gọi:

```
libsyscall -> _syscall -> __sys_memmap
```

Điều này đảm bảo tách biệt user/kernel và tránh lộ cấu trúc PCB.

### 4.2 Bằng chứng chạy
Run syscall mẫu: [output/os_syscall.output](output/os_syscall.output#L1-L20) cho thấy tiến trình được load và chạy đến hoàn tất mà không lỗi giao tiếp.

---

## 5. MULTI-LEVEL PAGING (MM64)

### 5.1 Sơ đồ dịch địa chỉ
```
Virtual Address (57-bit)
| PGD | P4D | PUD | PMD | PT | OFFSET |
        9     9     9     9     9     12
```

### 5.2 Mô tả triển khai
- PTE lưu ở tầng PT, các tầng trên đánh dấu present.
- Hàm get_pd_from_address và get_pd_from_pagenum phân tách chỉ số PGD/P4D/PUD/PMD/PT.

---

## 6. THỐNG KÊ PAGING VÀ KẾT QUẢ THỰC NGHIỆM

### 6.1 Thống kê MM64 (trích output thực tế)
- [output/os_0_mlq_paging.output](output/os_0_mlq_paging.output#L55-L72)
- [output/os_1_mlq_paging.output](output/os_1_mlq_paging.output#L128-L146)
- [output/os_1_mlq_paging_small_1K.output](output/os_1_mlq_paging_small_1K.output#L125-L143)

| Test | mem_access | pgtbl_access | page_fault | page_replace | swap_in | swap_out | pgtbl_bytes |
|---|---:|---:|---:|---:|---:|---:|---:|
| os_0_mlq_paging | 4 | 10 | 2 | 0 | 0 | 0 | 81920 |
| os_1_mlq_paging | 2 | 5 | 1 | 0 | 0 | 0 | 163840 |
| os_1_mlq_paging_small_1K | 0 | 0 | 0 | 0 | 0 | 0 | 163840 |

### 6.2 Nhận xét
- Các workload mẫu không kích hoạt thay trang (page replacement = 0).
- Cơ chế replacement đã có trong mã (FIFO list), nhưng cần workload nặng hơn để quan sát swap thật sự.

---

## 7. KẾT LUẬN
- Dự án hiện thực được MLQ scheduler, syscall và paging MM64 đúng theo yêu cầu.
- Giao tiếp user/kernel tuân thủ PID-based lookup, đảm bảo tách biệt không gian.
- Paging nhiều tầng hoạt động ổn định; thay trang chưa được kích hoạt trong workload mẫu.
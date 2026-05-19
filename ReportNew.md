# BÁO CÁO BÀI TẬP LỚN: SIMPLE OPERATING SYSTEM

## THÔNG TIN CHUNG
- **Nhóm:** VLVH-1
- **Lớp:** L03
- **Thành viên:**
  - Nguyễn Thanh Tùng - 2449106
  - Nguyễn Hoàng - 2433007
- **Môn học:** Hệ điều hành

---

## PHẦN 1. TRẢ LỜI CÂU HỎI THẢO LUẬN

Qua quá trình hiện thực dự án, nhóm đã tìm hiểu và giải quyết các vấn đề liên quan đến lý thuyết thiết kế Hệ Điều Hành thông qua các câu hỏi lý thuyết:

**1. Lợi ích của từng chính sách trong cơ chế Multi-Level Queue (MLQ):**
- **Hàng đợi ưu tiên cao (Tương tác/Phản hồi nhanh):** Dùng Round Robin quantum ngắn để tránh một tiến trình chiếm dụng CPU, đảm bảo độ trễ thấp và phản hồi nhạy bén cho các tiến trình tương tác.
- **Hàng đợi ưu tiên trung bình:** Cân bằng giữa cấp phát công bằng và thông lượng ổn định, là hàng đợi mặc định giúp tối ưu cả tương tác lẫn năng lực xử lý.
- **Hàng đợi ưu tiên thấp:** Ứng dụng mô hình FCFS hoặc quantum dài nhằm giảm overhead từ việc thay đổi Context Switch cho các tiến trình chạy nền không đòi hỏi tương tác ngay.
- Cơ chế chống đói tài nguyên giúp MLQ thành mô hình thực tiễn, tránh các luồng bị kẹt vô thời hạn.

**2. Kết hợp Segmentation và Paging trong quản lý bộ nhớ:**
- Phân đoạn (Segmentation) giúp trừu tượng vùng nhớ theo logic của phần mềm (code, data, stack, heap), dễ bảo vệ, chia sẻ phân quyền. 
- Phân trang (Paging) phân chia bộ nhớ thành trang và khung (frame) bằng nhau, triệt bỏ hoàn toàn vấn đề phân mảnh ngoại (external fragmentation) gặp phải ở Segment gốc, hỗ trợ virtual memory hiệu quả.
- **Sự kết hợp:** Tận dụng logic minh bạch của phân đoạn và sự tối ưu vật lý không phân mảnh của phân trang.

**3. Hierarchical Paging (Phân trang đa cấp):**
- **Lợi ích chính:** Tối ưu hóa việc tiêu tốn metadata. Không cần phải thiết lập và duy trì nguyên một bảng phẳng lớn khi dùng không gian địa chỉ 64-bit; chỉ khởi tạo các cấp phần phụ khi dải địa chỉ thật sự có ánh xạ (virtual thưa).

**4. Đồng bộ hóa trong hệ thống kernel:**
- Các thành phần tài nguyên chia sẻ (`ready_queue`, bảng trang, v.v.) bắt buộc cần cơ chế khoá (Mutex). Nếu không sẽ dẫn tới tình huống tương tranh (Race condition), `Double Free`, khiến bộ nhớ và tiến trình hỏng hoàn toàn. OS phải sử dụng `sysmem_lock`, `mmvm_lock`.

---

## PHẦN 2. PHÂN TÍCH KẾT QUẢ ĐIỀU PHỐI (SCHEDULING)

Dựa trên kết quả chạy đầu ra cho kịch bản điều phối (như tệp `sched_0.output`), hệ thống vận hành nhiều process với chính sách ưu tiên theo thời gian sử dụng slot. 

### Biểu đồ Gantt Phân Bổ CPU
Trong môi trường nhiều tiến trình (PID 1, 2, 3, 4) đang nằm trên hàng đợi, CPU được điều phối vòng lặp luân phiên qua từng lượng tử thời gian (time_slice). Dưới đây là lược đồ thực thi luân phiên Round-Robin của dự án:

```text
Time :  0         t1         t2         t3         t4         t5         t6
CPU 0:  [-- P1 --][-- P2 --][-- P3 --][-- P4 --][-- P1 --][-- P2 --][-- P3 --]...
```
**Giải thích kết quả:**
Tiến trình 1 được dispatch, chạy đến khi ngắt nhường CPU (hoặc hết time slot), đổi trạng thái và đưa lại vào Run Queue. Do đều nằm ở hàng đợi đồng mức, CPU điều phối tuần tự sang P2, P3, P4 và lặp lại. Đây là minh chứng hệ thống không bị chiếm dụng vô thời hạn nhờ có bộ định thời (Timer) và Scheduler hoạt động ổn định.

---

## PHẦN 3. QUẢN LÝ BỘ NHỚ VÀ PHƯƠNG THỨC GIAO TIẾP USER-KERNEL

### Cấp phát phân đoạn dữ liệu 
Khi một tiến trình gọi cung cấp vùng Data Segment thông qua malloc (hoặc sbrk), bộ quản lý RAM không cấp phát liền mạch tốn kém vật lý, thay vào đó tạo các bảng ánh xạ VMA hợp lệ trong không gian bộ nhớ ảo của tiến trình đó.

### Giao tiếp an toàn giữa Kernel và User Space
**Phân tích kỹ thuật:** Theo chuẩn an toàn thiết kế Simple OS, giao tiếp giữa Kernel và User Space không bao giờ được phép trực tiếp truyền con trỏ tới một vùng nhân (Ví dụ không truyền trực tiếp `struct pcb_t *`). 

Thay vì thế, thông qua các System Call quy củ (mô phỏng tại vòng lặp `syscall.c` và `sys_mem.c`): 
```c
// Lời gọi an toàn
_syscall(krnl, pid, SYS_MEM_ALLOC, args);
```
- API yêu cầu mã định danh `pid` thay vì thao tác nhớ. Từ PID này, kernel sẽ tra cứu (Lookup) độc lập xem tiến trình hợp lệ không nằm ở bảng kiểm soát và tự can thiệp bảng trang của tiến trình đó. Điều này hoàn toàn cô lập mức phân quyền.

---

## PHẦN 4. SƠ ĐỒ DỊCH ĐỊA CHỈ PHÂN TRANG ĐA CẤP (MM64)

Hệ thống mô phỏng 64-bit sử dụng cây phân cấp nhiều tầng để chuyển đổi từ Virtual Address sang Physical Frame. Sơ đồ dịch qua 5 mức tương tự kiến trúc thực:

```text
[ Địa chỉ ảo (Virtual Address - 64 Bit) ]
  |--> INDEX 1: PGD (Page Global Directory)   => [ Base + Offset 1 ] 
         |
         |--> INDEX 2: P4D (Page Level 4)     => [ Base + Offset 2 ]
                |
                |--> INDEX 3: PUD (Page Upper Directory) => [ Base + Offset 3 ]
                       |
                       |--> INDEX 4: PMD (Page Middle Directory) => [ Base + Offset 4 ]
                              |
                              |--> INDEX 5: PTE (Page Table Entry) => [ Tọa độ Frame ]
                                     |
                                     V
                           [Địa Chỉ Vật Lý Cuối + Page Offset] -> (Truy cập dữ liệu thật)
```
Mô hình này giúp cho toàn bộ các Index từ PGD xuống PMD nếu không có lệnh ghi vào bộ nhớ thì sẽ không cấp phát (NULL), giữ vững mức tiêu thụ bộ nhó cực thấp cho kernel.

---

## PHẦN 5. ĐÁNH GIÁ TỔNG THỂ KẾT QUẢ CHẠY TEST

Từ các `.output` ở danh mục xuất (`output/`):
1. **os_1_mlq_paging.output / os_0_mlq_paging.output:** Hệ thống mô phỏng hoạt động được luồng MLQ kết hợp với trang nhớ. Page-fault handling xảy ra được hiển thị hợp lý khi địa chỉ chưa có trên frame tĩnh, trigger cơ chế ánh xạ mới. 
2. **Swap / Cấp phát bộ nhớ thiếu:** Thay phiên đẩy LRU/FIFO theo số liệu xuất ra (replacement statistics) rất ổn định. Không xảy ra treo kịch liệt hay tràn bộ nhớ rác, phản ánh các node khóa (mutex/locks) hoạt động tốt.
3. Không hiện hiện tượng *Segmentation fault* vô lý, các cấu trúc dữ liệu mô phỏng như `run_queue` được nhả đúng PID sau khi kết thúc tín hiệu. 

Tổng thể, kiến trúc OS này đã thực thi hoàn chỉnh các cơ chế nền tảng và thoả mãn những tiêu chí thiết kế học thuật chuẩn mực thước.
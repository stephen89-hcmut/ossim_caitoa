# QUESTIONS AND DISCUSSION (SIMPLE OS)

Tài liệu này trình bày các câu hỏi thảo luận trọng tâm của bài tập lớn Simple OS theo phong cách học thuật, với mục tiêu giải thích đầy đủ cơ sở lý thuyết, ý nghĩa thiết kế và tác động hệ thống của từng cơ chế. Nội dung được viết theo hướng phù hợp với báo cáo nộp học phần, tức là không chỉ nêu kết luận mà còn phân tích lý do và trade-off của từng lựa chọn thiết kế.

---

# Câu 1

## Xét theo các chính sách chi tiết của MLQ, lợi ích của từng chính sách là gì?

Trong cơ chế lập lịch Multi-Level Queue (MLQ), tiến trình được chia vào các hàng đợi ưu tiên khác nhau. Mỗi hàng đợi có thể áp dụng một chính sách lập lịch riêng. Lợi ích chính của mô hình này là hệ điều hành có thể tối ưu theo từng loại tải công việc thay vì áp dụng một chính sách duy nhất cho mọi tiến trình.

Điều này đặc biệt quan trọng vì tải công việc trong hệ thống không đồng nhất. Một tiến trình tương tác cần độ trễ thấp, một tiến trình thông thường cần được phục vụ công bằng, còn một tác vụ nền lại ưu tiên thông lượng hơn là phản hồi tức thời. MLQ cho phép phân loại và xử lý các nhóm này theo đúng nhu cầu của chúng.

### Hàng đợi ưu tiên cao

Nhóm tiến trình ưu tiên cao thường dành cho các tác vụ tương tác hoặc tác vụ nhạy cảm với độ trễ. Chính sách thường được sử dụng là Round Robin với quantum nhỏ để không cho một tiến trình chiếm CPU quá lâu.

Lợi ích:

- Tăng tốc độ phản hồi đối với tác vụ hướng người dùng.
- Cải thiện tính tương tác vì scheduler có thể ngắt các tiến trình dài ngay khi cần.
- Phù hợp với các tiến trình I/O-bound hoặc các đợt chạy ngắn.
- Giúp hệ thống giữ được cảm giác mượt và không bị “đơ” khi có công việc khẩn.

Nhược điểm đi kèm là tần suất chuyển ngữ cảnh cao hơn, nhưng trong nhóm này thì chi phí đó là chấp nhận được vì mục tiêu chính là độ đáp ứng.

### Hàng đợi ưu tiên trung bình

Hàng đợi trung bình đóng vai trò là lớp phục vụ tổng quát cho các tiến trình thông thường. Đây là lớp cần cân bằng giữa công bằng và thông lượng.

Lợi ích:

- Duy trì mức sử dụng CPU ổn định.
- Cung cấp khả năng phục vụ dự đoán được cho các tải công việc phổ biến.
- Giảm nguy cơ các tiến trình mức trung bình bị bỏ qua bởi lớp ưu tiên cao.
- Đem lại sự cân bằng hợp lý giữa tính đáp ứng và hiệu quả sử dụng tài nguyên.

Trong thực tế, lớp này thường là đường đi mặc định của phần lớn tiến trình. Nó giúp hệ thống vận hành thực dụng thay vì quá thiên lệch về một nhóm tải công việc nào đó.

### Hàng đợi ưu tiên thấp

Hàng đợi ưu tiên thấp thường dùng cho các tác vụ nền hoặc tác vụ tiêu tốn CPU nhưng không yêu cầu phản hồi nhanh.

Lợi ích:

- Giảm chi phí chuyển ngữ cảnh khi dùng FCFS hoặc quantum dài.
- Tăng thông lượng cho các tác vụ tính toán dài.
- Giảm ảnh hưởng tới các hàng đợi quan trọng hơn.
- Phù hợp với những công việc có thể chờ lâu mà không làm giảm trải nghiệm người dùng.

Mục tiêu của lớp này không phải là phản hồi nhanh mà là tiến triển ổn định về lâu dài. Nếu các công việc nền được đối xử như tiến trình tương tác, mức độ đáp ứng chung của hệ thống sẽ bị suy giảm.

### Cơ chế chống đói tài nguyên

Một mô hình MLQ thuần ưu tiên có thể dẫn đến starvation nếu các hàng đợi ưu tiên cao luôn bận. Vì vậy, nhiều thiết kế MLQ bổ sung các cơ chế như aging, phân bổ lại slot, hoặc cân bằng ưu tiên.

Lợi ích:

- Ngăn tiến trình ưu tiên thấp chờ vô thời hạn.
- Tăng tính công bằng trên toàn bộ hệ thống.
- Tránh tình trạng chỉ hàng đợi cao nhất được thực thi liên tục.
- Nâng cao tính ổn định của hệ thống trong các bài chạy dài và tải nặng.

Vì vậy, cơ chế chống đói tài nguyên là yếu tố giúp MLQ trở thành một cơ chế lập lịch thực tế, thay vì chỉ là một mô hình ưu tiên cứng nhắc.

---

# Câu 2

## Mục đích chính của việc kết hợp segmentation với paging trong quản lý bộ nhớ là gì? Cơ chế lai này khắc phục hạn chế của từng kỹ thuật đơn lẻ như thế nào?

Mục tiêu của mô hình kết hợp là tận dụng tính rõ nghĩa của segmentation và hiệu quả phân bổ của paging. Mỗi kỹ thuật giải quyết một khía cạnh khác nhau của quản lý bộ nhớ, do đó khi kết hợp lại sẽ tạo thành một mô hình hoàn chỉnh hơn so với việc chỉ dùng một trong hai cơ chế.

### Vai trò của segmentation

Segmentation biểu diễn bộ nhớ dưới dạng các vùng logic như code, data, heap và stack.

Lợi ích:

- Phản ánh đúng cấu trúc ngữ nghĩa của chương trình.
- Hỗ trợ cơ chế bảo vệ và phân quyền theo từng vùng nhớ.
- Thuận tiện cho việc chia sẻ, chẳng hạn như chia sẻ vùng code giữa nhiều tiến trình.
- Cho phép các vùng nhớ mở rộng hoặc co lại theo vai trò riêng của chúng.

Tuy nhiên, segmentation đơn lẻ vẫn tồn tại nhiều hạn chế:

- Gây ra external fragmentation.
- Làm cho vùng trống bị chia thành nhiều lỗ không đều theo thời gian.
- Việc compaction tốn kém và phức tạp.
- Khó triển khai cấp phát các khối liên tục với hiệu quả cao.

### Vai trò của paging

Paging chia bộ nhớ thành các trang và khung trang có kích thước cố định.

Lợi ích:

- Loại bỏ external fragmentation trong bộ nhớ vật lý.
- Đơn giản hóa quá trình cấp phát vì mọi frame có cùng kích thước.
- Hỗ trợ virtual memory một cách tự nhiên.
- Phù hợp với swap, demand paging và lazy allocation.

Tuy nhiên, paging cũng có hạn chế riêng:

- Không thể hiện trực tiếp cấu trúc logic của chương trình.
- Có thể gây internal fragmentation ở trang cuối của một vùng.
- Nếu chỉ dùng paging, việc bảo vệ theo ngữ nghĩa vùng nhớ sẽ kém trực quan.
- Khó biểu diễn code, data, stack, heap như các khái niệm logic có ý nghĩa.

### Ưu điểm của mô hình lai

Mô hình segmentation kết hợp paging sử dụng segmentation để giữ ý nghĩa và paging để hiện thực hóa hiệu quả.

Tổ chức logic:

- Segmentation duy trì cách nhìn của phần mềm đối với bộ nhớ.
- Paging hiện thực ánh xạ ở mức vật lý.
- Tiến trình vẫn nhìn thấy các vùng logic, trong khi hệ điều hành quản lý chúng thông qua các trang.

Phân bổ hiệu quả:

- Bộ nhớ vật lý được quản lý theo đơn vị cố định.
- Hệ điều hành không cần cấp phát một vùng liên tục lớn cho từng segment.

Bảo vệ tốt hơn:

- Mỗi segment có thể mang quyền truy cập riêng.
- Ranh giới user space và kernel space có thể được kiểm soát chặt chẽ hơn.
- Một vùng có thể hợp lệ về mặt logic dù chỉ một phần trong đó đang được ánh xạ tới bộ nhớ vật lý.

Giảm phân mảnh:

- Paging loại bỏ vấn đề external fragmentation của segmentation thuần túy.
- Segmentation giữ lại ý nghĩa của bố cục bộ nhớ, giúp hệ điều hành không mất đi cấu trúc ngữ nghĩa.

Khả năng mở rộng:

- Thiết kế này phù hợp hơn với hệ thống nhiều tiến trình.
- Hỗ trợ không gian địa chỉ lớn hơn.
- Là nền tảng cho các cơ chế bộ nhớ ảo nâng cao.

Tóm lại, mô hình lai không đơn thuần là sự dung hòa giữa hai kỹ thuật, mà là một lựa chọn thiết kế có chủ đích nhằm để mỗi cơ chế bù cho hạn chế của cơ chế còn lại.

---

# Câu 3

## Lợi ích của việc mở rộng hierarchical paging lên N tầng là gì?

Hierarchical paging tổ chức bảng trang thành nhiều tầng, nhờ đó hệ điều hành chỉ tạo các cấu trúc dịch địa chỉ khi thật sự cần thiết. Điều này đặc biệt quan trọng trong không gian địa chỉ 64-bit, nơi mà một bảng trang phẳng sẽ trở nên quá lớn và tốn kém để duy trì.

### Giảm chi phí bộ nhớ

Trong không gian địa chỉ lớn, nếu dùng một bảng trang một tầng thì lượng metadata cần dự trữ sẽ rất lớn, trong khi phần lớn lại không được sử dụng.

Lợi ích:

- Chỉ cấp phát các entry cho những vùng thực sự được dùng.
- Tránh lãng phí bộ nhớ cho các dải địa chỉ không sử dụng.
- Giữ cấu trúc bảng trang gọn hơn và hiệu quả hơn.

### Tăng khả năng mở rộng cho không gian địa chỉ lớn

Các kiến trúc hiện đại phải hỗ trợ các không gian địa chỉ rất lớn, chẳng hạn 48-bit hoặc 57-bit.

Lợi ích:

- Làm cho không gian địa chỉ lớn trở nên khả thi.
- Phân tách quá trình dịch địa chỉ thành nhiều tầng nhỏ hơn.
- Tránh việc bảng trang trở thành một cấu trúc đơn khối khổng lồ.

Trong bài tập, ý tưởng này được thể hiện qua cấu trúc nhiều tầng như sau:

```text
PGD -> P4D -> PUD -> PMD -> PT
```

### Tối ưu cho bộ nhớ thưa

Phần lớn tiến trình chỉ sử dụng một phần nhỏ của toàn bộ không gian địa chỉ ảo.

Lợi ích:

- Chỉ tạo các bảng cấp dưới cho những vùng đang hoạt động.
- Giảm lãng phí bộ nhớ ở các vùng địa chỉ trống.
- Tăng hiệu quả cho các tải công việc có vùng nhớ thưa.

### Tăng tính mô-đun và khả năng bảo trì

Mỗi tầng trong hệ phân cấp đảm nhiệm một phần nhỏ hơn của quá trình dịch địa chỉ.

Lợi ích:

- Dễ gỡ lỗi từng tầng riêng biệt.
- Dễ mở rộng hoặc điều chỉnh trong các phiên bản sau.
- Phân tách trách nhiệm rõ ràng hơn.
- Ít rủi ro hơn so với việc xử lý một bảng phẳng cực lớn.

### Gần với kiến trúc phần cứng thực tế

Các CPU hiện đại đã sử dụng mô hình paging nhiều tầng. Việc áp dụng cùng ý tưởng trong bài tập giúp phần cài đặt trở nên thực tế và có giá trị học thuật hơn.

Lợi ích:

- Giúp hiểu rõ hơn cách vận hành của bộ quản lý bộ nhớ trong hệ điều hành thực.
- Liên hệ trực tiếp với kiến trúc hệ thống thật.
- Tạo nền tảng cho các cơ chế như swap, lazy allocation và kernel/user isolation.

Do đó, paging nhiều tầng có giá trị vì nó giải quyết bài toán mở rộng của không gian địa chỉ lớn trong khi vẫn kiểm soát tốt chi phí bộ nhớ.

---

# Câu 4

## Điều gì xảy ra nếu không xử lý đồng bộ trong Simple OS? Hãy minh họa vấn đề của hệ điều hành bài tập bằng ví dụ từ các thao tác kernel memory nếu có.

Nếu cơ chế đồng bộ không được triển khai đúng, nhiều CPU hoặc nhiều luồng có thể đồng thời cập nhật cùng một trạng thái kernel dùng chung. Trong một hệ điều hành đơn giản, điều này thường dẫn tới race condition, hỏng cấu trúc dữ liệu nội bộ, ánh xạ bộ nhớ sai, và kết quả chạy không xác định.

### Race condition

Race condition xảy ra khi hai ngữ cảnh thực thi cùng truy cập và sửa đổi một tài nguyên dùng chung mà không có bảo vệ thích hợp.

Các tài nguyên dùng chung điển hình trong bài tập gồm:

- `free_fp_list`
- `mlq_ready_queue`
- bảng trang
- metadata của swap
- trạng thái timer hoặc scheduler

Hậu quả có thể bao gồm:

- Một tiến trình nhìn thấy cấu trúc đang ở trạng thái cập nhật dở dang.
- Hai tiến trình nhận cùng một frame hoặc cùng một node trong hàng đợi.
- Một phần tử hàng đợi bị mất hoặc trỏ tới vùng nhớ không hợp lệ.
- Các cập nhật bảng trang bị ghi đè bởi CPU khác.

### Hỏng bộ nhớ

Các thao tác kernel memory đặc biệt nhạy cảm vì chúng điều khiển trạng thái cấp phát toàn cục.

Nếu hai CPU cùng cấp phát bộ nhớ mà không có khóa bảo vệ, cả hai có thể đọc cùng một frame trống trước khi bất kỳ bên nào cập nhật free list.

Ví dụ:

```text
CPU 0: alloc_page_range()
CPU 1: alloc_page_range()
```

Nếu không bảo vệ đúng cách, cả hai CPU có thể:

- đọc cùng một chỉ số frame trống
- loại bỏ cùng một node khỏi free-frame list
- ghi đè các entry bảng trang chồng lấn nhau

Kết quả có thể là:

```text
Process A -> frame 15
Process B -> frame 15
```

Điều này phá vỡ tính cô lập vì hai tiến trình khác nhau lại vô tình trỏ tới cùng một trang vật lý.

### Deadlock

Đồng bộ cũng có thể thất bại nếu các khóa được lấy theo thứ tự sai hoặc không được nhả đúng cách.

Ví dụ:

- CPU 0 giữ khóa của memory manager và chờ khóa của scheduler.
- CPU 1 giữ khóa của scheduler và chờ khóa của memory manager.

Lúc này không CPU nào có thể tiếp tục và hệ thống bị kẹt.

Dấu hiệu nhận biết:

- terminal bị treo
- hệ thống dừng phản hồi
- không còn output mới
- mô phỏng bị đứng vô thời hạn

### Biểu hiện quan sát được trong bài tập

Khi đồng bộ sai, lỗi thường không biểu hiện dưới dạng lỗi biên dịch mà xuất hiện khi hệ thống đang chạy.

Các dấu hiệu thường gặp:

- `Segmentation fault`
- `double free detected`
- `invalid page table entry`
- `queue corruption`
- `system halted`
- terminal bị treo khi chạy các cấu hình paging hoặc MLQ

Những lỗi này thường khó truy vết vì nguyên nhân gốc thường xảy ra sớm hơn, ngay tại thời điểm hai luồng cùng chạm vào một trạng thái dùng chung mà không có bảo vệ.

### Vai trò của mutex

Mutex dùng để bảo vệ vùng găng nhằm bảo đảm tại một thời điểm chỉ có một ngữ cảnh thực thi được phép cập nhật cấu trúc kernel dùng chung.

Mẫu sử dụng điển hình:

```c
pthread_mutex_lock(&mem_lock);
/* cập nhật các cấu trúc bộ nhớ dùng chung */
pthread_mutex_unlock(&mem_lock);
```

Cơ chế này bảo đảm:

- trạng thái cấp phát nhất quán
- cập nhật hàng đợi chính xác
- thao tác bảng trang an toàn
- output ổn định qua nhiều lần chạy

Trong bài tập này, đồng bộ không phải là một tối ưu tùy chọn mà là một yêu cầu bắt buộc để đảm bảo tính đúng đắn của mọi tài nguyên kernel có thể bị truy cập đồng thời.

---
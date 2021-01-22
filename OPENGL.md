1. OpenGL là gì? 

Open Graphics Library là một API đa nền tảng, đa ngôn ngữ thường được sử dụng để tương tác với bộ xử lý đồ họa GPU nhằm tận dụng tốc độ render phần cứng. 

2. OpenGL ES là gì? 

Là một phiên bản, tập con của OpenGL có thể hoạt động trên các hệ thống nhúng. Nó rút gọn cũng như có một số tiện ích mở rộng đặc điểm riêng để phù hợp với các thiệt bị có phần cứng đơn giản. 

3. Khả năng hỗ trợ 

Hiện tại OpenGL và GLES có rất nhiều các phiên bản khác nhau với các chức năng khác nhau. Tuy nhiên apple đã dừng hỗ trợ OpenGL và dừng ở phiên bản 4.1. Đối với chính OpenGL thì cũng đã dừng lại để tập chung cho việc phát triển thư viện mới Vulkan thay thế dần cho OpenGL. 



=> Tuy nhiên chúng ta vẫn tiếp tục tìm hiểu về OpenGL, các tính năng mà nó có ở phiên bản mới nhất hiện tại. Bởi vì, OpenGL có mức trừu tượng cao nên việc tiếp cận, số lượng mã, các khái niệm được tiếp cận dễ dàng hơn. Chúng ta sẽ tập trung vào các khái niệm cao, cũng như các tính năng chính mà OpenGL có, sau đó dựa trên những điều đó sẽ chuyển dần qua các thư viện đồ hoạ khác cũng như đi xuống các mức thấp hơn. 

 

Bản thân OpenGL được chia thành hai là Core và Compatibility. Đối với Core chỉ bao gồm những gì cốt lõi, các chức năng hiện tại của phiên bản còn Compatibility bao gồm cả chức năng không dùng nữa. Chúng ta sẽ sử dụng Core. 

OpenGL được thiết kế theo hướng thủ tục. Dựa vào OpenGL 4.6 API Reference Guide có thể thấy các hàm có chung format và được chia thành nhiều nhóm khác nhau. (Sẽ tìm hiểu chi tiết sau) còn bây giờ sẽ tập trung tìm hiểu ở mức khái niệm. 

OpenGL Loading: OpenGL chỉ mô tả các API, không không phải là một thư viện. Điều này có nghĩa việc triển khai thực tế API dự trên phần cứng, hệ điều hành và trìn điểu kiển đồ họa. Do đó các OpenGL được triển khai trên các hệ điều hành, phần cứng hay trình điều khiển khác nhau sẽ có thể khác nhau. Vì không cố định nên chúng ta không thể triển khai một file header liên kết tĩnh mà khai báo và tải các hàm, kiểm tra chúng trong thời gian chạy xem có hỗ trợ không. Bạn có thể tự tải hoặc sử dụng một số thư viện như glew, glad, ... 

Window surface: với mỗi một hệ điều hành khác nhau sẽ có hệ thống cửa sổ khác nhau. Như Window là Win32-HWND, OSX-Cocoa, Linux-X, ... Và vùng window surface là vùng để hiện thị OpenGL lên cửa sổ đó. Không nhất thiết đầu đầu ra của phải là bề mặt một cửa sổ, nó có thể là một ảnh, bản in, .. 

Context: là những thông tin liên quan gắn liền với bối cảnh nào đó thường được sử dụng để cài đặt hoặc lấy thông tin biết được chuyện gì đang diễn ra để rồi làm việc nào đó. Ngoài ra nó còn chứa các công cụ dịch vụ khác nhau để hỗ trợ các công việc khác nhau.  

OpenGL Context: như là một đối tượng chứa tất cả các thành phần của OpenGL. Khi dịch qua tiếng việt theo google dịch thì là ngữ cảnh tuy nhiên từ ngữ cảnh gắn với khái niêm văn học hơn vì vậy mình sẽ dùng từ Thể Hiện OpenGL. 

Rendering Pipeline: quy trình xử lý đồ họa là một chuỗi các bước mà OpenGL thực hiện để kết xuất các đối  tượng. Một số quy trình có thể được lập trình và cài đặt trên GPU 

Vertex Specification: Phân tích thuộc tính đỉnh như màu sắc, tọa độ kết cấu... 

Vertex Shader: giai đoạn này có thể lập trình được thực hiện việc tính toán thông tin của mỗi đỉnh 

Tessellation: đây là giai đoạn không bắt buộc và có thể lập trình. Giai đoạn này dữ liệu các đỉnh được phân tách để tăng chi tiết 

Geometry Shader: đây là giai đọan không bắt buộc và có thể lập trình. Giai đoạn này lấy nguyên mẫu đầu vào và tạo ra không hoặc nhiều bản sao, nó cũng có thể loại bỏ, phân tách hay chuyển đổi dữ liệu ban đầu. 

Vertex Post Processing: là chức năng cố định, chủ yếu dùng để cắt bỏ dữ liệu nằm ngoài vùng hiển thị 

Primitive Assembly: giai đoạn này thu thập và chia nhỏ dữ liệu thành các tập con như đường, điểm, tam giác 

Rasterization: giai đoạn này có thể lập trình, nó xác định chính xác mầu sắc cũng như thông tin khác tương ứng với  mỗi pixel được nội suy từ dữ liệu trước đó. 

Per-sample Operations: giai đoạn này bao gồm một số thao tác để xác định các lớp hình ảnh được kết hợp với nhau. Như độ sâu, hoà trộn... 

Scissor Test: giới hạn phạm vi hiển thị là một hình chữ nhật. 

Stencil Test:  

Depth Test: 

Blending: 

Logical Operation: 

Write Mask: 

                                                                                                                                                                                                                                                                                  
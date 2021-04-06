1. OpenGL là gì? 

Open Graphics Library là một API đa nền tảng, đa ngôn ngữ thường được sử dụng để tương tác với bộ xử lý đồ họa GPU nhằm tận dụng tốc độ render phần cứng. 

2. OpenGL ES là gì? 

Là một phiên bản, tập con của OpenGL có thể hoạt động trên các hệ thống nhúng. Nó rút gọn cũng như có một số tiện ích mở rộng đặc điểm riêng để phù hợp với các thiệt bị có phần cứng đơn giản. 

3. Khả năng hỗ trợ 

Hiện tại OpenGL và GLES có rất nhiều các phiên bản khác nhau với các chức năng khác nhau. Tuy nhiên apple đã dừng hỗ trợ OpenGL và dừng ở phiên bản 4.1. Đối với chính OpenGL thì cũng đã dừng lại để tập chung cho việc phát triển thư viện mới Vulkan thay thế dần cho OpenGL. 



=> Tuy nhiên chúng ta vẫn tiếp tục tìm hiểu về OpenGL, các tính năng mà nó có ở phiên bản mới nhất hiện tại. Bởi vì, OpenGL có mức trừu tượng cao nên việc tiếp cận, số lượng mã, các khái niệm được tiếp cận dễ dàng hơn. Chúng ta sẽ tập trung vào các khái niệm cao, cũng như các tính năng chính mà OpenGL có, sau đó dựa trên những điều đó sẽ chuyển dần qua các thư viện đồ hoạ khác cũng như đi xuống các mức thấp hơn. 

 

Bản thân OpenGL được chia thành hai là Core và Compatibility. Đối với Core chỉ bao gồm những gì cốt lõi, các chức năng hiện tại của phiên bản còn Compatibility bao gồm cả chức năng không dùng nữa. Chúng ta sẽ sử dụng Core. 

## Basic
OpenGL được thiết kế theo hướng thủ tục. Dựa vào OpenGL 4.6 API Reference Guide có thể thấy các hàm có chung format và được chia thành nhiều nhóm khác nhau. (Sẽ tìm hiểu chi tiết sau) còn bây giờ sẽ tập trung tìm hiểu ở mức khái niệm. 

**OpenGL Loading**: OpenGL chỉ mô tả các API, không không phải là một thư viện. Điều này có nghĩa việc triển khai thực tế API dự trên phần cứng, hệ điều hành và trìn điểu kiển đồ họa. Do đó các OpenGL được triển khai trên các hệ điều hành, phần cứng hay trình điều khiển khác nhau sẽ có thể khác nhau. Vì không cố định nên chúng ta không thể triển khai một file header liên kết tĩnh mà khai báo và tải các hàm, kiểm tra chúng trong thời gian chạy xem có hỗ trợ không. Bạn có thể tự tải hoặc sử dụng một số thư viện như glew, glad, ... 

**Window surface**: với mỗi một hệ điều hành khác nhau sẽ có hệ thống cửa sổ khác nhau. Như Window là Win32-HWND, OSX-Cocoa, Linux-X, ... Và vùng window surface là vùng để hiện thị OpenGL lên cửa sổ đó. Không nhất thiết đầu đầu ra của phải là bề mặt một cửa sổ, nó có thể là một ảnh, bản in, .. 

**Context**: là những thông tin liên quan gắn liền với bối cảnh nào đó thường được sử dụng để cài đặt hoặc lấy thông tin biết được chuyện gì đang diễn ra để rồi làm việc nào đó. Ngoài ra nó còn chứa các công cụ dịch vụ khác nhau để hỗ trợ các công việc khác nhau.  

**OpenGL Context**: như là một đối tượng chứa tất cả các thành phần của OpenGL. Khi dịch qua tiếng việt theo google dịch thì là ngữ cảnh tuy nhiên từ ngữ cảnh gắn với khái niêm văn học hơn vì vậy mình sẽ dùng từ Thể Hiện OpenGL. 

**Rendering Pipeline**: quy trình xử lý đồ họa là một chuỗi các bước mà OpenGL thực hiện để kết xuất các đối  tượng. Một số quy trình có thể được lập trình và cài đặt trên GPU 

**Vertex Specification**: Phân tích thuộc tính đỉnh như màu sắc, tọa độ kết cấu... 

**Vertex Shader:** giai đoạn này có thể lập trình được thực hiện việc tính toán thông tin của mỗi đỉnh 

**Tessellation**: đây là giai đoạn không bắt buộc và có thể lập trình. Giai đoạn này dữ liệu các đỉnh được phân tách để tăng chi tiết 

**Geometry Shader**: đây là giai đọan không bắt buộc và có thể lập trình. Giai đoạn này lấy nguyên mẫu đầu vào và tạo ra không hoặc nhiều bản sao, nó cũng có thể loại bỏ, phân tách hay chuyển đổi dữ liệu ban đầu. 

**Vertex Post Processing**: là chức năng cố định, chủ yếu dùng để cắt bỏ dữ liệu nằm ngoài vùng hiển thị 

**Primitive Assembly**: giai đoạn này thu thập và chia nhỏ dữ liệu thành các tập con như đường, điểm, tam giác 

**Rasterization**: giai đoạn này có thể lập trình, nó xác định chính xác mầu sắc cũng như thông tin khác tương ứng với  mỗi pixel được nội suy từ dữ liệu trước đó. 

**Per-sample Operations**: giai đoạn này bao gồm một số thao tác để xác định các lớp hình ảnh được kết hợp với nhau. Như độ sâu, hoà trộn... 

**Scissor Test**: giới hạn phạm vi hiển thị là một hình chữ nhật. 

**Stencil Test**:  phép kiểm tra bộ đệm khuân

**Depth Test**: phép kiểm tra bộ đệm độ sâu

**Blending**: Hòa trộn

**Logical Operation**: Thao tác logic giữa các lần tính toán phân mảnh với bộ đệm mầu.

**Write Mask**: Tạo mặt nạ cho phép ngăn cản các thành phần như màu, độ sâu, khuân được thao tác với bộ đệm.

## Descriptions
### OpenGL Object
Là một cấu trúc chứa một số trạng thái. Khi nó rằng buộc với một thể hiện cụ thể chúng sẽ chứ các trạng thái của thể hiện đó. OpenGL được định nghĩa như một "state machine" có nghĩa là các lệnh của OpenGL sẽ truy vấn thay đổi kiểm soát sử dụng các trạng thái để thực hiện điều gì đó.

Để tạo ra một đối tượng gọi ```void glGen*(GLsizei n, GLuint *objects)```. Đối tượng trả về luôn là kiểu nguyên dương, tuy nhiên đây không phải là con trỏ. Chúng chỉ là định danh, con số xác định đội tượng.

Để không sử dụng nữa gọi ```glDelete*(GLsizei n, GLuint *objects)```. Chú ý khi gọi lệnh này không đảm bảo việc phá huỷ toàn bộ nội dung của đối tượng sẽ được thực hiện ngay lập tức.

Để sử dụng một đối tượng cần rằng buộc nó với một thể hiện. Gọi ```glBind*```. Ngoài ra cũng có thể Multibind cho phép rằng buộc một nhóm đối tượng.

Đối tượng được chia sẻ với nhiều thể hiện của OpenGL, thông thường các thể hiện này ở trên các luồng khác nhau. Tuy nhiên, không phải tất cả các đối tượng đều được chia sẻ. Cũng như, các thay đổi trạng thái trên đối tượng này không nhất thiết thực thi ngay lập tức trong thể hiện khác. Nếu sử dụng phân luồng, nên tự mình thực hiện đồng bộ.

Đối tượng chia thành hai loại là đối tượng thông thường và đối tượng chứa. Thông thường bao gồm: Buffer, Query, Renderbuffer, Sampler, Texture và chứa bao gồm Framebuffer, Program Pipeline, Transform Feedback, Vertex Array.

Đối tượng là một số nguyên dương vì vậy không trực quan cho việc gỡ lỗi. Vậy nên, từ OpenGL 4.3 hoặc ```KHR_debug``` hỗ trợ việc gán nhán cho đối tượng. Gọi ```glObjectLabel``` với đội tượng thông thường hoặc ```glObjectPtrLabel``` đối tượng đồng bộ hóa được xác định bởi con trỏ. Và để truy xuất ngược lại sử dụng ```glGet``` tương tứng.

### Buffer Objects
Lưu trữ mạng bộ nhớ chưa định dạng. Chúng có thể sử dụng để lưu trữ đỉnh, dữ liệu điểm ảnh hoặc bộ đệm khung, ...

Sử dụng ```glBindBuffer```, đối tượng bộ đệm chứa một mảng bộ nhớ tuyến tính có kích thước tuỳ ý. Bộ nhớ này phải được cấp phát trước khi có thể tải lên dữ liệu hoặc sử dụng. Có hai cách để cấp phát, có thể thay đổi kích thước mutable và không thay đổi kích thước immutable.

Với immutable, sẽ không thể phân bổ lại bộ nhớ đó. Vẫn có thể vô hiệu hóa thông qua lệnh vô hiệu rõ ràng invalidation ```glInvalidate*``` hoặc ánh xạ bộ đệm. Nhưng không thể thực hiện thủ thuật ```glBufferData(..., NULL)```. Sử dụng gọi ```glBufferStorage``` với OpenGL từ 4.4 hoặc có hỗ trợ ```ARG_buffer_storage```. Hàm này sử dụng tham số flags là cờ để xác định cách người dùng có thể trực tiếp đọc ghi vào bộ đệm. Tuy nhiên điều này chỉ hạn chế ở phí người dùng còn các thao tác từ phía máy chủ luôn khả dụng.

Với mutable sử dụng ```glBufferData```, Hàm này sử dụng tham số usage để gới ý về cách sử dụng bộ đệm. Cách đọc/ghi và tuần suất thay đổi. 
- ```DRAW```: Ghi dữ liệu vào bộ đệm nhưng không đọc nó
- ```READ```: Sẽ không ghi mà chỉ đọc
- ```COPY```: Người dùng không ghi hoặc đọc dữ liệu
- ```STATIC```: Thiết lập dữ liệu một lần
- ```DYNAMIC```: Thi thoảng thay đổi dữ liệu, nhiều nhất là mỗi khung hình thay đổi một lần
- ```STREAM```: Thay đổi dữ liệu sau mỗi lần sử dụng

Với ```glBufferData``` sẽ phân bổ lại bộ đệm điều này không thực hiện được với immutable. Sử dụng ```glBufferSubData``` để thay đổi dữ liệu mà không phân bổ lại bộ đệm

Dưới đây là thủ thật sử dụng orphaning để hỗ trợ quá trình đồng bộ dữ liệu tự đông nhằm tăng hiệu suất: https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
```cpp
glBindBuffer(GL_ARRAY_BUFFER, my_buffer_object);
glBufferData(GL_ARRAY_BUFFER, data_size , NULL ,GL_STREAM_DRAW);
glBufferData(GL_ARRAY_BUFFER, data_size , mydata_ptr ,GL_STREAM_DRAW);
```

Từ OpenGL 4.3 hoặc hỗ trợ ```ARB_clear_buffer_object``` để xóa sử dụng ```glClearBuffer*``` hoạt động tương tự  pixel transfer mặt dù có một số khác biệt. cho phép đặt một phần hoặc toàn bộ bộ đệm với giá trị cụ thể.

Để sao chép dữ liệu từ một bộ đệm này qua bộ đệm khác sử dụng ```glCopyBufferSubData```

Sử dụng ```(glMapBuffer, glUnmapBuffer)```hoặc ```(glMapBufferRange, glFlushMappedBufferRange)``` để  thiết lập ánh xạ đến vùng nhớ. Nó trả về một con trỏ đại diện cho khối bộ nhớ, sau đó có thể truy cập trực tiếp thay đổi bộ nhớ và huỷ ánh xạ để nó sẽ truyền lại các thay đổi vào bộ đệm trên thiết bị đồ họa. Cơ bản thì nó giống với ```glBufferSubData``` tuy nhiên các truy cập dữ liệu hơi khác một chút. Một số kiểm tra và đánh giá trên một số trang thì dường như sử dụng ánh xạ là tốt hơn.

> Thủ thuật: Tạo ra một bộ đệm lớn, sử dụng nửa đầu để hiển thị, nửa sau để cập nhật và chuyển đổi khi upload thành công (x2 bộ đệm thủ công)

Sử dụng ```glInvalidateBufferData``` hoặc ```glInvalidateBufferSubData``` để vê hiệu hóa bộ đệm.

Khi sử dụng bộ đệm việc đồng bộ là phức tạp, thông thường nó sẽ được đồng bộ ngầm. Tuy nhiên điều này dẫn đến các vấn đề về hiệu năng vì việc phải chờ đợi không đáng có. Vì vậy có một số cách cơ bản để tránh việc đồng bộ ngầm.
- Bộ đệm vòng, tạo thêm hoặc nhiều bộ đệm khác, trong khi bộ đệm này được sử dụng thì bộ đệm khác cập nhật.
- Bộ đệm rỗng orphaning. tương tự như bộ đệm vòng nhưng nó đượng diễn ra bên trong OpenGL tự động. được sử dụng bằng cách gọi ```glBufferData``` với ```NULL```. Như có đề cập phía trên. Vì nó diễn ra ở trong trình điều khiển đồ họa nên hiệu năng có thể khác nhau giữa các trình thiết bị hay ```glBufferData``` hoặc ```glBufferSubData```. Tuy nhiên khi sử dụng với ```glMapBufferRange``` thì theo một số thông tin thì hiệu suất bị giảm đi đến 10 lần trên một số thiết bị.
- Thủ công sử dụng fence: Bảo với trình điều khiển là bộ đệm được sử dụng không đồng bộ bằng việc thiết lập cờ ```GL_MAP_UNSYNCHRONIZED_BIT``` của hàm ```glMapBufferRange```

### Depth buffer
Là bộ đệm giống như bộ đệm mầu, tuy nhiên nó được tạo ra tự động và lưu trữ giá trị độ sâu. Định dạng sử dụng là kiểu thực với kích thước 16, 24 hoặc 32 bit. Khi tính năng kiểm tra độ sâu được bật, OpenGL trong giai đoạn fragment sẽ kiểm tra giá trị độ sâu với bộ đệm độ sâu. Nếu kiểm tra không thành không thì điểm ảnh đó sẽ bị loại bỏ.

Ngày này hầu hết các GPU có bổ xung thêm tính năng phần cứng cho phép kiểm tra độ sâu sớm hơn trước khi đến giai đọan fragment.

Bộ đệm này có giá trị từ [0.0 -> 1.0] và được tính toán tuyến tính. Ngoài ra với độ sai lệch trong tính toán số thực khi hai mặt cần hiển thị quá gần nhau sẽ dẫn đến trường hợp trồng chất. Z-Fighting. Để khắc phục hiện tượng này có thể tăng độ chính xác của bộ đệm, hoặc bù trừ khoảng cách giữa các mặt gần nhau với một định lượng bias.

Đọc giá trị:
- Sử dụng `glReadPixels` hoặc `glCopyTexImage2D` với `GL_DEPTH_COMPONENT` tuy nhiên thì ví dụ như OpenGL ES 3.2 không hỗ trợ hoàn toàn.
- Sử dụng bộ đệm mầu để lấy ra thông tin `gl_FragCorrd.z` tuy nhiên bộ đệm mầu sẽ thường sử dụng 8bit cho mỗi kênh và việc lưu trữ như vậy không được chuẩn so với kích thước mà bộ đệm depth có.
- Sử dụng FrameBuffer sử dụng `GL_DEPTH_ATTACHMENT`

Ghi giá trị:
- Sử dụng `glDrawPixels` với `GL_DEPTH_COMPONENT` tuy nhiên cũng giống như việc đọc giá trị có thể không được hỗ trợ.

### Stencil buffer
Là một phần mở rộng tuỳ chọn của bộ đệm depth cho phép kiểm soát nhiều hơn việc hiển thị hay không hiển thị điểm ảnh. Giống như bộ đệm độ sau, một giá trị được lưu trữ cho mỗi pixel, nhưng lần này có thể kiểm soát được giá trị này tốt hơn.

Về cơ bản việc đọc ghi giá trị cũng có thể sử dụng `glReadPixels` và `glDrawPixels` như với depth buffer. Ngoài ra còn có thể thẳng việc render objects để cập nhật vào bộ đệm stencil buffer.                                                                                                                                                                                                                                                                           
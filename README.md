# learn-gl

3rdparty: drive/Projects/redbean/3rdparty.zip</br>
delete .DS_Store files on windows: cmd: del /s /q /f /a .DS_STORE_

## Questions
1. Có bắt buộc phải sử dụg VAO hay không: Là bắt buộc khi sử dụng Core (OpenGL 3.3+ & GLES 3.0+).
2. Có cần thiết phải unbind: Thông thường sẽ không cần phải unbind, điều này thường được sử dụng để tránh việc thao tác thay đổi trên trạng thái trước. Tuy nhiên lời khuyên là nên đặt lại trạng thái trước khi bắt đầu làm điều gì đó thay vì unbind sau khi thao tác.
3. Định dạng texture BGRA có nhanh hơn: < Cần kiểm tra >
4. Có phải tất cả các hàm OpenGL đều tốn chi phí trên GPU: Có thể có có thể không, bởi vì bản thân OpenGL chỉ là một định nghĩa và việc triển khai phụ thuộc vào nhiều yếu tố như thiết bị phần cứng, nhà thiết kế sản xuất. Miễn là các triển hai này tuân theo các đặc điểm kỹ thuật cũng như cho ra kết quả đúng như mong muốn. Với các triển khai khác nhau có thể làm các công việc khác nhau, tối ưu hóa khác nhau.
5. Thứ tự thực hiện testing như depth test, stencil test, alpha test: Thứ tự thực hiện không được áp dụng một cách bắt buộc với tất cả phần cứng và API.

## Common Mistakes
1. **Extensions and OpenGL Versions**: Cần kiểm tra phiên bản của OpenGL và các api mở rộng xem có hỗ trợ không trước khi sử dụng.
2. **The Object Oriented Language Problem**: Khi đóng gói thành phẩn của OpenGL theo hướng đối tượng cần chú ý việc gọi lệnh là hợp lệ, ví dụ OpenGL Context cần được tạo trước khi gọi bất cứ hàm nào liên quan đến Texture. Có một số giải pháp như tránh sử dụng hàm tạo và hàm huỷ mà thay vào đó là các hàm rõ ràng, luôn kiểm tra các điều kiện cần trước khi gọi hàm, tạo ra một lớp quản lý tất cả các thành phần của OpenGL, ... .
3. **Texture upload and pixel reads**: Nếu gặp sự cố khi tải texture hoặc có các đường chéo xuất hiện trong hình thì điều này có thể do kích thước của chiều rộng không phải là bội của 4. Ví dụ mỗi pixel là 3 bytes RGB và chiều rộng của ảnh là 401 => 401*3=1203, 1203 không chia hết cho 4. Tại sao cần chia hết cho bốn bởi vì hầu hết các GPU hoạt động với từng khối 4 bytes (Tìm hiểu thêm Memory access granualarity). Vì vậy lời khuyên là bạn nên sử dụng 8, 16, 32, 64, ... bytes cho mỗi pixel. Ví dụ như với Metal sẽ không hỗ trợ kết cấu r8g8b8.
4. **Image precision**: với `glTexImage2D` với tham số internalformat yêu cầu cách mà OpenGL lưu trữ còn format và type định nghĩa dữ liệu thô của bạn trông như thế nào. Đối với internalformat đây chỉ là mong muốn cách mà OpenGL lưu trữ tuy nhiên trình điểu khiển có thể quyết định sử dụng định dạng khác mà nó hỗ trợ. Sử dụng `glGetInternalFormat` (Core 4.3+ or ARB_internalformat_query2) để lấy thông tin chính xác.
5. **Depth buffer precision**: Bộ đệm độ sâu có phạm vi từ [0, 1] với kích thước 16, 24, 32bits. Việc chuyển đổi từ bộ đệm độ sâu sang float sẽ phải sử dụng CPU. Vì vậy nên lấy chính xác nếu kích thước bộ đệm. VD: với bộ đệm 24bits sử dụng `GL_UNSIGNED_SHORT`.
6. **Creating a complete texture**: Vật liệu sẽ không hoạt động nếu nó chưa được hoàn thiện việc cài đặt. Mã lệnh tốt là sử dụng `glTexStorage2D` để xác định thông tin vật liệu và sử dụng `glTexSubImage2D` để tải dữ liệu. Hoặc thiết lập `GL_TEXTURE_MIN_FILTER` hoặc (`GL_TEXTURE_BASE_LEVEL` và `GL_TEXTURE_MAX_LEVEL`).
7. **Automatic minmap generation**: Một vật liệu có thể được tạo tự động bằng `glGenerateMipmap`.
8. **Depth testing Doesn't Work**: Hãy kiểm tra bộ đệm độ sâu có hoạt động không, đảm bảo nó được enable và các DepthFunc và DepthRange thích hợp. Kiểm quá trình tạo thể hiện OpenGL.
9. **Slow pixel transfer performance**: Để đạt được hiệu suất tốt nhất trong quá trình truyền tải vật liệu ta nên kiểm tra định dạng khi đọc, lưu trữ và hoạt động mà OpenGL là giống nhau để tránh chi phí trong quá trình chuyển đổi. Có thể kiểm tra và lấy thêm thông tin thông qua `glGetInternalFormat` (Core 4.3+ or ARB_internalformat_query2).
10. **Swap Buffers**: Trên một số phần cứng cũ việc gọi clear bộ đệm trước khi thực hiện bất kỳ thao tác nào là không cần thiết, thậm chí là với phần cứng gần đây. Tuy nhiên điều này có thể sẽ kiến mọi thứ chậm hơn trên một số phần cứng, vì vậy tốt hơn hết luôn rõ ràng.
11. **The Pixel Ownership Problem**: Nếu vùng cửa sổ cần hiển thị bị che phủ thì GPU có thể không hiển thị phần đó và việc đọc thông tin khu vực đó có thể dẫn đến dữ liệu rác. Giải pháp là kết xuất hình ảnh ra một bộ đệm khác thay vì bộ đệm mặc định.
12. **Bitfield enumerators**: Việc kết hợp bit sẽ chỉ hoạt động đối với các enum có kết thúc bằng _BIT. VD việc gọi như sau là không hợp lệ: `glEnable(GL_BLEND | GL_DRAW_BUFFER)`.
13. **Triple Buffering**: bạn không thể biết được liệu trình điều khiển có hỗ trợ bộ đệm ba hay không. Vì vậy nói chung là nên bỏ qua bộ đệm ba với OpenGL.
14. **Paletted textures**: Hỗ trợ EXT_paletted_texture đã bị lại bỏ nhiều. vì vậy nếu cần sử dụng tính năng này bạn có thể sử dụng shaders. VD: sử dụng texture có kích thước 256x1 pixels.
15. **glGetFloatv glGetBooleanv glGetDoublev glGetIntegerv**: các hàm này là chậm, các trình điều khiển khuyên bạn nên tự theo dõi thông tin.


https://www.khronos.org/opengl/wiki/Common_Mistakes

## Demo
1. Hiển thị hình ảnh đơn giản, cài đặt và truy vấn thông tin cơ bản
2. Sử dụng đa luồng
3. Shader gồm GLES và SPIR-V
4. Buffer bao gồm việc sử dụng, đọc, ghi tất cả hoặc từng đoạn
5. Texture bao gồm việc sử dụng, đọc, ghi tất cả hoặc từng vùng ảnh
6. Scissor Test
7. Depth Test bao gồm việc sử dụng, đọc và thay đổi thông tin bộ đệm
8. Stencil Test bao gồm việc sử dụng, đọc và thay đổi thông tin bộ đệm
9. Blending hiển thị toàn bộ các chế độ hoà trộn


## Depth buffer
Là bộ đệm giống như bộ đệm mầu, tuy nhiên nó được tạo ra tự động và lưu trữ giá trị độ sâu. Định dạng sử dụng là kiểu thực với kích thước 16, 24 hoặc 32 bit. Khi tính năng kiểm tra độ sâu được bật, OpenGL trong giai đoạn fragment sẽ kiểm tra giá trị độ sâu với bộ đệm độ sâu. Nếu kiểm tra không thành không thì điểm ảnh đó sẽ bị loại bỏ.

Ngày này hầu hết các GPU có bổ xung thêm tính năng phần cứng cho phép kiểm tra độ sâu sớm hơn trước khi đến giai đọan fragment.

Bộ đệm này có giá trị từ [0.0 -> 1.0] và được tính toán tuyến tính. Ngoài ra với độ sai lệch trong tính toán số thực khi hai mặt cần hiển thị quá gần nhau sẽ dẫn đến trường hợp trồng chất. Z-Fighting. Để khắc phục hiện tượng này có thể tăng độ chính xác của bộ đệm, hoặc bù trừ khoảng cách giữa các mặt gần nhau với một định lượng bias.

Đọc giá trị:
- Sử dụng `glReadPixels` hoặc `glCopyTexImage2D` với `GL_DEPTH_COMPONENT` tuy nhiên thì ví dụ như OpenGL ES 3.2 không hỗ trợ hoàn toàn.
- Sử dụng bộ đệm mầu để lấy ra thông tin `gl_FragCorrd.z` tuy nhiên bộ đệm mầu sẽ thường sử dụng 8bit cho mỗi kênh và việc lưu trữ như vậy không được chuẩn so với kích thước mà bộ đệm depth có.
- Sử dụng FrameBuffer sử dụng `GL_DEPTH_ATTACHMENT`

Ghi giá trị:
- Sử dụng `glDrawPixels` với `GL_DEPTH_COMPONENT` tuy nhiên cũng giống như việc đọc giá trị có thể không được hỗ trợ.

## Stencil buffer
Là một phần mở rộng tuỳ chọn của bộ đệm depth cho phép kiểm soát nhiều hơn việc hiển thị hay không hiển thị điểm ảnh. Giống như bộ đệm độ sau, một giá trị được lưu trữ cho mỗi pixel, nhưng lần này có thể kiểm soát được giá trị này tốt hơn.

Về cơ bản việc đọc ghi giá trị cũng có thể sử dụng `glReadPixels` và `glDrawPixels` như với depth buffer. Ngoài ra còn có thể thẳng việc render objects để cập nhật vào bộ đệm stencil buffer.
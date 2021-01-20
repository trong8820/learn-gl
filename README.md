# learn-gl

3rdparty: drive/Projects/redbean/3rdparty.zip</br>
delete .DS_Store files on windows: cmd: del /s /q /f /a .DS_STORE_

## Questions
1. Có bắt buộc phải sử dụg VAO hay không: Là bắt buộc khi sử dụng Core (OpenGL 3.3+ & GLES 3.0+).
2. Có cần thiết phải unbind: Thông thường sẽ không cần phải unbind, điều này thường được sử dụng để tránh việc thao tác thay đổi trên trạng thái trước. Tuy nhiên lời khuyên là nên đặt lại trạng thái trước khi bắt đầu làm điều gì đó thay vì unbind sau khi thao tác.
3. Định dạng texture BGRA có nhanh hơn: < Cần kiểm tra >
4. Có phải tất cả các hàm OpenGL đều tốn chi phí trên GPU: ?

## Common Mistakes
1. **Extensions and OpenGL Versions**: Cần kiểm tra phiên bản của OpenGL và các api mở rộng xem có hỗ trợ không trước khi sử dụng.
2. **The Object Oriented Language Problem**: Khi đóng gói thành phẩn của OpenGL theo hướng đối tượng cần chú ý việc gọi lệnh là hợp lệ, ví dụ OpenGL Context cần được tạo trước khi gọi bất cứ hàm nào liên quan đến Texture. Có một số giải pháp như tránh sử dụng hàm tạo và hàm huỷ mà thay vào đó là các hàm rõ ràng, luôn kiểm tra các điều kiện cần trước khi gọi hàm, tạo ra một lớp quản lý tất cả các thành phần của OpenGL, ... .
3. **Texture upload and pixel reads**: Nếu gặp sự cố khi tải texture hoặc có các đường chéo xuất hiện trong hình thì điều này có thể do kích thước của chiều rộng không phải là bội của 4. Ví dụ mỗi pixel là 3 bytes RGB và chiều rộng của ảnh là 401 => 401*3=1203, 1203 không chia hết cho 4. Tại sao cần chia hết cho bốn bởi vì hầu hết các GPU hoạt động với từng khối 4 bytes (Tìm hiểu thêm Memory access granualarity). Vì vậy lời khuyên là bạn nên sử dụng 8, 16, 32, 64, ... bytes cho mỗi pixel. Ví dụ như với Metal sẽ không hỗ trợ kết cấu r8g8b8.
4. **Image precision**: với `glTexImage2D` với tham số internalformat yêu cầu cách mà OpenGL lưu trữ còn format và type định nghĩa dữ liệu thô của bạn trông như thế nào. Đối với internalformat đây chỉ là mong muốn cách mà OpenGL lưu trữ tuy nhiên trình điểu khiển có thể quyết định sử dụng định dạng khác mà nó hỗ trợ. Sử dụng `glGetInternalFormat` (Core 4.3+ or ARB_internalformat_query2) để lấy thông tin chính xác.
5. **Depth buffer precision**: 

https://www.khronos.org/opengl/wiki/Common_Mistakes
# learn-gl

3rdparty: drive/Projects/redbean/3rdparty.zip</br>
delete .DS_Store files on windows: cmd: del /s /q /f /a .DS_STORE_

## Questions
1. Có bắt buộc phải sử dụg VAO hay không: Là bắt buộc khi sử dụng Core (OpenGL 3.3+ & GLES 3.0+).
2. Có cần thiết phải unbind: Thông thường sẽ không cần phải unbind, điều này thường được sử dụng để tránh việc thao tác thay đổi trên trạng thái trước. Tuy nhiên lời khuyên là nên đặt lại trạng thái trước khi bắt đầu làm điều gì đó thay vì unbind sau khi thao tác.

https://www.khronos.org/opengl/wiki/Common_Mistakes
The texture won't work because it is incomplete. The default GL_TEXTURE_MIN_FILTER state is GL_NEAREST_MIPMAP_LINEAR. And because OpenGL defines the default GL_TEXTURE_MAX_LEVEL to be 1000, OpenGL will expect there to be mipmap levels defined. Since you have only defined a single mipmap level, OpenGL will consider the texture incomplete until the GL_TEXTURE_MAX_LEVEL is properly set, or the GL_TEXTURE_MIN_FILTER parameter is set to not use mipmaps. 
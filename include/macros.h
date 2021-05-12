#ifndef MACROS_H_
#define MACROS_H_

#include <iostream>

#define GL_CHECK(_call) \
	_call; \
	{ GLenum err; \
	while ((err = glGetError()) != GL_NO_ERROR) { \
        std::cout <<"GL error: "<< err <<" in "<< __FILE__ <<" "<< __FUNCTION__ <<" "<< __LINE__ <<" - for "<<#_call<<std::endl; \
	}}

#define GL_CHECK_RETURN(_call) \
	_call; \
	{ bool iserr = false; { GLenum err; \
	while ((err = glGetError()) != GL_NO_ERROR) { \
		iserr = true; std::cout <<"GL error: "<< err <<" in "<< __FILE__ <<" "<< __FUNCTION__ <<" "<< __LINE__ <<" - for "<<#_call<<std::endl; \
	}} if (iserr) return false; }

#endif // MACROS_H_
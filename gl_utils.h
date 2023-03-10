#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <GL/glew.h>

const char *gl_error_string(GLenum error);

void raise_gl_error(GLenum error, const char *call, const char *file, int line);
#ifndef NO_GL_ERROR_CHECK
#define GL_CALL(fnCall) fnCall; \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            raise_gl_error(err, #fnCall, __FILE__, __LINE__); \
        } \
    } while (0)
#else
#define GL_CALL(fnCall) fnCall
#endif

GLuint gl_compile_shader(const char *vertex, const char *fragment, const char *frag_bind);

#define UNUSED(x) ((void)(x))

#endif // GL_UTILS_H

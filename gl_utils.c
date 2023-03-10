#include "gl_utils.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

const char *gl_error_string(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

void raise_gl_error(GLenum err, const char *call, const char *file, int line)
{
    const int NMaxErrors = 10;

    int nErrors = 0;
    while (err != GL_NO_ERROR && nErrors < NMaxErrors)
    {
        if (call != NULL)
        {
            printf("%s:%d: %s raised Error %u (%s).\n", file, line, call, err, gl_error_string(err));
        }
        else
        {
            printf("%s:%d: Caught Error %u (%s).\n", file, line, err, gl_error_string(err));
        }

        err = glGetError();
        nErrors++;
    }

    if (nErrors >= NMaxErrors)
    {
        printf("Stopped after %d errors. Too many errors could be a sign of a missing GL Context!\n", NMaxErrors);
    }

    if (nErrors > 0)
    {
        abort();
    }
}

GLuint gl_compile_shader(const char* vertex, const char* fragment, const char* frag_bind)
{
    // Shader Loading
    // 1. Create shader and load source
    GLuint vertexShader = GL_CALL(glCreateShader(GL_VERTEX_SHADER));
    GL_CALL(glShaderSource(vertexShader, 1, &vertex, NULL));

    // 2. Compile shader + check for compiler errors
    GL_CALL(glCompileShader(vertexShader));
    GLint status;
    GL_CALL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status));
    if (status == GL_TRUE) printf("Compile Vertex Shader: SUCCESS\n");
    else
    {
        char buffer[512];
        GL_CALL(glGetShaderInfoLog(vertexShader, 512, NULL, buffer));
        printf("ERROR: Vertex Shader compilation failed:\n%s\n", buffer);
        abort();
    }

    // Repeat 1. + 2. for fragment shader
    GLuint fragmentShader = GL_CALL(glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(fragmentShader, 1, &fragment, NULL));
    GL_CALL(glCompileShader(fragmentShader));
    GL_CALL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status));
    if (status == GL_TRUE) printf("Compile Fragment Shader: SUCCESS\n");
    else
    {
        char buffer[512];
        GL_CALL(glGetShaderInfoLog(fragmentShader, 512, NULL, buffer));
        printf("ERROR: Fragment Shader compilation failed:\n%s\n", buffer);
        abort();
    }

    // 3. Combine shaders into program
    GLuint program = GL_CALL(glCreateProgram());
    GL_CALL(glAttachShader(program, vertexShader));
    GL_CALL(glAttachShader(program, fragmentShader));

    // 4. Map Fragment Shader output to framebuffers
    GL_CALL(glBindFragDataLocation(program, 0, frag_bind));

    // 5. Link Program
    GL_CALL(glLinkProgram(program));

    // 5.1 Check for linking errors (TODO)

    // 5.2 Delete shaders (marked for deletion, deleted on program deletion)
    GL_CALL(glDeleteShader(vertexShader));
    GL_CALL(glDeleteShader(fragmentShader));

    return program;
}
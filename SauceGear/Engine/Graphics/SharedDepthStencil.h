#pragma once
#include <glad/glad.h>

class SharedDepthStencil {
public:
    SharedDepthStencil(int w, int h) { Init(w, h); }

    void Init(int w, int h) {
        width = w;
        height = h;

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            width, height
        );
    }

    void Resize(int w, int h) {
        if (w == width && h == height) return;

        width = w;
        height = h;

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            width, height
        );
    }

    GLuint GetRBO() const { return rbo; }

    ~SharedDepthStencil() {
        glDeleteRenderbuffers(1, &rbo);
    }

private:
    GLuint rbo = 0;
    int width = 0, height = 0;
};

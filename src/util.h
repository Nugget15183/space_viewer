#ifndef SPACE_VIEWER_UTIL_H
#define SPACE_VIEWER_UTIL_H

#include <iostream>
#include <ostream>
#include <fstream>
#include <iterator>
#include <string>

using namespace std;

enum LogType {
    INFO,
    WARN,
    ERRO,
};

namespace util {
    inline void log(LogType l, const char* message) {
        const char* prefix = "INFO";

        switch (l) {
            case WARN: {prefix = "WARN"; break;}
            case ERRO: prefix = "ERRO";
        }

        cout << "[" << prefix << "] " << message << endl;
    }

    inline string readFile(const char* path) {
        std::fstream file(path);

        if (!file.is_open()) {
            log(WARN, ("Failed to open file at location '" + std::string(path) + "'").c_str());
            return "";
        }

        std::string s((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

        if (s.size() >= 3 && (unsigned char) s[0] == 0xEF && (unsigned char) s[1] == 0xBB && (unsigned char) s[2] == 0xBF) {
            s.erase(0, 3);
        }

        return s;
    }

    inline glm::dvec3 toGL(const glm::dvec3& v) {
        return { v.x, v.z, -v.y };
    }

    inline unsigned int loadTexture(const char* path, int channels) {
        int w, h, n;

        unsigned char* data = stbi_load(path, &w, &h, &n, channels);

        if (!data) {
            log(WARN, ("Could not find texture at location '" + string(path) + "'").c_str());
            return 0;
        }

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        GLint internalFormat = (channels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;

        unsigned int t;

        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return t;
    }

    static unsigned int compileShader(GLenum type, const char* path) {
        string src = readFile(path);

        const GLchar* source = src.c_str();
        unsigned int shader = glCreateShader(type);

        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        int succ;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &succ);
        if (!succ) {
            char buf[1024];

            glGetShaderInfoLog(shader, sizeof(buf), NULL, buf);
            log(ERRO, (string("Failed to compile ") + path + ": " + buf).c_str());

            return 0;
        }

        return shader;
    }

    static unsigned int buildProgram(const char* vpath, const char* fpath) {
        unsigned int v = compileShader(GL_VERTEX_SHADER, vpath);
        unsigned int f = compileShader(GL_FRAGMENT_SHADER, fpath);

        if (!v || !f) return 0;

        unsigned int p = glCreateProgram();
        glAttachShader(p, v);
        glAttachShader(p, f);
        glLinkProgram(p);

        int succ;
        glGetProgramiv(p, GL_LINK_STATUS, &succ);

        if (!succ) {
            char buf[1024];

            glGetProgramInfoLog(p, sizeof(buf), NULL, buf);
            log(ERRO, (string("Failed to link ") + fpath + ": " + buf).c_str());

            return 0;
        }

        glDeleteShader(v);
        glDeleteShader(f);

        return p;
    }
}

#endif //SPACE_VIEWER_UTIL_H
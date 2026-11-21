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

        return std::string((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
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

        unsigned int t;

        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return t;
    }
}

#endif //SPACE_VIEWER_UTIL_H
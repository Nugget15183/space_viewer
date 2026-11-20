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
}

#endif //SPACE_VIEWER_UTIL_H
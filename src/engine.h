//
// Created by conte on 11/19/2026.
//

#ifndef SPACE_VIEWER_ENGINE_H
#define SPACE_VIEWER_ENGINE_H

#include "orbits.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

struct body {
    const char* name;

    //nullptr for sun
    const orbits::elements* el;

    double radiusKm;
    glm::vec3 color;
    bool emissive;

    //heliocentric
    glm::dvec3 posKm;
};

class engine {
public:
    void close();
    int init();

    void update();
    void render();

    void onResize(int w, int h);
    void onScroll(double dy);

    GLFWwindow* getWindow() {
        return window;
    }

    double timeScale = 1.0f;
private:
    GLFWwindow* window;
    glm::vec2 winSize = {1280, 720};

    unsigned int shader_program=0;
    unsigned int viewLoc, projLoc, modelLoc, camPosLoc, sunDirLoc, colorLoc, emissiveLoc;

    double startup = glfwGetTime();
    double lastfpsIncrement = startup;
    double lastframeT = startup;

    int frames = 0;

    double dt = 0;
    double simClock = startup;

    std::vector<body> bodies = {
        {"sun", nullptr, 695700, {1, 1, 1}, true, {}},
        {"mercury", &orbits::MERCURY, 2439.7, {0.2f, 0.2f, 0.2f}, false, {}},
        {"venus", &orbits::VENUS, 6051.8, {0.2f, 0.2f, 0.2f}, true, {}},
        {"earth", &orbits::EARTH, 6371.0, {0.2f, 0.2f, 0.2f}, false, {}},
        {"mars", &orbits::MARS, 3389.5, {0.2f, 0.2f, 0.2f}, false, {}},
        {"jupiter", &orbits::JUPITER, 69911, {0.2f, 0.2f, 0.2f}, false, {}},
        {"saturn", &orbits::SATURN, 58232, {0.2f, 0.2f, 0.2f}, false, {}},
        {"uranus", &orbits::URANUS, 25362, {0.2f, 0.2f, 0.2f}, false, {}},
        {"neptune", &orbits::NEPTUNE, 24622, {0.2f, 0.2f, 0.2f}, false, {}}
    };

    static constexpr int camIndex = 3;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float FOV = 40.0f;

    glm::vec2 lastM = {};

    unsigned int sVAO, sVBO, sEBO;
    int sic = 0;

    void createSphere(int stacks, int slices);
    void pointAt(int bIndex);
};


#endif //SPACE_VIEWER_ENGINE_H
//
// Created by conte on 11/19/2026.
//

#ifndef SPACE_VIEWER_ENGINE_H
#define SPACE_VIEWER_ENGINE_H

#include "orbits.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <string>

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct body {
    const char* name;

    //nullptr for sun
    const orbits::elements* el;

    double radiusKm;
    glm::vec3 color;
    bool emissive;

    //heliocentric
    glm::dvec3 posKm;

    double poleRA = 0;
    double poleDec = 0;

    double flattening = 0;

    double w0 = 0;
    double wRate = 0;

    unsigned int texid = 0;

    float limbDark = 0.0f;

    glm::vec3 atmoColor = glm::vec3(0.0f);

    float atmoH = 0.0f;
    float atmoDensity = 0.0f;

    //forward scatter
    float atmoG = 0.6f;
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

    unsigned int viewLoc, projLoc, modelLoc, camPosLoc, sunDirLoc, colorLoc, emissiveLoc, texLoc;
    unsigned int isRingLoc, inRingLoc, outRingLoc, hasRingLoc, oblateLoc, sunLocalLoc, ringTexLoc;
    unsigned int viewUnitLoc, limbDarkLoc, isAtmoLoc, atmoColorLoc, atmoHLoc, atmoDensLoc, atmoGLoc;

    unsigned int sceneFBO = 0;
    unsigned int sceneCol = 0;
    unsigned int sceneDepth = 0;
    unsigned int blurFBO[2] = {0, 0};
    unsigned int blurTex[2] = {0, 0};
    unsigned int quadVAO = 0;
    unsigned int blurProg = 0;
    unsigned int compProg;

    bool showUi = true;
    bool paused = false;

    //atmospheric seeing
    float seeingArcsec = 1.0f;
    float focusBlur = 0.0f; //0 - 1
    float exposure = 1.0f;
    float vignette = 0.25f;

    bool fieldStop = true;

    double startup = glfwGetTime();
    double lastfpsIncrement = startup;
    double lastframeT = startup;

    int frames = 0;

    double dt = 0;
    double simClock = startup;

    std::vector<body> bodies = {
        {"sun",     nullptr,          695700,   {1,1,1}, true,  {}, 286.13,  63.87,  0,       84.176,   14.1844},
        {"mercury", &orbits::MERCURY, 2439.7,   {1,1,1}, false, {}, 281.01,  61.416, 0,       329.5988, 6.1385108},
        {"venus",   &orbits::VENUS,   6051.8,   {1,1,1}, false, {}, 272.76,  67.16,  0,       160.20,   -1.4813688},
        {"earth",   &orbits::EARTH,   6378.137, {1,1,1}, false, {}, 0,       90,     0.00335, 190.147,  360.9856235},
        {"mars",    &orbits::MARS,    3396.2,   {1,1,1}, false, {}, 317.681, 52.886, 0.00589, 176.630,  350.89198226},
        {"jupiter", &orbits::JUPITER, 71492,    {1,1,1}, false, {}, 268.057, 64.495, 0.06487, 284.95,   870.536642},
        {"saturn",  &orbits::SATURN,  60268,    {1,1,1}, false, {}, 40.589,  83.537, 0.09796, 38.90,    810.7939024},
        {"uranus",  &orbits::URANUS,  25559,    {1,1,1}, false, {}, 257.311, -15.175, 0.0229, 203.81,   -501.1600928},
        {"neptune", &orbits::NEPTUNE, 24764,    {1,1,1}, false, {}, 299.36,  43.46,  0.0171,  249.978,  541.1397757}
    };

    static constexpr int camIndex = 3;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float FOV = 40.0f;

    glm::vec2 lastM = {};

    unsigned int sVAO, sVBO, sEBO;
    int sic = 0;

    unsigned int rVAO, rVBO, rEBO;
    int ric = 0;
    unsigned int ringTex;

    bool showMap = true;
    bool mapLog = true;
    bool mapOrbits = true;
    bool mapLabels = true;
    float mapZoom = 0.0f;

    void createSphere(int stacks, int slices);
    void pointAt(int bIndex);
    void createRings(int segments);
    void createTargets();
    void postProcess();
    void drawUI();
};


#endif //SPACE_VIEWER_ENGINE_H
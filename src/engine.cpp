#include "engine.h"

#include <algorithm>
#include <complex>

#include "stb_image.h"
#include "util.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"

#include <ctime>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace util;

void setWindowCallbacks(GLFWwindow* win) {
    glfwSetScrollCallback(win, [](GLFWwindow* w, double, double dy) {
        engine* e = (engine*) glfwGetWindowUserPointer(w);
        e->onScroll(dy);
    });
}

int engine::init() {
    log(INFO, "Initalizing...");

    if (!glfwInit()) {
        log(ERRO, "GLFW Failed to initalize");
        return -1;
    }

    //window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //window creation
    window = glfwCreateWindow(winSize[0], winSize[1], "Space Viewer", NULL, NULL);
    if (window == nullptr) {
        log(ERRO, "GLFW Failed to create a window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //load glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        log(ERRO, "GLAD Failed to load");
        return -1;
    }

    glViewport(0, 0, winSize[0], winSize[1]); //size of the window
    setWindowCallbacks(window);

    //Shader initalization
    string rv = readFile("resources/main.vert");
    string rf = readFile("resources/main.frag");
    const GLchar *vert_src = rv.c_str();
    const GLchar *frag_src = rf.c_str();

    unsigned int vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_src, NULL);
    glCompileShader(vert_shader);

    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_src, NULL);
    glCompileShader(frag_shader);

    //confirm vertex shader compiled successfully
    int vsucc;
    char vinfo_log[512];
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vsucc);
    if (!vsucc) {
        glGetShaderInfoLog(vert_shader, sizeof(vinfo_log), NULL, vinfo_log);
        log(ERRO, ("Failed to compile vertex shader: " + string(vinfo_log)).c_str());
        return -1;
    }

    //confirm frag shader compiled successfully
    int fsucc;
    char finfo_log[512];
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &fsucc);
    if (!fsucc) {
        glGetShaderInfoLog(frag_shader, sizeof(finfo_log), NULL, finfo_log);
        log(ERRO, ("Failed to compile fragment shader: " + string(finfo_log)).c_str());
        return -1;
    }

    //create shader program and link
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    //link shaders and confirm success
    int lsucc;
    char linfo_log[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &lsucc);
    if (!lsucc) {
        glGetProgramInfoLog(shader_program, sizeof(linfo_log), NULL, linfo_log);
        log(ERRO, ("Failed to link shaders: " + string(linfo_log)).c_str());
        return -1;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    //get uniform locs
    viewLoc = glGetUniformLocation(shader_program, "view");
    projLoc = glGetUniformLocation(shader_program, "proj");
    modelLoc = glGetUniformLocation(shader_program, "model");
    camPosLoc = glGetUniformLocation(shader_program, "camPosition");
    sunDirLoc = glGetUniformLocation(shader_program, "sunDir");
    colorLoc = glGetUniformLocation(shader_program, "baseColor");
    emissiveLoc = glGetUniformLocation(shader_program, "emissive");

    //opengl settings and misc stuff
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);

    log(INFO, "Initalized successfully");

    double time = glfwGetTime();

    startup = time;
    lastfpsIncrement = time;
    lastframeT = time;

    createSphere(64, 128);
    simClock = orbits::julianDate((double) std::time(nullptr));

    return 0;
}

void engine::close() {
    double time = glfwGetTime();

    glfwTerminate();

    log(INFO, ("Session lasted " + to_string(time - startup) + "s").c_str());
}

void engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glm::dvec3 cam = bodies[camIndex].posKm;
    const double R = 100.0;

    glm::vec3 fwd(std::cos(pitch) * std::cos(yaw), std::sin(pitch), std::cos(pitch) * std::sin(yaw));
    glm::mat4 view = glm::lookAt(glm::vec3(0), fwd, glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(FOV), winSize.x / winSize.y, 0.1f, 1000.0f);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    struct Item {int i; double d;};
    std::vector<Item> order;

    for (int i = 0; i < (int)bodies.size(); i++) {
        if (i!=camIndex) {
            order.push_back({
                i,
                glm::length(bodies[i].posKm- cam),
            });
        }
    }

    std::sort(order.begin(), order.end(), [](const Item& a, const Item& b)-> bool {
        return a.d > b.d;
    });

    glBindVertexArray(sVAO);
    for (const auto& it : order) {
        const auto &b = bodies[it.i];

        //draw each body as its own scene
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::dvec3 relGL = toGL(b.posKm - cam);
        glm::vec3 p = glm::vec3(relGL / it.d * R);

        float s = float(b.radiusKm * R / it.d);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), p) * glm::scale(glm::mat4(1.0f), glm::vec3(s));
        glm::vec3 sunDir = glm::vec3(toGL(glm::normalize(-b.posKm)));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(sunDirLoc, 1, glm::value_ptr(sunDir));
        glUniform3fv(colorLoc, 1, glm::value_ptr(b.color));
        glUniform1i(emissiveLoc, b.emissive ? 1 : 0);
        glDrawElements(GL_TRIANGLES, sic, GL_UNSIGNED_INT, 0);
    }

    glfwSwapBuffers(window);
}

void engine::update() {
    glfwPollEvents();

    double t = glfwGetTime();
    dt = t - lastframeT;
    lastframeT = t;

    //z speeds up time x slows
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) timeScale *= 1.0 + 2.0 * dt;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) timeScale /= 1.0 + 2.0 * dt;

    simClock += dt * timeScale / 86400.0;
    double T = orbits::centuries(simClock);
    for (auto& b : bodies)
        b.posKm = b.el ? orbits::position(*b.el, T) * orbits::AU_KM : glm::dvec3(0.0);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        float degPerPx = FOV / winSize.y;

        yaw -= glm::radians(float(mx - lastM.x) * degPerPx);
        pitch += glm::radians(float(my - lastM.y) * degPerPx);

        pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));
    }
    lastM = {mx, my};

    for (int i = 0; i < (int) bodies.size() && i <= 9; i++) {
        if (i != camIndex && glfwGetKey(window, GLFW_KEY_0 + i) == GLFW_PRESS) {
            pointAt(i);
        }
    }

    if (t - lastfpsIncrement >= 1) {
        glfwSetWindowTitle(window, ("Space Viewer | FPS " + to_string(frames) + " | " + to_string(timeScale) + "x | FOV " + to_string(FOV)).c_str());
        lastfpsIncrement = t;
        frames = 0;
    }
    frames++;
}

void engine::createSphere(int stacks, int slices) {
    std::vector<float> v;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= stacks; i++) {
        float phi = orbits::PI * i / stacks;
        for (int j = 0; j <= slices; j++) {
            float th = 2.0f * orbits::PI * j / slices;

            float x = std::sin(phi) * std::cos(th);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(th);

            v.insert(v.end(), {x, y, z, x, y, z, float(j)/slices, 1.0f - float(i)/stacks});
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            unsigned a = i * (slices + 1) + j;
            unsigned b = a + slices + 1;

            idx.insert(idx.end(), {a, a+ 1, b, b, a + 1, b + 1});
        }
    }

    sic = (int)idx.size();

    //buffer setup
    glGenVertexArrays(1, &sVAO);

    glGenBuffers(1, &sVBO);
    glGenBuffers(1, &sEBO);

    glBindVertexArray(sVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);

    //vert attribs
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void engine::pointAt(int bIndex) {
    glm::dvec3 d = toGL(glm::normalize(bodies[bIndex].posKm - bodies[camIndex].posKm));

    yaw = (float) std::atan2(d.z, d.x);
    pitch = (float) std::asin(d.y);
}

void engine::onScroll(double dy) {
    FOV = glm::clamp(float(FOV * std::pow(0.85, dy)), 0.005f, 90.0f);
}
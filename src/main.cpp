#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"

#include "engine.h"

int main() {
    engine* e = new engine();
    if (e->init()!=0) return -1;

    while (!glfwWindowShouldClose(e->getWindow())) {
        e->update();
        e->render();
    }

    e->close();
}
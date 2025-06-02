#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

#define NUM_PARTICLES 1000

struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec4 color;
    float life;
};

std::vector<Particle> particles(NUM_PARTICLES);
float lastTime = 0.0f;

GLuint shaderProgram, vao, vbo;

void initParticles() {
    for (auto& p : particles) {
        p.pos = glm::vec2(0.0f);
        float angle = ((rand() % 360) / 360.0f) * 6.2831f;
        float speed = (rand() % 50) / 50.0f;
        p.vel = glm::vec2(cos(angle), sin(angle)) * speed;
        p.color = glm::vec4(1.0, (rand() % 100) / 100.0f, 0.2, 1.0);
        p.life = (rand() % 100) / 100.0f + 1.0f;
    }
}

void updateParticles(float dt) {
    for (auto& p : particles) {
        p.life -= dt;
        if (p.life < 0.0f) {
            float angle = ((rand() % 360) / 360.0f) * 6.2831f;
            float speed = (rand() % 50) / 50.0f;
            p.pos = glm::vec2(0.0f);
            p.vel = glm::vec2(cos(angle), sin(angle)) * speed;
            p.life = 1.0f + (rand() % 100) / 100.0f;
            p.color.a = 1.0f;
        } else {
            p.vel.y -= 0.98f * dt; // gravity
            p.pos += p.vel * dt;
            p.color.a = p.life;
        }
    }
}

GLuint loadShader(const char* path, GLenum type) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char* src = new char[len + 1];
    fread(src, 1, len, f);
    src[len] = 0;
    fclose(f);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    delete[] src;

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "Shader Error: " << log << std::endl;
    }
    return shader;
}

void setup() {
    GLuint vs = loadShader("shaders/particle.vert", GL_VERTEX_SHADER);
    GLuint fs = loadShader("shaders/particle.frag", GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * particles.size(), nullptr, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, color)));
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * particles.size(), particles.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDrawArrays(GL_POINTS, 0, particles.size());
}

int main() {
    srand((unsigned int)time(0));
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Particle System", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    gladLoadGL();

    glViewport(0, 0, 800, 600);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_PROGRAM_POINT_SIZE);

    setup();
    initParticles();

    while (!glfwWindowShouldClose(window)) {
        float current = glfwGetTime();
        float dt = current - lastTime;
        lastTime = current;

        updateParticles(dt);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


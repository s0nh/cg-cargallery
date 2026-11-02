#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "InitShader.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include "Camera.h"
#include "Room.h"
#include "Pedestal.h"
#include "Car.h"
#include "CustomizePanel.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
int fbWidth = SCR_WIDTH, fbHeight = SCR_HEIGHT;  // current framebuffer size, for the projection aspect

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window, float dt);

Camera camera(glm::vec3(0.0f, 1.7f, 10.0f));  // 1.7 = eye height; near the open front of the hall, looking at the bays

// mouse state: the first event only records the position, otherwise the view jumps
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;

// Tab toggles UI mode: the cursor is freed so ImGui can be clicked, and the view stops turning
bool uiMode = false;

// the walker's size on the floor plan for collision: a circle this wide around the camera
const float BODY_RADIUS = 0.3f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CarGallery", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // register our callbacks BEFORE ImGui init: ImGui chains to callbacks that already exist
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projLoc = glGetUniformLocation(program, "projection");

    // round hall: 14 m radius, 6 m to the flat ceiling.
    // 5 bays 54 degrees apart around the back half (-90 = straight back); the front (+z) stays open
    // as the entrance side. Listed left to right as seen from the entrance.
    Room room(14.0f, 6.0f, { 162.0f, -144.0f, -90.0f, -36.0f, 18.0f }, program);

    // one pedestal per bay (radius 3 m, 30 cm high), in the middle of the bay's round end
    std::vector<Pedestal> pedestals;
    for (const Bay& bay : room.bays())
        pedestals.emplace_back(bay.center, 3.0f, 0.3f, program);

    // each car faces the hall center, then turns 30 degrees more so the visitor sees front and side.
    // a turn of y degrees points the car's front (+z) at (sin y, 0, cos y)
    std::vector<float> carYaw;
    for (const Bay& bay : room.bays())
        carYaw.push_back(glm::degrees(atan2(bay.inward.x, bay.inward.z)) + 30.0f);

    // arguments: name, OBJ file (made by tools/gltf2obj.py), paint material name, where to stand,
    // turn in degrees, real length (m)
    std::vector<Car> cars;
    cars.emplace_back("Lamborghini Countach", "assets/cars/countach.obj", "lLamborghini_CountachLPI8004_2022Paint_Material1", pedestals[0].top(), carYaw[0], 4.9f, program);
    cars.emplace_back("Tesla Roadster", "assets/cars/roadster.obj", "car_main_paint", pedestals[1].top(), carYaw[1], 4.6f, program);
    cars.emplace_back("Mercedes-Benz S65", "assets/cars/s65.obj", "sw222_paint", pedestals[2].top(), carYaw[2], 5.1f, program);
    cars.emplace_back("Jeep Gladiator", "assets/cars/gladiator.obj", "Jeep_GladiatorRewardRecycled_2019Paint_Material", pedestals[3].top(), carYaw[3], 5.5f, program);
    cars.emplace_back("Kia Optima K5", "assets/cars/optima.obj", "insta_ua1k.006", pedestals[4].top(), carYaw[4], 4.9f, program);

    // starting paint, left to right in rainbow order; the Customize panel can change each one
    const glm::vec3 rainbow[5] = {
        glm::vec3(0.85f, 0.10f, 0.10f),  // red
        glm::vec3(0.95f, 0.45f, 0.05f),  // orange
        glm::vec3(0.95f, 0.80f, 0.10f),  // yellow
        glm::vec3(0.15f, 0.60f, 0.20f),  // green
        glm::vec3(0.10f, 0.30f, 0.90f),  // blue
    };
    for (int i = 0; i < 5; i++)
        cars[i].config.bodyColor = rainbow[i];
    CustomizePanel customizePanel;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);  // a room face with the wrong winding disappears, so mistakes are visible

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;

        glfwPollEvents();
        processInput(window, dt);

        // collision after moving: out of the stands (and so the cars on them), then back inside the walls
        for (const Pedestal& p : pedestals)
            camera.position = p.pushOut(camera.position, BODY_RADIUS);
        camera.position = room.collide(camera.position, BODY_RADIUS);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Camera");
        ImGui::Text("pos   %.2f %.2f %.2f", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("yaw   %.1f", camera.yaw);
        ImGui::Text("pitch %.1f", camera.pitch);
        ImGui::End();
        customizePanel.draw(cars);
        ImGui::Render();

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glm::mat4 projMat = glm::perspective(glm::radians(60.0f), (float)fbWidth / fbHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));
        room.draw(program);
        for (const Pedestal& p : pedestals) p.draw(program);
        for (const Car& car : cars) car.draw(program);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// held keys are polled every frame (a key callback only fires on press/repeat, so movement would stutter)
void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float forward = 0.0f, right = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) forward += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) forward -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) right += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) right -= 1.0f;
    bool sprint = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    camera.move(forward, right, sprint ? dt * 2.0f : dt);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        uiMode = !uiMode;
        glfwSetInputMode(window, GLFW_CURSOR, uiMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        firstMouse = true;  // the cursor jumps when the mode changes; don't turn that jump into a rotation
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (uiMode) return;
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos;  // screen y grows downward, so flip it
    lastX = (float)xpos;
    lastY = (float)ypos;
    camera.rotate(dx, dy);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    // minimizing gives 0x0; keep the old size so the aspect ratio never divides by zero
    if (width > 0 && height > 0) {
        fbWidth = width;
        fbHeight = height;
    }
}

#include "imgui_window.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstring>
#include <filesystem>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>

bool pickSceneXmlFromDialog(
    char* sceneDirBuf, size_t sceneDirBufSize, char* sceneNameBuf, size_t sceneNameBufSize
) {
    char filePath[MAX_PATH] = "";
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "XML Scene (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "xml";

    if (!GetOpenFileNameA(&ofn)) return false;

    const std::filesystem::path selectedPath(filePath);
    const std::string sceneDir = selectedPath.parent_path().string();
    const std::string sceneName = selectedPath.stem().string();

    strncpy_s(sceneDirBuf, sceneDirBufSize, sceneDir.c_str(), _TRUNCATE);
    strncpy_s(sceneNameBuf, sceneNameBufSize, sceneName.c_str(), _TRUNCATE);
    return true;
}

RtImGuiWindow::RtImGuiWindow(
    int width, int height, const char* title, const char* defaultSceneDir,
    const char* defaultSceneName
)
    : windowWidth(width), WindowHeight(height) {
    strncpy_s(sceneDirBuf, sizeof(sceneDirBuf), defaultSceneDir, _TRUNCATE);
    strncpy_s(sceneNameBuf, sizeof(sceneNameBuf), defaultSceneName, _TRUNCATE);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    initImGui();
    createTexture();
}

RtImGuiWindow::~RtImGuiWindow() {
    cleanup();
    glfwTerminate();
}

bool RtImGuiWindow::shouldClose() const { return glfwWindowShouldClose(window); }

void RtImGuiWindow::beginFrame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

void RtImGuiWindow::endFrame() {
    ImGui::Render();
    int displayW = 0, displayH = 0;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClearColor(0.08f, 0.08f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

RtWindowActions RtImGuiWindow::renderControls(RtRenderState& state) {
    RtWindowActions actions;

    ImGui::Begin("Controls");

    if (ImGui::Button("Start")) {
        state.rendering = true;
        state.currentSample = 0;
        actions.restart = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(state.rendering ? "Pause" : "Resume")) { state.rendering = !state.rendering; }
    ImGui::SameLine();
    if (ImGui::Button("Save")) { actions.save = true; }
    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        if (pickSceneXmlFromDialog(
                sceneDirBuf, sizeof(sceneDirBuf), sceneNameBuf, sizeof(sceneNameBuf)
            )) {
            state.rendering = false;
            state.currentSample = 0;
            actions.importModel = true;
        }
    }

    ImGui::Text("Scene Dir: %s", sceneDirBuf);
    ImGui::Text("Scene Name: %s", sceneNameBuf);

    ImGui::SliderInt("Max Sample", &state.maxSamples, 1, 3000);
    ImGui::SliderInt("Max Depth", &state.maxDepth, 1, 32);
    ImGui::SliderInt("Samples / Frame", &state.samplesPerFrame, 1, 8);
    ImGui::Checkbox("Debug Normal", &state.debugNormal);
    if (state.currentSample >= state.maxSamples) state.rendering = false;
    ImGui::Text("Current Sample:%d", state.currentSample);
    ImGui::Text("Rendering:%s", state.rendering ? "Yes" : "No");
    ImGui::Text("Time: %.2f seconds", state.elapsTime);
    ImGui::ProgressBar(
        static_cast<float>(state.currentSample) / static_cast<float>(std::max(1, state.maxSamples)),
        ImVec2(0.0f, 0.0f)
    );

    ImGui::End();
    return actions;
}

void RtImGuiWindow::renderTexture(const Film& film, int sampleCount) {
    ImGui::Begin("Ray Tracing Preview");

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const int newViewportW = std::max(1, static_cast<int>(avail.x));
    const int newViewportH = std::max(1, static_cast<int>(avail.y));
    if (newViewportW != viewportWidth || newViewportH != viewportHeight) {
        viewportWidth = newViewportW;
        viewportHeight = newViewportH;
        viewportResized = true;
    }

    updateTexture(film, sampleCount);
    ImGui::Image(
        (void*)(intptr_t)textureID,
        ImVec2(static_cast<float>(film.width), static_cast<float>(film.height))
    );
    ImGui::End();
}

bool RtImGuiWindow::consumeViewportResize(int& width, int& height) {
    if (!viewportResized) return false;
    width = viewportWidth;
    height = viewportHeight;
    viewportResized = false;
    return true;
}

std::string RtImGuiWindow::getSceneDir() const { return sceneDirBuf; }

std::string RtImGuiWindow::getSceneName() const { return sceneNameBuf; }

void RtImGuiWindow::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void RtImGuiWindow::cleanup() {
    glDeleteTextures(1, &textureID);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void RtImGuiWindow::createTexture() {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void RtImGuiWindow::updateTexture(const Film& film, int sampleCount) {
    std::vector<unsigned char> imageData;
    film.toRGB8(imageData, sampleCount, true);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, film.width, film.height, 0, GL_RGB, GL_UNSIGNED_BYTE,
        imageData.data()
    );
}

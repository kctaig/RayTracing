#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <film.hpp>

struct RtWindowActions {
    bool restart = false;
    bool save = false;
    bool importModel = false;
};

struct RtRenderState {
    int currentSample = 0;
    int maxSamples = 100;
    int samplesPerFrame = 1;
    float elapsTime = 0.0f;
    bool rendering = false;
};

class RtImGuiWindow {
  public:
    RtImGuiWindow(
        int width, int height, const char* title, const char* defaultSceneDir,
        const char* defaultSceneName
    );
    ~RtImGuiWindow();

    bool shouldClose() const;
    void beginFrame();
    void endFrame();
    RtWindowActions renderControls(RtRenderState& state);
    void renderTexture(const Film& film, int sampleCount = 1);
    bool consumeViewportResize(int& width, int& height);
    std::string getSceneDir() const;
    std::string getSceneName() const;

  private:
    GLFWwindow* window;
    unsigned int textureID;
    int windowWidth, WindowHeight;
    int viewportWidth{0};
    int viewportHeight{0};
    bool viewportResized{false};
    char sceneDirBuf[256]{};
    char sceneNameBuf[128]{};

    void initImGui();
    void cleanup();
    void createTexture();
    void updateTexture(const Film& film, int sampleCount);
};
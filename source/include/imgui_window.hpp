#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <film.hpp>
#include <functional>

struct RtWindowActions {
  bool restart = false;
  bool save = false;
  bool importModel = false;
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
  RtWindowActions renderControls(
    bool& rendering, int& currentSample, int& maxSamples, int& samplesPerFrame
  );
    void renderTexture(const Film& film, int sampleCount = 1);
    bool consumeViewportResize(int& width, int& height);
  std::string getSceneDir() const;
  std::string getSceneName() const;

    void setRenderCallback(std::function<void()> callback) { renderCallback = callback; }

  private:
    GLFWwindow* window;
    unsigned int textureID;
    int windowWidth, WindowHeight;
    int viewportWidth{0};
    int viewportHeight{0};
    bool viewportResized{false};
  char sceneDirBuf[256]{};
  char sceneNameBuf[128]{};
    std::function<void()> renderCallback;

    void initImGui();
    void cleanup();
    void createTexture();
    void updateTexture(const Film& film, int sampleCount);
};
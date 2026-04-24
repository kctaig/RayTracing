#include <iostream>

#include "imgui_window.hpp"
#include "render.hpp"

int main() {
    std::string sceneDir = "../../datasets/cornell-box";
    std::string fileName = "cornell-box";
    Render render(sceneDir, fileName);
    render.setMaxDepth(5);
    render.setNumIter(10);
    render.setNumSamples(100);

    RtImGuiWindow window(1280, 720, "Ray Tracing", sceneDir.c_str(), fileName.c_str());

    bool rendering = false;
    int currentSample = 0;
    int maxSamples = 100;
    int samplesPerFrame = 1;

    while (!window.shouldClose()) {
        window.beginFrame();
        const RtWindowActions actions =
            window.renderControls(rendering, currentSample, maxSamples, samplesPerFrame);

        if (actions.importModel) {
            sceneDir = window.getSceneDir();
            fileName = window.getSceneName();
            if (!sceneDir.empty() && !fileName.empty() && fileExists(sceneDir, fileName)) {
                render = Render(sceneDir, fileName);
                render.setMaxDepth(5);
                render.setNumIter(10);
                render.setNumSamples(100);
            } else {
                std::cerr << "Import failed. Scene XML not found for '" << fileName
                          << "' under base dir: " << sceneDir << std::endl;
            }
            currentSample = 0;
        }

        if (actions.restart) {
            currentSample = 0;
            render.film->clear();
        }

        if (actions.save) { render.film->saveToFile(fileName, std::max(1, currentSample)); }

        if (rendering) {
            for (int i = 0; i < samplesPerFrame && currentSample < maxSamples; ++i) {
                render.renderOneSample();
                ++currentSample;
            }
            if (currentSample >= maxSamples) rendering = false;
        }

        window.renderTexture(*render.film, currentSample);
        int viewportWidth = 0;
        int viewportHeight = 0;
        if (window.consumeViewportResize(viewportWidth, viewportHeight)) {
            render.film->reset(viewportWidth, viewportHeight);
            currentSample = 0;
            rendering = false;
        }
        window.endFrame();
    }

    return 0;
}
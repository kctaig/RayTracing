#include <chrono>
#include <iostream>

#include "render.hpp"
#include "window.hpp"


int main() {
    const fs::path projectRoot = fs::current_path();
    std::string sceneDir = (projectRoot / "../datasets/CornellBox").string();
    std::string fileName = "CornellBox-Sphere";
    auto buildRender = [](const std::string& dir, const std::string& name) {
        auto r = std::make_shared<Render>(dir, name);
        r->setMaxDepth(10);
        r->setNumIter(10);
        r->setNumSamples(100);
        return r;
    };
    auto render = buildRender(sceneDir, fileName);
    RtRenderState renderState;
    renderState.rendering = true;
    renderState.currentSample = 0;
    renderState.maxSamples = 100;
    renderState.maxDepth = 10;
    renderState.samplesPerFrame = 1;
    renderState.elapsTime = 0.0f;
    renderState.debugNormal = false;
    bool lastDebugNormal = renderState.debugNormal;
    int lastMaxDepth = renderState.maxDepth;

    RtImGuiWindow window(1024, 700, "Ray Tracing", sceneDir.c_str(), fileName.c_str());
    auto renderStartTime = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose()) {
        window.beginFrame();
        const RtWindowActions actions = window.renderControls(renderState);

        if (actions.importModel) {
            sceneDir = window.getSceneDir();
            fileName = window.getSceneName();
            if (!sceneDir.empty() && !fileName.empty() && fileExists(sceneDir, fileName)) {
                render = buildRender(sceneDir, fileName);
                render->setMaxDepth(renderState.maxDepth);
                render->setDebugNormalMode(renderState.debugNormal);
                renderState.currentSample = 0;
                renderState.elapsTime = 0.0f;
                renderState.rendering = false;
                renderStartTime = std::chrono::high_resolution_clock::now();
            } else {
                std::cerr << "Import failed. Scene XML not found for '" << fileName
                          << "' under base dir: " << sceneDir << std::endl;
            }
        }

        if (actions.restart) {
            renderState.currentSample = 0;
            renderState.elapsTime = 0.0f;
            render->film->clear();
            renderStartTime = std::chrono::high_resolution_clock::now();
        }

        if (renderState.debugNormal != lastDebugNormal) {
            renderState.currentSample = 0;
            renderState.elapsTime = 0.0f;
            renderState.rendering = false;
            render->film->clear();
            renderStartTime = std::chrono::high_resolution_clock::now();
            lastDebugNormal = renderState.debugNormal;
        }

        if (renderState.maxDepth != lastMaxDepth) {
            renderState.currentSample = 0;
            renderState.elapsTime = 0.0f;
            renderState.rendering = false;
            render->film->clear();
            renderStartTime = std::chrono::high_resolution_clock::now();
            lastMaxDepth = renderState.maxDepth;
        }

        render->setMaxDepth(renderState.maxDepth);
        render->setDebugNormalMode(renderState.debugNormal);

        if (renderState.rendering) {
            for (int i = 0; i < renderState.samplesPerFrame &&
                            renderState.currentSample < renderState.maxSamples;
                 ++i) {
                render->renderOneSample(renderState.currentSample);
                ++renderState.currentSample;
            }
            if (renderState.currentSample >= renderState.maxSamples) {
                renderState.rendering = false;
            }
            const auto currentTime = std::chrono::high_resolution_clock::now();
            renderState.elapsTime =
                std::chrono::duration<float>(currentTime - renderStartTime).count();
        }

        if (actions.save) {
            render->film->saveToFile(fileName, std::max(1, renderState.currentSample));
        }

        window.renderTexture(*render->film, renderState.currentSample);

        int viewportWidth = 0, viewportHeight = 0;
        if (window.consumeViewportResize(viewportWidth, viewportHeight)) {
            render->film->reset(viewportWidth, viewportHeight);
            renderState.currentSample = 0;
            renderState.rendering = false;
        }

        window.endFrame();
    }

    return 0;
}

#include "render.hpp"

int main() {
    const std::string sceneDir = "../../datasets";
    // const std::string fileName = "cornell-box";
    // const std::string fileName = "veach-mis";
    const std::string fileName = "bathroom2";

    Render render(sceneDir + "/" + fileName, fileName);
    // render.film->scale(0.5f);
    render.setMaxDepth(5);
    render.setNumIter(10);
    render.setNumSamples(5000);
    render.render();
    return 0;
}
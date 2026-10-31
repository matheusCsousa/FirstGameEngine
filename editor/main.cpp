#include "Editor.hpp"
#include "../core/Game/Game.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main(int argc, char** argv) {
    try {
        bool smokeTest = false;
        std::filesystem::path assets = std::filesystem::absolute(argv[0]).parent_path() / "assets";
        for (int i = 1; i < argc; ++i) {
            const std::string_view argument(argv[i]);
            if (argument == "--smoke-test") smokeTest = true;
            else if (argument == "--assets" && i + 1 < argc) assets = argv[++i];
            else throw std::invalid_argument("Usage: fersa_editor [--smoke-test] [--assets DIRECTORY]");
        }
        for (const auto* file : {"models/cube.obj", "shader/shader.vert", "shader/shader.frag", "textures/brick.jpg"}) {
            if (!std::filesystem::is_regular_file(assets / file))
                throw std::runtime_error("Missing sample asset: " + (assets / file).string());
        }
        Core::GameSpecs specs;
        specs.title = "Fersa - Scene Editor";
        specs.windowSpec.width = 1280;
        specs.windowSpec.height = 720;
        specs.windowSpec.visible = !smokeTest;
        Core::Game game(specs);
        game.pushLogic<Editor>(assets);
        game.run(smokeTest ? 3 : 0);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Engine startup failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}

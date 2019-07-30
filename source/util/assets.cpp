#include "assets.h"

#include <fstream>
#include <sstream>
#include <SDL2/SDL_filesystem.h>
#include <iostream>

namespace Utils {
    namespace {
        std::string DoGetDataPath() {
            char *base_path = SDL_GetBasePath();
            if (base_path) {
                std::string ret {base_path};
                SDL_free(base_path);
                return ret + "/assets/";
            } else {
                return "./assets/";
            }
        }

        std::string GetDataPath() {
            static std::string dataPath = DoGetDataPath();
            return dataPath;
        }
    }

    std::string Assets::LoadFullText(const std::string& rel) {
        std::ifstream t = LoadAsStream(rel);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    std::unique_ptr<char[]> Assets::LoadFullBytes(const std::string& rel) {
        std::ifstream t = LoadAsStream(rel, std::ios::binary);
        t.seekg(0, std::ios::end);
        const auto size = t.tellg();
        auto buffer = std::make_unique<char[]>(size);
        t.seekg(0);
        t.read(buffer.get(), size);
        return buffer;
    }

    std::ifstream Assets::LoadAsStream(const std::string& rel, std::ios::openmode mode) {
        auto fs = std::ifstream(GetDataPath() + rel, mode);
        if (fs.good()) {
            return fs;
        }
        else {
            throw NotExist();
        }
    }
}
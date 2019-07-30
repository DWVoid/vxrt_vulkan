#pragma once

#include <string>
#include <memory>
#include <fstream>
#include "exceptions.h"

namespace Utils {
    class Assets {
    public:
        VXRT_EXCEPTION(NotExist, "Asset Does Not Exist");
        static std::ifstream LoadAsStream(const std::string& rel, std::ios::openmode mode = 0);
        static std::string LoadFullText(const std::string& rel);
        static std::unique_ptr<char[]> LoadFullBytes(const std::string& rel);
    };
}

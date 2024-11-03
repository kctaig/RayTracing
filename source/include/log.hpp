#pragma once 

#include "head_include.hpp"
#include <fstream>

class Logger {

public:
    Logger(const std::string& filename);

    ~Logger();
    // 手动恢复原始缓冲区的函数
    void RestoreOriginalBuffers();

private:
    std::streambuf* originalCoutBuffer;
    std::streambuf* originalCerrBuffer;
    std::ofstream logFileStream;
};
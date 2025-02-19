#include "log.hpp"

Logger::Logger(const std::string &filename)
{
    // 保存原始的 cout 和 cerr 的缓冲区指针
    originalCoutBuffer = std::cout.rdbuf();
    originalCerrBuffer = std::cerr.rdbuf();
    // 打开日志文件
    logFileStream.open(filename, std::ios::out);
    // logFileStream.open(filename, std::ios::out | std::ios::app);
    if (!logFileStream.is_open())
    {
        std::cerr << "无法打开日志文件: " << filename << std::endl;
        return;
    }
    // 将 cout 和 cerr 重定向到日志文件
    std::cout.rdbuf(logFileStream.rdbuf());
    std::cerr.rdbuf(logFileStream.rdbuf());
}

Logger::~Logger()
{
    RestoreOriginalBuffers();
    // 关闭日志文件
    if (logFileStream.is_open())
    {
        logFileStream.close();
    }
}

void Logger::RestoreOriginalBuffers()
{
    if (logFileStream.is_open())
    {
        std::cout.rdbuf(originalCoutBuffer);
        std::cerr.rdbuf(originalCerrBuffer);
    }
}
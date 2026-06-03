#pragma once

#include "logging/ILogSink.h"
#include <cstdio>
#include <mutex>
#include <string>

class FileLogSink : public ILogSink
{
public:
    explicit FileLogSink(const std::string& path);
    ~FileLogSink() override;

    bool IsOpen() const;
    void Write(const LogRecord& record) override;

private:
    std::FILE* fp;
    std::mutex mutex;
};

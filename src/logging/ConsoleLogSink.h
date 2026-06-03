#pragma once

#include "logging/ILogSink.h"

class ConsoleLogSink : public ILogSink
{
public:
    ConsoleLogSink();
    void Write(const LogRecord& record) override;
};

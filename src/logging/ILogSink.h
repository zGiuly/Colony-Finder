#pragma once

#include "logging/LogRecord.h"

class ILogSink
{
public:
    virtual ~ILogSink() = default;
    virtual void Write(const LogRecord& record) = 0;
};

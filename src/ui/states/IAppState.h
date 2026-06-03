#pragma once

class AppController;

class IAppState
{
public:
    virtual ~IAppState() = default;
    virtual void Render(AppController* controller) = 0;
};

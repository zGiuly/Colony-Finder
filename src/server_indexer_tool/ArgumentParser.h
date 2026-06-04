#pragma once
#include <string>

struct CliConfig
{
    std::string downloadUrl;
    std::string localFilePath;
    std::string downloadDir;
    std::string searchDir;
    int bufferSizeMb = 0;
    bool updateSchema = false;
    bool interactiveMode = false;
    bool selfUpdate = false;
    bool showHelp = false;
    bool hasError = false;
};

class ArgumentParser
{
public:
    static CliConfig Parse(int argc, char** argv);
    static int ParseBufferSize(const std::string& sizeStr);
};

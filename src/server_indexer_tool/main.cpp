#include "ArgumentParser.h"
#include "CliAppController.h"
#include "CliSelfUpdater.h"

int main(int argc, char** argv)
{
    CliConfig config = ArgumentParser::Parse(argc, argv);
    if (config.hasError)
    {
        return 1;
    }
    if (config.showHelp)
    {
        return 0;
    }
    if (config.selfUpdate)
    {
        return CliSelfUpdater::Run() ? 0 : 1;
    }

    CliAppController controller;
    return controller.Run(config);
}

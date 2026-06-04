#include "ArgumentParser.h"
#include <iostream>
#include <unordered_map>

enum class Option
{
    Help,
    Interactive,
    UpdateSchema,
    Download,
    Local,
    BufferSize,
    DownloadDir,
    SearchDir,
    Update,
    Unknown
};

namespace
{
    void PrintHelp()
    {
        std::cout << "ColonyFinder Server Indexer Tool\n"
                  << "Usage: colony_indexer [options]\n\n"
                  << "Options:\n"
                  << "  -h, --help                Show this help message.\n"
                  << "  -i, --interactive         Run in interactive mode (fallback menu).\n"
                  << "  -d, --download <option>   Download the database. Option can be:\n"
                  << "                            'full'    - Download full database (galaxy.json.gz)\n"
                  << "                            '1month'  - Download 1 month database (galaxy_1month.json.gz)\n"
                  << "                            <url>     - Download from a custom URL\n"
                  << "  -l, --local <file>        Process and index a local .json or .json.gz file.\n"
                  << "  -b, --buffer-size <mb>    Decompression buffer size in MB (1-64). Default is 16.\n"
                  << "  --download-dir <path>     Directory where downloaded files are saved. Default is '.'.\n"
                  << "  --search-dir <path>       Directory where index files are generated/searched. Default is '.'.\n"
                  << "  -s, --update-schema       Update Spansh database schema and exit.\n"
                  << "  -u, --update              Check for colony_indexer updates and apply them.\n";
    }



    Option ParseOption(const std::string& arg)
    {
        static const std::unordered_map<std::string, Option> optionMap = {
            {"-h", Option::Help},
            {"--help", Option::Help},
            {"-i", Option::Interactive},
            {"--interactive", Option::Interactive},
            {"-s", Option::UpdateSchema},
            {"--update-schema", Option::UpdateSchema},
            {"-d", Option::Download},
            {"--download", Option::Download},
            {"-l", Option::Local},
            {"--local", Option::Local},
            {"-b", Option::BufferSize},
            {"--buffer-size", Option::BufferSize},
            {"--download-dir", Option::DownloadDir},
            {"--search-dir", Option::SearchDir},
            {"-u", Option::Update},
            {"--update", Option::Update}
        };
        auto it = optionMap.find(arg);
        if (it == optionMap.end())
        {
            return Option::Unknown;
        }
        return it->second;
    }

    bool CheckMissingArg(int i, int argc, const std::string& optName)
    {
        if (i + 1 >= argc)
        {
            std::cerr << "Error: Option " << optName << " requires an argument.\n";
            return true;
        }
        return false;
    }
}

CliConfig ArgumentParser::Parse(int argc, char** argv)
{
    CliConfig config;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        Option opt = ::ParseOption(arg);

        switch (opt)
        {
            case Option::Help:
                ::PrintHelp();
                config.showHelp = true;
                return config;
            case Option::Interactive:
                config.interactiveMode = true;
                continue;
            case Option::UpdateSchema:
                config.updateSchema = true;
                continue;
            case Option::Update:
                config.selfUpdate = true;
                continue;
            case Option::Download:
                if (::CheckMissingArg(i, argc, arg))
                {
                    config.hasError = true;
                    return config;
                }
                {
                    std::string val = argv[++i];
                    config.downloadUrl = (val == "full") ? "https://downloads.spansh.co.uk/galaxy.json.gz" :
                                         (val == "1month") ? "https://downloads.spansh.co.uk/galaxy_1month.json.gz" : val;
                }
                break;
            case Option::Local:
                if (::CheckMissingArg(i, argc, arg))
                {
                    config.hasError = true;
                    return config;
                }
                config.localFilePath = argv[++i];
                break;
            case Option::BufferSize:
                if (::CheckMissingArg(i, argc, arg))
                {
                    config.hasError = true;
                    return config;
                }
                config.bufferSizeMb = ArgumentParser::ParseBufferSize(argv[++i]);
                if (config.bufferSizeMb == 0)
                {
                    std::cerr << "Error: Buffer size must be between 1 and 64.\n";
                    config.hasError = true;
                    return config;
                }
                break;
            case Option::DownloadDir:
                if (::CheckMissingArg(i, argc, arg))
                {
                    config.hasError = true;
                    return config;
                }
                config.downloadDir = argv[++i];
                break;
            case Option::SearchDir:
                if (::CheckMissingArg(i, argc, arg))
                {
                    config.hasError = true;
                    return config;
                }
                config.searchDir = argv[++i];
                break;
            case Option::Unknown:
            default:
                std::cerr << "Unknown option: " << arg << "\n";
                ::PrintHelp();
                config.hasError = true;
                return config;
        }
    }

    return config;
}

int ArgumentParser::ParseBufferSize(const std::string& sizeStr)
{
    try
    {
        int val = std::stoi(sizeStr);
        if (val < 1 || val > 64)
        {
            return 0;
        }
        return val;
    }
    catch (...)
    {
    }
    return 0;
}

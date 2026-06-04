#pragma once

namespace UiStrings
{
    namespace App
    {
        inline constexpr const char* WindowTitle       = "ColonyFinder - Elite Dangerous Tool";
    }

    namespace Common
    {
        inline constexpr const char* Back              = "< Back";
        inline constexpr const char* Cancel            = "Cancel";
        inline constexpr const char* CancelDownload    = "Cancel Download";
        inline constexpr const char* Settings          = "Settings";
        inline constexpr const char* BackToSetup       = "Back to Setup";
        inline constexpr const char* BrowseTargetDir   = "Browse Target Dir...";
        inline constexpr const char* BrowseSearchDir   = "Browse Search Dir...";
        inline constexpr const char* TargetDirectory   = "Target Directory:";
        inline constexpr const char* SearchDirectory   = "Search Directory:";
    }

    namespace Welcome
    {
        inline constexpr const char* Title             = ">> COLONY FINDER - ELITE DANGEROUS DATABASE";
        inline constexpr const char* Greeting          = "Welcome, Commander.";
        inline constexpr const char* DumpDetectedFmt   = "[+] Spansh dump file detected: %s";
        inline constexpr const char* NoDumpDetected    = "[!] No Spansh database file detected in default search path.";
        inline constexpr const char* EnterApplication  = "ENTER APPLICATION >";
        inline constexpr const char* ConfigureAndDl    = "CONFIGURE & DOWNLOAD >";
    }

    namespace Setup
    {
        inline constexpr const char* Title             = ":: SPANSH DATABASE SETUP";
        inline constexpr const char* OnlineSection     = "[Download] Online Downloader";
        inline constexpr const char* OnlineDescription = "Download the data dump directly from Spansh servers.";
        inline constexpr const char* LocalSection      = "[Local] Database Link";
        inline constexpr const char* LocalDescription  = "Search for a database file already stored in your storage.";
        inline constexpr const char* PrebuiltSection   = "[Prebuilt] Download Ready-Made Index";
        inline constexpr const char* PrebuiltDescription = "Skip extraction and indexing by downloading a prebuilt index file. Only use trusted sources.";
        inline constexpr const char* PrebuiltOpenButton = "Download Prebuilt Index...";
        inline constexpr const char* MemorySection     = "[Settings] Decompression Memory Allocation";
        inline constexpr const char* SystemRamFmt      = "System RAM: %.1f GB Total, %.1f GB Available";
        inline constexpr const char* RecommendedFmt    = "Recommended allocation: %d MB";
        inline constexpr const char* BufferSliderLabel = "Decompression Buffer Size (MB)";
        inline constexpr const char* HighAllocWarning  = "Warning: High allocation may cause system slowdown or swapping on low memory.";
        inline constexpr const char* DetectedFmt       = "[+] Detected: %s";
        inline constexpr const char* NoLocalDump       = "No Spansh dump detected in this directory.";
        inline constexpr const char* UseThisDatabase   = "Use This Database";
        inline constexpr const char* UpdateSchema      = "Update Schema";
        inline constexpr const char* DownloadGalaxy1MonthFmt   = "Download Galaxy 1 Month (%.2f GB)";
        inline constexpr const char* DownloadGalaxy1MonthBusy  = "Download Galaxy 1 Month (Querying...)";
        inline constexpr const char* DownloadGalaxy1MonthOff   = "Download Galaxy 1 Month (~16.0 GB) (Offline)";
        inline constexpr const char* DownloadGalaxyFullFmt     = "Download Full Galaxy (%.2f GB)";
        inline constexpr const char* DownloadGalaxyFullBusy    = "Download Full Galaxy (Querying...)";
        inline constexpr const char* DownloadGalaxyFullOff     = "Download Full Galaxy (~100.0 GB) (Offline)";
    }

    namespace PrebuiltIndex
    {
        inline constexpr const char* Title             = ":: PREBUILT INDEX DOWNLOAD";
        inline constexpr const char* SourceSection     = "[Source] Choose Index Origin";
        inline constexpr const char* OfficialOption    = "Official source (trusted)";
        inline constexpr const char* CustomOption      = "Custom URL";
        inline constexpr const char* UrlLabel          = "Index URL:";
        inline constexpr const char* UrlInputLabel     = "Index URL";
        inline constexpr const char* WarningHeader     = "[!] WARNING: Only download prebuilt index files from sources you fully trust.";
        inline constexpr const char* WarningBody       = "An index file is loaded as raw binary memory by the application. A tampered file may cause crashes, incorrect search results, or expose your system to risk. The official source is locked to the Colony Finder GitHub releases. Custom URLs are accepted but used at your own risk.";
        inline constexpr const char* StartDownload     = "Start Index Download";
    }

    namespace PrebuiltDownload
    {
        inline constexpr const char* Title             = ":: DOWNLOADING PREBUILT INDEX";
        inline constexpr const char* Notice            = "The prebuilt index is currently downloading in the background. Do NOT close the application during this process, otherwise the downloaded file might get corrupted.";
    }

    namespace DownloadDump
    {
        inline constexpr const char* Title             = ":: DOWNLOADING DATABASE";
        inline constexpr const char* Notice            = "The database is currently downloading in the background. Do NOT close the application during this process, otherwise the downloaded files might get corrupted.";
        inline constexpr const char* StatusFmt         = "Progress: %.1f MB / %.1f MB | Speed: %.2f MB/s | Est. Time Left: %dm %ds";
        inline constexpr const char* StatusCalcFmt     = "Progress: %.1f MB / %.1f MB | Speed: 0.00 MB/s | Est. Time Left: Calculating...";
    }

    namespace SchemaUpdate
    {
        inline constexpr const char* Title             = ":: UPDATING DATABASE SCHEMA";
        inline constexpr const char* Notice            = "Downloading Spansh schema file...";
    }

    namespace Extraction
    {
        inline constexpr const char* Title             = ":: PROCESS DATABASE FILES";
        inline constexpr const char* Notice            = "Decompressing & indexing Spansh dump (streaming)...";
        inline constexpr const char* EtaMinSecFmt      = "Estimated time remaining: %dm %ds";
        inline constexpr const char* EtaSecFmt         = "Estimated time remaining: %ds";
        inline constexpr const char* EtaCalculating    = "Estimated time remaining: Calculating...";
    }

    namespace IndexOutdated
    {
        inline constexpr const char* Title             = ":: INDEX FORMAT OUTDATED";
        inline constexpr const char* Alert             = "[!] The existing index is no longer compatible with this build.";
        inline constexpr const char* Description       = "The application has been updated and the index file format has changed. The existing index must be regenerated from the source file before the database can be used.";
        inline constexpr const char* Detail            = "The old index will be deleted and a new one will be generated using the existing source file.";
        inline constexpr const char* RegenerateButton  = "Regenerate Index";
    }

    namespace Error
    {
        inline constexpr const char* Title             = "[!] SYSTEM CRITICAL ERROR";
        inline constexpr const char* DefaultBackLabel  = "< RETURN TO SETUP";
        inline constexpr const char* IndexBackLabel    = "< RETURN TO INDEX DOWNLOAD";
    }

    namespace UpdateAvailable
    {
        inline constexpr const char* Title             = ">> UPDATE AVAILABLE";
        inline constexpr const char* CurrentFmt        = "Current version: %s";
        inline constexpr const char* LatestFmt         = "Latest  version: %s";
        inline constexpr const char* Downloaded        = "[+] Update downloaded. Restart to apply.";
        inline constexpr const char* Downloading       = "[~] Downloading update...";
        inline constexpr const char* Prompt            = "A newer version of ColonyFinder is available. Update now?";
        inline constexpr const char* RestartNow        = "RESTART NOW >";
        inline constexpr const char* UpdateNow         = "UPDATE NOW >";
        inline constexpr const char* Skip              = "SKIP";
    }

    namespace SettingsScreen
    {
        inline constexpr const char* Title             = ":: APPLICATION SETTINGS";
        inline constexpr const char* ThemeSection      = "[Theme] Color Customization";
        inline constexpr const char* ThemeNotice       = "Changes are applied immediately and saved automatically.";
        inline constexpr const char* ResetTheme        = "Reset Theme to Defaults";
        inline constexpr const char* ColorOrangePrimary    = "Orange Primary";
        inline constexpr const char* ColorOrangeMuted      = "Orange Muted";
        inline constexpr const char* ColorOrangeActive     = "Orange Active";
        inline constexpr const char* ColorBgDark           = "Background Dark";
        inline constexpr const char* ColorBgPanel          = "Background Panel";
        inline constexpr const char* ColorTextNormal       = "Text Normal";
        inline constexpr const char* ColorTextMuted        = "Text Muted";
        inline constexpr const char* ColorTextAlert        = "Text Alert";
        inline constexpr const char* ColorTextSuccess      = "Text Success";
        inline constexpr const char* ColorBorder           = "Border";
        inline constexpr const char* ColorRowHover         = "Row Hover";
        inline constexpr const char* ColorRowHoverActive   = "Row Hover Active";
        inline constexpr const char* ColorRowSelected      = "Row Selected";
    }

    namespace Main
    {
        inline constexpr const char* Title             = ":: COLONY FINDER - SYSTEM SEARCH";
    }

    namespace Filters
    {
        inline constexpr const char* LocationSection   = "Location & Range";
        inline constexpr const char* StartSystem       = "Start System:";
        inline constexpr const char* FilterDistance    = "Filter Distance";
        inline constexpr const char* FilterName        = "Filter Name:";
        inline constexpr const char* ColonizedOnly     = "Colonized Only";
        inline constexpr const char* FilterPopulation  = "Filter Population";
        inline constexpr const char* FilterBodyCount   = "Filter Body Count";
        inline constexpr const char* LandableRequired  = "Landable Required";
        inline constexpr const char* RunQuery          = "RUN QUERY";
        inline constexpr const char* StarO             = "O Star";
        inline constexpr const char* StarB             = "B Star";
        inline constexpr const char* StarA             = "A Star";
        inline constexpr const char* StarF             = "F Star";
        inline constexpr const char* StarG             = "G Star";
        inline constexpr const char* StarK             = "K Star";
        inline constexpr const char* StarM             = "M Star";
        inline constexpr const char* StarLTY           = "L/T/Y Dwarf";
        inline constexpr const char* StarNeutron       = "Neutron";
        inline constexpr const char* StarBlackHole     = "Black Hole";
        inline constexpr const char* StarWhiteDwarf    = "White Dwarf";
        inline constexpr const char* RangeLy           = "Range (Ly)";
        inline constexpr const char* RangeLyFormat     = "%.1f Ly";
        inline constexpr const char* MinPop            = "Min Pop";
        inline constexpr const char* MaxPop            = "Max Pop";
        inline constexpr const char* MinBodies         = "Min Bodies";
        inline constexpr const char* MaxBodies         = "Max Bodies";
        inline constexpr const char* StarTypesNode     = "Star Types";
        inline constexpr const char* PlanetTypesNode   = "Planet Types";
        inline constexpr const char* Min               = "Min";
        inline constexpr const char* Max               = "Max";
        inline constexpr const char* PlanetEarthLike   = "Earth-like (ELW)";
        inline constexpr const char* PlanetWaterWorld  = "Water World (WW)";
        inline constexpr const char* PlanetAmmonia     = "Ammonia World (AMW)";
        inline constexpr const char* PlanetHighMetal   = "High Metal (HMC)";
        inline constexpr const char* PlanetMetalRich   = "Metal-rich";
        inline constexpr const char* PlanetRocky       = "Rocky";
        inline constexpr const char* PlanetIcy         = "Icy";
        inline constexpr const char* PlanetGasGiant    = "Gas Giant";
    }

    namespace Results
    {
        inline constexpr const char* Searching         = "Searching...";
        inline constexpr const char* FoundFmt          = "Found %zu systems:";
        inline constexpr const char* PrevPage          = "< Prev";
        inline constexpr const char* NextPage          = "Next >";
        inline constexpr const char* PageInfoFmt       = "Page %d / %d  (showing %zu-%zu)";
        inline constexpr const char* DistanceLyFmt     = "%.2f Ly";
        inline constexpr const char* HintRunQuery      = "Configure the filters on the left panel and click 'RUN QUERY' to locate systems.";
        inline constexpr const char* NoneFound         = "No systems found matching the specified search criteria.";
        inline constexpr const char* FoundCappedFmt    = "Found %zu+ systems (cap reached, refine filters for more):";
        inline constexpr const char* CopySystemName    = "Copy system name";
        inline constexpr const char* ColSystemName     = "System Name";
        inline constexpr const char* ColDistance       = "Distance";
        inline constexpr const char* ColBodies         = "Bodies";
        inline constexpr const char* ColPopulation     = "Population";
        inline constexpr const char* Uninhabited       = "Uninhabited";
    }
}

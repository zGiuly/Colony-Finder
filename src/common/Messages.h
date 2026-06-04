#pragma once

namespace Messages
{
    namespace Database
    {
        inline constexpr const char* IndexUrlEmpty             = "Index URL is empty.";
        inline constexpr const char* PrebuiltIndexInvalid      = "Downloaded file is not a valid or compatible index.";
        inline constexpr const char* SchemaDownloadFailed      = "Failed to download schema.";
        inline constexpr const char* IndexingJsonParseFailed   = "Indexing failed. The JSON file could not be parsed.";
    }

    namespace Download
    {
        inline constexpr const char* GetSizeFailed             = "Failed to retrieve file size.";
        inline constexpr const char* CreateDestinationFailed   = "Failed to create destination file.";
        inline constexpr const char* Cancelled                 = "Download cancelled.";
        inline constexpr const char* Incomplete                = "Download incomplete.";
    }

    namespace Streaming
    {
        inline constexpr const char* WorkerTmpOpenFailed       = "Failed to open worker tmp files.";
        inline constexpr const char* DecompressionFailed       = "Decompression failed. The downloaded file may be corrupted.";
        inline constexpr const char* NoSystemsIndexed          = "Indexing produced no systems. The JSON format may be unexpected.";
        inline constexpr const char* IndexOutputOpenFailed     = "Failed to open index output file.";
        inline constexpr const char* WorkerReadFailed          = "Failed to read worker records.";
    }

    namespace Update
    {
        inline constexpr const char* CannotResolveExePath      = "cannot resolve current executable path";
        inline constexpr const char* CannotOpenTempFile        = "cannot open temp file";
        inline constexpr const char* CurlInitFailed            = "curl init failed";
        inline constexpr const char* DownloadFailedPrefix      = "download failed: ";
    }
}

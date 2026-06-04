#pragma once

#include <string>
#include <atomic>

/**
 * \todo PENDING IMPLEMENTATION: not yet wired into the extraction pipeline.
 *       The galaxy.schema.json file is downloaded and the SAX validator
 *       (DynamicSaxValidator) is implemented, but Validate() is currently
 *       unreferenced. Hook this into StreamingProcessor::Process once the
 *       validation stage is enabled.
 */
class JsonStreamValidator
{
public:
    [[maybe_unused]] static bool Validate(const std::string& filePath, const std::string& schemaPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag);
};

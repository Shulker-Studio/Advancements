#include "mod/advancement/AdvancementLoader.h"

#include "ll/api/io/FileUtils.h"
#include "mod/advancement/AdvancementParser.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <format>

namespace advancements {
namespace {

std::string pathToUtf8(std::filesystem::path const& path) {
    return reinterpret_cast<char const*>(path.generic_u8string().c_str());
}

std::string buildAdvancementId(std::filesystem::path const& dataRoot, std::filesystem::path const& filePath) {
    auto relative = std::filesystem::relative(filePath, dataRoot);
    auto it       = relative.begin();
    if (it == relative.end()) {
        return {};
    }
    std::string id = reinterpret_cast<char const*>(it->u8string().c_str());
    ++it;
    if (it == relative.end() || *it != "advancements") {
        return {};
    }
    ++it;

    std::filesystem::path advancementPath;
    for (; it != relative.end(); ++it) {
        advancementPath /= *it;
    }
    advancementPath.replace_extension();
    if (id.empty() || advancementPath.empty()) {
        return {};
    }
    id += ':';
    id += pathToUtf8(advancementPath);
    return id;
}

void addIssue(LoadResult& result, std::filesystem::path path, std::string message) {
    result.issues.emplace_back(std::move(path), std::move(message));
}

std::vector<std::filesystem::path> collectAdvancementFiles(std::filesystem::path const& dataRoot) {
    std::vector<std::filesystem::path> files;
    if (!std::filesystem::exists(dataRoot)) {
        return files;
    }
    for (auto const& entry : std::filesystem::recursive_directory_iterator(dataRoot)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }

        auto relative = std::filesystem::relative(entry.path(), dataRoot);
        auto it       = relative.begin();
        if (it == relative.end()) {
            continue;
        }
        ++it;
        if (it != relative.end() && *it == "advancements") {
            files.emplace_back(entry.path());
        }
    }
    std::ranges::sort(files);
    return files;
}

void validateParents(LoadResult& result) {
    for (auto const& [id, advancement] : result.advancements) {
        if (advancement.parent && !result.advancements.contains(*advancement.parent)) {
            addIssue(
                result,
                advancement.sourcePath,
                std::format("{} references missing parent {}", id, *advancement.parent)
            );
        }
    }
}

} // namespace

LoadResult loadAdvancements(std::filesystem::path const& modDir) {
    LoadResult result;
    auto const dataRoot = modDir / "data";

    for (auto const& filePath : collectAdvancementFiles(dataRoot)) {
        auto id = buildAdvancementId(dataRoot, filePath);
        if (id.empty()) {
            addIssue(result, filePath, "invalid advancement path");
            continue;
        }
        if (result.advancements.contains(id)) {
            addIssue(result, filePath, std::format("duplicate advancement id {}", id));
            continue;
        }

        auto content = ll::file_utils::readFile(filePath);
        if (!content) {
            addIssue(result, filePath, "failed to read file");
            continue;
        }

        auto json = nlohmann::json::parse(*content, nullptr, false, true);
        if (json.is_discarded()) {
            addIssue(result, filePath, "invalid JSON");
            continue;
        }

        auto parsed = parseAdvancement(json, std::move(id), filePath);
        for (auto& error : parsed.errors) {
            addIssue(result, filePath, std::move(error));
        }
        if (parsed.advancement) {
            auto parsedId = parsed.advancement->id;
            result.advancements.emplace(std::move(parsedId), std::move(*parsed.advancement));
        }
    }

    validateParents(result);
    return result;
}

} // namespace advancements

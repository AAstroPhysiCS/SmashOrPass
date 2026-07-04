#include "smashorpass/persistence/SettingsStore.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "smashorpass/persistence/SettingsJson.hpp"

namespace sop {

namespace {

constexpr int kSettingsVersion = 3;

[[nodiscard]] std::string IoError(const std::filesystem::path& path, const char* operation) {
    return std::string{"Failed to "} + operation + " settings '" + path.string() + "'";
}

}  // namespace

Result<Settings> SettingsStore::Load(const std::filesystem::path& path) {
    std::error_code error;
    const bool exists = std::filesystem::exists(path, error);
    if (error) {
        return Err(std::string{"Failed to check settings '"} + path.string() +
                   "': " + error.message());
    }

    if (!exists) {
        return Ok(Settings{});
    }

    std::ifstream file(path);
    if (!file) {
        return Err(IoError(path, "open"));
    }

    try {
        Settings settings = nlohmann::json::parse(file).get<Settings>();
        settings.Clamp();
        return Ok(settings);
    } catch (const nlohmann::json::exception& e) {
        return Err(std::string{"Failed to parse settings '"} + path.string() + "': " + e.what());
    }
}

Result<void> SettingsStore::Save(const std::filesystem::path& path, const Settings& settings) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return Err(std::string{"Failed to create settings directory '"} +
                   path.parent_path().string() + "': " + error.message());
    }

    Settings sanitizedSettings = settings;
    sanitizedSettings.Clamp();

    nlohmann::json json = sanitizedSettings;
    json["version"] = kSettingsVersion;

    std::ofstream file(path);
    if (!file) {
        return Err(IoError(path, "write"));
    }

    file << json.dump(4) << '\n';
    if (!file) {
        return Err(IoError(path, "write"));
    }

    return Ok();
}

}  // namespace sop

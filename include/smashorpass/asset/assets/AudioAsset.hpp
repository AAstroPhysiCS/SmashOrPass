#pragma once

#include <SDL3_mixer/SDL_mixer.h>

#include <filesystem>
#include <string>
#include <vector>

#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

enum class AudioAssetType {
    SoundEffect,
    Music,
    Ambience,
};

struct AudioAssetLoadJob {
    std::filesystem::path Path;
    AudioAssetType Type = AudioAssetType::SoundEffect;
    bool Predecode = true;

    [[nodiscard]] std::string DebugName() const {
        return Path.filename().string();
    }
    [[nodiscard]] struct AudioRawAssetData ToRawAssetData(AppCtx& ctx);
};

struct AudioRawAssetData {
    std::filesystem::path Path;
    AudioAssetType Type = AudioAssetType::SoundEffect;
    bool Predecode = true;

    [[nodiscard]] struct AudioAssetData ToAssetData(AppCtx& ctx);
};

class AudioAssetData final {
   public:
    AudioAssetData() = default;

    explicit AudioAssetData(MIX_Audio* audio, AudioAssetType type, std::filesystem::path path);

    ~AudioAssetData();

    AudioAssetData(const AudioAssetData&) = delete;
    AudioAssetData& operator=(const AudioAssetData&) = delete;

    AudioAssetData(AudioAssetData&& other) noexcept;
    AudioAssetData& operator=(AudioAssetData&& other) noexcept;

    [[nodiscard]] MIX_Audio* NativeHandle() const noexcept {
        return m_Audio;
    }

    [[nodiscard]] AudioAssetType Type() const noexcept {
        return m_Type;
    }

    [[nodiscard]] const std::filesystem::path& Path() const noexcept {
        return m_Path;
    }

    [[nodiscard]] bool IsValid() const noexcept {
        return m_Audio != nullptr;
    }

    [[nodiscard]] static AudioAssetData Default(AppCtx& ctx, const AudioAssetLoadJob& loadJob);

   private:
    void Destroy();

   private:
    MIX_Audio* m_Audio = nullptr;
    AudioAssetType m_Type = AudioAssetType::SoundEffect;
    std::filesystem::path m_Path;
};

class AudioAssetDiscoverer final {
   public:
    [[nodiscard]] static Result<std::vector<AudioAssetLoadJob>> ListAvailableAssets(AppCtx& ctx);
};

}  // namespace sop
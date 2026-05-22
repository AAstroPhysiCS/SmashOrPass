#include "smashorpass/asset/assets/AudioAsset.hpp"

#include <SDL3/SDL_error.h>

#include <utility>

#include "smashorpass/audio/AudioSystem.hpp"
#include "smashorpass/core/AppCtx.hpp"

namespace sop {

AudioRawAssetData AudioAssetLoadJob::ToRawAssetData(AppCtx&) {
    return AudioRawAssetData{
        .Path = Path,
        .Type = Type,
        .Predecode = Predecode,
    };
}

AudioAssetData AudioRawAssetData::ToAssetData(AppCtx& ctx) {
    MIX_Mixer* mixer = ctx.audioSystem.Mixer();

    if (mixer == nullptr) {
        return AudioAssetData{};
    }

    MIX_Audio* audio = MIX_LoadAudio(mixer, Path.string().c_str(), Predecode);

    if (audio == nullptr) {
        return AudioAssetData{};
    }

    return AudioAssetData{
        audio,
        Type,
        Path,
    };
}

AudioAssetData::AudioAssetData(MIX_Audio* audio, AudioAssetType type, std::filesystem::path path)
    : m_Audio(audio), m_Type(type), m_Path(std::move(path)) {}

AudioAssetData::~AudioAssetData() {
    Destroy();
}

AudioAssetData::AudioAssetData(AudioAssetData&& other) noexcept
    : m_Audio(std::exchange(other.m_Audio, nullptr)),
      m_Type(other.m_Type),
      m_Path(std::move(other.m_Path)) {}

AudioAssetData& AudioAssetData::operator=(AudioAssetData&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    Destroy();

    m_Audio = std::exchange(other.m_Audio, nullptr);
    m_Type = other.m_Type;
    m_Path = std::move(other.m_Path);

    return *this;
}

void AudioAssetData::Destroy() {
    if (m_Audio != nullptr) {
        MIX_DestroyAudio(m_Audio);
        m_Audio = nullptr;
    }
}

AudioAssetData AudioAssetData::Default(AppCtx&, const AudioAssetLoadJob& loadJob) {
    return AudioAssetData{
        nullptr,
        loadJob.Type,
        loadJob.Path,
    };
}

Result<std::vector<AudioAssetLoadJob>> AudioAssetDiscoverer::ListAvailableAssets(AppCtx&) {
    return Ok(std::vector<AudioAssetLoadJob>{});
}

}  // namespace sop
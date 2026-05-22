#pragma once

#include <SDL3/SDL_audio.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/asset/assets/AudioAsset.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/util.hpp"

namespace sop {

enum class AudioBus : std::uint8_t {
    Master,
    Music,
    Sfx,
    Ui,
    Ambience,
};

struct PlayAudioDesc {
    Asset<AudioAssetData> Asset;

    AudioBus Bus = AudioBus::Sfx;

    float Gain = 1.0f;
    float Pitch = 1.0f;

    int Loops = 0;

    std::optional<Vec2> WorldPosition = std::nullopt;
    std::optional<Vec2> ListenerPosition = std::nullopt;

    std::string_view DebugTag = {};
};

struct MusicDesc {
    Asset<AudioAssetData> Asset;

    float Gain = 1.0f;
    int Loops = -1;

    std::int64_t FadeOutMs = 400;
};

class AudioSystem final {
   public:
    AudioSystem() = default;
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    AudioSystem(AudioSystem&&) = delete;
    AudioSystem& operator=(AudioSystem&&) = delete;

    [[nodiscard]] Result<void> Initialize();
    void Shutdown();

    [[nodiscard]] MIX_Mixer* Mixer() const noexcept {
        return m_Mixer;
    }

    [[nodiscard]] Result<void> Play(AppCtx& ctx, const PlayAudioDesc& desc);
    [[nodiscard]] Result<void> PlayMusic(AppCtx& ctx, const MusicDesc& desc);

    [[nodiscard]] Result<void> StopBus(AudioBus bus, std::int64_t fadeOutMs = 0);
    [[nodiscard]] Result<void> PauseBus(AudioBus bus);
    [[nodiscard]] Result<void> ResumeBus(AudioBus bus);

    [[nodiscard]] Result<void> SetBusGain(AudioBus bus, float gain);
    [[nodiscard]] float GetBusGain(AudioBus bus) const;

    [[nodiscard]] Result<void> SetMasterGain(float gain);

   private:
    struct TrackSlot {
        MIX_Track* Track = nullptr;
        AudioBus Bus = AudioBus::Sfx;
        bool ReservedForMusic = false;
    };

    [[nodiscard]] Result<void> CreateTracks();

    [[nodiscard]] TrackSlot* FindFreeSfxTrack();
    [[nodiscard]] TrackSlot* MusicTrack();

    [[nodiscard]] static const char* BusTag(AudioBus bus);

    [[nodiscard]] Result<void> ConfigureTrack(MIX_Track* track,
                                              MIX_Audio* audio,
                                              AudioBus bus,
                                              float gain,
                                              float pitch,
                                              int loops,
                                              std::optional<Vec2> worldPosition,
                                              std::optional<Vec2> listenerPosition);

    [[nodiscard]] static float ClampGain(float gain);

    [[nodiscard]] static MIX_Point3D ToMixerPosition(Vec2 worldPosition, Vec2 listenerPosition);

   private:
    static constexpr int s_SfxTrackCount = 32;

    MIX_Mixer* m_Mixer = nullptr;

    std::vector<TrackSlot> m_Tracks;

    std::array<float, 5> m_BusGains{
        1.0f,   // Master
        0.75f,  // Music
        1.0f,   // Sfx
        1.0f,   // Ui
        0.8f,   // Ambience
    };
};

}  // namespace sop
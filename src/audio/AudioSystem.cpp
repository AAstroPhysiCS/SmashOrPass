#include "smashorpass/audio/AudioSystem.hpp"

#include <SDL3/SDL_error.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

AudioSystem::~AudioSystem() {
    Shutdown();
}

Result<void> AudioSystem::Initialize() {
    if (!MIX_Init()) {
        return Err(SdlError("MIX_Init"));
    }

    SDL_AudioSpec preferredSpec{};
    preferredSpec.freq = 48000;
    preferredSpec.format = SDL_AUDIO_F32;
    preferredSpec.channels = 2;

    m_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &preferredSpec);

    if (m_Mixer == nullptr) {
        MIX_Quit();
        return Err(SdlError("MIX_CreateMixerDevice"));
    }

    TRY_VOID(CreateTracks());
    TRY_VOID(SetMasterGain(m_BusGains[static_cast<std::size_t>(AudioBus::Master)]));

    return Ok();
}

void AudioSystem::Shutdown() {
    for (TrackSlot& slot : m_Tracks) {
        if (slot.Track != nullptr) {
            MIX_DestroyTrack(slot.Track);
            slot.Track = nullptr;
        }
    }

    m_Tracks.clear();

    if (m_Mixer != nullptr) {
        MIX_DestroyMixer(m_Mixer);
        m_Mixer = nullptr;
    }

    MIX_Quit();
}

Result<void> AudioSystem::CreateTracks() {
    m_Tracks.reserve(s_SfxTrackCount + 1);

    for (int i = 0; i < s_SfxTrackCount; ++i) {
        MIX_Track* track = MIX_CreateTrack(m_Mixer);

        if (track == nullptr) {
            return Err(SdlError("MIX_CreateTrack"));
        }

        if (!MIX_TagTrack(track, BusTag(AudioBus::Sfx))) {
            return Err(SdlError("MIX_TagTrack"));
        }

        m_Tracks.push_back(TrackSlot{
            .Track = track,
            .Bus = AudioBus::Sfx,
            .ReservedForMusic = false,
        });
    }

    MIX_Track* musicTrack = MIX_CreateTrack(m_Mixer);

    if (musicTrack == nullptr) {
        return Err(SdlError("MIX_CreateTrack music"));
    }

    if (!MIX_TagTrack(musicTrack, BusTag(AudioBus::Music))) {
        return Err(SdlError("MIX_TagTrack music"));
    }

    m_Tracks.push_back(TrackSlot{
        .Track = musicTrack,
        .Bus = AudioBus::Music,
        .ReservedForMusic = true,
    });

    return Ok();
}

Result<void> AudioSystem::Play(AppCtx& ctx, const PlayAudioDesc& desc) {
    if (m_Mixer == nullptr) {
        return Err(std::string{"AudioSystem::Play failed: audio system is not initialized."});
    }

    TRY(audioData, ctx.assets.GetAssetData(desc.asset));

    MIX_Audio* audio = audioData.get().NativeHandle();

    if (audio == nullptr) {
        return Err(std::string{"AudioSystem::Play failed: audio asset is not loaded yet: "} +
                   audioData.get().Path().string());
    }

    TrackSlot* slot = FindFreeSfxTrack();

    if (slot == nullptr) {
        return Ok();
    }

    TRY_VOID(ConfigureTrack(slot->Track,
                            audio,
                            desc.Bus,
                            desc.Gain,
                            desc.Pitch,
                            desc.Loops,
                            desc.WorldPosition,
                            desc.ListenerPosition));

    if (!MIX_PlayTrack(slot->Track, 0)) {
        return Err(SdlError("MIX_PlayTrack"));
    }

    return Ok();
}

Result<void> AudioSystem::PlayMusic(AppCtx& ctx, const MusicDesc& desc) {
    if (m_Mixer == nullptr) {
        return Err(std::string{"AudioSystem::PlayMusic failed: audio system is not initialized."});
    }

    TRY(audioData, ctx.assets.GetAssetData(desc.asset));

    MIX_Audio* audio = audioData.get().NativeHandle();

    if (audio == nullptr) {
        return Err(std::string{"AudioSystem::PlayMusic failed: music asset is not loaded yet: "} +
                   audioData.get().Path().string());
    }

    TrackSlot* slot = MusicTrack();

    if (slot == nullptr) {
        return Err(std::string{"AudioSystem::PlayMusic failed: music track does not exist."});
    }

    if (MIX_TrackPlaying(slot->Track)) {
        if (!MIX_StopTrack(slot->Track, MIX_TrackMSToFrames(slot->Track, desc.FadeOutMs))) {
            return Err(SdlError("MIX_StopTrack music fadeout"));
        }
    }

    TRY_VOID(ConfigureTrack(slot->Track,
                            audio,
                            AudioBus::Music,
                            desc.Gain,
                            1.0f,
                            desc.Loops,
                            std::nullopt,
                            std::nullopt));

    if (!MIX_PlayTrack(slot->Track, 0)) {
        return Err(SdlError("MIX_PlayTrack music"));
    }

    return Ok();
}

Result<void> AudioSystem::ConfigureTrack(MIX_Track* track,
                                         MIX_Audio* audio,
                                         AudioBus bus,
                                         float gain,
                                         float pitch,
                                         int loops,
                                         std::optional<Vec2> worldPosition,
                                         std::optional<Vec2> listenerPosition) {
    if (!MIX_SetTrackAudio(track, audio)) {
        return Err(SdlError("MIX_SetTrackAudio"));
    }

    if (!MIX_SetTrackLoops(track, loops)) {
        return Err(SdlError("MIX_SetTrackLoops"));
    }

    const float busGain = GetBusGain(bus);
    const float finalGain = ClampGain(gain) * busGain;

    if (!MIX_SetTrackGain(track, finalGain)) {
        return Err(SdlError("MIX_SetTrackGain"));
    }

    if (!MIX_SetTrackFrequencyRatio(track, std::max(0.05f, pitch))) {
        return Err(SdlError("MIX_SetTrackFrequencyRatio"));
    }

    if (worldPosition.has_value() && listenerPosition.has_value()) {
        const MIX_Point3D point = ToMixerPosition(*worldPosition, *listenerPosition);

        if (!MIX_SetTrack3DPosition(track, &point)) {
            return Err(SdlError("MIX_SetTrack3DPosition"));
        }
    } else {
        const MIX_Point3D center{
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f,
        };

        if (!MIX_SetTrack3DPosition(track, &center)) {
            return Err(SdlError("MIX_SetTrack3DPosition reset"));
        }
    }

    MIX_UntagTrack(track, BusTag(AudioBus::Sfx));
    MIX_UntagTrack(track, BusTag(AudioBus::Ui));
    MIX_UntagTrack(track, BusTag(AudioBus::Ambience));
    MIX_UntagTrack(track, BusTag(AudioBus::Music));

    if (!MIX_TagTrack(track, BusTag(bus))) {
        return Err(SdlError("MIX_TagTrack bus"));
    }

    return Ok();
}

AudioSystem::TrackSlot* AudioSystem::FindFreeSfxTrack() {
    for (TrackSlot& slot : m_Tracks) {
        if (slot.ReservedForMusic) {
            continue;
        }

        if (!MIX_TrackPlaying(slot.Track)) {
            return &slot;
        }
    }

    return nullptr;
}

AudioSystem::TrackSlot* AudioSystem::MusicTrack() {
    for (TrackSlot& slot : m_Tracks) {
        if (slot.ReservedForMusic) {
            return &slot;
        }
    }

    return nullptr;
}

Result<void> AudioSystem::StopBus(AudioBus bus, std::int64_t fadeOutMs) {
    if (m_Mixer == nullptr) {
        return Ok();
    }

    if (!MIX_StopTag(m_Mixer, BusTag(bus), fadeOutMs)) {
        return Err(SdlError("MIX_StopTag"));
    }

    return Ok();
}

Result<void> AudioSystem::PauseBus(AudioBus bus) {
    if (m_Mixer == nullptr) {
        return Ok();
    }

    if (!MIX_PauseTag(m_Mixer, BusTag(bus))) {
        return Err(SdlError("MIX_PauseTag"));
    }

    return Ok();
}

Result<void> AudioSystem::ResumeBus(AudioBus bus) {
    if (m_Mixer == nullptr) {
        return Ok();
    }

    if (!MIX_ResumeTag(m_Mixer, BusTag(bus))) {
        return Err(SdlError("MIX_ResumeTag"));
    }

    return Ok();
}

Result<void> AudioSystem::SetBusGain(AudioBus bus, float gain) {
    const std::size_t index = static_cast<std::size_t>(bus);
    m_BusGains[index] = ClampGain(gain);

    if (m_Mixer == nullptr) {
        return Ok();
    }

    if (bus == AudioBus::Master) {
        return SetMasterGain(gain);
    }

    if (!MIX_SetTagGain(m_Mixer, BusTag(bus), m_BusGains[index])) {
        return Err(SdlError("MIX_SetTagGain"));
    }

    return Ok();
}

float AudioSystem::GetBusGain(AudioBus bus) const {
    return m_BusGains[static_cast<std::size_t>(bus)];
}

Result<void> AudioSystem::SetMasterGain(float gain) {
    m_BusGains[static_cast<std::size_t>(AudioBus::Master)] = ClampGain(gain);

    if (m_Mixer == nullptr) {
        return Ok();
    }

    if (!MIX_SetMixerGain(m_Mixer, m_BusGains[static_cast<std::size_t>(AudioBus::Master)])) {
        return Err(SdlError("MIX_SetMixerGain"));
    }

    return Ok();
}

const char* AudioSystem::BusTag(AudioBus bus) {
    switch (bus) {
        case AudioBus::Master:
            return "master";

        case AudioBus::Music:
            return "music";

        case AudioBus::Sfx:
            return "sfx";

        case AudioBus::Ui:
            return "ui";

        case AudioBus::Ambience:
            return "ambience";
    }

    return "unknown";
}

float AudioSystem::ClampGain(float gain) {
    return std::clamp(gain, 0.0f, 4.0f);
}

MIX_Point3D AudioSystem::ToMixerPosition(Vec2 worldPosition, Vec2 listenerPosition) {
    const Vec2 relative{
        worldPosition.x - listenerPosition.x,
        worldPosition.y - listenerPosition.y,
    };

    constexpr float pixelsToMeters = 1.0f / 120.0f;

    return MIX_Point3D{
        .x = relative.x * pixelsToMeters,
        .y = 0.0f,
        .z = relative.y * pixelsToMeters,
    };
}

}  // namespace sop
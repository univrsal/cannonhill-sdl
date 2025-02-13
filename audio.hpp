#pragma once
#include <string>
#include <map>
#include <memory>
#include "mixer.hpp"

namespace audio {

enum file {
    NO_SOUND,
    APPLAUSE,
    CREDITS,
    CREDITS_MUSIC,
    EXPLOSION,
    PARACHUTE,
    PARACHUTE2,
    RIFLE,
    GRENADE,
    REGISTER,
    CLICK,
    CRACKLING,
    LASER,
    MAGNET_SHIELD,
    MOUSE1,
    MOUSE2,
    MOUSE3,
    MOUSE4,
    TANK,
    RICOCHET,
    ROCKET,
    ACID,
    SHIELD,
    SPLINTER,
    STONE,
    TITLE_MUSIC,
    YIPPIEE
};

class manager {
    map<file, shared_ptr<wav_file>> m_audio_files{};

public:
    manager();
    ~manager();

    void play(file f, id chan = id::SFX, uint8_t vol = 255, bool loop = false, bool limit = true);
    void stop(file file) {
        for (auto& channel : active_playback_device->channels()) {
            for (auto& clip : channel.second.clips()) {
                if (clip.wav() == m_audio_files[file]) {
                    clip.set_vol(0);
                    clip.set_done();
                }
            }
        }
    }

    static void stop_channel(id chan) { active_playback_device->stop_channel(chan); }

    void init();
};

}

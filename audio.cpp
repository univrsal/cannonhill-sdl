#include "audio.hpp"
#include <SDL.h>

namespace audio {

struct audio_file {
    file id;
    const char *path;
};

static audio_file files[] = {
    {APPLAUSE, "./sounds/applaus.wav"},
    {CREDITS, "./sounds/credits.wav"},
    {CREDITS_MUSIC, "./sounds/creditsmusik.wav"},
    {EXPLOSION, "./sounds/explosiong.wav"},
    {PARACHUTE, "./sounds/fallschirm.wav"},
    {PARACHUTE2, "./sounds/fallschirm2.wav"},
    {RIFLE, "./sounds/gewehr.wav"},
    {GRENADE, "./sounds/granate.wav"},
    {REGISTER, "./sounds/kasse.wav"},
    {CLICK, "./sounds/klick.wav"},
    {CRACKLING, "./sounds/knistern.wav"},
    {LASER, "./sounds/laser.wav"},
    {MAGNET_SHIELD, "./sounds/magnetschild.wav"},
    {MOUSE1, "./sounds/mouse1.wav"},
    {MOUSE2, "./sounds/mouse1.wav"},
    {MOUSE3, "./sounds/mouse1.wav"},
    {MOUSE4, "./sounds/mouse1.wav"},
    {TANK, "./sounds/panzer.wav"},
    {RICOCHET, "./sounds/querschuss.wav"},
    {ROCKET, "./sounds/rakete.wav"},
    {ACID, "./sounds/saeure.wav"},
    {SHIELD, "./sounds/schild.wav"},
    {SPLINTER, "./sounds/splitter.wav"},
    {STONE, "./sounds/stein.wav"},
    {TITLE_MUSIC, "./sounds/titelmusik.wav"},
    {YIPPIEE, "./sounds/yippee.wav"},
};

manager::manager()
{
    init();
}

manager::~manager() {}

void manager::init()
{
    m_audio_files.clear();
    for (const auto &file : files) {
        m_audio_files[file.id] = make_shared<wav_file>(file.path);
    }
}

void manager::play(file f, id chan, uint8_t vol, bool loop, bool limit)
{
    if (m_audio_files[f] && audio::active_playback_device) {
        if (limit && SDL_GetTicks() - m_audio_files[f]->last_play_time() < m_audio_files[f]->duration() * 1000)
            return;
        m_audio_files[f]->set_last_play_time(SDL_GetTicks());
        audio::active_playback_device->play(chan, {m_audio_files[f], vol, loop});
    }
}

}

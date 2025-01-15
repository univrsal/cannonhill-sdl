#include "audio.hpp"
#include "panzer.h"
#include <SDL.h>

extern SHORT Soundzustand;
namespace audio {

struct audio_file {
    file id;
    const char *path;
};

static audio_file files[] = {
    {APPLAUSE, PATH_PREFIX "./sounds/applaus.wav"},
    {CREDITS, PATH_PREFIX "./sounds/credits.wav"},
    {CREDITS_MUSIC, PATH_PREFIX "./sounds/creditsmusik.wav"},
    {EXPLOSION, PATH_PREFIX "./sounds/explosiong.wav"},
    {PARACHUTE, PATH_PREFIX "./sounds/fallschirm.wav"},
    {PARACHUTE2, PATH_PREFIX "./sounds/fallschirm2.wav"},
    {RIFLE, PATH_PREFIX "./sounds/gewehr.wav"},
    {GRENADE, PATH_PREFIX "./sounds/granate.wav"},
    {REGISTER, PATH_PREFIX "./sounds/kasse.wav"},
    {CLICK, PATH_PREFIX "./sounds/klick.wav"},
    {CRACKLING, PATH_PREFIX "./sounds/knistern.wav"},
    {LASER, PATH_PREFIX "./sounds/laser.wav"},
    {MAGNET_SHIELD, PATH_PREFIX "./sounds/magnetschild.wav"},
    {MOUSE1, PATH_PREFIX "./sounds/mouse1.wav"},
    {MOUSE2, PATH_PREFIX "./sounds/mouse1.wav"},
    {MOUSE3, PATH_PREFIX "./sounds/mouse1.wav"},
    {MOUSE4, PATH_PREFIX "./sounds/mouse1.wav"},
    {TANK, PATH_PREFIX "./sounds/panzer.wav"},
    {RICOCHET, PATH_PREFIX "./sounds/querschuss.wav"},
    {ROCKET, PATH_PREFIX "./sounds/rakete.wav"},
    {ACID, PATH_PREFIX "./sounds/saeure.wav"},
    {SHIELD, PATH_PREFIX "./sounds/schild.wav"},
    {SPLINTER, PATH_PREFIX "./sounds/splitter.wav"},
    {STONE, PATH_PREFIX "./sounds/stein.wav"},
    {TITLE_MUSIC, PATH_PREFIX "./sounds/titelmusik.wav"},
    {YIPPIEE, PATH_PREFIX "./sounds/yippee.wav"},
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
    if (Soundzustand <= 0) return;
    if (m_audio_files[f] && audio::active_playback_device) {
        if (limit && SDL_GetTicks() - m_audio_files[f]->last_play_time() < m_audio_files[f]->duration() * 1000)
            return;
        m_audio_files[f]->set_last_play_time(SDL_GetTicks());
        audio::active_playback_device->play(chan, {m_audio_files[f], vol, loop});
    }
}

}

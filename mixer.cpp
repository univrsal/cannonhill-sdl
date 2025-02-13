#include "mixer.hpp"
#include <algorithm>

namespace audio {

wav_file::wav_file(const string &path)
{
    if (SDL_LoadWAV(path.c_str(), &m_spec, &m_buf, &m_len) == nullptr)
        printf("Couldn't load '%s': '%s'\n", path.c_str(), SDL_GetError());
    m_duration = (double)m_len / (m_spec.freq * m_spec.channels * (SDL_AUDIO_BITSIZE(m_spec.format) / 8));
}

wav_file::~wav_file()
{
    SDL_FreeWAV(m_buf);
    m_buf = nullptr;
    m_len = 0;
}

clip::clip(shared_ptr<wav_file> w, uint8_t vol, bool loop) : m_vol(vol), m_wav(w), m_loop(loop)
{
    m_done = false;
    m_fade = false;
    m_len = w->len();
    m_buf = w->buf();

}

clip::~clip()
{
    m_buf = nullptr;
    m_len = 0;
}

void channel::add_clip(const clip &c)
{
    m_clips.emplace_back(c);
}

void channel::stop()
{
    for (auto &c : m_clips)
        c.set_done();
    auto del = remove_if(m_clips.begin(), m_clips.end(), [](clip &c) { return c.done(); });
    m_clips.erase(del, m_clips.end());
}

void channel::audio_tick(uint8_t *stream, int len)
{
    bool music = false;
    Uint32 tmplen = 0;
    for (auto &clip : m_clips) {
        if (clip.len() > 0) {
            if (clip.fade() && clip.loop()) {
                music = true;
                if (clip.vol() > 0)
                    clip.set_vol(clip.vol()-1);
                else
                    clip.set_len(0);
            }

            if (len > clip.len()) {
                SDL_MixAudioFormat(stream, clip.buf(), clip.wav()->spec().format, clip.len(), clip.vol() * m_volume);
                if (clip.loop()) {
                    // mix in the rest of the clip
                    auto left = len - clip.len();
                    clip.buf() = clip.wav()->buf();
                    SDL_MixAudioFormat(stream, clip.buf(), clip.wav()->spec().format, left, clip.vol() * m_volume);
                    clip.set_len(clip.wav()->len() - left);
                } else {
                    clip.set_done();
                    clip.set_len(0);
                }
            } else {
                SDL_MixAudioFormat(stream, clip.buf(), clip.wav()->spec().format, len, clip.vol() * m_volume);
                clip.buf() += len;
                clip.set_len(clip.len() -len);
            }
            // if (music && clip.loop()) {
            //     tmplen = 0;
            // } else {
            //     tmplen = SDL_min((uint32_t)len, clip.len());
            // }
            // SDL_MixAudioFormat(stream, clip.buf(), clip.wav()->spec().format, tmplen, clip.vol() * m_volume);

        } else if (clip.loop() && clip.fade()) {
            clip.buf() = clip.wav()->buf();
            clip.set_len(clip.wav()->len());
        } else if (clip.loop()) {
            clip.buf() = clip.wav()->buf(); /* reset */
            clip.set_len(clip.wav()->len());
        } else {
            clip.buf() = clip.wav()->buf(); /* reset */
            clip.set_len(clip.wav()->len());
            clip.set_done();
        }
    }
    /* Remove finished clips */
    auto del = remove_if(m_clips.begin(), m_clips.end(), [](clip &c) { return c.done(); });
    m_clips.erase(del, m_clips.end());
}

device::device(const string &name, enum type t) : m_name(name), m_type(t) {}

device::~device()
{
    close();
}

void device::close()
{
    if (m_audio_enabled) {
        pause();
        SDL_CloseAudioDevice(m_id);
        m_id = 0;
        m_audio_enabled = false;
    }
}

void device::pause()
{
    if (m_audio_enabled)
        SDL_PauseAudioDevice(m_id, 1);
}

void device::unpause()
{
    if (m_audio_enabled)
        SDL_PauseAudioDevice(m_id, 0);
}

bool device::open(const SDL_AudioSpec &s)
{
    SDL_AudioSpec obt{};
    auto id = SDL_OpenAudioDevice(m_name.c_str(), m_type == CAPTURE, &s, &obt, SDL_AUDIO_ALLOW_CHANGES);
    if (id <= 0) {
        printf("Failed to open audio device '%s': '%s'\n", m_name.c_str(), SDL_GetError());
        m_audio_enabled = false;
        return false;
    }
    m_spec = obt;
    printf("Obtained audio device '%s' with %d channels, %d Hz, %d samples\n", m_name.c_str(), m_spec.channels, m_spec.freq, m_spec.samples);
    m_id = id;
    m_audio_enabled = true;
    unpause();
    return true;
}

speaker::speaker(const string &name) : device(name, PLAYBACK)
{
#if _WIN32
    m_channels[id::MUSIC] = channel(0.1);
    m_channels[id::SFX] = channel(0.1);
#endif
}

void speaker::play_audio(uint8_t *stream, int len)
{
    for (auto &channel : m_channels)
        channel.second.audio_tick(stream, len);
}

void speaker::stop_channel(id chan)
{
    SDL_LockAudioDevice(m_id);
    m_channels[chan].stop();
    SDL_UnlockAudioDevice(m_id);
}

void speaker::play(id chan, const clip &clip)
{
    SDL_LockAudioDevice(m_id);
    m_channels[chan].add_clip(clip);
    SDL_UnlockAudioDevice(m_id);
}

microphone::microphone(const string &name) : device(name, CAPTURE) {}

shared_ptr<speaker> active_playback_device = nullptr;
shared_ptr<microphone> active_capture_device = nullptr;
vector<shared_ptr<device>> devices;
vector<string> drivers;
string active_driver = "";

void audio_callback(void *, uint8_t *stream, int len)
{
    SDL_memset(stream, 0, len);
    if (active_playback_device)
        active_playback_device->play_audio(stream, len);
}

bool init()
{
    SDL_AudioQuit();
    drivers.clear();
    devices.clear();
    active_driver = "";
    for (auto &dev : devices)
        dev->close();
    active_playback_device = nullptr;
    string driver = "";

    for (int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        if (driver.empty())
            driver = SDL_GetAudioDriver(i);
        drivers.emplace_back(SDL_GetAudioDriver(i));
        printf("Available audio driver: %s\n", SDL_GetAudioDriver(i));
    }

#if _WIN32
    // check if we have directsound
    if (find(drivers.begin(), drivers.end(), "directsound") != drivers.end()) {
        driver = "directsound";
    }
#endif

    if (driver.empty()) {
        printf("No available audio driver found\n");
        return false;
    }

    /* Initialize Driver */
    if (SDL_AudioInit(driver.c_str()) < 0) {
        printf("Audio driver initialization failed: '%s'\n", SDL_GetError());
        return false;
    }

    active_driver = driver;

    for (int i = 0; i < SDL_GetNumAudioDevices(0); i++) {
        auto *name = SDL_GetAudioDeviceName(i, 0);
        printf("Available playback device: %s\n", name);
        auto dev = make_shared<speaker>(name);
        devices.emplace_back(dev);
    }
    if (devices.empty()) {
        printf("No Playback devices\n");
        return false;
    }

    Uint32 devidx = 0;
    bool success = false;
    for (; devidx < devices.size();) {
        active_playback_device = dynamic_pointer_cast<speaker>(devices[devidx++]);
        SDL_AudioSpec want{};
        want.freq = AUDIO_FREQUENCY;
        want.format = AUDIO_FORMAT;
        want.channels = AUDIO_CHANNELS;
        want.samples = AUDIO_SAMPLES;
        want.callback = audio_callback;

        if (active_playback_device->open(want)) {
            success = true;
            printf("Initialized '%s' as playback device\n", active_playback_device->name().c_str());
            break;
        } else {
            printf("Couldn't initialize '%s': '%s'\n", active_playback_device->name().c_str(), SDL_GetError());
        }
    }

    if (!success) {
        printf("No valid playback device was found\n");
        return false;
    }

    for (int i = 0; i < SDL_GetNumAudioDevices(1); i++) {
        auto *name = SDL_GetAudioDeviceName(i, 1);
        printf("Available capture device: %s\n", name);
        auto dev = make_shared<microphone>(name);
        devices.emplace_back(dev);
        if (!active_capture_device)
            active_capture_device = dev;
    }
    return true;
}

void close()
{
    devices.clear();
    SDL_AudioQuit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

}

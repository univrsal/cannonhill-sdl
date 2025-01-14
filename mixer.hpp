#pragma once
#include <SDL.h>
#include <memory>
#include <vector>
#include <map>
#include <string>

#define AUDIO_FORMAT AUDIO_S16SYS

#define AUDIO_FREQUENCY 48000
#define AUDIO_SAMPLES 8192
#define AUDIO_CHANNELS 1
#define AUDIO_MAX_SOUNDS 25
 #define SDL_AUDIO_ALLOW_CHANGES 0 //SDL_AUDIO_ALLOW_ANY_CHANGE

using namespace std;
namespace audio {

enum class id { MUSIC, SFX };

class wav_file {
    string m_path{};
    uint8_t *m_buf{};
    uint32_t m_len{};
    SDL_AudioSpec m_spec{};
    Uint32 m_last_play_time{};
    double m_duration{};

public:
    uint32_t &len() { return m_len; }
    uint8_t *buf() { return m_buf; }
    const SDL_AudioSpec &spec() { return m_spec; }
    Uint32 const &last_play_time() const{ return m_last_play_time; }
    void set_last_play_time(Uint32 t) { m_last_play_time = t; }
    double const &duration() const { return m_duration; }
    wav_file(const string &path);

    ~wav_file();
};

class clip {
    uint32_t m_len{}; /* Modified during mixing */
    uint8_t *m_buf{}; /* Modified during mixing */
    uint8_t m_vol{};
    shared_ptr<wav_file> m_wav{};
    bool m_loop{}, m_fade{}, m_done{};

public:
    clip(shared_ptr<wav_file> w, uint8_t vol = 255, bool loop = false);
    ~clip();

    bool const& loop() const { return m_loop; }
    bool const& fade() const { return m_fade; }
    bool const& done() const { return m_done; }
    uint8_t const &vol() const { return m_vol; }
    uint32_t const &len() const { return m_len; }
    uint8_t * const&buf() const { return m_buf; }
    uint8_t * &buf()  { return m_buf; }
    shared_ptr<wav_file> wav() const { return m_wav; }

    void set_fade(bool f = true) { m_fade = f; }
    void set_done(bool d = true) { m_done = d; }
    void set_vol(uint8_t v) { m_vol = v; }
    void set_len(uint32_t l) { m_len = l; }
};

class device {
protected:
    bool m_audio_enabled;
    string m_name;
    enum type { CAPTURE, PLAYBACK } m_type;
    SDL_AudioSpec m_spec;
    SDL_AudioDeviceID m_id;

public:
    device(const string &name, enum type t);
    ~device();
    void pause();
    void unpause();
    virtual bool open(const SDL_AudioSpec &s);
    virtual void close();
    type type() const { return m_type; }
    const string &name() const { return m_name; }
};

class channel {
    vector<clip> m_clips;
    float m_volume;

public:
    /* Groundbreaking technology: A game with the volume at 50% by default.
     * GOTY $CURRENT_YEAR 10/10 - IGN
     */
    channel(float vol = 0.5) : m_volume(vol) {}
    void add_clip(const clip &c);
    void audio_tick(uint8_t *stream, int len);
    void stop();

    vector<clip> &clips() { return m_clips; }
};

class speaker : public device {
    map<id, channel> m_channels;
    void play_audio(uint8_t *stream, int len);
    friend void audio_callback(void *user, uint8_t *stream, int len);

public:
    speaker(const string &name);
    virtual ~speaker() = default;

    void play(id chan, const clip &c);
    void stop_channel(id chan);

    map<id, channel> &channels() { return m_channels; }
};

class microphone : public device {
public:
    microphone(const string &name);
};

extern shared_ptr<speaker> active_playback_device;
extern shared_ptr<microphone> active_capture_device;
extern vector<shared_ptr<device>> devices;
extern vector<string> drivers;
extern string active_driver;

extern bool init();
extern void close();
}

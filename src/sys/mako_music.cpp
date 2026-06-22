#include "mako_music.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include "common.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#include "stb_vorbis.h"

struct DecodedChunk {
	const Uint8* data;
	int frames;
};

class MakoMusicDecoder {
public:
	virtual ~MakoMusicDecoder() = default;
	virtual bool is_open() const = 0;
	virtual const SDL_AudioSpec& spec() const = 0;
	virtual DecodedChunk decode(int max_frames) = 0;
	virtual void seek_start() = 0;
};

namespace {

class Mp3Decoder final : public MakoMusicDecoder {
public:
	explicit Mp3Decoder(const std::string& path)
	{
		if (!drmp3_init_file(&mp3, path.c_str(), nullptr)) {
			WARNING("drmp3_init_file failed: %s", path.c_str());
			return;
		}
		initialized = true;
		spec_.freq = static_cast<int>(mp3.sampleRate);
		spec_.format = AUDIO_S16SYS;
		spec_.channels = static_cast<Uint8>(mp3.channels);
	}

	~Mp3Decoder() override
	{
		if (initialized)
			drmp3_uninit(&mp3);
	}

	bool is_open() const override { return initialized; }
	const SDL_AudioSpec& spec() const override { return spec_; }

	DecodedChunk decode(int max_frames) override
	{
		int frames = static_cast<int>(drmp3_read_pcm_frames_s16(&mp3, max_frames, pcm.data()));
		return { reinterpret_cast<const Uint8*>(pcm.data()), frames };
	}

	void seek_start() override { drmp3_seek_to_pcm_frame(&mp3, 0); }

private:
	drmp3 mp3{};
	SDL_AudioSpec spec_{};
	std::array<int16_t, 1024 * 2> pcm;
	bool initialized = false;
};

class OggDecoder final : public MakoMusicDecoder {
public:
	explicit OggDecoder(const std::string& path)
	{
		int error = 0;
		vorbis = stb_vorbis_open_filename(path.c_str(), &error, nullptr);
		if (!vorbis) {
			WARNING("stb_vorbis_open_filename failed: %s (error %d)", path.c_str(), error);
			return;
		}
		stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		spec_.freq = static_cast<int>(info.sample_rate);
		spec_.format = AUDIO_S16SYS;
		spec_.channels = static_cast<Uint8>(info.channels);
	}

	~OggDecoder() override
	{
		if (vorbis)
			stb_vorbis_close(vorbis);
	}

	bool is_open() const override { return vorbis != nullptr; }
	const SDL_AudioSpec& spec() const override { return spec_; }

	DecodedChunk decode(int max_frames) override
	{
		int frames = stb_vorbis_get_samples_short_interleaved(
			vorbis, spec_.channels, pcm.data(), max_frames * spec_.channels);
		return { reinterpret_cast<const Uint8*>(pcm.data()), frames };
	}

	void seek_start() override { stb_vorbis_seek_start(vorbis); }

private:
	stb_vorbis* vorbis = nullptr;
	SDL_AudioSpec spec_{};
	std::array<int16_t, 1024 * 2> pcm;
};

class WavDecoder final : public MakoMusicDecoder {
public:
	explicit WavDecoder(const std::string& path)
	{
		if (!SDL_LoadWAV(path.c_str(), &spec_, &data, &data_size)) {
			WARNING("SDL_LoadWAV failed: %s: %s", path.c_str(), SDL_GetError());
			return;
		}
		frame_size = SDL_AUDIO_BITSIZE(spec_.format) / 8 * spec_.channels;
		if (data_size == 0 || frame_size <= 0)
			WARNING("Invalid or empty WAV file: %s", path.c_str());
	}

	~WavDecoder() override
	{
		if (data)
			SDL_FreeWAV(data);
	}

	bool is_open() const override { return data_size > 0 && frame_size > 0; }
	const SDL_AudioSpec& spec() const override { return spec_; }

	DecodedChunk decode(int max_frames) override
	{
		size_t available_frames = (data_size - pos) / frame_size;
		int frames = static_cast<int>(std::min<size_t>(available_frames, max_frames));
		const Uint8* chunk_data = data + pos;
		pos += frames * frame_size;
		return { chunk_data, frames };
	}

	void seek_start() override { pos = 0; }

private:
	SDL_AudioSpec spec_{};
	Uint8* data = nullptr;
	Uint32 data_size = 0;
	size_t pos = 0;
	int frame_size = 0;
};

bool has_extension(const std::string& path, const char* ext)
{
	size_t len = strlen(ext);
	if (path.size() < len)
		return false;
	std::string tail = path.substr(path.size() - len);
	std::transform(tail.begin(), tail.end(), tail.begin(),
				   [](unsigned char c) { return std::tolower(c); });
	return tail == ext;
}

std::unique_ptr<MakoMusicDecoder> create_decoder(const std::string& path)
{
	std::unique_ptr<MakoMusicDecoder> decoder;
	if (has_extension(path, ".ogg") || has_extension(path, ".oga"))
		decoder = std::make_unique<OggDecoder>(path);
	else if (has_extension(path, ".wav"))
		decoder = std::make_unique<WavDecoder>(path);
	else
		decoder = std::make_unique<Mp3Decoder>(path);
	if (!decoder->is_open())
		return nullptr;
	return decoder;
}

} // namespace

MakoMusic::MakoMusic(const std::string& path, int loops, const SDL_AudioSpec& device_spec)
	: decoder(create_decoder(path)), loops_(loops)
{
	if (!decoder)
		return;
	const SDL_AudioSpec& src_spec = decoder->spec();

	stream = SDL_NewAudioStream(src_spec.format, src_spec.channels, src_spec.freq,
							   device_spec.format, device_spec.channels, device_spec.freq);
	if (!stream) {
		WARNING("SDL_NewAudioStream failed: %s", SDL_GetError());
		return;
	}
	playing = true;
}

MakoMusic::~MakoMusic()
{
	if (stream)
		SDL_FreeAudioStream(stream);
}

void MakoMusic::decode()
{
	constexpr int CHUNK_FRAMES = 1024;

	DecodedChunk chunk = decoder->decode(CHUNK_FRAMES);

	if (chunk.frames == 0) {
		if (loops_ && --loops_ == 0) {
			SDL_AudioStreamFlush(stream);
			input_finished = true;
		} else {
			decoder->seek_start();
		}
		return;
	}
	const SDL_AudioSpec& spec = decoder->spec();
	int bytes = chunk.frames * SDL_AUDIO_BITSIZE(spec.format) / 8 * spec.channels;
	if (SDL_AudioStreamPut(stream, chunk.data, bytes) < 0) {
		WARNING("SDL_AudioStreamPut failed: %s", SDL_GetError());
		input_finished = true;
	}
}

void MakoMusic::mix(Uint8* out, int len)
{
	if (!stream || !playing)
		return;

	while (SDL_AudioStreamAvailable(stream) < len && !input_finished)
		decode();

	Uint8* tmp = SDL_stack_alloc(Uint8, len);
	int got = SDL_AudioStreamGet(stream, tmp, len);
	if (got > 0)
		SDL_MixAudioFormat(out, tmp, AUDIO_S16SYS, got, SDL_MIX_MAXVOLUME);
	SDL_stack_free(tmp);
	if (input_finished && SDL_AudioStreamAvailable(stream) <= 0)
		playing = false;
}

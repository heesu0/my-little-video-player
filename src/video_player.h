#ifndef __VIDEO_PLAYER_H__
#define __VIDEO_PLAYER_H__

#include "media_queue.h"

#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct AVPacket;
struct AVFrame;

class Demuxer;
class Decoder;
class VideoConverter;
class VideoRenderer;
class AudioBuffer;
class AudioResampler;
class AudioRenderer;

class VideoPlayer
{
public:
    explicit VideoPlayer(const std::string& filename);
    ~VideoPlayer() = default;

    void Init();
    void Run();

private:
    void start();
    void stop();
    void seekPlayback(int seekDuration);
    void demuxing();
    void decodingVideo();
    void decodingAudio();
    void convertingVideo();
    void resamplingAudio();
    void renderingVideo();
    void renderingAudio();
    void flushMediaQueue();

    std::shared_ptr<Demuxer> demuxer_;
    std::shared_ptr<Decoder> video_decoder_;
    std::shared_ptr<Decoder> audio_decoder_;
    std::shared_ptr<VideoConverter> video_converter_;
    std::shared_ptr<AudioResampler> audio_resampler_;
    std::shared_ptr<VideoRenderer> video_renderer_;
    std::shared_ptr<AudioRenderer> audio_renderer_;

    MediaQueue<AVPacket> video_decoder_queue_;
    MediaQueue<AVPacket> audio_decoder_queue_;
    MediaQueue<AVFrame> video_converter_queue_;
    MediaQueue<AVFrame> audio_resampler_queue_;
    MediaQueue<AVFrame> video_renderer_queue_;
    MediaQueue<AudioBuffer> audio_renderer_queue_;

    std::string filename_;
    std::vector<std::future<void>> task_queue_;

    std::atomic<int> demuxing_count_{0};
    std::mutex mutex_;
    std::mutex demuxer_mutex_;
    std::mutex video_decoder_mutex_;
    std::mutex audio_decoder_mutex_;
    std::mutex video_converter_mutex_;
    std::mutex audio_resampler_mutex_;
    std::mutex video_renderer_mutex_;
    std::mutex audio_renderer_mutex_;

    bool is_demuxing_completed_ = false;
    bool is_running_ = true;
};

#endif

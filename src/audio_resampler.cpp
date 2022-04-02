#include "audio_resampler.h"
#include "log.h"
#include <algorithm>

namespace {

// FIXME : How to manage target settings
constexpr int OUT_CHANNELS = 6;
constexpr int OUT_SAMPLE_RATE = 48000;
const AVSampleFormat OUT_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;

}// namespace

AudioResampler::AudioResampler(std::shared_ptr<AVCodecContext>& codec_context)
    : codec_context_(codec_context) {}

AudioResampler::~AudioResampler() = default;

void AudioResampler::init() {
  swr_context_ = std::shared_ptr<SwrContext>(
          swr_alloc_set_opts(
                  nullptr,
                  av_get_default_channel_layout(codec_context_->channels),
                  OUT_SAMPLE_FORMAT, codec_context_->sample_rate,
                  av_get_default_channel_layout(codec_context_->channels),
                  codec_context_->sample_fmt, codec_context_->sample_rate, 0,
                  nullptr),
          [](SwrContext* swr_context) { swr_free(&swr_context); });
  if (swr_init(swr_context_.get())) {
    LOG::ERROR("Couldn't initialize SwrContext");
  }
}

void AudioResampler::resampleFrame(
        std::shared_ptr<AVFrame>& input_frame,
        std::shared_ptr<AudioBuffer>& output_buffer) {
  uint8_t** input_data = input_frame->data;
  int input_nb_samples = input_frame->nb_samples;

  int output_size = av_samples_get_buffer_size(
          nullptr, codec_context_->channels, input_frame->nb_samples,
          codec_context_->sample_fmt, 1);
  if (output_size < 0) {
    LOG::ERROR("Couldn't get audio buffer size");
  }

  int output_nb_samples = static_cast<int>(av_rescale_rnd(
          swr_get_delay(swr_context_.get(), input_frame->sample_rate) +
                  input_frame->nb_samples,
          OUT_SAMPLE_RATE, OUT_SAMPLE_FORMAT, AV_ROUND_INF));

  auto resampled_data = new uint8_t[output_size];
  int output_samples =
          swr_convert(swr_context_.get(), &resampled_data, output_nb_samples,
                      (const uint8_t**) input_data, input_nb_samples);
  if (output_samples < 0) {
    LOG::ERROR("Couldn't convert audio samples");
  }

  int resampled_data_size = OUT_CHANNELS * output_samples *
                            av_get_bytes_per_sample(OUT_SAMPLE_FORMAT);
  output_buffer =
          std::make_shared<AudioBuffer>(resampled_data_size, input_frame->pts);
  std::copy(resampled_data, resampled_data + resampled_data_size,
            output_buffer->buffer());

  delete[] resampled_data;
}

#include "audio_resampler.h"
#include "log.h"

namespace {

constexpr int out_channels = 2;
constexpr int out_sample_rate = 48000;
const AVSampleFormat out_sample_format = AV_SAMPLE_FMT_S16;

}// namespace

AudioResampler::AudioResampler(std::shared_ptr<AVCodecContext>& codec_context)
    : codec_context_(codec_context) {}

AudioResampler::~AudioResampler() = default;

void AudioResampler::init() {
  swr_context_ = std::shared_ptr<SwrContext>(
          swr_alloc_set_opts(
                  nullptr, av_get_default_channel_layout(out_channels),
                  out_sample_format, out_sample_rate,
                  av_get_default_channel_layout(codec_context_->channels),
                  codec_context_->sample_fmt, codec_context_->sample_rate, 0,
                  nullptr),
          [](SwrContext* swr_context) { swr_free(&swr_context); });
  if (swr_init(swr_context_.get())) {
    LOG::ERROR("Couldn't initialize SwrContext");
  }
}

int AudioResampler::resampleFrame(std::shared_ptr<AVFrame>& input_frame,
                                  uint8_t* output_buffer) {
  uint8_t** input_buffer = input_frame->data;
  int input_nb_samples = input_frame->nb_samples;

  int output_size = av_samples_get_buffer_size(
          nullptr, codec_context_->channels, input_frame->nb_samples,
          codec_context_->sample_fmt, 1);
  if (output_size < 0) {
    LOG::ERROR("Couldn't get audio buffer size");
  }

  // memcpy(output_buffer, input_frame->data[0], output_size);

  int output_nb_samples = static_cast<int>(av_rescale_rnd(
          swr_get_delay(swr_context_.get(), input_frame->sample_rate) +
                  input_frame->nb_samples,
          out_sample_rate, out_sample_format, AV_ROUND_INF));
  int output_samples =
          swr_convert(swr_context_.get(), &output_buffer, output_nb_samples,
                      (const uint8_t**) input_buffer, input_nb_samples);
  if (output_samples < 0) {
    LOG::ERROR("Couldn't convert audio samples");
  }

  return out_channels * output_samples *
         av_get_bytes_per_sample(out_sample_format);
}

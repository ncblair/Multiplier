#pragma once

#include <atomic>

class Shifter {
  public:
    Shifter(double sample_rate_, int max_delay, int channel_, float* RAM);
    ~Shifter();
    void setShift(float semi);
    void process(const float* in_buf, float *out_buf, int size);
    void setMix(float dry_wet);
  private:
    float processSingle(float sample);
    // void fill_window(int size);
    // float pitch_shift_sample(float* buffer, float* window_buf, int idx, int num_samples);
    inline int I_IDX(int idx) { return 2 * idx + channel;}
    float lerp(float* buffer, float idx, int buffer_size);
    float window_func(float phase);

    double sample_rate;
    int max_delay_samples;//, hop_size;
    std::atomic<float> shift;
    // float* process_block; // block_size
    // float* OLA_processed_block; // hop_size
    // float* OLA_unprocessed_block;
    // float* window; // block_size

    float* circular_in_buffer;
    // float* circular_out_buffer;
    int circular_in_idx;
    // int circular_out_idx;
    // int num_ready_samples;



    int channel;


    float phase{0.0f}; // 0 to window_size
    int window_size{8192}; // number of sample per grain
    std::atomic<float> mix{1.0f};
    // const int OG_WINDOW_SIZE{8192};
    // float window_jitter_samples = 128.0f;
};
#pragma once

class Shifter;

#include <memory>
#include <atomic>

class Harmonizer {
  public:
    Harmonizer(double sample_rate_, int max_delay_samples, int num_voices_, float* RAM);
    ~Harmonizer();
    void setSemi(int semi, int voice);
    void setDryWet(float dry_wet_);
    void setVoiceOn(bool is_on, int voice);
    void process(const float* in_buf, float *out_buf, int num_samples);
  private:
    
    Shifter** shifters;
    std::atomic<bool>* voice_on;
    int num_voices;
    // std::atomic<float> dry_wet;
};
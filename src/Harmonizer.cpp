#include "Harmonizer.h"
#include "PitchShifter.h"

Harmonizer::Harmonizer(double sample_rate_, int max_delay_samples, int num_voices_, float* RAM) : num_voices(num_voices_){

    
    shifters = (Shifter**) malloc(sizeof(Shifter*) * num_voices * 2); // one for each channel
    // shifters = (Shifter*) malloc(sizeof(Shifter*) * num_voices * 2); // one for each channel
    voice_on = (std::atomic<bool>*) malloc(sizeof(std::atomic<bool>) * num_voices);
    for (int v = 0; v < num_voices; ++v) {
        shifters[2 * v] = new Shifter(sample_rate_, max_delay_samples, 0, RAM + v * 2 * max_delay_samples);
        shifters[2 * v + 1] = new Shifter(sample_rate_, max_delay_samples, 1, RAM + (v * 2 + 1) * max_delay_samples);
        voice_on[v] = true;
    }

    // dry_wet = 1.0f;
}

Harmonizer::~Harmonizer() {
    for (int v = 0; v < num_voices; ++v) {
        delete shifters[2*v];
        delete shifters[2*v + 1];
    }
    free(shifters);
    free(voice_on);
}

void Harmonizer::setSemi(int semi, int voice) {
    shifters[2 * voice]->setShift(semi);
    shifters[2 * voice + 1]->setShift(semi);
}

void Harmonizer::setDryWet(float dry_wet_) {
    for (int v = 0; v < num_voices; ++v) {
        if (!voice_on[v].load()) continue;
        shifters[2 * v]->setMix(dry_wet_);
        shifters[2 * v + 1]->setMix(dry_wet_);
    }
    // dry_wet.store(dry_wet_);
}

void Harmonizer::process(const float* in_buf, float *out_buf, int num_samples) {
    for (int i = 0; i < 2 * num_samples; ++i) {
        out_buf[i] = 0.0f;
    }
    for (int v = 0; v < num_voices; ++v) {
        if (!voice_on[v].load()) continue;
        shifters[2 * v]->process(in_buf, out_buf, num_samples);
        shifters[2 * v + 1]->process(in_buf, out_buf, num_samples);
    }
    // auto mix = dry_wet.load();
    // for (int i = 0; i < 2 * num_samples; ++i) {
        // out_buf[i] = (1.0f - mix) * in_buf[i] + mix * out_buf[i];
    // }
}

void Harmonizer::setVoiceOn(bool is_on, int voice) {
    voice_on[voice].store(is_on);
}
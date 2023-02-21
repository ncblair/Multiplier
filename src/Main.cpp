#include "daisy_seed.h"

#include <ctime>
#include <memory>
#include <atomic>

#include "Harmonizer.h"
 
using namespace daisy;
using namespace daisy::seed;
 
DaisySeed hw;

float sample_rate;
const int  NUM_VOICES = 3;
const int MAX_DELAY_SAMPLES = 8192;
const float major_scale[8] = {0.0f, 2.0f, 4.0f, 5.0f, 7.0f, 9.0f, 11.0f, 12.0f};

std::unique_ptr<Harmonizer> harmonizer;

float DSY_SDRAM_BSS RAM[NUM_VOICES * 2 * MAX_DELAY_SAMPLES];

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    harmonizer->process(in, out, size / 2);
}

int scale_note(float note) {
    auto major_note = std::lower_bound(major_scale, major_scale + 8, note);
    if (std::abs(*major_note - note) < std::abs(*(major_note - 1) - note)) {
        return int(*major_note);
    }
    else {
        return int(*(major_note - 1));
    }
}

int main(void) {

    size_t blocksize = 64;
    // Initialize the Daisy Seed
    hw.Init();

    GPIO my_led;
    my_led.Init(D28, GPIO::Mode::OUTPUT);
    
    // Create a GPIO object
    GPIO buttons[NUM_VOICES] = {};
    
    // Initialize the GPIO object
    buttons[0].Init(D16, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    buttons[1].Init(D17, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    buttons[2].Init(D18, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    AdcChannelConfig knobs[NUM_VOICES + 1] = {};
    knobs[0].InitSingle(D19);
    knobs[1].InitSingle(D20);
    knobs[2].InitSingle(D21);
    knobs[3].InitSingle(D22);
    hw.adc.Init(knobs, NUM_VOICES + 1);
    hw.adc.Start();


    sample_rate = hw.AudioSampleRate();
    hw.SetAudioBlockSize(2 * blocksize);
    hw.StartAudio(AudioCallback);

    harmonizer = std::make_unique<Harmonizer>(sample_rate, MAX_DELAY_SAMPLES, NUM_VOICES, RAM);
    
    while(1) {
        for (int v = 0; v < NUM_VOICES; ++v) {
            float knob_val = hw.adc.GetFloat(v);
            bool button_state = buttons[v].Read();
            harmonizer->setSemi((2.0f * knob_val - 1.0f) * 12.0f, v);
            harmonizer->setVoiceOn(button_state, v);
        }
        float mix_val = hw.adc.GetFloat(3);
        harmonizer->setDryWet(1.0 - mix_val);

        // hw.SetLed(mix_val > 0.5);
        System::Delay(1);
    }
}
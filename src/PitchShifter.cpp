#include "PitchShifter.h"
#include <algorithm>
#include <cassert>

Shifter::Shifter(double sample_rate_, int max_delay, int channel_, float* RAM) 
    : sample_rate(sample_rate_), max_delay_samples(max_delay), 
    // hop_size(block_size_ / 2), 
    channel(channel_)
{

    // "ALLOCATE" MEMORY
    // process_block = RAM;
    circular_in_buffer = RAM;
    // OLA_processed_block = RAM + max_delay_samples;
    // OLA_unprocessed_block = OLA_processed_block + hop_size;
    // circular_in_buffer = OLA_unprocessed_block + hop_size;
    // circular_out_buffer = circular_in_buffer + max_delay_samples;
    // window = circular_out_buffer + max_delay_samples;

    circular_in_idx = 0;
    // circular_out_idx = 0;
    // num_ready_samples = 0;
    for (int i = 0; i < max_delay_samples; ++i) {
        // process_block[i] = 0.0f;
        circular_in_buffer[i] = 0.0f;
        // circular_out_buffer[i] = 0.0f;
    }
    // for (int i = 0; i < hop_size; ++i) {
    //     OLA_processed_block[i] = 0.0f;
    //     OLA_unprocessed_block[i] = 0.0f;
    // }

    // window = (float*) malloc(sizeof(float) * max_delay_samples);
    shift.store(0.0f);
    // fill_window(max_delay_samples);
}

Shifter::~Shifter() {

}

void Shifter::setShift(float semi) {
    shift.store(std::pow(2.0f, semi / 12.0f));
}

void Shifter::process(const float *in_buf, float* out_buf, int size) {
    for (int i = 0; i < size; ++i) {
        out_buf[I_IDX(i)] += processSingle(in_buf[I_IDX(i)]);
    }
}

float Shifter::processSingle(float sample) {
    // the last blocksize samples are stored in circular_in_buffer already right now. so we just run process on that for a single sample
    circular_in_buffer[circular_in_idx] = sample;
    circular_in_idx++;
    if (circular_in_idx == max_delay_samples) circular_in_idx = 0;

    // phase is 0 to window_size
    auto idx_0 = circular_in_idx - 1 - window_size + phase;
    if (idx_0 < 0) idx_0 += max_delay_samples;
    auto idx_1 = idx_0 + window_size * 0.5;
    if (idx_1 > max_delay_samples) idx_1 -= max_delay_samples;

    auto out_1 = lerp(circular_in_buffer, idx_0, max_delay_samples);
    auto out_2 = lerp(circular_in_buffer, idx_1, max_delay_samples);

    auto result = window_func(phase / window_size) * out_1 + window_func(phase / window_size + 0.5) * out_2;

    phase += shift.load() - 1.0f; // -1 because the default movement of no delay is normal playback speed
    if (phase >= window_size) { 
        phase -= window_size;
        // window_size = OG_WINDOW_SIZE + (std::rand() / (RAND_MAX + 1u)) * window_jitter_samples;
    }
    else if (phase < 0) {
        phase += window_size;
        // window_size = OG_WINDOW_SIZE + (std::rand() / (RAND_MAX + 1u)) * window_jitter_samples;
    }
    auto m = mix.load();
    return result * m + circular_in_buffer[circular_in_idx] * (1.0 - m);
}

float Shifter::window_func(float phase) {
    return std::pow(std::sin(3.14159265358979323f * phase), 2.0f);
}

float Shifter::lerp(float* buffer, float idx, int buffer_size) {
    int i1 = int(idx);
    int i2 = (i1 + 1) % buffer_size;
    float alpha = idx - int(idx);
    return (1.0f - alpha) * buffer[i1] + alpha * buffer[i2];
}

void Shifter::setMix(float dry_wet) {
    mix.store(dry_wet);
}
// void Shifter::process(const float *in_buf, float* out_buf, int size) { // buffer is interleaved
//     // SIZE = 256
//     // BLOCKSIZE >= 256
//     // BUFFER IS INTERLEAVED
//     // EVERYTHING ELSE IS NOT INTERLEAVED


//     // push to circular_buffer
//     for (int i = 0; i < size; ++i) {
//         circular_in_buffer[circular_in_idx] = in_buf[I_IDX(i)];
//         circular_in_idx++;
//         if (circular_in_idx == max_delay_samples) circular_in_idx = 0;
//     }
//     num_ready_samples += size;
//     if (num_ready_samples >= max_delay_samples) {
//         num_ready_samples -= max_delay_samples;
//         auto start_idx = circular_in_idx - max_delay_samples;
//         if (start_idx < 0) start_idx += max_delay_samples;
        
//         auto obi = circular_out_idx;
//         for (int i = 0; i < max_delay_samples; ++i) {
//             circular_out_buffer[obi] = 0.0f;
//             obi++;
//             if (obi == max_delay_samples) obi = 0;
//         }

//         // ADD PROCESSED AND UNPROCESSED INFO FROM LAST BLOCK
//         obi = circular_out_idx;
//         for (int i = 0; i < hop_size; ++i) {
//             circular_out_buffer[obi] += OLA_processed_block[i];
//             obi++;
//             if (obi == max_delay_samples) obi = 0;
//             process_block[i] = OLA_unprocessed_block[i];
//         }

//         // ADD UNPROCESSED INFO FROM CURRENT BLOCK TO OLA
//         auto ibi = (start_idx + hop_size) % max_delay_samples;
//         for (int i = hop_size; i < block_size; ++i) {
//             OLA_unprocessed_block[i - hop_size] = circular_in_buffer[ibi];
//             ibi++;
//             if (ibi == block_size) ibi = 0;
//         }

//         // FILL PROCESS BLOCK WITH FIRST HALF OF SAMPLES FOR FIRST PROCESS
//         ibi = start_idx;
//         for (int i = 0; i < hop_size; ++i) {
//             process_block[i + hop_size] = circular_in_buffer[ibi];
//             ibi++;
//             if (ibi == block_size) ibi = 0;
//         }

//         // PROCESS PROCESS BLOCK
//         obi = circular_out_idx;
//         for (int i = 0; i < block_size; ++i) {
//             circular_out_buffer[obi] += window[i] * pitch_shift_sample(process_block, window, i, block_size);
//             obi++;
//             if (obi == block_size) obi = 0;
//         }
        
//         // FILL PROCESS BLOCK WITH CURRENT SAMPLES
//         ibi = start_idx;
//         for (int i = 0; i < block_size; ++i) {
//             process_block[i] = circular_in_buffer[ibi];
//             ibi++;
//             if (ibi == block_size) ibi = 0;
//         }

//         // PROCESS PROCESS BLOCK SECOND TIME (filling into buffer + ola)
//         obi = (circular_out_idx + hop_size) % block_size;
//         for (int i = 0; i < block_size; ++i) {
//             if (i < hop_size) {
//                 circular_out_buffer[obi] += window[i] * pitch_shift_sample(process_block, window, i, block_size);
//             } else {
//                 OLA_processed_block[i - hop_size] = window[i] * pitch_shift_sample(process_block, window, i, block_size);
//             }
//             obi++;
//             if (obi == block_size) obi = 0;
//         }
//     }

//     // ADD circular_out_buffer TO OUT_BUF
//     for (int i = 0; i < size; ++i) {
//         out_buf[I_IDX(i)] += circular_out_buffer[circular_out_idx];
//         circular_out_idx++;
//         if (circular_out_idx == block_size) circular_out_idx = 0;
//     }
// }

// float Shifter::pitch_shift_sample(float* buffer, float* window_buf, int idx, int num_samples) {
//     // auto grain_size_samples = num_samples - 15;

//     float shifted_idx = idx * shift;
//     int i1 = int(shifted_idx) % num_samples;
//     int i2 = (i1 + 1) % num_samples;
//     float alpha = shifted_idx - int(shifted_idx);


//     return (1.0f - alpha) * buffer[i1] * window[i1] + alpha * buffer[i2] * window[i2];
// }

// void Shifter::fill_window(int size)
// {
//     for (int i = 0; i < size; ++i) {
//         window[i] = std::sqrt(std::pow(std::sin(3.14159265358979323f * float(i)/float(size)), 2.0f));
//     }
// }

#include "ap.h"
#include "csi_math.cpp"
#include "esp_log.h"
#include <algorithm>
#include <cmath>
#include <fasttransforms.h>
#include <vector>
#include <complex>
#include <numeric>

namespace features {


    void complex_to_float_vec(std::vector<std::complex<float> > data, std::vector<float> &output_data) {
        for (int i = 0; i < data.size() * 2; i++) {
            output_data.emplace_back(data[i].real());
            output_data.emplace_back(data[i + 1].imag());
        }
    }

    void preprocess_data(std::vector<float> &values, std::vector<std::complex<float> > &output) {
        const int N = values.size() / 2;
        std::vector<std::complex<float> > H;
        H.reserve(N);

        for (int i = 0; i < values.size(); i += 2) {
            H.emplace_back(values.at(i), values.at(i + 1));
        }

        std::vector<float> phases;
        phases.reserve(N);

        for (auto &c: H) {
            phases.push_back(std::atan2(std::imag(c), std::real(c)));
        }
        std::vector<float> phases_raw;
        phases_raw.reserve(N);

        for (auto &c: H) {
            phases_raw.push_back(std::atan2(std::imag(c), std::real(c)));
        }

        float prev = phases[0];
        for (int i = 0; i < N; i++) {
            float dp = phases[i] - prev;
            if (std::abs(dp) > M_PI) {
                dp -= 2 * M_PI * std::round(dp / (2 * M_PI));
                phases[i] = prev + dp;
            }
            prev = phases[i];
        }

        float mean_phase = csi_math::mean(phases);


        float phase1 = phases.front();
        float phase31 = phases.back();

        float b_phase = (phase31 - phase1) / (2.0f * M_PI * 28.0f);

        std::vector<int> freq_idx;
        for (int i = -28; i < 0; i++)
            freq_idx.push_back(i);
        for (int i = 1; i <= 28; i++)
            freq_idx.push_back(i);
        int L = std::min(static_cast<int>(freq_idx.size()), N);


        for (int i = 0; i < L; i++) {
            float mag = std::abs(H[i]);
            float new_angle = phases_raw[i] - mean_phase - b_phase * freq_idx[i];
            output.emplace_back(mag * std::exp(std::complex(0.0f, new_angle)));
        }
    }
    void preprocess_data_cnn(std::vector<float> &values) {
        std::vector<std::complex<float> > normalized_complex;
        normalized_complex.reserve(56);
        preprocess_data(values, normalized_complex);
        complex_to_float_vec(normalized_complex, values);
    }
    void extractFeatures(int8_t *values, std::vector<float> &features) {
        std::vector<float> csi_processed;
        csi_processed.reserve(112);
        csi_processed.insert(csi_processed.end(), values, values + 112);

        std::vector<std::complex<float> > csi_processed_complex;
        csi_processed_complex.reserve(56);

        preprocess_data(csi_processed, csi_processed_complex);
        const int N = csi_processed_complex.size();
        std::vector<float> amps(N), phases(N);

        for (int i = 0; i < N; ++i) {
            float re = std::real(csi_processed_complex[i]);
            float im = std::imag(csi_processed_complex[i]);
            amps[i] = std::abs(csi_processed_complex[i]);
            phases[i] = std::atan2(im, re);
        }

        float amp_mean = csi_math::mean(amps);
        float amp_std = csi_math::stdev(amps, amp_mean);
        float amp_min = *std::min_element(amps.begin(), amps.end());
        float amp_max = *std::max_element(amps.begin(), amps.end());
        float amp_p25 = csi_math::percentile(amps, 0.25f);
        float amp_p75 = csi_math::percentile(amps, 0.75f);
        float phase_mean = csi_math::circular_mean(phases);
        float phase_std = csi_math::stdev(phases, phase_mean);

        alglib::complex_1d_array fft_input;
        fft_input.setlength(N);
        for (int i = 0; i < N; ++i)
            fft_input[i] = alglib::complex(amps[i], 0.0);
        alglib::fftc1d(fft_input);

        features.clear();
        features.push_back(amp_mean);
        features.push_back(amp_std);
        features.push_back(amp_min);
        features.push_back(amp_max);
        features.push_back(amp_p25);
        features.push_back(amp_p75);
        features.push_back(phase_mean);
        features.push_back(phase_std);

        for (int i = 0; i < std::min(13, N); ++i) {
            double re = fft_input[i].x;
            double im = fft_input[i].y;
            features.push_back(static_cast<float>(sqrt(re * re + im * im)));
        }
    }


    void vector_to_r1da(const std::vector<float> &input,
                        alglib::real_1d_array &output) {
        int N = input.size();
        std::vector<double> temp(N);
        for (int i = 0; i < N; ++i) {
            temp[i] = static_cast<double>(input[i]);
        }
        output.setcontent(N, temp.data());
    }
} // namespace features

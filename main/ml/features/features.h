#include "ap.h"
#include <vector>
#include <complex>
namespace features {
    void preprocess_data(std::vector<float> &values, std::vector<std::complex<float> > &output);
    void extractFeatures(int8_t *values,  std::vector<float> &features);
    void vector_to_r1da(const std::vector<float> &input,alglib::real_1d_array &output);
    void preprocess_data_cnn(std::vector<float> &values);
}; // namespace features

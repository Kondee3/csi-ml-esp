#include <vector>
#include <cmath>
#include <algorithm>

class csi_math {
private:
public:
    static float mean(const std::vector<float> &data) {
        float sum = 0;
        for (float x: data)
            sum += x;
        return sum / data.size();
    }

    static float circular_mean(const std::vector<float> &data) {
        float sin_sum = 0.0f;
        float cos_sum = 0.0f;
        for (float x: data) {
            sin_sum += std::sinf(x);
            cos_sum += std::cosf(x);
        }
        return std::atan2f(sin_sum/data.size(), cos_sum/data.size());
    }

    static float stdev(const std::vector<float> &data, float data_mean) {
        float sum_sq = 0;

        for (float x: data)
            sum_sq += pow(x - data_mean, 2);
        return sqrt(sum_sq / data.size());
    }

    static float percentile(std::vector<float> data, float p) {
        sort(data.begin(), data.end());
        float idx = p * (data.size() - 1);
        int i = static_cast<int>(idx);
        float frac = idx - i;

        if (i + 1 < data.size()) {
            return data[i] + frac * (data[i + 1] - data[i]);
        }
        return data[i];
    }
};

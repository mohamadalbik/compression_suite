#include "JPEGModules.h"

#include<opencv2/opencv.hpp>
#include<cmath>

constexpr auto PI = 3.14159265358979323846;
const auto INV_SQRT_2 = (1.0 / sqrt(2.0));
void IDCT1D(const float* input, float* output) {

    const float scale = 0.5;  

    for (int x = 0; x < 8; x++) {
        float sum = 0.0;

        for (int u = 0; u < 8; u++) {
            float Cu = (u == 0) ? INV_SQRT_2 : 1.0;
            float alpha = Cu * input[u];
            float cosine = cos((2 * x + 1) * u * PI / (2.0 * 8));

            sum += alpha * cosine;
        }

        output[x] = scale * sum;
    }
}



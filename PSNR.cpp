#include "JPEGModules.h"

double PSNR(const Mat& original, const Mat& reconstructed) {

	Mat diff;
	absdiff(original, reconstructed, diff);

	diff.convertTo(diff, CV_32F);

	diff = diff.mul(diff);

	double mse = sum(diff)[0] / (double)(original.total());

	if (mse == 0) return 100;

	double psnr = 10.0 * log10((255 * 255) / mse);

	return psnr;


}

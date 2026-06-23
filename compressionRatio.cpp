#include "JPEGModules.h"
#include<fstream>
double compressionRatio(const Mat& original,const std::string& filename){

	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	std::streamsize size = file.tellg();

	file.close();

	double originSize = original.total() * original.elemSize();
	
	double ratio = (originSize / size) * 100;

	return ratio;

}

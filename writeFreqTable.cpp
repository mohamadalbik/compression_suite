#include"FileFormating.h"

void writeFrequencyTable(std::ofstream& file, const std::map<int, int>& freqTable) {

	int size = freqTable.size();

	file.write((char*)&size, sizeof(int));

	for(auto& p : freqTable){
	
		int symbol = p.first;
		int freq = p.second;

		file.write((char*)&symbol, sizeof(int));
		file.write((char*)&freq, sizeof(int));
	
	}

}

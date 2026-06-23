#include"FileFormating.h"

std::map<int, int> readFrequencyTable(std::ifstream& file) {

	std::map<int, int> freqTable;

	int size = 0;
	file.read((char*)&size, sizeof(int));

	for (int i = 0; i < size; i++)
	{

		int symbol = 0;
		int freq = 0;

		file.read((char*)&symbol, sizeof(int));
		file.read((char*)&freq, sizeof(int));

		freqTable[symbol] = freq;

	}

	return freqTable;

}

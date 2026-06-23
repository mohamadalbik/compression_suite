#include "FileFormating.h"

void readImg(const std::string& fileName, PackedBitStream& Y, PackedBitStream& Cr, PackedBitStream& Cb, int& width, int& height)
{
	
	std::ifstream file(fileName, std::ios::binary);

	if (!file) {
		std::cout << "file not found!\n";
		return;
	}

	char magic[6] = { 0 };

	file.read(magic, 5);

	if (std::string(magic) != "BIKGP") {
		std::cout << "Invalid file\n";
		return;
	}

	file.read((char*)&width, sizeof(int));
	file.read((char*)&height, sizeof(int));

	Y.frequencyTable = readFrequencyTable(file);
	Cr.frequencyTable = readFrequencyTable(file);
	Cb.frequencyTable = readFrequencyTable(file);

	int ySize = 0, CrSize = 0, CbSize = 0;

	file.read((char*)&ySize, sizeof(int));
	file.read((char*)&CrSize, sizeof(int));
	file.read((char*)&CbSize, sizeof(int));

	int Yvalid = 0, Crvalid = 0, Cbvalid = 0;
	int Ywidth = 0, Crwidth = 0, Cbwidth = 0;
	int Yheight = 0, Crheight = 0, Cbheight = 0;


	file.read((char*)&Yvalid, sizeof(int));
	file.read((char*)&Ywidth, sizeof(int));
	file.read((char*)&Yheight, sizeof(int));

	file.read((char*)&Crvalid, sizeof(int));
	file.read((char*)&Crwidth, sizeof(int));
	file.read((char*)&Crheight, sizeof(int));

	file.read((char*)&Cbvalid, sizeof(int));
	file.read((char*)&Cbwidth, sizeof(int));
	file.read((char*)&Cbheight, sizeof(int));

	Y.data.resize(ySize);
	file.read((char*)Y.data.data(), ySize);
	
	Cr.data.resize(CrSize);
	file.read((char*)Cr.data.data(), CrSize);
	


	Cb.data.resize(CbSize);
	file.read((char*)Cb.data.data(), CbSize);
	
	Y.validBits = Yvalid;
	Y.width = Ywidth;
	Y.height = Yheight;

	Cr.validBits = Crvalid;
	Cr.width = Crwidth;
	Cr.height = Crheight;

	Cb.validBits = Cbvalid;
	Cb.width = Cbwidth;
	Cb.height = Cbheight;


	file.close();

}

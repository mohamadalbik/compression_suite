#include "FileFormating.h"

void writeImg(std::string& filename, const PackedBitStream& Y, const PackedBitStream& Cr, const PackedBitStream& Cb) {

	std::ofstream file(filename, std::ios::binary);

	if (!file) {
		std::cout << "Error creating the file\n";
		return;
	}

	file.write("BIKGP", 5);

	int width = Y.width;
	int height = Y.height;

	file.write((char*)&width, sizeof(int));
	file.write((char*)&height, sizeof(int));

	writeFrequencyTable(file, Y.frequencyTable);
	writeFrequencyTable(file, Cr.frequencyTable);
	writeFrequencyTable(file, Cb.frequencyTable);

	int Ysize = (Y.validBits + 7) / 8;
	int Crsize = (Cr.validBits + 7) / 8;
	int Cbsize = (Cb.validBits + 7) / 8;

	file.write((char*)&Ysize, sizeof(int));
	file.write((char*)&Crsize, sizeof(int));
	file.write((char*)&Cbsize, sizeof(int));

	int Yvalid = Y.validBits, Crvalid = Cr.validBits, Cbvalid = Cb.validBits;
	int Ywidth = Y.width, Crwidth = Cr.width, Cbwidth = Cb.width;
	int Yheight = Y.height, Crheight = Cr.height, Cbheight = Cb.height;

	file.write((char*)&Yvalid, sizeof(int));
	file.write((char*)&Ywidth, sizeof(int));
	file.write((char*)&Yheight, sizeof(int));

	file.write((char*)&Crvalid, sizeof(int));
	file.write((char*)&Crwidth, sizeof(int));
	file.write((char*)&Crheight, sizeof(int));

	file.write((char*)&Cbvalid, sizeof(int));
	file.write((char*)&Cbwidth, sizeof(int));
	file.write((char*)&Cbheight, sizeof(int));

	file.write((char*)Y.data.data(), Ysize);
	file.write((char*)Cr.data.data(), Crsize);
	file.write((char*)Cb.data.data(), Cbsize);

	file.close();

	std::cout << "File written successfully\n";



}

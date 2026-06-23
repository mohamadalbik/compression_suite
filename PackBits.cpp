#include "BitPacking.h"
#include<iostream>
PackedBitStream PackBits(const std::string& bitstream) {

	PackedBitStream packed;
	uint8_t current_byte = 0b00000000;
	int bitcount = 0;

	for (char b : bitstream) {

		current_byte <<= 1;

		if (b == '1') current_byte |= 1;
		
		bitcount++;

		if (bitcount == 8) {
		
			packed.data.push_back(current_byte);
			current_byte = 0b00000000;
			bitcount = 0;
		}

	}

	if (bitcount > 0) {

		current_byte <<= (8 - bitcount);
		packed.data.push_back(current_byte);

	}

	packed.validBits = bitstream.size();
	return packed;

}

std::string bitUnpacking(const std::vector<uint8_t>& data, int validBits) {
	
	std::string bitstream;
	bitstream.reserve(validBits);

	int bitCounter = 0;

	for (uint8_t byte : data) {
	
		for (int i = 7; i >= 0; i--)
		{

			if (bitCounter >= validBits) return bitstream;

			char bit = ((byte >> i) & 1) ? '1' : '0';
			bitstream.push_back(bit);
			bitCounter++;
		}
	
	}
	return bitstream;
}


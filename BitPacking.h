#ifndef BitPacking_H
#define BitPacking_H
#include<vector>	
#include<iostream>
#include<map>
#include<stdint.h>
/* ===============================
   Bit Package Struct
   =============================== */

struct PackedBitStream
{

	std::vector<uint8_t> data;
	int validBits = 0;

	std::map<int, int> frequencyTable;

	int height = 0;
	int width = 0;

};

/* ===============================
   Bit Packing
   =============================== */


PackedBitStream PackBits(const std::string& bitstream);


/* ===============================
   Bit Unpacking
   =============================== */

std::string bitUnpacking(const std::vector<uint8_t>& data, int validBits);

#endif // !BitPacking_H

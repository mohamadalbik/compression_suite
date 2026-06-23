#ifndef FILE_FORMATING
#define FILE_FORMATING
#include "BitPacking.h"
#include<fstream>

/* ===============================
   Serialization Of the Frquency Table
   =============================== */

/* ===============================
   Write Frquency Table
   =============================== */

void writeFrequencyTable(std::ofstream& file, const std::map<int, int>& freqTable);

/* ===============================
   Read Frquency Table
   =============================== */


std::map<int, int> readFrequencyTable(std::ifstream& file);

/* ===============================
   File Formation
   =============================== */

/* ===============================
   File writing 
   =============================== */

void writeImg(std::string& filename, const PackedBitStream& Y, const PackedBitStream& Cr, const PackedBitStream& Cb);

/* ===============================
   Channel Reading 
   =============================== */

void readImg(const std::string& fileName, PackedBitStream& Y, PackedBitStream& Cr, PackedBitStream& Cb, int& width, int& height);

#endif // !1


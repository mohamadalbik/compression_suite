#ifndef RLE_H
#define RLE_H
#include<vector>

/* ===============================
   Run Length 
   =============================== */

struct RLES
{
	int value;
	int count;
};


/* ===============================
   Run Length Encode
   =============================== */

void rle(std::vector<int>& zigedBlock, std::vector<RLES>& RLEvec);

/* ===============================
   Run Length Decode
   =============================== */

std::vector<RLES> RLEdecode(std::vector<int>& symbols);

/* ===============================
   Run Length Block splitting
   =============================== */

std::vector<std::vector<RLES>> splitRLE(const std::vector<RLES>& rleStream);

#endif // !RLE_H

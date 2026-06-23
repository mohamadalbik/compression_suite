#ifndef Huffman_H
#define Huffman_H

#include "RLE.h"
#include<map>
#include<queue>
#include<vector>
#include<string>
/* ===============================
   Huffman Node Struct
   =============================== */

struct HuffmanNode
{

	int value;
	int freq;
	HuffmanNode* left;
	HuffmanNode* right;
	
	HuffmanNode(int v, int f)
		: value(v), freq(f), left(nullptr), right(nullptr) {}

	HuffmanNode(HuffmanNode* l, HuffmanNode* r)
		: value(-1), freq(l->freq + r->freq), left(l), right(r) {}

};

/* ===============================
   Custome Compare For Min-heap Priority Queue
   =============================== */

struct Compare
{

	bool operator()(HuffmanNode* a, HuffmanNode* b){
			return a->freq > b->freq;
		}

};

/* ===============================
   Frequency Table Builder
   =============================== */

std::map<int, int> BuildFrequencyTable(const std::vector<RLES>& rle);

/* ===============================
   Huffman Tree Builder
   =============================== */

HuffmanNode* BuildHuffmanTree(const std::map<int, int>& freq);

/* ===============================
   Code Generator
   =============================== */

void generateCodes(HuffmanNode* node, const std::string& code, std::map<int, std::string>& table);

/* ===============================
   Huffman Encode
   =============================== */

std::string huffmanEncode(const std::vector<RLES>& rle, const std::map<int, std::string>& table);

/* ===============================
   Huffman Decode
   =============================== */

std::vector<int> huffmanDecode(const std::string& bitstream, HuffmanNode* root);

#endif // !Huffman_H


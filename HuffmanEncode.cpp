#include<map>
#include<queue>
#include "RLE.h"
#include "Huffman.h"

std::map<int, int> BuildFrequencyTable(const std::vector<RLES>& rle) {

	std::map<int, int> freq;

	for (const auto& p : rle) {

		freq[p.value] += p.count;

	}

	return freq;

}

HuffmanNode* BuildHuffmanTree(const std::map<int, int>& freq) {

	std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;

	for (auto& p : freq) {

		pq.push(new HuffmanNode(p.first, p.second));

	}

	while (pq.size() > 1) {

		HuffmanNode* left = pq.top(); pq.pop();
		HuffmanNode* right = pq.top(); pq.pop();

		HuffmanNode* parent = new HuffmanNode(left, right);

		pq.push(parent);

	}

	return pq.top();

}

void generateCodes(HuffmanNode* node, const std::string& code, std::map<int, std::string>& table) {

	if (!node) return;

	if (!node->left && !node->right) {

		table[node->value] = code;
		return;

	}

	generateCodes(node->left, code + "0", table);
	generateCodes(node->right, code + "1", table);

}

std::string huffmanEncode(const std::vector<RLES>& rle, const std::map<int, std::string>& table) {

	std::string bitstream;

	for (const auto& p : rle)
	{

		for (int i = 0; i < p.count; i++)
		{

			bitstream += table.at(p.value);

		}
	}
	return bitstream;
}

std::vector<int> huffmanDecode(const std::string& bitstream, HuffmanNode* root) {

	std::vector<int> symbols;
	HuffmanNode* current = root;

	for (char bit : bitstream) {

		current = (bit == '0') ? current->left : current->right;

		if (!current->left && !current->right) {

			symbols.push_back(current->value);
			current = root;

		}

	}

	return symbols;

}




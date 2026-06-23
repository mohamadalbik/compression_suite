#include "ImageCodec.h"

Mat decodeChannel( PackedBitStream encodedChannel, const Mat Q) {

	
	std::string unpacked = bitUnpacking(encodedChannel.data, encodedChannel.validBits);

	HuffmanNode* root = BuildHuffmanTree(encodedChannel.frequencyTable);

	std::vector<int> symbols = huffmanDecode(unpacked, root);

	std::vector<RLES> rleFlat = RLEdecode(symbols);

	std::vector<std::vector<RLES>> rleBlocks = splitRLE(rleFlat);


	Mat reconstructed = Mat::zeros(encodedChannel.height, encodedChannel.width, CV_32F);

	int blockInd = 0;
	Mat block;
	for (int i = 0; i < encodedChannel.height; i+=8)
	{
		for (int j = 0; j < encodedChannel.width; j+=8)
		{

			
			block = InverseZigzag8x8(rleBlocks[blockInd++]);

			dequantization(block, Q);

			idct(block, block);

			reconstruct(block, i, j, reconstructed);


		}
	}

	reconstructed.convertTo(reconstructed, CV_8UC1);
	delete root;
	return reconstructed;

}

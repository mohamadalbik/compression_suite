#include"ImageCodec.h"

PackedBitStream encodeChannel(Mat channel, const Mat Q)
{
    
    std::vector<RLES> allRLES; 
        
    for (int row = 0; row < channel.rows; row += 8)
    {
        for (int col = 0; col < channel.cols; col += 8)
        {
            Mat block = channel(Rect(col, row, 8, 8)).clone();

            block.convertTo(block, CV_32F);

            dct(block, block);

            quantize(block, Q);

            std::vector<int> zigedBlock;
            zigzag8x8(block, zigedBlock);

            std::vector<RLES> rleBlock;
            rle(zigedBlock, rleBlock);

            allRLES.insert(allRLES.end(), rleBlock.begin(), rleBlock.end());

            
        }
    }

    std::map<int, int> freqTable = BuildFrequencyTable(allRLES);
    HuffmanNode* root = BuildHuffmanTree(freqTable);
    std::map<int, std::string> huffmanTable;
    generateCodes(root, "", huffmanTable);

    std::string bitstream = huffmanEncode(allRLES, huffmanTable);
    delete root;
    allRLES.clear();

    PackedBitStream packed = PackBits(bitstream);

    packed.frequencyTable = freqTable;

    packed.height = channel.rows;
    packed.width = channel.cols;

    return packed;
}
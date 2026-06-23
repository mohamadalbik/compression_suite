#include "RLE.h"

void rle(std::vector<int>& zigedBlock, std::vector<RLES>& RLEvec)
{

	RLEvec.clear();

	int current = zigedBlock[0];
	int count = 1;

	for (int i = 1; i < zigedBlock.size(); ++i)
	{
		if (zigedBlock[i] == current) { count++; }
		else {
			RLEvec.push_back({ current, count });
			current = zigedBlock[i];
			count = 1;
		}
	}
	RLEvec.push_back({ current, count });
}

std::vector<RLES> RLEdecode(std::vector<int>& symbols){

	std::vector<RLES> rle;
	if (symbols.empty()) return rle;

	int current = symbols[0];
	int count = 1;

	for (size_t i = 1; i < symbols.size(); i++)
	{

		if (symbols[i] == current) count++;
		else {
		
			rle.push_back({ current, count });
			current = symbols[i];
			count = 1;
		
		}

	}

	rle.push_back({ current, count });

	return rle;

}

std::vector<std::vector<RLES>> splitRLE(const std::vector<RLES>& rleStream)
{
    std::vector<std::vector<RLES>> blocks;
    std::vector<RLES> currentBlock;
    int coeffCount = 0;

    for (const auto& sym : rleStream) {
        int count = sym.count;
        int value = sym.value;

        while (count > 0) {
            int available = 64 - coeffCount;
            int take = std::min(count, available);

            currentBlock.push_back({ value, take });
            coeffCount += take;
            count -= take;

            if (coeffCount == 64) {
      
                blocks.push_back(currentBlock);
                currentBlock.clear();
                coeffCount = 0;
            }
        }
    }

    if (coeffCount > 0) {
        
        if (coeffCount < 64) {
            currentBlock.push_back({ 0, 64 - coeffCount });
        }
        blocks.push_back(currentBlock);
    }

    return blocks;
}
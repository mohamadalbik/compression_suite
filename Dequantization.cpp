
#include "JPEGModules.h"

using namespace cv;

void dequantization(Mat& block, const Mat Q) {

    
        CV_Assert(block.rows == 8 && block.cols == 8);
        CV_Assert(Q.rows == 8 && Q.cols == 8);
        //CV_Assert(block.type() == CV_32F);  
        //CV_Assert(Q.type() == CV_32F);

        for (int i = 0; i < 8; i++)
        {

            for (int j = 0; j < 8; j++)
            {

                block.at<float>(i, j) *= Q.at<float>(i, j);
            
            }
        
        }
    
}

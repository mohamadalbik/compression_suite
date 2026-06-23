#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include "JPEGModules.h"
#include "RLE.h"
#include "Huffman.h"
#include "BitPacking.h"
#include "ImageCodec.h"
#include "FileFormating.h"
#include"VideoCodec.h"
using namespace cv;

Mat Q_Residual = (Mat_<float>(8, 8) <<
    32, 22, 20, 32, 48, 80, 102, 122,
    24, 24, 28, 38, 52, 116, 120, 110,
    28, 26, 32, 48, 80, 114, 138, 112,
    28, 34, 44, 58, 102, 174, 160, 124,
    36, 44, 74, 112, 136, 218, 206, 154,
    48, 70, 110, 128, 162, 208, 226, 184,
    98, 128, 156, 174, 206, 242, 240, 202,
    144, 184, 190, 196, 224, 200, 206, 198
    );


int main(int argc, std::string argv[]) {

    //JPEG Testing
    {
        /*const Mat img = imread("ChatGPT Image Aug 7, 2025, 08_05_07 PM.png");

        if (img.empty()) {
            std::cout << "Image not found!" << std::endl;
            return -1;
        }*/

        /*Mat Y(img.rows, img.cols, CV_8UC1);
        Mat Cb(img.rows, img.cols, CV_8UC1);
        Mat Cr(img.rows, img.cols, CV_8UC1);
        BGRtoYCbCr(img, Y, Cr, Cb);
        Mat ycbcr, re;
        imshow("original", img);
        cvtColor(img, ycbcr, COLOR_BGR2YCrCb);
        imshow("ycbr", ycbcr);
        std::vector<Mat> channels;
        split(ycbcr, channels);
        Y = channels[0];
        Cr = channels[1];
        Cb = channels[2];
        Mat cr_ds, cb_ds;
        DownSampling420(Cb, Cr, cb_ds, cr_ds);
        PackedBitStream packed_y = encodeChannel(Y, Q_Y);
        PackedBitStream packed_Cr = encodeChannel(cr_ds, Q_C);
        PackedBitStream packed_Cb = encodeChannel(cb_ds, Q_C);
        Mat y_decoded = decodeChannel(packed_y, Q_Y);
        Mat cr_decoded = decodeChannel(packed_Cr, Q_C);
        Mat cb_decoded = decodeChannel(packed_Cb, Q_C);
        imshow("Y Channel", y_decoded);
        resize(cb_decoded, Cb, Size(Y.cols, Y.rows), 0, 0, INTER_LINEAR);
        resize(cr_decoded, Cr, Size(Y.cols, Y.rows), 0, 0, INTER_LINEAR);
        imshow("Cb Channel", Cb);
        imshow("Cr Channel", Cr);
        std::vector<Mat> channels;
        Mat re;
        channels = { y_decoded, Cr, Cb };
        merge(channels, re);
        cvtColor(re, re, COLOR_YCrCb2BGR);
        imshow("img", re);
        waitKey(0);
        */

        /*Mat Y(img.rows, img.cols, CV_8UC1);
        Mat Cr(img.rows, img.cols, CV_8UC1);
        Mat Cb(img.rows, img.cols, CV_8UC1);

        BGRtoYCbCr(img, Y, Cr, Cb);

        Mat Cr_ds, Cb_ds;

        DownSampling420(Cb, Cr, Cr_ds, Cb_ds);*/


        //imshow("Y Channel", Y);
        //imshow("Cb Channel", Cb_ds);
        //imshow("Cr Channel", Cr_ds);


        /*std::vector<Mat> Y_blocks;

        _8x8Block(Y, Y_blocks);

        std::cout << "number of blocks" << Y_blocks.size() << std::endl;

        imshow("first block", Y_blocks[0]);

        std::cout << "before " << std::endl << Y_blocks[0] << std::endl;

        std::vector<RLES> RLESresult;
        for (auto& block : Y_blocks) {
            block.convertTo(block, CV_32F);
            dct(block, block);
            quantize(block, Q_Y);
            std::vector<int> zigedBlock;
            zigzag8x8(block, zigedBlock);
            std::vector<RLES> rle1;
            rle(zigedBlock, rle1);
            RLESresult.insert(RLESresult.end(), rle1.begin(), rle1.end());
          }

          auto freqTable = BuildFrequencyTable(RLESresult);

          HuffmanNode* root = BuildHuffmanTree(freqTable);

          std::map<int, std::string> huffmanTable;
          generateCodes(root, "", huffmanTable);

          std::string bitstream = huffmanEncode(RLESresult, huffmanTable);

          std::cout << "Total RLE symbols: " << RLESresult.size() << std::endl;
          std::cout << "Huffman symbols: " << huffmanTable.size() << std::endl;
          std::cout << "Bitstream length (bits): " << bitstream.size() << std::endl;

          std::cout << "Code for zero: " << huffmanTable[0] << std::endl;

          auto packed = PackBits(bitstream);
          auto unpacked = bitUnpacking(packed.data, packed.validBits);
          std::cout << "Packed bytes: " << packed.data.size() << std::endl;
          std::cout << "Valid bits: " << packed.validBits << std::endl;
          CV_Assert(bitstream == unpacked);
          std::cout << "unpacking okay" << std::endl;
          std::vector<int> symbols = huffmanDecode(unpacked, root);
          std::vector<RLES> RLEdecoded = RLEdecode(symbols);
          std::vector<std::vector<RLES>> blocks = splitRLE(RLEdecoded);
          Mat coeffBlock;
          Mat reconstructed = Mat::zeros(img.rows, img.cols, CV_32F);
          int blockIndex = 0;
          for (int row = 0; row < img.rows; row += 8) {
              for (int col = 0; col < img.cols; col += 8) {
                  coeffBlock = InverseZigzag8x8(blocks[blockIndex++]);
                  dequantization(coeffBlock, Q_Y);
                  idct(coeffBlock, coeffBlock);
                  reconstruct(coeffBlock, row, col, reconstructed);
              }
          }
          reconstructed.convertTo(reconstructed, CV_8UC1);
          imshow("reconstructed", reconstructed);
          imshow("original", Y);*/


          //imshow("reconstructed_Cr", reconstructed_Cr);
          //imshow("original_Cr", Cr);

      //auto start = std::chrono::high_resolution_clock::now();
      /*PackedBitStream packed_y = encodeChannel(Y, Q_Y);
      PackedBitStream packed_cr = encodeChannel(Cr_ds, Q_C);
      PackedBitStream packed_cb = encodeChannel(Cb_ds, Q_C);
      PackedBitStream packed_y, packed_cr, packed_cb;*/
      /*EncodedImg encodedimg =  encodeImage(img);
      auto end = std::chrono::high_resolution_clock::now();
      double encodeTime = std::chrono::duration<double>(end - start).count();
      std::cout << "encode time : " << encodeTime << "sec\n";*/

      //std::string filename = "firstImg.bikgp";
      //writeImg(filename, packed_y, packed_cr, packed_cb);

      //int height = 0, width = 0;

      //PackedBitStream packed_y_1;
      //PackedBitStream packed_cr_1;
      //PackedBitStream packed_cb_1;

      //readImg(filename, packed_y_1, packed_cr_1, packed_cb_1, width, height); 


      //start = std::chrono::high_resolution_clock::now();
      /*Mat Y_decoded = decodeChannel(packed_y, Q_Y);
      Mat cr_decoded = decodeChannel(packed_cr, Q_C);
      Mat cb_decoded = decodeChannel(packed_cb, Q_C);*/

      //Mat Y_decoded_1 = decodeChannel(packed_y_1, Q_Y);
      //Mat cr_decoded_1 = decodeChannel(packed_cr_1, Q_C);
      //Mat cb_decoded_1 = decodeChannel(packed_cb_1, Q_C);

      /*Mat re = decodeImage(encodedimg);

      end = std::chrono::high_resolution_clock::now();
      double decodeTime = std::chrono::duration<double>(end - start).count();
      std::cout << "decode time : " << decodeTime << "sec\n";
      */
      //Mat re_1 = decodeImage(packed_y_1, packed_cb_1, packed_cr_1, Q_Y, Q_C);

      //EncodedImg encodedImg = encodeImage(img);

      //Mat re = decodeImage(encodedImg);

      //Mat re = compressChannel_3(Y, Q_Y); 

      //Mat re = compressChannel_3(Y, Q_Y);

      //imshow("re", re);
      //imshow("re_1", re_1);
      //imwrite("re.jpeg", re);
      //imshow("re_cr", cr_decoded);
      //imshow("re_cb", cb_decoded);

      //waitKey(0);
    }
    


    /*std::string videoPath = argv[1];
    std::string output = argv[2];
    */
    int fps = 0;

    /*std::vector<cv::Mat> frames;

    extractFrame(videoPath, frames, fps);

    std::cout << "Frames: " << frames.size() << std::endl;

    
    if (!frames.empty()) {
        cv::imshow("First Frame", frames[0]);
        cv::waitKey(0);
    }*/

    std::vector<frameData> encodedframes;
    auto start = std::chrono::high_resolution_clock::now();
    encodeVideo("video_7.y4m", encodedframes, fps);
    //encodeVideoToFile(videoPath,"output_6.myvid");
    auto end = std::chrono::high_resolution_clock::now();
    double encodeTime = std::chrono::duration<double>(end - start).count();
    std::cout << "Encoding time : " << encodeTime << std::endl;
    
    decodeVideo("output_7.mp4", encodedframes, fps);
    //decodeVideoFromFile("output_6.myvid","output_6.mp4");
        

    return 0;       
}

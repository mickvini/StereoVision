#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

// Добавляем параметры для сжатия
void EncodeWithFFmpeg(const cv::Mat& frame, int compressionQuality = 75); // По умолчанию качество 75 для JPEG

void DecodeAndDisplayFrame(const std::string& filename);

void CompressAndSaveImage(const cv::Mat& frame, const std::string& outputFile, int quality = 75);
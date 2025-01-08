#pragma once
#include <iostream>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <opencv2/opencv.hpp>

class StereoCamera {
public:
	// Тип для обработчика кадров
	using FrameCallback = std::function<void(const cv::Mat& leftFrame)>;//, const cv::Mat& rightFrame)>;

	// Конструктор
	StereoCamera(int width, int height, int fps, int deviceIndex = 0)
		: width_(width), height_(height), fps_(fps), deviceIndex_(deviceIndex), isRunning_(false), isPaused_(false) {}

	// Деструктор
	~StereoCamera() {
		Stop();
	}

	// Метод для установки пользовательского обработчика кадров
	void SetFrameCallback(FrameCallback callback);

	// Запуск видеопотока
	void Start();

	// Остановка видеопотока
	void Stop();

	// Пауза видеопотока
	void Pause();
	bool IsPaused() const;

private:
	int width_;
	int height_;
	int fps_;
	int deviceIndex_;
	std::atomic<bool> isRunning_;
	std::atomic<bool> isPaused_;
	std::thread captureThread_;
	FrameCallback frameCallback_;

	// Основной цикл захвата кадров
	void CaptureLoop();
};
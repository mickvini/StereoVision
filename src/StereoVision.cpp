#include "StereoVision.h"

// Метод для установки пользовательского обработчика кадров
void StereoCamera::SetFrameCallback(FrameCallback callback) {	
	frameCallback_ = callback;	
}
// Запуск видеопотока
void StereoCamera::Start() {
	if(isPaused_)
	{
		isPaused_ = false;
		return;
	}
	if (isRunning_) return;
	isRunning_ = true;
	captureThread_ = std::thread(&StereoCamera::CaptureLoop, this);
}

// Остановка видеопотока
void StereoCamera::Stop() {
	if (!isRunning_) return;
	isRunning_ = false;
	if (captureThread_.joinable()) {
		captureThread_.join();
	}
}

// Пауза видеопотока
void StereoCamera::Pause() {
	isPaused_ = true;
}
bool StereoCamera::IsPaused() const {
	return isPaused_;
}
// Основной цикл захвата кадров
void StereoCamera::CaptureLoop() {
	cv::VideoCapture leftCamera(0);
	

	// Настройка параметров камеры
	leftCamera.set(cv::CAP_PROP_FRAME_WIDTH, width_);
	leftCamera.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
	leftCamera.set(cv::CAP_PROP_FPS, fps_);

	if (!leftCamera.isOpened()) {
		std::cerr << "Не удалось открыть устройство с индексом: " << deviceIndex_ << std::endl;
		return;
	}

	while (isRunning_) {
		if (!isPaused_)
		{
			cv::Mat leftFrame;
			leftCamera >> leftFrame;

			if (leftFrame.empty()) {
				std::cerr << "Ошибка чтения кадров!" << std::endl;
				continue;
			}

			// Вызываем пользовательский обработчик кадра
			if (frameCallback_) {
				frameCallback_(leftFrame);
			}

			
		}
		else
		{
			system("cls");
			std::cerr << "Стоим на паузе";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps_));
		}
	}

	leftCamera.release();
}
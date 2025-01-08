#include "StereoVision.h"

// ����� ��� ��������� ����������������� ����������� ������
void StereoCamera::SetFrameCallback(FrameCallback callback) {	
	frameCallback_ = callback;	
}
// ������ �����������
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

// ��������� �����������
void StereoCamera::Stop() {
	if (!isRunning_) return;
	isRunning_ = false;
	if (captureThread_.joinable()) {
		captureThread_.join();
	}
}

// ����� �����������
void StereoCamera::Pause() {
	isPaused_ = true;
}
bool StereoCamera::IsPaused() const {
	return isPaused_;
}
// �������� ���� ������� ������
void StereoCamera::CaptureLoop() {
	cv::VideoCapture leftCamera(0);
	

	// ��������� ���������� ������
	leftCamera.set(cv::CAP_PROP_FRAME_WIDTH, width_);
	leftCamera.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
	leftCamera.set(cv::CAP_PROP_FPS, fps_);

	if (!leftCamera.isOpened()) {
		std::cerr << "�� ������� ������� ���������� � ��������: " << deviceIndex_ << std::endl;
		return;
	}

	while (isRunning_) {
		if (!isPaused_)
		{
			cv::Mat leftFrame;
			leftCamera >> leftFrame;

			if (leftFrame.empty()) {
				std::cerr << "������ ������ ������!" << std::endl;
				continue;
			}

			// �������� ���������������� ���������� �����
			if (frameCallback_) {
				frameCallback_(leftFrame);
			}

			
		}
		else
		{
			system("cls");
			std::cerr << "����� �� �����";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps_));
		}
	}

	leftCamera.release();
}
#pragma once
#include "StereoVision.h"
#include "Encode.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <process.h>
#include <conio.h>
#include <cstdlib>

// ����� ��� ������������ ��������� ���������
std::vector<int> ScanAvailableDevices() {
    std::vector<int> availableDevices;
    for (int i = 0; i < 1; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            availableDevices.push_back(i);
            cap.release();
        }
    }
    return availableDevices;
}

// ����� ��� ������ ������ ��������� ���������
void PrintAvailableDevices(const std::vector<int>& availableDevices) {
    std::cout << "��������� ����������:" << std::endl;
    for (int i = 0; i < availableDevices.size(); ++i) {
        std::cout << "[" << i << "] ���������� � �������� " << availableDevices[i] << std::endl;
    }
}

// ����� ��� ������ ���������� �������������
int SelectDevice(const std::vector<int>& availableDevices) {
    int selectedDeviceIndex;
    while (true) {
        std::cout << "�������� ���������� (������� �����): ";
        std::cin >> selectedDeviceIndex;

        // ���������, ��������� �� ����
        if (std::cin.fail() || selectedDeviceIndex < 0 || selectedDeviceIndex >= availableDevices.size()) {
            std::cin.clear(); // ���������� ���� ������
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ������� ����� �����
            std::cerr << "�������� ����� ����������! ����������, ������� ����� �� 0 �� "
                << availableDevices.size() - 1 << "." << std::endl;
        }
        else {
            break; // ���� ���������, ������� �� �����
        }
    }
    return selectedDeviceIndex;
}

// ����� ��� ��������� ������
void FrameHandler(const cv::Mat& leftFrame) {
    std::cout << "������� ����� ����!" << std::endl;
    cv::imshow("Left Frame", leftFrame);
    /*CompressAndSaveImage(leftFrame, "C:\\Users\\romanyuk_ei\\Pictures\\StereoVision\\output.jpg");
    DecodeAndDisplayFrame("C:\\Users\\romanyuk_ei\\Pictures\\StereoVision\\output.jpg");*/
    cv::waitKey(1);
}

int main() {
    putenv("OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS=0");
    std::setlocale(LC_ALL, "Russian");   
    // ��������� ��������� ����������
    std::vector<int> availableDevices = ScanAvailableDevices();

    if (availableDevices.empty()) {
        std::cerr << "��� ��������� ���������!" << std::endl;
        return -1;
    }

    PrintAvailableDevices(availableDevices);

    int selectedDeviceIndex = SelectDevice(availableDevices);

    // ������� ������ ������ � ��������� �����������
    StereoCamera camera(1280, 960, 30, selectedDeviceIndex);

    // ������������� ���������������� ���������� ������
    camera.SetFrameCallback(FrameHandler);

    // ������ �����������
    camera.Start();

    
    while (true) {
        if (_kbhit()) {
            char key = _getch(); 
            if (key == 32) { // 32 - ��� �������
                if (camera.IsPaused()) {
                    camera.Start();
                    std::cout << "���������� �����������." << std::endl;
                }
                else {
                    camera.Pause();
                    std::cout << "���������� �� �����." << std::endl;
                }
            }
            else if (key == 27) { // 27 - ��� ������� ESC
                break; 
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // ��������� �����������
    camera.Stop();

    return 0;
}
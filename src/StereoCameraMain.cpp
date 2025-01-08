#pragma once
#include "StereoVision.h"
#include "Encode.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <process.h>
#include <conio.h>
#include <cstdlib>

// Метод для сканирования доступных устройств
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

// Метод для вывода списка доступных устройств
void PrintAvailableDevices(const std::vector<int>& availableDevices) {
    std::cout << "Доступные устройства:" << std::endl;
    for (int i = 0; i < availableDevices.size(); ++i) {
        std::cout << "[" << i << "] Устройство с индексом " << availableDevices[i] << std::endl;
    }
}

// Метод для выбора устройства пользователем
int SelectDevice(const std::vector<int>& availableDevices) {
    int selectedDeviceIndex;
    while (true) {
        std::cout << "Выберите устройство (введите номер): ";
        std::cin >> selectedDeviceIndex;

        // Проверяем, корректен ли ввод
        if (std::cin.fail() || selectedDeviceIndex < 0 || selectedDeviceIndex >= availableDevices.size()) {
            std::cin.clear(); // Сбрасываем флаг ошибки
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Очищаем буфер ввода
            std::cerr << "Неверный выбор устройства! Пожалуйста, введите число от 0 до "
                << availableDevices.size() - 1 << "." << std::endl;
        }
        else {
            break; // Ввод корректен, выходим из цикла
        }
    }
    return selectedDeviceIndex;
}

// Метод для обработки кадров
void FrameHandler(const cv::Mat& leftFrame) {
    std::cout << "Получен новый кадр!" << std::endl;
    cv::imshow("Left Frame", leftFrame);
    /*CompressAndSaveImage(leftFrame, "C:\\Users\\romanyuk_ei\\Pictures\\StereoVision\\output.jpg");
    DecodeAndDisplayFrame("C:\\Users\\romanyuk_ei\\Pictures\\StereoVision\\output.jpg");*/
    cv::waitKey(1);
}

int main() {
    putenv("OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS=0");
    std::setlocale(LC_ALL, "Russian");   
    // Сканируем доступные устройства
    std::vector<int> availableDevices = ScanAvailableDevices();

    if (availableDevices.empty()) {
        std::cerr << "Нет доступных устройств!" << std::endl;
        return -1;
    }

    PrintAvailableDevices(availableDevices);

    int selectedDeviceIndex = SelectDevice(availableDevices);

    // Создаем объект камеры с выбранным устройством
    StereoCamera camera(1280, 960, 30, selectedDeviceIndex);

    // Устанавливаем пользовательский обработчик кадров
    camera.SetFrameCallback(FrameHandler);

    // Запуск видеопотока
    camera.Start();

    
    while (true) {
        if (_kbhit()) {
            char key = _getch(); 
            if (key == 32) { // 32 - код пробела
                if (camera.IsPaused()) {
                    camera.Start();
                    std::cout << "Видеопоток возобновлен." << std::endl;
                }
                else {
                    camera.Pause();
                    std::cout << "Видеопоток на паузе." << std::endl;
                }
            }
            else if (key == 27) { // 27 - код клавиши ESC
                break; 
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Остановка видеопотока
    camera.Stop();

    return 0;
}
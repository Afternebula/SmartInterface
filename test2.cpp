#include <opencv2/opencv.hpp>
#include <windows.h>
#include <iostream>

int main() {

    cv::VideoCapture stream(0);

    if (!stream.isOpened())
    {
        stream.open(1);

        if (!stream.isOpened())
        {
            std::cerr<<"Не удалось отркрыть камеру"<<std::endl;
            return -1;
        }
    }

    cv::Mat frame, hsv, mask;
    int lastX = -1; //Координата предыдущего кадра
    int gestureCooldown = 0;
    std::cout << ">>> Presentasion clicker running <<<" << std::endl;

    while (true)
    {
        stream >> frame; // Захват текущего кадра
        if (frame.empty()) break;
        cv::flip(frame, frame, 1); // Отзеркаливаение кадра по горизонтали
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV); //Конвертация палитры BGR в цветовое пространство HSV
        cv::inRange(hsv, cv::Scalar(90, 50, 50), cv::Scalar(130, 255, 255), mask); // Выделение синего цвета маркера
        
        // Пиксельный анализ матрицы
        long long sumX = 0; //Сумма координат
        long long whitePixels = 0; //Счетчик площади
        //Перебор маски
        for (int y = 0; y < mask.rows; y++) 
        {
            uchar* row_ptr = mask.ptr<uchar>(y); // Прямой адрес начала строки
            //Сдвиг указателя по массиву байт как row_ptr[x]
            for (int x = 0; x < mask.cols; x++)
            {
                if (row_ptr[x] == 255)
                {
                    // Фиксация белого пикселя маски
                    //Прибавляем координату x к общей сумме координат sumX
                    sumX += x;
                    whitePixels++;
                }
            }
        }
        
        // Логика конечного автомата жестов
        if (whitePixels > 500)
        {
            int cx = sumX / whitePixels; // Вычисление геометрического центра маркера
            if (lastX != -1 && gestureCooldown == 0)
            {
                int speed = cx - lastX; // Вектор мгновенной скорости
                if (speed > 40) 
                {
                    std::cout << ">>> NEXT SLIDE <<<" << std::endl;
                    keybd_event(VK_RIGHT, 0, 0, 0); //Имитация нажатия стрелки вправо
                    keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0); //Имитация отпущенной кнопки с помощью
                    gestureCooldown = 20; // Заморозка триггера на 20 кадров 
                }
                else if (speed < -40)
                {
                    std::cout << "<<< PREVIOUS SLIDE <<<" << std::endl;
                    keybd_event(VK_LEFT, 0, 0, 0);

                    keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
                    gestureCooldown = 20;
                }
            }
            lastX = cx;
        }

        // Обновление таймера блокировки ложных срабатываний
        if (gestureCooldown > 0) gestureCooldown--;
        // Ограничение частоты кадров (30 FPS)
        Sleep(33);
    }
    
    stream.release();
    return 0;
}
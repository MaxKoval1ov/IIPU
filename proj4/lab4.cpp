
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Setupapi.lib")

#include <stdio.h>
#include <Windows.h>
#include <setupapi.h>
#include <locale.h>
#include <string.h> 
#include <iostream>
#include <wdmguid.h>
#include <devguid.h>
#include <iomanip>

#include <map>
#include <thread>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void Da_Phota() {
    VideoCapture camera(0);
    if (camera.isOpened()) {
        Mat frame;
        camera >> frame;
        imwrite("hDa_Pic.jpg", frame);
    }
}

void Da_Videa() {
    static bool tracking = false;
    static VideoCapture camera;
    static VideoWriter video;

    if (!tracking) {
        tracking = true;
        thread scanner([]() {
            camera.open(0);
            if (!camera.isOpened()) {
                return;
            }

            double frame_width = camera.get(CAP_PROP_FRAME_WIDTH);
            double frame_height = camera.get(CAP_PROP_FRAME_HEIGHT);

            video.open("Da_Videa.avi",
                VideoWriter::fourcc('M', 'P', '4', '2'), // mpeg2 avi
                20,
                Size(frame_width, frame_height), true);
            if (!video.isOpened()) {
                return;
            }

            while (true) {
                Mat frame;
                if (!camera.read(frame)) {
                    break;
                }
                if (frame.empty())
                    break;
                video << frame;
            }
            });
        scanner.detach();
    }
    else {
        camera.release();
        video.release();
        tracking = false;
    }
}

HHOOK hKeyboardHook;

__declspec(dllexport)
LRESULT
CALLBACK
KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam) {
    static bool state = 0;
    if ((nCode == HC_ACTION) &&
        ((wParam == WM_SYSKEYDOWN) ||
            (wParam == WM_KEYDOWN))) {
        KBDLLHOOKSTRUCT key = *reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (key.vkCode == 'P')
        {
            Da_Phota();
            cout << "Phota" << endl;
        }
        if (key.vkCode == 'O')
        {
            Da_Videa();
            cout << "Videa_toggle" << endl;
        }
        if (key.vkCode == 'H')
        {
            state ? ShowWindow(::GetConsoleWindow(), SW_HIDE) : ShowWindow(::GetConsoleWindow(), SW_SHOW);
            state = !state;

        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam); // передаем управление дальше
}

void MessageLoop() {
    MSG message; // получаем символ кнопки из потока сообщений
    while (GetMessage(&message, nullptr, 0, 0)) {
        TranslateMessage(&message); //извлекаем его
        DispatchMessage(&message); // и доставляем
    }
}

int main() {
    ShowWindow(::GetConsoleWindow(), SW_HIDE);
    setlocale(LC_ALL, "rus");
    HDEVINFO hDeviceInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA, NULL, NULL, DIGCF_PRESENT);
    if (hDeviceInfo == INVALID_HANDLE_VALUE)
        return 1;
    SP_DEVINFO_DATA spDeviceInfoData = { 0 };
    ZeroMemory(&spDeviceInfoData, sizeof(SP_DEVINFO_DATA));
    spDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    SetupDiEnumDeviceInfo(hDeviceInfo, 0, &spDeviceInfoData);
    PBYTE deviceName[256];
    PBYTE deviceMan[256];
    PBYTE deviceID[256];
    SetupDiGetDeviceRegistryProperty(hDeviceInfo, &spDeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)deviceName, sizeof(deviceName), NULL);
    SetupDiGetDeviceRegistryProperty(hDeviceInfo, &spDeviceInfoData, SPDRP_MFG, NULL, (PBYTE)deviceMan, sizeof(deviceMan), NULL);
    SetupDiGetDeviceRegistryProperty(hDeviceInfo, &spDeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)deviceID, sizeof(deviceID), NULL);

    cout << "NAME: " << deviceName << endl;
    cout << "MANUFACTURER: " << deviceMan << endl;
    cout << "CAMERA HARDWARE ID: " << deviceID << endl << endl;
    cout << "Hi there" << endl;
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    hKeyboardHook = SetWindowsHookEx( // хукер
        WH_KEYBOARD_LL,
        (HOOKPROC)KeyboardEvent,
        hInstance,
        NULL);
    MessageLoop(); // получаем символы
    UnhookWindowsHookEx(hKeyboardHook); // отпускаем клавиатуру
    SetupDiDestroyDeviceInfoList(hDeviceInfo);
    return 1;
}

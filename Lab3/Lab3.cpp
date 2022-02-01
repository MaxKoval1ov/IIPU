#include<iostream>
#include<Windows.h>
#include<winbase.h>
#include<powrprof.h>
#pragma comment(lib, "PowrProf.lib")
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    //AC power
    SYSTEM_POWER_STATUS status;
    while (1) {
        if (!GetSystemPowerStatus(&status)) {
            cout << GetLastError() << endl;
        }
        else {

            cout << "Power mode:";
            int powerMode = status.ACLineStatus;
            if (powerMode == 0) {
                cout << "battery powered" << endl;
            }
            else if (powerMode == 1) {
                cout << "AC power" << endl;
            }
            else {
                cout << "Unknown status" << endl;
            }
            cout << "Battery level:";
            int life = status.BatteryLifePercent;
            cout << life << "%" << endl;
            cout << "Current power saving mode:";
            int energyStatus = status.BatteryFlag;
            if (energyStatus == 0) {
                cout << "Power saving mode is disabled." << endl;
            }
            else if (energyStatus == 1) {
                cout << "Power saving mode is enabled." << endl;
            }
            if (!powerMode) {
                cout << "Battery life:";
                unsigned long int batteryLifeSeconds = status.BatteryLifeTime;
                if (batteryLifeSeconds) {
                    cout << "The battery will work - " << batteryLifeSeconds / 3600 <<
                        "h:" << batteryLifeSeconds / 60 % 60 << "min:" << batteryLifeSeconds % 3600 % 60 <<
                        "sec" << endl;
                }
                else {
                    cout << "The number of seconds is unknown || the device is connected to a power source" << endl;
                }
            }

        }
        cout << "Choose mode:\n1-SLEEP\n2-HIBERNATION\n3-EXIT\n";
        char a;
        a = getchar();
        if (a == '1') {
            SetSuspendState(TRUE, FALSE, FALSE);
        }
        else if (a == '2') {
            SetSuspendState(FALSE, FALSE, FALSE);
        }
        else if (a == '3') break;
        system("cls");
    }
    return 0;
}

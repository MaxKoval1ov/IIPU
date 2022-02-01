#include "pcidevice.h"

QString parseIds(const QString _id, const QString _key, const char _separator) {
    QString parsedId = "";
    for (short i = _id.indexOf(_key) + _key.size(); i < _id.size() && _id[i] != _separator; i++) {
        parsedId += _id[i];
    }
    return parsedId;
}

QList<PciDevice> readDevices() {
    QList<PciDevice> devices;
    QSettings hKey("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\PCI", QSettings::NativeFormat);
    QStringList devicesIds = hKey.childGroups();

    for (int i = 0; i < devicesIds.size(); i++) {
        PciDevice tmpDevice;
        QString venId = "VendorID - ", devId = "DeviceID - ", devName;

        QSettings chKey("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\" + devicesIds[i], QSettings::NativeFormat);
        QStringList chDir = chKey.childGroups();

        QSettings resKey("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\PCI\\" + devicesIds[i] + "\\" + chDir[0], QSettings::NativeFormat);

        devName = parseIds(resKey.value("DeviceDesc").toString(), ";", ';');
        venId += parseIds(devicesIds[i], "VEN_", '&');
        devId += parseIds(devicesIds[i], "DEV_", '&');

        tmpDevice.setName(devName);
        tmpDevice.setDeviceID(devId);
        tmpDevice.setVendorID(venId);

        devices.push_back(tmpDevice);
    }
    return devices;
}
















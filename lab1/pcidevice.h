#ifndef PCIDEVICE_H
#define PCIDEVICE_H
#include <QString>
#include <QSettings>
#include <QStringList>

class PciDevice {
private:
    QString deviceName;
    QString vendorID;
    QString deviceID;
public:
    inline QString getName() { return this->deviceName; }
    inline QString getDeviceID() { return this->deviceID; };
    inline QString getVendorID() { return this->vendorID; };

    void setDeviceID(const QString _deviceID) {
        this->deviceID = _deviceID;
    }
    void setVendorID(const QString _vendorID) {
        this->vendorID = _vendorID;
    }
    void setName(const QString _name) {
        this->deviceName = _name;
    }
};

QString parseIds(const QString, const QString, const char);
QList<PciDevice> readDevices();

#endif // PCIDEVICE_H

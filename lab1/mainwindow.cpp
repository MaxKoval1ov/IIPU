#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pcidevice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QListWidget listWidget;
    QList<PciDevice>devices;
    devices = readDevices();
    for (int i = devices.size() - 1; i >= 0; i--) {
        ui->devicesNamesList->insertItem(0, QString::number(i + 1) + ". " + devices[i].getName());
        ui->vendorIDList->insertItem(0, devices[i].getVendorID());
        ui->deviceIDList->insertItem(0, devices[i].getDeviceID());
    }
    ui->devicesNamesList->count();
    ui->devicesNamesList->show();

}

MainWindow::~MainWindow() {
    delete ui;
}

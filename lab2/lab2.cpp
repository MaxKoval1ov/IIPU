#include <Windows.h>
#include <Setupapi.h>
#include <ntddscsi.h>
#include <string.h>
#pragma comment(lib, "setupapi.lib")

#include <iostream>
#include <string>
#include <sstream>

#define BUFFER_SIZE 1024

using namespace std;


std::string busType[] = { "TypeUnknown","Scsi","Atapi","Ata","1394","Ssa",
							"Fibre","Usb","RAID","Scsi","Sas","Sata","Sd",
							"Mmc","Virtual","FileBackedVirtual","Spaces",
							"Nvme",	"SCM","Ufs","Max",	"Reserved" };

void getMemoryInfo(DWORD drive_number, HANDLE& drive_handle, STORAGE_PROPERTY_QUERY& storage_proterty_query) {
	string path;
	_ULARGE_INTEGER diskSpace;
	_ULARGE_INTEGER freeSpace;

	diskSpace.QuadPart = 0;
	freeSpace.QuadPart = 0;

	_ULARGE_INTEGER totalDiskSpace;
	_ULARGE_INTEGER totalFreeSpace;

	totalDiskSpace.QuadPart = 0;
	totalFreeSpace.QuadPart = 0;

	//A bit mask, representing all the logical drives (volumes)
	unsigned long int logicalDrivesCount = GetLogicalDrives();

	for (char volume_letter = 'A'; volume_letter < 'Z'; volume_letter++) {
		if ((logicalDrivesCount >> volume_letter - 65) & 1 && volume_letter) {
			path = volume_letter;
			path.append(":\\");

			GetDiskFreeSpaceExA(path.c_str(), 0, &diskSpace, &freeSpace);
			diskSpace.QuadPart /= pow(1024, 2);
			freeSpace.QuadPart /= pow(1024, 2);


			string open_path = "\\\\.\\";
			open_path += volume_letter;
			open_path += ":";

			HANDLE volume_handle = CreateFileA(
				open_path.c_str(),
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);
			if (volume_handle == INVALID_HANDLE_VALUE)
			{
				cout << GetLastError();
				CloseHandle(drive_handle);
				system("pause");
				exit(EXIT_FAILURE);
			}

			const DWORD out_buffer_size = 12;
			BYTE* out_buffer = new BYTE[out_buffer_size];
			ZeroMemory(out_buffer, out_buffer_size);
			DWORD bytes_returned = 0;

			if (!DeviceIoControl(volume_handle,
				IOCTL_STORAGE_GET_DEVICE_NUMBER,
				&storage_proterty_query,
				sizeof(STORAGE_PROPERTY_QUERY),
				out_buffer,
				out_buffer_size,
				&bytes_returned,
				NULL))
			{
				cout << GetLastError();
				CloseHandle(volume_handle);
				CloseHandle(drive_handle);
				system("pause");
				exit(EXIT_FAILURE);
			}

			_STORAGE_DEVICE_NUMBER* device_number = (_STORAGE_DEVICE_NUMBER*)out_buffer;

			if (device_number->DeviceNumber == drive_number) {
				if (diskSpace.QuadPart != 0)
				{
					totalDiskSpace.QuadPart += diskSpace.QuadPart;
					totalFreeSpace.QuadPart += freeSpace.QuadPart;

					//cout << "-------------------" << endl;
					cout << "Volume: " << volume_letter << endl;
					cout << "Total space[GB]" << diskSpace.QuadPart / 1024 << endl;
					cout << "Free space[GB]" << freeSpace.QuadPart / 1024 << endl;
					cout << "Busy space[GB]" << (diskSpace.QuadPart - freeSpace.QuadPart) / 1024 << endl;
				}
			}

			delete[] out_buffer;
		}
	}
	//cout << "-------------------";

	cout << endl;
	cout << "Overall Device's memory" << endl;
	cout << "Total space[GB]" << totalDiskSpace.QuadPart / 1024 << endl;
	cout << "Free space[GB]" << totalFreeSpace.QuadPart / 1024 << endl;
	cout << "Busy space[GB]" << (totalDiskSpace.QuadPart - totalFreeSpace.QuadPart) / 1024 << endl;
}

void getAtaPioDmaSupportStandarts(HANDLE diskHandle)
{
	DWORD bytes;

	UCHAR identifyDataBuffer[512 + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };

	ATA_PASS_THROUGH_EX& PTE = *(ATA_PASS_THROUGH_EX*)identifyDataBuffer;
	PTE.Length = sizeof(PTE);
	PTE.TimeOutValue = 10;
	PTE.DataTransferLength = 512;
	PTE.DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);
	PTE.AtaFlags = ATA_FLAGS_DATA_IN;

	if (!DeviceIoControl(diskHandle,
		IOCTL_ATA_PASS_THROUGH,
		&PTE,
		sizeof(identifyDataBuffer),
		&PTE,
		sizeof(identifyDataBuffer),
		NULL,
		NULL))
	{
		if (GetLastError() == 5)
		{
			std::cout << "Access denied!" << std::endl;
			return;
		}
	}


	WORD* data = (WORD*)(identifyDataBuffer + sizeof(ATA_PASS_THROUGH_EX));
	int bitArray[16];



	unsigned short dmaSupportedBytes = data[63];

	for (int i = 15; i >= 0; --i)
	{
		bitArray[i] = dmaSupportedBytes & (int)pow(2, 15) ? 1 : 0;
		dmaSupportedBytes = dmaSupportedBytes << 1;
	}


	std::cout << "DMA";
	for (int i = 0; i < 8; i++)
		if (bitArray[i])
			std::cout << "DMA" << i << ", ";
	std::cout << "\b\b" << std::endl;


	unsigned short pioSupportedBytes = data[64];

	for (int i = 15; i >= 0; --i)
	{
		bitArray[i] = pioSupportedBytes & (int)pow(2, 15) ? 1 : 0;
		pioSupportedBytes = pioSupportedBytes << 1;
	}


	std::cout << "PIO";
	for (int i = 0; i < 2; i++)
		if (bitArray[i])
			std::cout << "PIO" << i + 3 << ", ";
	std::cout << "\b\b" << std::endl;
}
HANDLE getDiskInfo(const char* name)
{
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	storagePropertyQuery.QueryType = PropertyStandardQuery;
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	HANDLE diskHandle = CreateFileA((std::string("\\\\.\\") + name).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (diskHandle == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = static_cast<STORAGE_DEVICE_DESCRIPTOR*>(calloc(1024, 1));
	if (!DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery), deviceDescriptor, 1024, NULL, 0)) {
		return NULL;
	}

	char* tmp = (char*)deviceDescriptor;

	const std::string model(tmp + deviceDescriptor->ProductIdOffset);
	std::cout << "Manufacturer: " << model.substr(0, model.find_first_of(' ')) << std::endl;
	std::cout << "Model: " << model.substr(model.find_first_of(' ') + 1) << std::endl;

	std::cout << "Serial Number: " << tmp + deviceDescriptor->SerialNumberOffset << std::endl;
	std::cout << "Firmware: " << tmp + deviceDescriptor->ProductRevisionOffset << std::endl;
	std::cout << "Bus type: " << busType[deviceDescriptor->BusType] << std::endl;
	getAtaPioDmaSupportStandarts(diskHandle);

	CloseHandle(diskHandle);
	return diskHandle;
}

int main() {
	setlocale(LC_ALL, "Russian");

	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;	//Flag for retreiving divece's descriptor 
	storagePropertyQuery.QueryType = PropertyStandardQuery;		//Querry to return divece's descriptor

	HANDLE drive_handl;
	short i = 0;
	std::ostringstream ss;
	ss << i;
	std::string tmp = "PhysicalDrive" + ss.str();
	drive_handl = getDiskInfo(tmp.c_str());

	getMemoryInfo(i, drive_handl, storagePropertyQuery);
	std::cout << std::endl;
	system("pause");
	return 0;
}

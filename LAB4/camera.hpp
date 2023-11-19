#include <Windows.h>
#include <conio.h>
#include <Setupapi.h>
#include <devguid.h>
#include <iostream>

void main_func() {
	HDEVINFO devInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_CAMERA, "USB", nullptr, DIGCF_PRESENT);
	if (devInfo == INVALID_HANDLE_VALUE)
		return;

	SP_DEVINFO_DATA devInfoData;
	wchar_t buffer[1024];
	char name[256];
	char manufacturer[256];
	char instanceIDBuffer[1024];

	for (int i = 0; ; i++)
	{
		devInfoData.cbSize = sizeof(devInfoData);
		if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData) == FALSE)
			break;

		memset(buffer, 0, sizeof(buffer));
		SetupDiGetDeviceInstanceIdA(devInfo, &devInfoData, (PSTR)instanceIDBuffer, 1024, nullptr);
		std::string instanceID(instanceIDBuffer);

		SetupDiGetDeviceRegistryPropertyA(devInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)name, sizeof(name), nullptr);
		SetupDiGetDeviceRegistryPropertyA(devInfo, &devInfoData, SPDRP_MFG, nullptr, (PBYTE)manufacturer, sizeof(manufacturer), nullptr);
		SetupDiDestroyDeviceInfoList(devInfo);

		std::cout << "Information about camera:" << std::endl;
		std::cout << "Vendor name: " << (char*)manufacturer << std::endl;
		std::cout << "Device name: " << name << std::endl;
		std::cout << "Instance ID: " << instanceID << std::endl;
		SetupDiDeleteDeviceInfo(devInfo, &devInfoData);
	}
	SetupDiDestroyDeviceInfoList(devInfo);
}
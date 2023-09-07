#include "Battery.hpp"

Battery::Battery()
{
	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus(&status);
	this->power_supply = get_power_supply(status);
	this->percent = status.BatteryLifePercent;
	this->life_time = status.BatteryLifeTime;
	this->state_charge = get_state_charge(status);
	this->saving_mode = get_saving_mode(status);
}

std::string Battery::get_power_supply(SYSTEM_POWER_STATUS status)
{
	if (status.ACLineStatus==0) {
		return "battery";
	}
	else if (status.ACLineStatus==1) {
		return "on-line";
	}
	else {
		return "unknown";
	}
}

std::string Battery::get_state_charge(SYSTEM_POWER_STATUS status)
{
	switch (status.BatteryFlag) {
	case 0:
		return "not charging";
	case 1:
		return "high ( > 66% )";
	case 2:
		return "low ( > 20% )";
	case 4:
		return "critical ( < 5% )";
	case 8:
		return "charging";
	case 128:
		return "No system battery";
	default:
		return "unknown";
	}
}

std::string Battery::get_saving_mode(SYSTEM_POWER_STATUS status)
{
	if (status.SystemStatusFlag) {
		return "on";
	}
	return "off";
}

void Battery::scan_battery_info()
{
	HDEVINFO DeviceInfoSet;
	DeviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, nullptr, nullptr,
			DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData = { 0 };
	ZeroMemory(&DeviceInterfaceData, sizeof(SP_DEVINFO_DATA));
	DeviceInterfaceData.cbSize = sizeof(SP_DEVINFO_DATA);

	SetupDiEnumDeviceInterfaces(DeviceInfoSet, nullptr, &GUID_DEVCLASS_BATTERY,
			0, &DeviceInterfaceData);
	DWORD cbRequired = 0;

	SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, nullptr,
			cbRequired, &cbRequired, nullptr);
	auto p_a = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
	p_a->cbSize = sizeof(*p_a);

	SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, p_a,
			cbRequired, &cbRequired, nullptr);
	HANDLE hBattery = CreateFile(p_a->DevicePath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);

	BATTERY_QUERY_INFORMATION BatteryQueryInformation = { 0 };
	DWORD bytesWait = 0;
	DWORD bytesReturned = 0;
	DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG, &bytesWait,
			sizeof(bytesWait),&BatteryQueryInformation.BatteryTag,
			sizeof(BatteryQueryInformation.BatteryTag), &bytesReturned,
			nullptr) && BatteryQueryInformation.BatteryTag;

	BATTERY_INFORMATION BatteryInfo = { 0 };
	BatteryQueryInformation.InformationLevel = BatteryInformation;

	DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &BatteryQueryInformation,
			sizeof(BatteryQueryInformation),&BatteryInfo, sizeof(BatteryInfo),
			&bytesReturned, nullptr);

	std::cout << "Battery type: ";
	for (unsigned char b : BatteryInfo.Chemistry) {
		std::cout << b;
	}
	std::cout << std::endl << "Cycles count: " << BatteryInfo.CycleCount << std::endl;

	LocalFree(p_a);
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}

std::ostream& operator<<(std::ostream& out, const Battery& battery)
{
	out << "BATTERY" << std::endl
		<< "Power source: " << battery.power_supply << std::endl
		<< "State: " << battery.state_charge << std::endl
		<< "Percentage: " << battery.percent << "%" << std::endl
		<< "Saving mode: " << battery.saving_mode << std::endl
		<< "Time remaining: " << battery.life_time << "s" << std::endl
		<< "Battery type: ";
	battery.scan_battery_info();
	out << std::endl;

	return out;
}
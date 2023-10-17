#include "hexioctrl.h"
#include "pci_codes.h"
#include <conio.h>
#include <cstdlib>

void device_info(int, int, int);
void parse_device_name(unsigned long, unsigned long, unsigned long, unsigned long , unsigned long);
unsigned long calculate_address(int, int, int, int);
unsigned long reg_data(unsigned long);

int main()
{
	ALLOW_IO_OPERATIONS;

	// Перебор всех возможных вариантов адресов устройств
	for(int bus = 0; bus < 256; bus++)	          // Номер шины, 8 бит, 2^8 = 256
		for(int dev = 0; dev < 32; dev++)		  // Номер устройства, 5 бит, 2^5 = 32
			for(int func = 0; func < 8; func++)	  // Номер функции, 3 бита, 2^3 = 8
				device_info(bus, dev, func);

	printf("\n\n");
	system("pause");
	return 0;
}

void device_info(int bus, int dev, int func)
{
	unsigned long configAddress = calculate_address(bus, device, function, 0x00); // Получить адрес регистра  для вызова конфиг. цикла
	unsigned long RegData = reg_data(configAddress);

	if (RegData == -1) // Если устройства с данным адресом не существует
		return;

	// Получение Device ID и Vendor ID из регистра 0x00
	unsigned long DeviceID = RegData >> 16;               // Сдвиг вправо на 16 бит (Device ID)
	unsigned long VendorID = RegData - (DeviceID << 16);

	// Получение Class Code и Revision ID из регистра 0x08
	configAddress = calculate_address(bus, device, function, 0x08);
	RegData = reg_data(configAddress);

	unsigned long ClassID = RegData >> 8;
	unsigned long RevisionID = RegData - (ClassID << 8);

	unsigned long BaseClassCode = ClassID >> 16;                   // Base Class Code
	unsigned long ProgInterface = ClassID - (BaseClassCode << 16); // 00000000 - Sub Class Code - Programming Interface
	unsigned long SubClassCode = ProgInterface >> 8;               // Sub Class Code
	ProgInterface = ProgInterface - (SubClassCode << 8);           // Programming Interface

	// Получение Subsystem ID и Subsystem Vendor ID из регистра 0x2C
	configAddress = calculate_address(bus, device, function, 0x2C);
	RegData = reg_data(configAddress);

	unsigned long SubsysID = RegData >> 16;
	unsigned long SubsysVendID = RegData - (SubsysID << 16);

	// Вывод на экран
	printf("PCI bus: %x, device: %x, function: %x\n", bus, device, function);
	printf("Device ID: %lx, Vendor ID: %lx, Class ID: %lx-%lx-%lx\n", DeviceID, VendorID, BaseClassCode, SubClassCode, ProgInterface);
	printf("Revision ID: %lx, Subsystem ID: %lx, Subsystem Vendor ID: %lx\n", RevisionID, SubsysID, SubsysVendID);

	// Вывести на экран текстовые строки наименований производителей, устройств и классов
	parse_device_name(DeviceID, VendorID, BaseClassCode, SubClassCode, ProgInterface);
	printf("--------------------------------------------------------------------------\n\n");
}

// Получить содержимое регистра по адресу configAddress
unsigned long reg_data(unsigned long configAddress)
{
	unsigned long regData;
	__asm
	{
			mov eax, configAddress
			mov dx, 0CF8h
			out dx, eax       // Записать в порт 0CF8h (CONFIG_ADDRESS) адрес регистра устройства
			mov dx, 0CFCh
			in eax, dx        // Прочитать содержимое регистра устройства из порта 0CFCh (CONFIG_DATA)
			mov regData, eax
	}
	return regData;
}

// Составить адрес конфигурационного регистра по номеру шины,
// номеру устройства, номеру функции и номеру регистра
unsigned long calculate_address(int bus, int device, int function, int reg)
{
	unsigned long address = 1;
	address = address << 15;
	address += bus;          // Номер шины, 8 бит
	address = address << 5;
	address += device;       // Номер устройства, 5 бит
	address = address << 3;
	address += function;     // Номер функции, 3 бита
	address = address << 8;
	address += reg;
	return address;
}


// Расшифровка наименований производителей, устройств и классов
void parse_device_name(unsigned long DeviceId, unsigned long VendorId, unsigned long BaseClass,
		unsigned long SubClass, unsigned long ProgIf)
{
	// Class code - основная функция устройства, его программный интерфейс
	for(int i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
	{
		if(PciClassCodeTable[i].BaseClass == BaseClass && PciClassCodeTable[i].SubClass == SubClass
				&& PciClassCodeTable[i].ProgIf == ProgIf)
			printf("%s ( %s %s)\n", PciClassCodeTable[i].BaseDesc, PciClassCodeTable[i].SubDesc, PciClassCodeTable[i].ProgDesc);
	}

	// Название и описание устройства
	for(int i = 0; i < PCI_DEVTABLE_LEN; i++)
	{
		if(PciDevTable[i].VenId == VendorId && PciDevTable[i].DevId == DeviceId)
			printf("%s, %s\n", PciDevTable[i].Chip, PciDevTable[i].ChipDesc);
	}

	// Название производителя устройства
	for(int i = 0; i < PCI_VENTABLE_LEN; i++)
	{
		if(PciVenTable[i].VenId == VendorId)
			printf("%s\n", PciVenTable[i].VenFull);
	}
}
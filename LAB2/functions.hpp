//
// Created by Alexandr on 9/26/2023.
//

#ifndef IAPD_FUNCTIONS_HPP
#define IAPD_FUNCTIONS_HPP

#include <conio.h>

unsigned long reg_data(unsigned long address)
{
	// from: https://wiki.osdev.org/PCI
	//Bit 31 	    Bits 30-24	Bits 23-16 	Bits 15-11 	    Bits 10-8 	        Bits 7-0
	//                              8           4               3
	//Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number 	Register Offset1
	unsigned long data;
	// WritePortULONG()
	_outpd(0xCF8, address); // CONFIG_ADDRESS
	// ReadPortULONG()
	data = _inpd(0xCFC);    //CONFIG_DATA
	return data;
}

unsigned long calculate_address(int bus, int device, int function, int reg)
{
	unsigned long address = 1;
	address = address << 15;
	address += bus;          // bus ID 8bit
	address = address << 5;
	address += device;       // dev ID 5bit
	address = address << 3;
	address += function;     // function ID 3bit
	address = address << 8;
	address += reg;          // register offset
	return address;
}


void parse_device_name(unsigned long DeviceId, unsigned long VendorId, unsigned long BaseClass,
		unsigned long SubClass, unsigned long ProgIf)
{
	// device class code
	for(int i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
	{
		if(PciClassCodeTable[i].BaseClass == BaseClass &&
			PciClassCodeTable[i].SubClass == SubClass &&
			PciClassCodeTable[i].ProgIf == ProgIf)
			printf("%s ( %s %s)\n", PciClassCodeTable[i].BaseDesc, PciClassCodeTable[i].SubDesc, PciClassCodeTable[i].ProgDesc);
	}

	// device description
	for(int i = 0; i < PCI_DEVTABLE_LEN; i++)
	{
		if(PciDevTable[i].VenId == VendorId &&
			PciDevTable[i].DevId == DeviceId)
			printf("%s, %s\n", PciDevTable[i].Chip, PciDevTable[i].ChipDesc);
	}
	printf("\n");
}

void device_info(int bus, int device, int function)
{
	unsigned long config_address = calculate_address(bus, device, function, 0x00); // config register dev info
	unsigned long data = reg_data(config_address);
	if (data == -1) // non-existing device checker
		return;

	unsigned long device_id = data >> 16;
	unsigned long vendor_id = data - (device_id << 16);
	//  Header Type 0x0 from state register
	//	Register 	Offset    Bits 31-24 	Bits 23-16 	Bits 15-8 	Bits 7-0
	//	0x0 	    0x0 	  Device ID 	            Vendor ID
	//	0x1 	    0x4 	  Status 	                Command
	//	0x2 	    0x8 	  Class code 	Subclass 	Prog IF 	Revision ID

	config_address = calculate_address(bus, device, function, 0x08); // register for class code and rev id
	data = reg_data(config_address);
	unsigned long class_id = data >> 8;
	unsigned long revision_id = data - (class_id << 8);

	unsigned long base_class_code = class_id >> 16;
	unsigned long prog_interface = class_id - (base_class_code << 16);
	unsigned long sub_class_code = prog_interface >> 8;
	prog_interface = prog_interface - (sub_class_code << 8);

	config_address = calculate_address(bus, device, function, 0x2C); // subsystem and vendor ID register
	data = reg_data(config_address);

	unsigned long SubsysID = data >> 16;
	unsigned long SubsysVendID = data - (SubsysID << 16);

	printf("Bus: %x, Device: %x, Function: %x\n", bus, device, function);
	printf("Device ID: %lx, Vendor ID: %lx\n", device_id, vendor_id);

	parse_device_name(device_id,
			vendor_id,
			base_class_code,
			sub_class_code,
			prog_interface);
}

#endif //IAPD_FUNCTIONS_HPP

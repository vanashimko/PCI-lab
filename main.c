#include <stdio.h>
#include <sys/io.h>
#include <errno.h>
#include <stdlib.h>

#include "pci_constants.h"
#include "pci.h"



char *GetVendorName(int vendorID)
{
	int i;
	for (i = 0; i < PCI_VENTABLE_LEN; i++)
		if (PciVenTable[i].VendorId == vendorID)
			return PciVenTable[i].VendorName;
	return NULL;	
}

char *GetDeviceName(int vendorID, int deviceID)
{
	int i;
	for (i = 0; i < PCI_DEVTABLE_LEN; i++)
		if (PciDevTable[i].VendorId == vendorID && PciDevTable[i].DeviceId == deviceID)
			return PciDevTable[i].DeviceName;	
	
	return NULL;
}

ulong GetDataFromRegister(ushort bus, ushort device, ushort function, ushort registerNumber)
{
	ulong command = CONTROL_BIT | (bus << BUS_SHIFT) | (device << DEVICE_SHIFT) | (function << FUNCTION_SHIFT) | (registerNumber << REGISTER_SHIFT);
	outl(command, CONTROL_PORT);
	return inl(INFO_PORT);	
}

ulong GetIDs(ushort bus, ushort device, ushort function)
{
	return GetDataFromRegister(bus, device, function, 0);
}

byte GetHeaderType(ushort bus, ushort device, ushort function)
{
	ulong answer = GetDataFromRegister(bus, device, function, HEADER_TYPE_REGISTER);
	return (answer >> 16) & 255;	
}


void PrintDeviceMainInfo(ushort deviceID, ushort vendorID)
{		
	char *vendorName = GetVendorName(vendorID);
	char *deviceName = GetDeviceName(vendorID, deviceID);
	
	printf("Vendor ID: %x ", vendorID);
	if (vendorName)
		printf("%s\n", GetVendorName(vendorID));
	else
		puts("Not found.");
		
	printf("Device ID: %x ", deviceID);
	if (deviceName)
		printf("%s\n", GetDeviceName(vendorID, deviceID));
	else
		puts("Not found.");	
}

ulong GetClassCode(ushort bus, ushort device, ushort function)
{
	return GetDataFromRegister(bus, device, function, 0x08) >> 8;
}

char *GetBaseClassDescription(byte baseClass)
{
	int i;
	for (i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
		if (baseClass == PciClassCodeTable[i].BaseClass)
			return PciClassCodeTable[i].BaseDesc;
	
	return NULL;	
}

char *GetSubclassDescription(byte baseClass, byte subclass)
{
	int i;
	for (i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
		if (subclass == PciClassCodeTable[i].SubClass && baseClass == PciClassCodeTable[i].BaseClass)
			return PciClassCodeTable[i].SubDesc;
	
	return NULL;	
}

char *GetProgrammingInterfaceDescription(byte baseClass, byte subclass, byte programmingInterface)
{
	int i;
	for (i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
		if (subclass == PciClassCodeTable[i].SubClass && 
			baseClass == PciClassCodeTable[i].BaseClass && 
			programmingInterface == PciClassCodeTable[i].ProgIf)
			
			return PciClassCodeTable[i].ProgDesc;
	
	return NULL;	
}

void PrintClassCodeInfo(ushort bus, ushort device, ushort function)
{	
	ulong classCode = GetDataFromRegister(bus, device, function, CLASS_CODE_REGISTER) >> 8;
	byte baseClass = (classCode >> 16) & 255;
	byte subclass = (classCode >> 8) & 255;
	byte programmingInterface = classCode & 255;
	
	printf("Class Code: %#x\n", classCode);
	
	printf("Base class: %#x ", baseClass);
	char *baseClassDescription = GetBaseClassDescription(baseClass);
	if (baseClassDescription)
		puts(baseClassDescription);
	else
		puts("Not found.");		
	
	printf("Subclass: %#x ", subclass);
	char *subclassDescription = GetSubclassDescription(baseClass, subclass);
	if (subclassDescription)
		puts(baseClassDescription);
	else
		puts("Not found.");
	
	
	printf("Specific register level programming interface : %#x ", programmingInterface);
	char *programmingInterfaceDescription = GetProgrammingInterfaceDescription(baseClass, subclass, programmingInterface);
	if (programmingInterfaceDescription)
		puts(programmingInterfaceDescription);
	else
		puts("Not found.");				
	
}

void PrintBARInfo(ulong registerInfo)
{
	if (!(registerInfo & 1))
	{
		puts("Memory");
		switch ((registerInfo >> 1) & 3)
		{
			case 0:
				puts("Any place in 32-bit address space.");
				break;
			case 1:
				puts("Only < 1MB");
				break;
			case 2:
				puts("Any place in 64-bit address space");
				break;
			default:
				break;
		}
		printf("Address: %#x\n", registerInfo >> 4);
				
	}
	else
	{
		puts("I\\O");
		printf("Address: %#x\n", registerInfo >> 2);
	}
}

void ProcessBARs(ushort bus, ushort device, ushort function)
{
	int i;
	puts("\nBAR info");
	for ( i = 4; i < 10; i++)
	{
		printf("Register %d:\n", i - 3);
		ulong registerInfo = GetDataFromRegister(bus, device, function, i);
		if (registerInfo)
			PrintBARInfo(GetDataFromRegister(bus, device, function, i));
		else
			puts("Not used");
	}
	
}

void ProcessDevice(ushort bus, ushort device, ushort function)
{
	
	ulong IDs = GetIDs(bus, device, function);
	
	if (IDs != 0xffffffff)
	{
		ushort deviceID = IDs >> 16;
		ushort vendorID = IDs & 65535;
		printf("%x.%x.%x\n", bus, device, function);
		PrintDeviceMainInfo(deviceID, vendorID);	
		
		byte isBridge = GetHeaderType(bus, device, function) >> 7;
		
		if (isBridge)
		{
			puts("\nBridge\n");
			
		}
		else
		{
			puts("\nNot a bridge\n");
			PrintClassCodeInfo(bus, device, function);
			ProcessBARs(bus, device, function);
			
		}	
		
		puts("-------------------");	
	}
}

int main(int argc, char **argv)
{
	if (iopl(3) )
	{
		printf("I/O Privilege level change error\n Try run under ROOT rights\n");
		return 1;
	}
	
	unsigned short bus, device, function;
	
	for (bus = 0; bus < 256; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (function = 0; function < 8; function++)
			{
				ProcessDevice(bus, device, function);
			}
			
		}
	}
	
	return 0;
}

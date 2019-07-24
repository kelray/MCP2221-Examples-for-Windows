/*	Project: I2C Slave Devices Scanner using MCP2221 USB-to-I2C bridge
*	Inspired from Arduino I2C Scanner Example: https://playground.arduino.cc/Main/I2cScanner/	
*/

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <iso646.h>
#include "mcp2221_dll_um.h"
#pragma comment(lib, "mcp2221_dll_um_x86.lib")

#define pi 22/7
#define I2cAddr7bit 1
#define I2cAddr8bit 0

//Global variables
void *handle;
wchar_t SerNum = 0x0000075428;
wchar_t LibVer[6];
wchar_t MfrDescriptor[30];
wchar_t ProdDescrip[30];
int ver = 0;
int error = 0;
int flag = 0;
unsigned int delay = 0;
unsigned int ReqCurrent;
unsigned int PID = 0xDD;
unsigned int VID = 0x4D8;
unsigned int NumOfDev = 0;
unsigned char PowerAttrib;
unsigned char RxData[7] = { 0 };
unsigned char Addr = 0x00;
unsigned char DummyByte = 0x00;

//Functions prototypes
void ExitFunc();
void Mcp2221_config();

void ExitFunc()
{
	//Cancel all I2C transfers prior to closing the MCP2221 device
	Mcp2221_I2cCancelCurrentTransfer(handle);
	//Close handle of all connected MCP2221 devices 
	Mcp2221_CloseAll();
}

void Mcp2221_config()
{
	ver = Mcp2221_GetLibraryVersion(LibVer);		//Get DLL version
	if (ver == 0)
	{
		printf("Library (DLL) version: %ls\n", LibVer);
	}
	else
	{
		error = Mcp2221_GetLastError();
		printf("Version can't be found, version: %d, error: %d\n", ver, error);
	}

	//Get number of connected devices with this VID & PID
	Mcp2221_GetConnectedDevices(VID, PID, &NumOfDev);
	if (NumOfDev == 0)
	{
		printf("No MCP2221 devices connected\n");
		//exit(0);
	}
	else
	{
		printf("Number of devices found: %d\n", NumOfDev);
	}

	//open device by S/N
	//handle = Mcp2221_OpenBySN(VID, PID, &SerNum);

	//Open device by index
	handle = Mcp2221_OpenByIndex(VID, PID, NumOfDev - 1);
	if (error == NULL)
	{
		printf("Connection successful\n");
	}
	else
	{
		error = Mcp2221_GetLastError();
		printf("Error message is %s\n", error);
	}

	//Get manufacturer descriptor
	flag = Mcp2221_GetManufacturerDescriptor(handle, MfrDescriptor);
	if (flag == 0)
	{
		printf("Manufacturer descriptor: %ls\n", MfrDescriptor);

	}
	else
	{
		printf("Error getting descriptor: %d\n", flag);
	}

	//Get product descriptor
	flag = Mcp2221_GetProductDescriptor(handle, ProdDescrip);
	if (flag == 0)
	{
		printf("Product descriptor: %ls\n", ProdDescrip);
	}
	else
	{
		printf("Error getting product descriptor: %d\n", flag);
	}

	//Get power attributes
	flag = Mcp2221_GetUsbPowerAttributes(handle, &PowerAttrib, &ReqCurrent);
	if (flag == 0)
	{
		printf("Power Attributes, %x\nRequested current units = %d\nRequested current(mA) = %d\n", PowerAttrib, ReqCurrent, ReqCurrent * 2);
	}
	else
	{
		printf("Error getting power attributes: %d\n", flag);
	}

	//Set I2C bus
	flag = Mcp2221_SetSpeed(handle, 500000);    //set I2C speed to 400 KHz
	if (flag == 0)
	{
		printf("I2C is configured\n");
	}
	else
	{
		printf("Error setting I2C bus: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
	}

	//Set I2C advanced parameters
	flag = Mcp2221_SetAdvancedCommParams(handle, 1, 1000);  //1ms timeout, try 1000 times
	if (flag == 0)
	{
		printf("I2C advanced settings set\n");
	}
	else
	{
		printf("Error setting I2C advanced settings: %d\n", flag);
	}
}

int main(int argc, char *argv[])
{
	//Register exit function
	atexit(ExitFunc);	

	//Configure any connected MCP2221
	Mcp2221_config();

	while (1)
	{
		//Scan all addresses from 1 to 127
		for (Addr = 1; Addr <= 127; Addr++)
		{
			flag = Mcp2221_I2cWrite(handle, sizeof(DummyByte), Addr, I2cAddr7bit, &DummyByte);    //issue start condition then address
			if (flag == 0)
			{
				printf("Device 0x%X Exist on the bus\n", Addr);
			}
			else
			{
				//printf("Error, Address not found: %d\n", flag);
				//Mcp2221_I2cCancelCurrentTransfer(handle);
			}
		}
		_sleep(2000);
	}
}
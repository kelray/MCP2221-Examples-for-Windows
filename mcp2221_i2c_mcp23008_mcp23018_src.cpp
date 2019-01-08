/*Interfacing MCP2221 with MCP23008 and MCP23018 I2C I/O Expanders.
* MCP23018 is open drain, the output is inverted if you connect an LED to it.
* This code toggles MCP23008 and MCP23018 GPIO pins, you can use LEDs to observe the output toggling.
* By: Karim El-Rayes
* Created: December 2018.
*/
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <iso646.h>
#include "dll\mcp2221_dll_um.h"
#pragma comment(lib, "mcp2221_dll_um_x86.lib")

#define pi 22/7
#define I2cAddr7bit 1
#define I2cAddr8bit 0

//Global variables
void *handle;
int i;
int phase = 0;

wchar_t SerNum = 0x0000075428;
wchar_t LibVer[6];
wchar_t MfrDescriptor[30];
wchar_t ProdDescrip[30];
int ver = 0;
int error = 0;
int flag = 0;
unsigned int ADCbuffer[3];
unsigned int delay = 0;
unsigned int ADCreading = 0;
unsigned int ReqCurrent;
unsigned int PID = 0xDD;
unsigned int VID = 0x4D8;
unsigned int NumOfDev = 0;
unsigned char PowerAttrib;
unsigned char DacVal = 31;
unsigned char SlaveAddress = 0x20;	//MCP23008 Address: 0100000 = 0x20 (A0, A1, A2 = 0)
unsigned char StartCmd = 0x00;
unsigned char StopCmd = 0x00;
unsigned char RxData[64];
unsigned int tempADCreading = 0;
unsigned char TxData[2];
float voltage = 0;
int ErrorCounter = 0;

//MCP23008 Registers
unsigned char MCP23008_ADDR = 0x20;			//MCP23008 Address: 0100000 = 0x20 (A0, A1, A2 = 0)
unsigned char MCP23008_IODIR = 0x00;
unsigned char MCP23008_IPOL = 0x01;
unsigned char MCP23008_GPINTEN = 0x02;
unsigned char MCP23008_DEFVAL = 0x03;
unsigned char MCP23008_INTCON = 0x04;
unsigned char MCP23008_IOCON = 0x05;
unsigned char MCP23008_GPPU = 0x06;
unsigned char MCP23008_INTF = 0x07;
unsigned char MCP23008_INTCAP = 0x08;
unsigned char MCP23008_GPIO = 0x09;
unsigned char MCP23008_OLAT = 0x0A;
unsigned char Config_bits = 0x00;

//MCP23018 Registers
unsigned char MCP23018_ADDR = 0x20;			//MCP23018 Address: 0100000 = 0x20 (A0, A1, A2 = 0)
unsigned char MCP23018_IODIRA = 0x00;
unsigned char MCP23018_IODIRB = 0x01;
unsigned char MCP23018_IPOLA = 0x02;
unsigned char MCP23018_IPOLB = 0x03;
unsigned char MCP23018_GPINTENA = 0x04;
unsigned char MCP23018_GPINTENB = 0x05;
unsigned char MCP23018_DEFVALA = 0x06;
unsigned char MCP23018_DEFVALB = 0x07;
unsigned char MCP23018_INTCONA = 0x08;
unsigned char MCP23018_INTCONB = 0x09;
unsigned char MCP23018_IOCON1 = 0x0A;
unsigned char MCP23018_IOCON2 = 0x0B;
unsigned char MCP23018_GPPUA = 0x0C;
unsigned char MCP23018_GPPUB = 0x0D;
unsigned char MCP23018_INTFA = 0x0E;
unsigned char MCP23018_INTFB = 0x0F;
unsigned char MCP23018_INTCAPA = 0x10;
unsigned char MCP23018_INTCAPB = 0x11;
unsigned char MCP23018_GPIOA = 0x12;
unsigned char MCP23018_GPIOB = 0x13;
unsigned char MCP23018_OLATA = 0x14;
unsigned char MCP23018_OLATB = 0x15;


struct GPIO_Config {
	unsigned char reg;
	unsigned char val;
}GPIO_Config;

//Functions prototypes
void ExitFunc();
void Mcp2221_config();

void ExitFunc()
{
	printf("Closing\n");
	Sleep(1000);
	Mcp2221_Close(handle);

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

	//Clear I2C transfers
	//Mcp2221_I2cCancelCurrentTransfer(handle);

	//Set I2C bus
	flag = Mcp2221_SetSpeed(handle, 500000); //46875   //set I2C speed to 400 KHz
	if (flag == 0)
	{
		printf("I2C is configured\n");
	}
	else
	{
		Mcp2221_I2cCancelCurrentTransfer(handle);
		printf("Error setting I2C bus: %d\n", flag);
	}

	//Set I2C advanced parameters
	flag = Mcp2221_SetAdvancedCommParams(handle, 100, 100);  //100ms timeout, try 100 times
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
	unsigned char pinFunc[4] = { MCP2221_GPFUNC_IO, MCP2221_GP_ADC, MCP2221_GP_ADC, MCP2221_GP_ADC };
	unsigned char pinDir[4] = { MCP2221_GPDIR_OUTPUT, NO_CHANGE, NO_CHANGE, NO_CHANGE };
	unsigned char OutValues[4] = { 0, 0, NO_CHANGE, 0 };
	struct GPIO_Config mcp23008;

	//Call exit function
	atexit(ExitFunc);

	//Configure any connected MCP2221
	Mcp2221_config();	

	unsigned char INIT_CMD1 = 0xFF;
	unsigned char INIT_CMD2 = 0x00;

	
	//Initialize MCP23008
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &MCP23008_IODIR);
	if (flag == 0)
	{
		printf("MCP23008 Initialization sequence started\n");
	}
	else
	{
		printf("Error initializing MCP23008, Error code: %d\n", flag);
	}
	Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &INIT_CMD1);
	for (i = 0; i < 9; i++)
	{
		Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &INIT_CMD2);
		printf("Writing %d 0x%x\n", i, INIT_CMD2);
	}

	//set IO direction  in the IODIR register
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &MCP23008_IODIR);
	if (flag == 0)
	{
		printf("Set IO direction Success\n");
	}
	else
	{
		printf("Error writing to register: 0x%x, Error code: %d\n", MCP23008_IODIR, flag);
	}

	Config_bits = 0x00;		//set all GPIOs to outputs
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &Config_bits);
	if (flag == 0)
	{
		printf("Set all bits in IODIR to: 0x%x\n", Config_bits);
	}
	else
	{
		printf("Error setting IO direction, Error code: %d\n", flag);
	}

	
	//Write 1 to all GPIOs
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &MCP23008_GPIO);
	if (flag == 0)
	{
		printf("Writing to all GPIO pins -> Success\n");
	}
	else
	{
		printf("Error writing to register: 0x%x, Error code: %d\n", MCP23008_GPIO, flag);
	}

	Config_bits = 0xff;		//set all GPIO to 1
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &Config_bits);
	if (flag == 0)
	{
		printf("Writing 0x%x to all GPIO pints\n", Config_bits);
	}
	else
	{
		printf("Error setting GPIO, Error code: %d\n", flag);
	}

	//Write 1 to all GPIO latches OLAT register
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &MCP23008_OLAT);
	if (flag == 0)
	{
		printf("Writing to all GPIO latches -> Success\n");
	}
	else
	{
		printf("Error writing to register: 0x%x, Error code: %d\n", MCP23008_OLAT, flag);
	}

	Config_bits = 0xff;		//set all GPIO latches to 1
	flag = Mcp2221_I2cWrite(handle, 1, MCP23008_ADDR, I2cAddr7bit, &Config_bits);
	if (flag == 0)
	{
		printf("Writing 0x%x to all GPIO pints\n", Config_bits);
	}
	else
	{
		printf("Error setting GPIO, Error code: %d\n", flag);
	}
	
	//Set Port B pins output
	TxData[0] = MCP23018_IODIRB;
	TxData[1] = 0x00;
	flag = Mcp2221_I2cWrite(handle, 2, MCP23018_ADDR, I2cAddr7bit, TxData);
	
	//Enable pull-ups
	TxData[0] = MCP23018_GPPUB;
	TxData[1] = 0xFF;
	flag = Mcp2221_I2cWrite(handle, 2, MCP23018_ADDR, I2cAddr7bit, TxData);
	
	while (1)
	{
		//MCP23008 - Set GPIO register
		TxData[0] = MCP23008_GPIO;
		TxData[1] = 0xFF;
		Mcp2221_I2cWrite(handle, 2, MCP23008_ADDR, I2cAddr7bit, TxData);

		//set latch register
		TxData[0] = MCP23008_OLAT;
		TxData[1] = 0xFF;
		Mcp2221_I2cWrite(handle, 2, MCP23008_ADDR, I2cAddr7bit, TxData);
		Sleep(500);

		TxData[0] = MCP23008_OLAT;
		TxData[1] = 0x00;
		Mcp2221_I2cWrite(handle, 2, MCP23008_ADDR, I2cAddr7bit, TxData);
		Sleep(500);

		//set latch register
		TxData[0] = MCP23018_OLATB;
		TxData[1] = 0xFF;
		flag = Mcp2221_I2cWrite(handle, 2, MCP23018_ADDR, I2cAddr7bit, TxData);
		if (flag != 0)
		{
			printf("Error setting OLATB, Error code: %d\n", flag);
		}
		Sleep(500);

		TxData[0] = MCP23018_OLATB;
		TxData[1] = 0x00;
		flag = Mcp2221_I2cWrite(handle, 2, MCP23018_ADDR, I2cAddr7bit, TxData);
		if (flag != 0)
		{
			printf("Error setting OLATB, Error code: %d\n", flag);
		}
		Sleep(500);

		//quit when user hit 'q'
		if (_kbhit())
		{
			if (_getch() == 'q')
				exit(0);
		}
	}
}
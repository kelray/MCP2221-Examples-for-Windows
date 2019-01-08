/* Interfacing Serial I2C 16x2 Character LCD with MCP221
* By: Karim El-Rayes  
* Original example code used to create this example: 
* https://github.com/sunfounder/SunFounder_SensorKit_for_RPi2/blob/master/C/30_i2c_lcd1602/i2c_lcd1602.c
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
unsigned char Ch1Address = 0x32;
unsigned char Ch2Address = 0x38;
unsigned char StartCmd = 0x00;
unsigned char StopCmd = 0x00;
unsigned char RxData[4];
unsigned int tempADCreading = 0;
unsigned char TxData[2];
float voltage = 0;
char message[] = "Hello World";
unsigned char temp = 0;

//LCD variables
unsigned char LCD_ADDR = 0x27;
int BLEN = 1;
int fd;

//LCD Commands
unsigned char LCD_CLEARDISPLAY = 0x01;
unsigned char LCD_RETURNHOME = 0x02;
unsigned char LCD_ENTRYMODESET = 0x04;
unsigned char LCD_DISPLAYCONTROL = 0x08;
unsigned char LCD_CURSORSHIFT = 0x10;
unsigned char LCD_FUNCTIONSET = 0x20;
unsigned char LCD_SETCGRAMADDR = 0x40;
unsigned char LCD_SETDDRAMADDR = 0x80;


//Flags for display on/off control
unsigned char LCD_DISPLAYON = 0x04;
unsigned char LCD_DISPLAYOFF = 0x00;
unsigned char LCD_CURSORON = 0x02;
unsigned char LCD_CURSOROFF = 0x00;
unsigned char LCD_BLINKON = 0x01;
unsigned char LCD_BLINKOFF = 0x00;


//Flags for display entry mode
unsigned char LCD_ENTRYRIGHT = 0x00;
unsigned char LCD_ENTRYLEFT = 0x02;
unsigned char LCD_ENTRYSHIFTINCREMENT = 0x01;
unsigned char LCD_ENTRYSHIFTDECREMENT = 0x00;

//Flags for display/cursor shift
unsigned char LCD_DISPLAYMOVE = 0x08;
unsigned char LCD_CURSORMOVE = 0x00;
unsigned char LCD_MOVERIGHT = 0x04;
unsigned char LCD_MOVELEFT = 0x00;

// flags for function set
unsigned char LCD_8BITMODE = 0x10;
unsigned char LCD_4BITMODE = 0x00;
unsigned char LCD_2LINE = 0x08;
unsigned char LCD_1LINE = 0x00;
unsigned char LCD_5x10DOTS = 0x04;
unsigned char LCD_5x8DOTS = 0x00;

//Functions prototypes
void ExitFunc();
void Mcp2221_config();
void LCD_clear();
void LCD_write(int x, int y, char data[]);

void ExitFunc()
{
	LCD_clear();
	Mcp2221_Reset(handle);
	//printf("Closing\n");

	//Cancel I2C transfer
	//Mcp2221_I2cCancelCurrentTransfer(handle);

	//Close all devices at exit
	//Mcp2221_CloseAll();
	//Mcp2221_Close(handle);
	//_sleep(100);
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
		printf("Error message is %d\n", error);
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
		printf("Power Attributes= %d\nRequested current units = %d\nRequested current(mA) = %d\n", PowerAttrib, ReqCurrent, ReqCurrent * 2);
	}
	else
	{
		printf("Error getting power attributes: %d\n", flag);
	}

	//Set I2C bus
	//Mcp2221_I2cCancelCurrentTransfer(handle);
	flag = Mcp2221_SetSpeed(handle, 500000);// 46875);		//500000: set I2C speed to 400 KHz
	if (flag == 0)
	{
		printf("I2C is configured\n");
	}
	else
	{
		printf("Error setting I2C bus: %d\n", flag);
	}

	//Set I2C advanced parameters
	flag = Mcp2221_SetAdvancedCommParams(handle, 10, 10000);  //10ms timeout, try 1000 times
	if (flag == 0)
	{
		printf("I2C advanced settings set\n");
	}
	else
	{
		printf("Error setting I2C advanced settings: %d\n", flag);
	}
}

void write_word(int data) {
	//int temp = data;
	temp = data;
	if (BLEN == 1)
		temp |= 0x08;
	else
		temp &= 0xF7;
	//wiringPiI2CWrite(fd, temp);
	Mcp2221_I2cWrite(handle, 1, LCD_ADDR, I2cAddr7bit, &temp);
}

void send_command(int comm) {
	int buf;
	// Send bit7-4 firstly
	buf = comm & 0xF0;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	_sleep(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (comm & 0x0F) << 4;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	_sleep(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void send_data(int data) {
	int buf;
	// Send bit7-4 firstly
	buf = data & 0xF0;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	_sleep(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (data & 0x0F) << 4;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	_sleep(2);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void LCD_init() {
	send_command(0x33);	// Must initialize to 8-line mode at first
	_sleep(5);
	send_command(0x32);	// Then initialize to 4-line mode
	_sleep(5);
	send_command(0x28);	// 2 Lines & 5*7 dots
	_sleep(5);
	send_command(0x0C);	// Enable display without cursor
	_sleep(5);
	send_command(0x01);	// Clear Screen
	//wiringPiI2CWrite(fd, 0x08);
	StartCmd = 0x08;
	Mcp2221_I2cWrite(handle, 1, LCD_ADDR, I2cAddr7bit, &StartCmd);
}

void LCD_clear() {
	send_command(0x01);	//clear Screen
}

void LCD_write(int x, int y, char data[]) {
	int addr, i;
	int tmp;
	if (x < 0)  x = 0;
	if (x > 15) x = 15;
	if (y < 0)  y = 0;
	if (y > 1)  y = 1;

	// Move cursor
	addr = 0x80 + 0x40 * y + x;
	send_command(addr);

	tmp = strlen(data);
	for (i = 0; i < tmp; i++) {
		send_data(data[i]);
	}
}

int main(int argc, char *argv[])
{
	atexit(ExitFunc);	//Call exit function
	Mcp2221_config();	//Configure any connected MCP2221
	LCD_init();

	while (1)
	{
		sprintf(message, "%d", rand());
		LCD_write(0, 0, message);
		_sleep(500);
		LCD_clear();
		if (_kbhit())
		{
			if (_getch() == 'q')
				exit(0);
		}
		Sleep(10);
	}
}
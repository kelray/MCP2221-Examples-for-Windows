/*	Project: Interfacing MCP9808 Temperature sensor with MCP2221 via I2C
*	By: Karim El-Rayes
*	Created: March 2, 2017
*	Notes: Inspired from the Arduino library for MCP9808 Temperature Sensor
*/

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <iso646.h>
#include "mcp2221_dll_um.h"
#include "bitsNbytesManup.h"
#pragma comment(lib, "mcp2221_dll_um_x86.lib")


#define pi 22/7
#define I2cAddr7bit 1
#define I2cAddr8bit 0

//MCP9808 registers and commands
unsigned char MCP9808_I2CADDR_DEFAULT		= 0x18;
unsigned char MCP9808_REG_CONFIG            = 0x01;

unsigned char MCP9808_REG_CONFIG_SHUTDOWN	= 0x0100;
unsigned char MCP9808_REG_CONFIG_CRITLOCKED	= 0x0080;
unsigned char MCP9808_REG_CONFIG_WINLOCKED  = 0x0040;
unsigned char MCP9808_REG_CONFIG_INTCLR     = 0x0020;
unsigned char MCP9808_REG_CONFIG_ALERTSTAT  = 0x0010;
unsigned char MCP9808_REG_CONFIG_ALERTCTRL  = 0x0008;
unsigned char MCP9808_REG_CONFIG_ALERTSEL   = 0x0004;
unsigned char MCP9808_REG_CONFIG_ALERTPOL   = 0x0002;
unsigned char MCP9808_REG_CONFIG_ALERTMODE  = 0x0001;

unsigned char MCP9808_REG_UPPER_TEMP        = 0x02;
unsigned char MCP9808_REG_LOWER_TEMP        = 0x03;
unsigned char MCP9808_REG_CRIT_TEMP         = 0x04;
unsigned char MCP9808_REG_AMBIENT_TEMP      = 0x05;
unsigned char MCP9808_REG_MANUF_ID          = 0x06;
unsigned char MCP9808_REG_DEVICE_ID         = 0x07;
unsigned char MCP9808_REG_RESOLUTION		= 0x08;

//File logging variables
FILE * logFile;		//Create file to save temperature log

//Global variables
void *handle;
int i;
int phase = 0;
unsigned int Temp2comp = 0;
unsigned int TempDec = 0;
double AmbTemp = 0.0;
int SignBit = 0;
unsigned int SoundData = 0;
unsigned char config[3] = {0};

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
unsigned char SlaveAddress = 0x63;
unsigned char StartCmd = 0x00;
unsigned char StopCmd = 0x00;
unsigned char RxData[7] = {0};
unsigned int tempADCreading = 0;
unsigned char TxData[2];
float voltage = 0;

unsigned char SlaveAddress1 = 0x18;			//MCP9808 address A0=0, A1=0, A2=0
unsigned char DACBuffer[] = {0x11, 0xff, 0xff};
unsigned char counter = 0x00;
unsigned char MSBcounter = 0x00;
unsigned char LSBcounter = 0x00;

//Functions prototypes
void ExitFunc();
unsigned int* ReadADC();
void Mcp2221_config();

void ExitFunc()
{
	Mcp2221_I2cCancelCurrentTransfer(handle);
	Mcp2221_Close(handle);
	//fclose(logFile);
	//_sleep(100);
}

unsigned int* ReadADC()
{
    flag = Mcp2221_GetAdcData(handle, ADCbuffer);
    if(flag != 0)
    {
        printf("Error reading from ADC, error: %d\n", flag);
        system("pause");
    }
    else
    {
        printf("ADC channel 1 is %d\n", ADCbuffer[0]);		//Reading from ADC channel 1
		printf("ADC channel 2 is %d\n", ADCbuffer[1]);		//Reading from ADC channel 2
		printf("ADC channel 3 is %d\n", ADCbuffer[2]);		//Reading from ADC channel 3
    }
    return ADCbuffer;
}

void Mcp2221_config()
{
    ver = Mcp2221_GetLibraryVersion(LibVer);		//Get DLL version
    if(ver == 0)
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
    if(NumOfDev == 0)
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
    handle = Mcp2221_OpenByIndex(VID, PID, NumOfDev-1);
    if(error == NULL)
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
    if(flag == 0)
    {
        printf("Manufacturer descriptor: %ls\n", MfrDescriptor);

    }
    else
    {
        printf("Error getting descriptor: %d\n", flag);
    }

    //Get product descriptor
    flag = Mcp2221_GetProductDescriptor(handle, ProdDescrip);
    if(flag == 0)
    {
        printf("Product descriptor: %ls\n", ProdDescrip);        
    }
    else
    {
        printf("Error getting product descriptor: %d\n", flag);        
    }

    //Get power attributes
    flag = Mcp2221_GetUsbPowerAttributes(handle, &PowerAttrib, &ReqCurrent);
    if(flag == 0)
    {
        printf("Power Attributes, %x\nRequested current units = %d\nRequested current(mA) = %d\n", PowerAttrib, ReqCurrent, ReqCurrent*2);
    }
    else
    {
        printf("Error getting power attributes: %d\n", flag);      
    }

    //Set I2C bus
    flag = Mcp2221_SetSpeed(handle, 500000);    //set I2C speed to 400 KHz
    if(flag == 0)
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
    if(flag == 0)
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
    unsigned char pinFunc[4] = {MCP2221_GPFUNC_IO, MCP2221_GP_ADC, MCP2221_GP_ADC, MCP2221_GP_ADC};
    unsigned char pinDir[4] = {MCP2221_GPDIR_OUTPUT, NO_CHANGE, NO_CHANGE, NO_CHANGE};
    unsigned char OutValues[4] = {0, 0, NO_CHANGE, 0};

    atexit(ExitFunc);	//Call exit function

    //Configure any connected MCP2221
    Mcp2221_config();

    //Set GPIO
    flag = Mcp2221_SetGpioSettings(handle, RUNTIME_SETTINGS, pinFunc, pinDir, OutValues);
    if(flag != 0)
    {
        printf("Error setting GPIO, error: %d\n", flag);
        system("pause");        
    }
    //Mcp2221_SetGpioValues(handle, OutValues);	//reset all pins at initialization

    //Set DAC reference to VDD
    flag = Mcp2221_SetDacVref(handle, RUNTIME_SETTINGS, VREF_VDD);
    if(flag != 0)
    {
        printf("Error setting DAC reference, error: %d\n", flag);
        system("pause");
    }

    //Set ADC voltage reference to VDD
    flag = Mcp2221_SetAdcVref(handle, RUNTIME_SETTINGS, VREF_VDD);
    if(flag != 0)
    {
        printf("Error setting ADC reference, error: %d\n", flag);
        system("pause");
	}

	//Request device ID from MCP9808
	flag = Mcp2221_I2cWrite(handle, 1, MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, &MCP9808_REG_DEVICE_ID);    //request device ID
	if(flag == 0)
	{
		printf("Request for Device ID sent\n");
	}
	else 
	{
		printf("Error sending request, Error code: %d\n", flag);
	}

	//Response for device ID
	flag = Mcp2221_I2cRead(handle, sizeof(RxData), MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, RxData);	
	if(flag == 0)
	{
		printf("Device ID is %X\n", ByteCat2(RxData[1], RxData[0]));		//device ID = 0x0400
	}
	else 
	{
		printf("Error receiving device ID, Error code: %d\n", flag);
	}

	//Get Manufacturer ID from MCP9808
	flag = Mcp2221_I2cWrite(handle, 1, MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, &MCP9808_REG_MANUF_ID);    //request device ID
	if(flag == 0)
	{
		printf("Request for Manufacturer ID sent\n");
	}
	else 
	{
		printf("Error sending request, Error code: %d\n", flag);
	}

	//Read response for Manufacturer ID
	flag = Mcp2221_I2cRead(handle, sizeof(RxData), MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, RxData);	
	if(flag == 0)
	{
		printf("Manufacturer ID is %X\n", ByteCat2(RxData[1], RxData[0]));		//device ID = 0x0054
	}
	else 
	{
		printf("Error receiving Manufacturer ID, Error code: %d\n", flag);
	}
	
	//set resolution
	config[0] = MCP9808_REG_RESOLUTION;
	config[1] = 0x03;	//set resolution to 0.0625C 
	flag = Mcp2221_I2cWrite(handle, 2, MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, config);    
	if(flag == 0)
	{
		//printf("Request for  ambient temperature sent\n");
	}
	else 
	{
		printf("Error setting resolution, Error code: %d\n", flag);
	}

	//Create file to save log with todays date & time
	logFile = fopen("TemplogFile.txt","w+");

    while(1)
    {
		//Get Ambient Temperature
		flag = Mcp2221_I2cWrite(handle, 1, MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, &MCP9808_REG_AMBIENT_TEMP);    //request device ID
		if(flag == 0)
		{
			//printf("Request for  ambient temperature sent\n");
		}
		else 
		{
			printf("Error requesting ambient temperature, Error code: %d\n", flag);
		}

		//Read ambient temperature from MCP9808
		flag = Mcp2221_I2cRead(handle, 2, MCP9808_I2CADDR_DEFAULT, I2cAddr7bit, RxData);	//sizeof(RxData)
		if(flag == 0)
		{
			//printf("Ambient temperature 2's compliment: %X\n", ByteCat2(RxData[1], RxData[0]));				
		}
		else 
		{
			printf("Error getting  ambient temperature, Error code: %d\n", flag);
		}

		RxData[0] =	RxData[0] & 0x1F;	//Clear flags
		if(CheckBit(RxData[0], 4))	
		{
			RxData[0] = RxData[0] & 0x0F;	//clear sign bit
			AmbTemp = 256 - (float(RxData[0])*16) + (float(RxData[1])/16);
			printf("Ambient temperature is: %3.3lf C\n", AmbTemp);
		}
		else						//Ta > 0
		{
			AmbTemp = (float(RxData[0])*16) + (float(RxData[1])/16);
			printf("Ambient temperature is: %3.3lf C\n", AmbTemp);
		}

		_sleep(500);
    }
}
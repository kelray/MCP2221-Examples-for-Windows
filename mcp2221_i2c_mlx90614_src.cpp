/*	Project: Interfacing MLX90614 IR Temperature sensor with MCP2221 via SMBus
*	By: Karim El-Rayes
*	Created: April, 2017
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

//MLX90614 configuration
unsigned char MLX90614_I2CADDR = 0x5A;
// RAM
unsigned char MLX90614_RAWIR1 = 0x04;
unsigned char MLX90614_RAWIR2 = 0x05;
unsigned char MLX90614_TA = 0x06;
unsigned char MLX90614_TOBJ1 = 0x07;
unsigned char MLX90614_TOBJ2 = 0x08;
// EEPROM
unsigned char MLX90614_TOMAX = 0x20;
unsigned char MLX90614_TOMIN = 0x21;
unsigned char MLX90614_PWMCTRL = 0x22;
unsigned char MLX90614_TARANGE = 0x23;
unsigned char MLX90614_EMISS = 0x24;
unsigned char MLX90614_CONFIG = 0x25;
unsigned char MLX90614_ADDR = 0x0E;
unsigned char MLX90614_ID1 = 0x3C;
unsigned char MLX90614_ID2 = 0x3D;
unsigned char MLX90614_ID3 = 0x3E;
unsigned char MLX90614_ID4 = 0x3F;

//Temperature variables
double ObjTemp = 0;
double AmbTemp = 0;

//File logging variables
FILE * logFile = NULL;		//Create file to save temperature log

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
unsigned char SlaveAddress = 0x63;
unsigned char StartCmd = 0x00;
unsigned char StopCmd = 0x00;
unsigned char RxData[7] = {0};
unsigned int tempADCreading = 0;
unsigned char TxData[2];
float voltage = 0;

unsigned char DACBuffer[] = {0x11, 0xff, 0xff};
unsigned char counter = 0x00;
unsigned char MSBcounter = 0x00;
unsigned char LSBcounter = 0x00;
unsigned char config[8] = {0};
unsigned char SpecialConfig[] = {0x19, 0x9F};

//Functions prototypes
void ExitFunc();
unsigned int* ReadADC();
void Mcp2221_config();

void ExitFunc()
{
	Mcp2221_I2cCancelCurrentTransfer(handle);
	Mcp2221_Close(handle);
	//Mcp2221_Reset(handle);
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
    flag = Mcp2221_SetSpeed(handle, 46875);    //set I2C speed to 400 KHz
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
	
	//Create file to save log with todays date & time
	//logFile = fopen("logFile.txt","w+");

    while(1)
    {
		flag = Mcp2221_SmbusReadWord(handle, MLX90614_I2CADDR, I2cAddr7bit, 1, MLX90614_TOBJ1, RxData);
		if(flag == 0)
		{
			printf("Reading from SMBus device %X successful\n", MLX90614_I2CADDR);
			printf("Device response is %X %X\n", RxData[0], RxData[1]);
		}
		else
		{
			printf("Error reading from SMBus device: %d\n", flag);
			Mcp2221_I2cCancelCurrentTransfer(handle);
		}
		ObjTemp = (double)ByteCat2(RxData[0], RxData[1]);
		ObjTemp = (ObjTemp*0.02) - 0.01;
		ObjTemp = ObjTemp - 273.15;
		printf("Object temperature %04.2f\n", ObjTemp);
		_sleep(500);
    }
}
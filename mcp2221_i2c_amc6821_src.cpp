/*	Project: Interfacing INA219 power monitor with MCP2221 via I2C
*	By: Karim El-Rayes
*/

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <iso646.h>
#include <time.h>
#include "mcp2221_dll_um.h"
#include "bitsNbytesManup.h"	
#pragma comment(lib, "mcp2221_dll_um_x86.lib")

#define pi 22/7

unsigned char I2cAddr7bit = 1;
unsigned char I2cAddr8bit = 0;

//AMC6821
unsigned char AMC6821_I2CADDR_DEFAULT	= 0x18;  //0011000

unsigned char AMC6821_DCY_REG = 0x22;		//duty cycle register
unsigned char AMC6821_PWM_FREQ_REG = 0x20;	//Fan characteristic register

unsigned char AMC6821_PWM_TACH_CONFIG_REG_1 = 0x00;	
unsigned char AMC6821_PWM_TACH_CONFIG_REG_2 = 0x01;
unsigned char AMC6821_PWM_TACH_CONFIG_REG_3 = 0x3F;
unsigned char AMC6821_PWM_TACH_CONFIG_REG_4 = 0x04;

unsigned char AMC6821_ID_REG = 0x3D;
unsigned char AMC6821_MANUFAC_ID_REG = 0x3E;

//Global variables
unsigned int config = 0;
unsigned char ReadBuffer[2] = {0};
unsigned char WriteBuffer[2] = {0};
unsigned char* SplitBuffer = NULL;
unsigned char CalBuffer[3];
unsigned char ConfigBuffer[3];
unsigned char AMC6821_config1 = 0x00;
unsigned char AMC6821_config2 = 0x05;
unsigned char AMC6821_config3 = 0x82;
unsigned char AMC6821_config4 = 0x88;
unsigned char PWM_freq = 0x00;	//1khz
unsigned char DCY_value = 0;

//File logging variables
FILE * logFile;		//Create file to save temperature log

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


//Functions prototypes
void ExitFunc();
void Mcp2221_config();
unsigned int* ReadADC();

void ExitFunc()
{
	Mcp2221_I2cCancelCurrentTransfer(handle);
	Mcp2221_Close(handle);
	Mcp2221_Reset(handle);
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
        printf("Error setting I2C bus speed: %d\n", flag);   
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

	//Create logfile
	logFile = fopen("logFile.csv","w");
	int i = 0;

	//Get device ID, response is 0x21
	flag = Mcp2221_SmbusReadWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_ID_REG, RxData);	
	if(flag == 0)
	{
		printf("Reading from SMBus device %X successful\n", AMC6821_I2CADDR_DEFAULT);
		printf("Device ID is %X\n", RxData[0]);
		//system("pause");
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	//Get manufacturer ID, response is 0x49
	flag = Mcp2221_SmbusReadWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_MANUFAC_ID_REG, RxData);
	if(flag == 0)
	{
		printf("Reading from SMBus device %X successful\n", AMC6821_I2CADDR_DEFAULT);
		printf("Manufacturer ID is %X\n", RxData[0]);
		//system("pause");
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	
	//change PWM config - 1 
	flag = Mcp2221_SmbusWriteWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_PWM_TACH_CONFIG_REG_1, &AMC6821_config1);
	if(flag == 0)
	{
		printf("Writing to register %X, SMBus device %X successful\n", AMC6821_PWM_TACH_CONFIG_REG_1, AMC6821_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	//change PWM config - 2
	flag = Mcp2221_SmbusWriteWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_PWM_TACH_CONFIG_REG_2, &AMC6821_config2);
	if(flag == 0)
	{
		printf("Writing to register %X, SMBus device %X successful\n", AMC6821_PWM_TACH_CONFIG_REG_2, AMC6821_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	//change PWM config - 3
	flag = Mcp2221_SmbusWriteWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_PWM_TACH_CONFIG_REG_3, &AMC6821_config3);
	if(flag == 0)
	{
		printf("Writing to register %X, SMBus device %X successful\n", AMC6821_PWM_TACH_CONFIG_REG_3, AMC6821_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	//change PWM config - 4
	flag = Mcp2221_SmbusWriteWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_PWM_TACH_CONFIG_REG_4, &AMC6821_config4);
	if(flag == 0)
	{
		printf("Writing to register %X, SMBus device %X successful\n", AMC6821_PWM_TACH_CONFIG_REG_4, AMC6821_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	//Change PWM frequency
	PWM_freq = 0xB8;
	flag = Mcp2221_SmbusWriteWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_PWM_FREQ_REG, &PWM_freq);
	if(flag == 0)
	{
		printf("Writing to register %X, SMBus device %X successful\n", AMC6821_PWM_FREQ_REG, AMC6821_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error reading from SMBus device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
		system("pause");
	}

	 while(1)
    {
		//DCY_value = 0xFF;

		flag = Mcp2221_SmbusWriteWord(handle, AMC6821_I2CADDR_DEFAULT, I2cAddr7bit, 0, AMC6821_DCY_REG, &DCY_value);
		if(flag == 0)
		{
			printf("Writing to SMBus device %X successful, PWM value is %d\n", AMC6821_I2CADDR_DEFAULT, DCY_value);
			DCY_value++;
			//Change PWM value
			if(DCY_value == 256)
			{
				DCY_value = 0;
				//_sleep(1000);
			}
			_sleep(10);
		}
		else
		{
			printf("Error reading from SMBus device: %d\n", flag);
			Mcp2221_I2cCancelCurrentTransfer(handle);
			system("pause");
		}
    }
}
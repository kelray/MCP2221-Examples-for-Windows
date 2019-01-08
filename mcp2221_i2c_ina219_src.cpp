/*	Project: Interfacing INA219 power monitor with MCP2221 via I2C
*	By: Karim El-Rayes
*	Note: Inspired from the INA219 Arduino library by Adafruit 
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

//INA219 Configuration
//I2C ADDRESS/BITS
unsigned char INA219_I2CADDR_DEFAULT	= 0x40;    // 1000000[] = A0+A1=GND)
unsigned char INA219_READ				= 0x01;
unsigned char INA219_REG_CONFIG			= 0x00;

// CONFIG REGISTER[] = R/W
unsigned int INA219_CONFIG_RESET   =0x8000;  // Reset Bit
	
unsigned int INA219_CONFIG_BVOLTAGERANGE_MASK  =0x2000;  // Bus Voltage Range Mask
unsigned int INA219_CONFIG_BVOLTAGERANGE_16V   =0x0000;  // 0-16V Range
unsigned int INA219_CONFIG_BVOLTAGERANGE_32V   =0x2000;  // 0-32V Range
	
unsigned int INA219_CONFIG_GAIN_MASK	=0x1800;  // Gain Mask
unsigned int INA219_CONFIG_GAIN_1_40MV	=0x0000;  // Gain 1, 40mV Range
unsigned int INA219_CONFIG_GAIN_2_80MV	=0x0800;  // Gain 2, 80mV Range
unsigned int INA219_CONFIG_GAIN_4_160MV	=0x1000;  // Gain 4, 160mV Range
unsigned int INA219_CONFIG_GAIN_8_320MV	=0x1800;  // Gain 8, 320mV Range
	
unsigned int INA219_CONFIG_BADCRES_MASK		=0x0780;  // Bus ADC Resolution Mask
unsigned int INA219_CONFIG_BADCRES_9BIT		=0x0080;  // 9-bit bus res = 0..511
unsigned int INA219_CONFIG_BADCRES_10BIT	=0x0100;  // 10-bit bus res = 0..1023
unsigned int INA219_CONFIG_BADCRES_11BIT	=0x0200;  // 11-bit bus res = 0..2047
unsigned int INA219_CONFIG_BADCRES_12BIT	=0x0400;  // 12-bit bus res = 0..4097

unsigned int INA219_CONFIG_SADCRES_MASK				=0x0078;  // Shunt ADC Resolution and Averaging Mask
unsigned int INA219_CONFIG_SADCRES_9BIT_1S_84US		=0x0000;  // 1 x 9-bit shunt sample
unsigned int INA219_CONFIG_SADCRES_10BIT_1S_148US	=0x0008;  // 1 x 10-bit shunt sample
unsigned int INA219_CONFIG_SADCRES_11BIT_1S_276US	=0x0010;  // 1 x 11-bit shunt sample
unsigned int INA219_CONFIG_SADCRES_12BIT_1S_532US	=0x0018;  // 1 x 12-bit shunt sample
unsigned int INA219_CONFIG_SADCRES_12BIT_2S_1060US	=0x0048;	 // 2 x 12-bit shunt samples averaged together
unsigned int INA219_CONFIG_SADCRES_12BIT_4S_2130US	=0x0050;  // 4 x 12-bit shunt samples averaged together
unsigned int INA219_CONFIG_SADCRES_12BIT_8S_4260US	=0x0058;  // 8 x 12-bit shunt samples averaged together
unsigned int INA219_CONFIG_SADCRES_12BIT_16S_8510US	=0x0060;  // 16 x 12-bit shunt samples averaged together
unsigned int INA219_CONFIG_SADCRES_12BIT_32S_17MS	=0x0068;  // 32 x 12-bit shunt samples averaged together
unsigned int INA219_CONFIG_SADCRES_12BIT_64S_34MS	=0x0070;  // 64 x 12-bit shunt samples averaged together
unsigned int INA219_CONFIG_SADCRES_12BIT_128S_69MS	=0x0078;  // 128 x 12-bit shunt samples averaged together
	
unsigned int INA219_CONFIG_MODE_MASK					=0x0007;  // Operating Mode Mask
unsigned int INA219_CONFIG_MODE_POWERDOWN				=0x0000;
unsigned int INA219_CONFIG_MODE_SVOLT_TRIGGERED			=0x0001;
unsigned int INA219_CONFIG_MODE_BVOLT_TRIGGERED			=0x0002;
unsigned int INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED		=0x0003;
unsigned int INA219_CONFIG_MODE_ADCOFF					=0x0004;
unsigned int INA219_CONFIG_MODE_SVOLT_CONTINUOUS		=0x0005;
unsigned int INA219_CONFIG_MODE_BVOLT_CONTINUOUS		=0x0006;
unsigned int INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS	=0x0007;

//SHUNT VOLTAGE REGISTER[] = R
unsigned char INA219_REG_SHUNTVOLTAGE = 0x01;

//BUS VOLTAGE REGISTER[] = R    
unsigned char INA219_REG_BUSVOLTAGE = 0x02;

//POWER REGISTER[] = R
unsigned char INA219_REG_POWER = 0x03;

//CURRENT REGISTER[] = R
unsigned char INA219_REG_CURRENT = 0x04;

//CALIBRATION REGISTER[] = R/W
unsigned char INA219_REG_CALIBRATION = 0x05;
unsigned int ina219_CalVal = 10240;		//Rshunt=0.1 ohm, 1A setting

//for 0.1 ohm, 1A
float ina219_currentDivider_mA = 25;			//Current LSB = 100uA per bit (1000/100 = 10) 
float ina219_powerDivider_mW = 1;				//Power LSB = 1mW per bit (2/1) 

//for 0.1 ohm, 2A
//float ina219_currentDivider_mA = 10;			//Current LSB = 100uA per bit (1000/100 = 10) 
//float ina219_powerDivider_mW = 2;				//Power LSB = 1mW per bit (2/1) 

//Global variables
unsigned int config = 0;
unsigned char ReadBuffer[2] = {0};
unsigned char WriteBuffer[2] = {0};
unsigned char* SplitBuffer = NULL;
unsigned char CalBuffer[3];
unsigned char ConfigBuffer[3];

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
void INA219_Config();
double INA219_ReadCurrent();
double INA219_ReadBusVoltage();
double INA219_ReadShuntVoltage();

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

void INA219_Config()
{	
	//write Calibration value 
	SplitBuffer = SplitInt16(ina219_CalVal);
	CalBuffer[0] = INA219_REG_CALIBRATION;
	CalBuffer[1] = SplitBuffer[1];
	CalBuffer[2] = SplitBuffer[0];
	
	flag = Mcp2221_I2cWrite(handle, sizeof(CalBuffer), INA219_I2CADDR_DEFAULT, I2cAddr7bit, CalBuffer);    //issue start condition then address
	if(flag == 0)
	{
		printf("Writing to device %X successful\n", INA219_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error writing to I2C device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
	}

	//printf("config = %X\n", config);
	printf("CalBuffer is: [0]%X [1]%X [2]%X\n", CalBuffer[0], CalBuffer[1], CalBuffer[2]);
	
	//configuration for Vbus=32v and Imax=1A
	config =		INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
	
	SplitBuffer = SplitInt16(config);
	ConfigBuffer[0] = INA219_REG_CONFIG;
	ConfigBuffer[1] = SplitBuffer[1];
	ConfigBuffer[2] = SplitBuffer[0];
	printf("config = %X\n", config);
	printf("ConfigBuffer is: [0]%X [1]%X [2]%X\n", ConfigBuffer[0], ConfigBuffer[1], ConfigBuffer[2]);
	
	flag = Mcp2221_I2cWrite(handle, sizeof(ConfigBuffer), INA219_I2CADDR_DEFAULT, I2cAddr7bit, ConfigBuffer);    //issue start condition then address
	if(flag == 0)
	{
		printf("Writing to device %X successful\n", INA219_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error writing to I2C device: %d\n", flag);
		//Mcp2221_I2cCancelCurrentTransfer(handle);
	}
}

double INA219_ReadCurrent()
{
	double Current_mA = 0.0;
	int Current_raw = 0;
	
	//Request raw current
	flag = Mcp2221_I2cWrite(handle, sizeof(INA219_REG_CURRENT), INA219_I2CADDR_DEFAULT, I2cAddr7bit, &INA219_REG_CURRENT);    //issue start condition then address
	if(flag == 0)
	{
		//printf("Writing to device %X successful\n", INA219_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error writing to I2C device: %d\n", flag);
		//Mcp2221_I2cCancelCurrentTransfer(handle);
	}
	_sleep(100);
	
	flag = Mcp2221_I2cRead(handle, sizeof(ReadBuffer), INA219_I2CADDR_DEFAULT, I2cAddr7bit, ReadBuffer); //sizeof(RxData)
	if(flag == 0)
	{
		Current_raw = ByteCat2(ReadBuffer[1], ReadBuffer[0]);
		Current_mA = Current_raw/ina219_currentDivider_mA;
		//printf("ReadBuffer[0]: %X, ReadBuffer[1]: %X\n", ReadBuffer[0], ReadBuffer[1]);
		printf("Raw current: %d, Current: %.2lf mA\n", Current_raw, Current_mA);
	}
	else 
	{
		printf("Error receiving raw current: %d\n", flag);
	}

	//Mcp2221_I2cCancelCurrentTransfer(handle);
	return Current_mA;
}

double INA219_ReadBusVoltage()
{
	double Voltage = 0.0;
	int RawVoltage = 0;
	
	//Request bus voltage
	flag = Mcp2221_I2cWrite(handle, sizeof(INA219_REG_BUSVOLTAGE), INA219_I2CADDR_DEFAULT, I2cAddr7bit, &INA219_REG_BUSVOLTAGE);    //issue start condition then address
	if(flag == 0)
	{
		//printf("Writing to device %X successful\n", INA219_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error writing to I2C device: %d\n", flag);
		Mcp2221_I2cCancelCurrentTransfer(handle);
	}
	//_sleep(50);
	flag = Mcp2221_I2cRead(handle, sizeof(ReadBuffer), INA219_I2CADDR_DEFAULT, I2cAddr7bit, ReadBuffer); //sizeof(RxData)
	if(flag == 0)
	{
		RawVoltage = (ByteCat2(ReadBuffer[1], ReadBuffer[0])>>3)*4;
		Voltage = RawVoltage * 0.001;
		printf("Raw Voltage: %d, Bus Voltage: %2.3f V\n", RawVoltage, Voltage);	
	}
	else 
	{
		printf("Error receiving raw current: %d\n", flag);
	}
	//Mcp2221_I2cCancelCurrentTransfer(handle);
	return Voltage;
}

double INA219_ReadShuntVoltage()
{
	double ShuntVoltage = 0.0;
	int ShuntVoltageRaw = 0;

	//Request bus voltage
	flag = Mcp2221_I2cWrite(handle, sizeof(INA219_REG_SHUNTVOLTAGE), INA219_I2CADDR_DEFAULT, I2cAddr7bit, &INA219_REG_SHUNTVOLTAGE);    //issue start condition then address
	if(flag == 0)
	{
		//printf("Writing to device %X successful\n", INA219_I2CADDR_DEFAULT);
	}
	else
	{
		printf("Error writing to I2C device: %d\n", flag);
		//Mcp2221_I2cCancelCurrentTransfer(handle);
	}
	//_sleep(50);
	flag = Mcp2221_I2cRead(handle, sizeof(ReadBuffer), INA219_I2CADDR_DEFAULT, I2cAddr7bit, ReadBuffer); //sizeof(RxData)
	if(flag == 0)
	{
		ShuntVoltageRaw = ByteCat2(ReadBuffer[1], ReadBuffer[0]);
		ShuntVoltage = ShuntVoltageRaw * 0.01;
		//printf("Raw shunt voltage = %X\n", ShuntVoltageRaw);
		//printf("Shunt voltage is = %3.3lf mV\n", ShuntVoltage);
	}
	else 
	{
		printf("Error receiving raw current: %d\n", flag);
	}
	//Mcp2221_I2cCancelCurrentTransfer(handle);
	return ShuntVoltage;
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
    flag = Mcp2221_SetAdvancedCommParams(handle, 1, 100);  //1ms timeout, try 1000 times
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
	//fprintf(logFile, "Current, ");
	//fprintf(logFile, "Bus Voltage, \n");
	
	//Configure INA219
	INA219_Config();

	int i = 0;
    while(1)
    {
		//INA219_ReadBusVoltage();
		//INA219_ReadCurrent();
		//INA219_ReadShuntVoltage();
		//_getch();
		//printf("Current is: %3.2lf mA\n", INA219_ReadCurrent());
		
		float tmp = (INA219_ReadShuntVoltage()/0.56);
		if(i++ % 10 == 0)
			printf("Current is: %6.2lf A\n", tmp);
		fprintf(logFile, "%6.2lf,\n", tmp);
		//fprintf(logFile, "%f,\n", INA219_ReadCurrent());
		//fprintf(logFile, "%f,\n", INA219_ReadBusVoltage());
		//_sleep(1000);
    }
}
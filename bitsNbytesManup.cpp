/* 
 * File:   bitsNbytesManup.c
 * Author: Karim El-Rayes
 * Created on August 19, 2016
 * Disclaimer: this code is available for public use 
 * use it on your own responsibility and liability, 
 * The developer doesn't claim any liability for this code.
 * You can distribute and share the code with anyone without 
 * any prior permissions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitsNbytesManup.h"

//Two bytes concatenation function
//unsigned int ByteCat2(unsigned char lowByte, unsigned char highByte, unsigned int PlacestoShift)
unsigned int ByteCat2(unsigned char lowByte, unsigned char highByte)
{
	unsigned int MSB = 0;
	unsigned int LSB = 0;
	unsigned int temp = 0;
	unsigned int value = 0;

	MSB = highByte;
	LSB = lowByte;
	temp = MSB << 8;	//<< PlacestoShift;
	value = temp | LSB;
	return value;
}

//Three bytes concatenation function
unsigned int ByteCat3(unsigned char lowByte, unsigned char middleByte, unsigned char highByte)
{
	unsigned int HiByte = 0;
	unsigned int MidByte = 0;
	unsigned int LoByte = 0;
	unsigned int temp = 0;
	unsigned int value = 0;

	HiByte = highByte;
	MidByte = middleByte;
	LoByte = lowByte;

	temp = HiByte << 8;
	temp = (temp | MidByte) << 8;
	value = temp | LoByte;
	return value;
}

//Four bytes concatenation function
unsigned int ByteCat4(unsigned char Byte0, unsigned char Byte1, unsigned char Byte2, unsigned char Byte3)
{
	unsigned int byte0 = 0;
	unsigned int byte1 = 0;
	unsigned int byte2 = 0;
	unsigned int byte3 = 0;

	unsigned int temp = 0;
	unsigned int value = 0;

	byte0 = Byte0;
	byte1 = Byte1;
	byte2 = Byte2;
	byte3 = Byte3;

	temp = byte3 << 8;
	temp = (temp | byte2) << 8;
	temp = (temp | byte1) << 8;
	value = temp | byte0;

	return value;
}

//Check if bit is 0 or 1
bool CheckBit(unsigned int number, unsigned int position)
{
	unsigned int temp = 0;
	unsigned int comp = 0x01;
	bool value = false;

	temp = number >> position;
	if(temp & comp)
		value = true;
	else
		value = false;
	return value;
}

//Sets a specific bit to 1
unsigned int SetBit(unsigned int number, unsigned int position)
{
	unsigned int temp = 0x01;
	unsigned int value = 0; 

	temp = temp << position;
	value = temp | number;
	return value;
}

//Resets a specific bit to 0
unsigned int ResetBit(unsigned int number, unsigned int position)
{
	unsigned int temp = 0x01;
	unsigned int value = 0; 

	temp = ~(temp << position);
	//temp = temp & 0xff;
	value = temp & number;
	return value;
}

//Get the Two's complement of a number
unsigned int TwoComplement(unsigned int number, unsigned int SizeInBits)
{
	unsigned int value = 0;
	value = ~number;
	value = (value + 0x01) << 32-SizeInBits;	//shift right to remove the extra 1's, SizeInBits can be any value between 0 and 32
	value = value >> 32-SizeInBits;				//shift left to return the number to LSB position
	return value;
}
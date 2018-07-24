/*
 *	Автор: Bitfixer (bitfixer.com, bitfixer@bitfixer.com)
 *	Модифицировал: Dubos (www.dubos.ru, dubos1210@yandex.ru)
 * 
 *	Программа является свободным программным обеспечением:
 *	Вы можете распространять и/или модифицировать его согласно 
 *	GNU General Public License версии 3 или новее. 
 *
 *	Программа распространяется с надеждой, что она будет полезной,
 *	но БЕЗ ВСЯКИХ ГАРАНТИЙ со стороны авторов.
 */

/*
    sd_routines.h
    SD Routines in the PETdisk storage device
    Copyright (C) 2011 Michael Hill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    Contact the author at bitfixer@bitfixer.com
    http://bitfixer.com
    
    SD routines inspired by CC Dharmani's microcontroller blog
    http://dharmanitech.com    
*/

#ifndef SDCARD
#define SDCARD 1

#include "SPI.h"

#define SD_CS_ASSERT     PORTB &=~ (1<<2)
#define SD_CS_DEASSERT   PORTB |= (1<<2)

//SD commands, many of these are not used here
#define GO_IDLE_STATE            0
#define SEND_OP_COND             1
#define SEND_IF_COND			 8
#define SEND_CSD                 9
#define STOP_TRANSMISSION        12
#define SEND_STATUS              13
#define SET_BLOCK_LEN            16
#define READ_SINGLE_BLOCK        17
#define READ_MULTIPLE_BLOCKS     18
#define WRITE_SINGLE_BLOCK       24
#define WRITE_MULTIPLE_BLOCKS    25
#define ERASE_BLOCK_START_ADDR   32
#define ERASE_BLOCK_END_ADDR     33
#define ERASE_SELECTED_BLOCKS    38
#define SD_SEND_OP_COND			 41   //ACMD
#define APP_CMD					 55
#define READ_OCR				 58
#define CRC_ON_OFF               59

volatile unsigned long _startBlock, _totalBlocks;
volatile unsigned char _SDHC_flag, _cardType, SDbuffer[512];

/******************************************************************
 * Функция		: отправка команды
 * Аргументы	: команда (unsigned char)
 *  			  аргумент (unsigned long)
 * Ответ		: ответ карты (unsigned char)
 *******************************************************************/
unsigned char SD_sendCommand(unsigned char cmd, unsigned long arg) {
	unsigned char response, retry=0, status;

	//SD card accepts byte address while SDHC accepts block address in multiples of 512
	//so, if it's SD card we need to convert block address into corresponding byte address by
	//multipying it with 512. which is equivalent to shifting it left 9 times
	//following 'if' loop does that

	if(_SDHC_flag == 0)
		if( cmd == READ_SINGLE_BLOCK     ||
			cmd == READ_MULTIPLE_BLOCKS  ||
			cmd == WRITE_SINGLE_BLOCK    ||
			cmd == WRITE_MULTIPLE_BLOCKS ||
			cmd == ERASE_BLOCK_START_ADDR||
			cmd == ERASE_BLOCK_END_ADDR ) {
		arg = arg << 9;
	}

	SD_CS_ASSERT;

	SPI_byteTxRx(cmd | 0x40); //send command, first two bits always '01'
	SPI_byteTxRx(arg>>24);
	SPI_byteTxRx(arg>>16);
	SPI_byteTxRx(arg>>8);
	SPI_byteTxRx(arg);

	if(cmd == SEND_IF_COND)	 //it is compulsory to send correct CRC for CMD8 (CRC=0x87) & CMD0 (CRC=0x95)
		SPI_byteTxRx(0x87);    //for remaining commands, CRC is ignored in SPI mode
	else
		SPI_byteTxRx(0x95);

	while((response = SPI_byteTxRx(0xFF)) == 0xff) //wait response
	if(retry++ > 0xfe) break; //time out error

	if(response == 0x00 && cmd == 58) { //checking response of CMD58
		status = SPI_byteTxRx(0xFF) & 0x40;     //first byte of the OCR register (bit 31:24)
		if(status == 0x40) _SDHC_flag = 1;  //we need it to verify SDHC card
		else _SDHC_flag = 0;

		SPI_byteTxRx(0xFF); //remaining 3 bytes of the OCR register are ignored here
		SPI_byteTxRx(0xFF); //one can use these bytes to check power supply limits of SD
		SPI_byteTxRx(0xFF);
	}

	SPI_byteTxRx(0xFF); //extra 8 CLK
	SD_CS_DEASSERT;

	return response; //return state
}

/******************************************************************
 * Функция	: инициализация карты памяти
 * Ответ	: 0, если инициализация прошла успешно
 *******************************************************************/
unsigned char SD_init(void) {
    unsigned char i, response, SD_version;
    unsigned int retry = 0 ;

    for(i = 0; i < 10; i++)
        SPI_byteTxRx(0xff);   //80 clock pulses spent before sending the first command

    SD_CS_ASSERT;
    do {
		response = SD_sendCommand(GO_IDLE_STATE, 0); //send 'reset & go idle' command
		retry++;
		if(retry>0x20) 
			return 1;   //time out, card not detected
    } while(response != 0x01);

    SD_CS_DEASSERT;
    SPI_byteTxRx (0xff);
    SPI_byteTxRx (0xff);

    retry = 0;

    SD_version = 2; //default set to SD compliance with ver2.x; 
                    //this may change after checking the next command
    do {
		response = SD_sendCommand(SEND_IF_COND,0x000001AA); //Check power supply status, mendatory for SDHC card
		retry++;
		if(retry>0xfe) {
			SD_version = 1;
			_cardType = 1;
			break;
		} //time out
    } while(response != 0x01);

    retry = 0;

    do {
		response = SD_sendCommand(APP_CMD,0); //CMD55, must be sent before sending any ACMD command
		response = SD_sendCommand(SD_SEND_OP_COND,0x40000000); //ACMD41

		retry++;
		if(retry>0xfe) {
			return 2;  //time out, card initialization failed
		}
    } while(response != 0x00);

    retry = 0;
    _SDHC_flag = 0;

    if (SD_version == 2) { 
		do {
			response = SD_sendCommand(READ_OCR,0);
			retry++;
			if(retry>0xfe) { 
				_cardType = 0;
				break;
			} //time out
		} while(response != 0x00);

		if(_SDHC_flag == 1) _cardType = 2;
		else _cardType = 3;
    }

    //SD_sendCommand(CRC_ON_OFF, OFF); //disable CRC; default - CRC disabled in SPI mode
    //SD_sendCommand(SET_BLOCK_LEN, 512); //set block size to 512; default size is 512

    return 0; //successful return
}

/******************************************************************
 * Функция		: чтение одного сектора (512 байт) в буфер SDbuffer[512]
 * Аргументы	: номер сектора (unsigned long)
 * Ответ		: 0, если успех
 *******************************************************************/
unsigned char SD_readSingleBlock(unsigned long startBlock) {
	unsigned char response;
	unsigned int i, retry=0;

	response = SD_sendCommand(READ_SINGLE_BLOCK, startBlock); //read a Block command
	if(response != 0x00) return response; //check for SD status: 0x00 - OK (No flags set)

	SD_CS_ASSERT;

	retry = 0;
	while(SPI_byteTxRx(0xFF) != 0xfe) //wait for start block token 0xfe (0x11111110)
		if(retry++ > 0xfffe) {
			SD_CS_DEASSERT;
			return 1; //return if time-out
		}

	for(i=0; i<512; i++) //read 512 bytes
	SDbuffer[i] = SPI_byteTxRx(0xFF);

	SPI_byteTxRx(0xFF); //receive incoming CRC (16-bit), CRC is ignored here
	SPI_byteTxRx(0xFF);

	SPI_byteTxRx(0xFF); //extra 8 clock pulses
	SD_CS_DEASSERT;

	return 0;
}

/******************************************************************
 * Функция		: запись одного сектора (512 байт) из буфера SDbuffer[512]
 * Аргументы	: номер сектора (unsigned long)
 * Ответ		: 0, если успех
 *******************************************************************/
unsigned char SD_writeSingleBlock(unsigned long startBlock) {
    unsigned char response;
    unsigned int i, retry=0;

    response = SD_sendCommand(WRITE_SINGLE_BLOCK, startBlock); //write a Block command
    if(response != 0x00)
		return response; //check for SD status: 0x00 - OK (No flags set)

    SD_CS_ASSERT;

    SPI_byteTxRx(0xfe);     //Send start block token 0xfe (0x11111110)

    for(i=0; i<512; i++)    //send 512 bytes data
        SPI_byteTxRx(SDbuffer[i]);

    SPI_byteTxRx(0xff);     //transmit dummy CRC (16-bit), CRC is ignored here
    SPI_byteTxRx(0xff);

    response = SPI_byteTxRx(0xFF);

    if( (response & 0x1f) != 0x05) { //response= 0xXXX0AAA1 ; AAA='010' - data accepted                              //AAA='101'-data rejected due to CRC error
        SD_CS_DEASSERT;              //AAA='110'-data rejected due to write error
        return response;
    }

    while(!SPI_byteTxRx(0xFF)) //wait for SD card to complete writing and get idle
        if(retry++ > 0xfffe) {
            SD_CS_DEASSERT;
            return 1;
        }

    SD_CS_DEASSERT;
    SPI_byteTxRx(0xff);   //just spend 8 clock cycle delay before reasserting the CS line
    SD_CS_ASSERT;         //re-asserting the CS line to verify if card is still busy

    while(!SPI_byteTxRx(0xFF)) //wait for SD card to complete writing and get idle
        if(retry++ > 0xfffe) {
            SD_CS_DEASSERT;
            return 1;
        }
		
    SD_CS_DEASSERT;

    return 0;
}


/******************************************************************
 * Функция		: запись строки
 * Аргументы	: номер сектора (unsigned long)
 *				  строка для записи
 *				  символ для заполнения свободного места в секторе
 * Ответ		: 0, если успех
 *******************************************************************/
unsigned char SD_writeString(unsigned long startBlock, const char *data, unsigned char fill) {
	unsigned char response, c;
	unsigned int a, b, retry=0;

	response = SD_sendCommand(WRITE_SINGLE_BLOCK, startBlock); //write a Block command
	if(response != 0x00)
		return response; //check for SD status: 0x00 - OK (No flags set)

	SD_CS_ASSERT;

	SPI_byteTxRx(0xfe);     //Send start block token 0xfe (0x11111110)
	
	a = 0;
	while(( c = *data++ )) {
		SPI_byteTxRx(c);
		a++;
	}
	for(b = 0; b < (512 - a); b++) {
		SPI_byteTxRx(fill);
	}

	SPI_byteTxRx(0xff);     //transmit dummy CRC (16-bit), CRC is ignored here
	SPI_byteTxRx(0xff);

	response = SPI_byteTxRx(0xFF);

	if( (response & 0x1f) != 0x05) { //response= 0xXXX0AAA1 ; AAA='010' - data accepted                              //AAA='101'-data rejected due to CRC error
		SD_CS_DEASSERT;              //AAA='110'-data rejected due to write error
		return response;
	}

	while(!SPI_byteTxRx(0xFF)) //wait for SD card to complete writing and get idle
	if(retry++ > 0xfffe) {
		SD_CS_DEASSERT;
		return 1;
	}

	SD_CS_DEASSERT;
	SPI_byteTxRx(0xff);   //just spend 8 clock cycle delay before reasserting the CS line
	SD_CS_ASSERT;         //re-asserting the CS line to verify if card is still busy

	while(!SPI_byteTxRx(0xFF)) //wait for SD card to complete writing and get idle
	if(retry++ > 0xfffe) {
		SD_CS_DEASSERT;
		return 1;
	}
	
	SD_CS_DEASSERT;

	return 0;
}


#endif
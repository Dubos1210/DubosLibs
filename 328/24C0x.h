/* 24C01/24C02 external EEPROM Library for Atmel AVR v0.1
 * EXTEEPROM_writeByte(0b10101110, 0x00, 0xFF);  //Запись значения 0xFF в ячейку 0x00 микросхемы с адресом 0b10101110
 * EXTEEPROM_readByte(0b10101110, 0x00);		 //Чтение из ячейки 0x00 микросхемы 0b10101110
 * 
 * Адрес микросхемы: старшие четыре бита - 1010, затем уровни на выводах A2, A1 и A0, затем ноль.
 * Например: A2 подключен к нулю; A1, A0 подключены к питанию. Тогда адрес - 0b10100110
 *
 * Нога WP: ноль - запись разрешена; единица - запись запрещена
 */

#ifndef DUBOS_EXTEEPROM
#define DUBOS_EXTEEPROM

#include "I2C.h"

/***************************************
 * Функция		: запись байта
 * Аргументы	: адрес микросхемы,
				  адрес ячейки памяти,
				  байт для записи
 ****************************************/
void EXTEEPROM_writeByte(unsigned char ic_addr, unsigned char addr, unsigned char data) {
	I2C_start();
	I2C_transmit(ic_addr);
	I2C_transmit(addr);
	I2C_transmit(data);
	I2C_stop();
}

/***************************************
 * Функция		: запись байта
 * Аргументы	: адрес микросхемы,
				  адрес ячейки памяти
 * Ответ		: считанный байт
 ****************************************/
unsigned char EXTEEPROM_readByte(unsigned char ic_addr, unsigned char addr) {
	unsigned char data = 0;
	I2C_start();
	I2C_transmit(ic_addr);
	I2C_transmit(addr);
	I2C_start();
	I2C_transmit(ic_addr + 1);
	data = I2C_receive();
	I2C_stop();
	
	return data;
}

#endif
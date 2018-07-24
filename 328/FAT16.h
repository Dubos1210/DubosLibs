#ifndef DUBOS-FAT16
#define DUBOS-FAT16

#include "SDcard.h"

volatile unsigned char SectorPerCluster, NumberOfFATs;
volatile unsigned int BytesPerSector, ReservedSectors, RootEntries, SectorPerFAT;

char str[16];

/******************************************************************
 * Функция	: чтение блока служебной информации
 * Ответ	: 0, если успех
 *******************************************************************/
unsigned char SD_readMBR(void) {	
	if(SD_readSingleBlock(0x0000) == 0) {
		BytesPerSector = SDbuffer[11] + (SDbuffer[12] << 8);
		SectorPerCluster = SDbuffer[13];
		ReservedSectors = SDbuffer[14] + (SDbuffer[15] << 8);
		NumberOfFATs = SDbuffer[16];
		RootEntries = SDbuffer[17] + (SDbuffer[18] << 8);
		SectorPerFAT = SDbuffer[22] + (SDbuffer[23] << 8);
	}
	else {
		return 1;
	}
	
	return 0;
}

/******************************************************************
 * Функция	 : поиск файла в корневом каталоге
 * Аргументы : имя файла, расширение файла
 * Ответ	 : 0, если ошибка
 *			   адрес первого сектора, если успех
 *******************************************************************/
unsigned int SD_searchFile(unsigned char name[8], unsigned char type[3]) {
	if(SD_readMBR() != 0) {
		return 1;
	}
	int SD_a = 0; 
	while(SD_a < (RootEntries/16)) {
		if(SD_readSingleBlock(0x01E8 + SD_a) != 0) {
			return 1;
		}
		
		for(int SD_i = 0; SD_i < 16; SD_i++) {
			if(SDbuffer[0 + (32*SD_i)] == name[0] &&
			SDbuffer[1 + (32*SD_i)] == name[1] &&
			SDbuffer[2 + (32*SD_i)] == name[2] &&
			SDbuffer[3 + (32*SD_i)] == name[3] &&
			SDbuffer[4 + (32*SD_i)] == name[4] &&
			SDbuffer[5 + (32*SD_i)] == name[5] &&
			SDbuffer[6 + (32*SD_i)] == name[6] &&
			SDbuffer[7 + (32*SD_i)] == name[7] &&
			SDbuffer[8 + (32*SD_i)] == type[0] &&
			SDbuffer[9 + (32*SD_i)] == type[1] &&
			SDbuffer[10 + (32*SD_i)] == type[2]) {
				return (SDbuffer[26 + (32*SD_i)] + (SDbuffer[27 + (32*SD_i)] << 8));
			}
		}
		SD_a++;
	}
	if(SD_readSingleBlock(0x01E8 + SD_a) != 0) {
		return 1;
	}
	for(int SD_i = 0; SD_i < (RootEntries%16); SD_i++) {
		if(SDbuffer[0 + (32*SD_i)] == name[0] &&
		SDbuffer[1 + (32*SD_i)] == name[1] &&
		SDbuffer[2 + (32*SD_i)] == name[2] &&
		SDbuffer[3 + (32*SD_i)] == name[3] &&
		SDbuffer[4 + (32*SD_i)] == name[4] &&
		SDbuffer[5 + (32*SD_i)] == name[5] &&
		SDbuffer[6 + (32*SD_i)] == name[6] &&
		SDbuffer[7 + (32*SD_i)] == name[7] &&
		SDbuffer[8 + (32*SD_i)] == type[0] &&
		SDbuffer[9 + (32*SD_i)] == type[1] &&
		SDbuffer[10 + (32*SD_i)] == type[2]) {
			return (SDbuffer[26 + (32*SD_i)] + (SDbuffer[27 + (32*SD_i)] << 8));
		}
	}
	
	return 0;
}

/******************************************************************
 * Функция	 : чтение файла в корневом каталоге
 * Аргументы : имя файла, расширение файла,
 *			   номер страницы файла (нумерация с 0)
 * Ответ	 : 0, если успех
 *******************************************************************/
unsigned char SD_readFilePage(unsigned char name[8], unsigned char type[3], unsigned int page) {
	int SD_addr = SD_searchFile(name, type);
	if(SD_addr == 0) {
		return 1;
	}
	SD_addr = ReservedSectors + NumberOfFATs*SectorPerFAT + (RootEntries >> 4) + ((SD_addr - 2) * SectorPerCluster) + page;
	if(SD_readSingleBlock(SD_addr) != 0) {
		return 1;		
	}
	
	return 0;
}

/******************************************************************
 * Функция	 : запись файла в корневом каталоге
 * Аргументы : имя файла, расширение файла,
 *			   номер страницы файла (нумерация с 0), 
 *             строка для записи, байт-заполнитель
 * Ответ	 : 0, если успех
 *******************************************************************/
unsigned char SD_writeFilePage(unsigned char name[8], unsigned char type[3], unsigned int page) {
	int SD_addr = SD_searchFile(name, type);
	if(SD_addr == 0) {
		return 1;
	}
	SD_addr = ReservedSectors + NumberOfFATs*SectorPerFAT + (RootEntries >> 4) + ((SD_addr - 2) * SectorPerCluster) + page;
	if(SD_writeSingleBlock(SD_addr) != 0) {
		return 1;		
	}
	
	return 0;
}

/******************************************************************
 * Функция	 : запись файла в корневом каталоге
 * Аргументы : имя файла, расширение файла,
 *			   номер страницы файла (нумерация с 0), 
 *             строка для записи, байт-заполнитель
 * Ответ	 : 0, если успех
 *******************************************************************/
unsigned char SD_writeStringToFilePage(unsigned char name[8], unsigned char type[3], unsigned int page, const char *data, unsigned int fill) {
	int SD_addr = SD_searchFile(name, type);
	if(SD_addr == 0) {
		return 1;
	}
	SD_addr = ReservedSectors + NumberOfFATs*SectorPerFAT + (RootEntries >> 4) + ((SD_addr - 2) * SectorPerCluster) + page;
	if(SD_writeString(SD_addr, data, fill) != 0) {
		return 1;		
	}
	
	return 0;
}

/******************************************************************
 * Функция	 : очиста корневого каталога, 
 *             создание файла размером 8МБ
 * Ответ	 : 0, если успех
 *******************************************************************/
unsigned char SD_format(void) {
	unsigned int SD_a, oldSD_a, SD_c;
	
	if(SD_readMBR() != 0) {
		return 1;
	}
	
	for(SD_a = 0; SD_a < 512; SD_a++) {
		SDbuffer[SD_a] = 0x00;
	}
	
	SD_a = 1;
	while(SD_a < (RootEntries/16)) {
		if(SD_writeSingleBlock(0x01E8 + SD_a) != 0) {
			return 1;
		}
		SD_a++;
	}
	
	if(RootEntries%16 > 0) {
		if(SD_writeSingleBlock(0x01E8 + SD_a) != 0) {
			return 1;
		}
	}
	//Очистили корневой каталог
	
	SD_a = 18;
	while(SD_a < SectorPerFAT) {
		if(SD_writeSingleBlock(0x0004 + SD_a) != 0) {
			return 1;
		}
		SD_a++;
	}
	//Очистили FAT1
	
	SD_a = 18;
	while(SD_a < SectorPerFAT) {
		if(SD_writeSingleBlock(0x00F6 + SD_a) != 0) {
			return 1;
		}
		SD_a++;
	}
	//Очистили FAT2
	
	SDbuffer[0]  = 0x30;
	SDbuffer[1]  = 0x20;
	SDbuffer[2]  = 0x20;
	SDbuffer[3]  = 0x20;
	SDbuffer[4]  = 0x20;
	SDbuffer[5]  = 0x20;
	SDbuffer[6]  = 0x20;
	SDbuffer[7]  = 0x20;	// Имя файла
	SDbuffer[8]  = 0x54;
	SDbuffer[9]  = 0x58;
	SDbuffer[10] = 0x54;	// Расширение файла
	SDbuffer[11] = 0x00;	// Флаги
	SDbuffer[12] = 0x00;	// Резерв
	SDbuffer[13] = 0x00;	// Сотые доли секунды времени создания файла
	SDbuffer[14] = 0x17;
	SDbuffer[15] = 0x60;	// Время создания файла
	SDbuffer[16] = 0xE1;	
	SDbuffer[17] = 0x4C;	// Дата создания файла
	SDbuffer[18] = 0xE1;	
	SDbuffer[19] = 0x4C;	// Дата последнего обращения к файлу
	SDbuffer[20] = 0x00;	
	SDbuffer[21] = 0x00;	// Старшее слово адреса первого кластера файла
	SDbuffer[22] = 0x17;	
	SDbuffer[23] = 0x60;	// Время последнего изменения
	SDbuffer[24] = 0xE1;	
	SDbuffer[25] = 0x4C;	// Дата последнего изменения
	SDbuffer[26] = 0x04;	
	SDbuffer[27] = 0x00;	// Младшее слово адреса первого кластера файла
	SDbuffer[28] = 0x00;	
	SDbuffer[29] = 0x00;	
	SDbuffer[30] = 0x80;	
	SDbuffer[31] = 0x00;	// Размер файла
	
	if(SD_writeSingleBlock(0x01E8) != 0) {
		return 1;
	}
	
	//Создали файл
	
	SDbuffer[0]  = 0xF8;
	SDbuffer[1]  = 0xFF;
	SDbuffer[2]  = 0xFF;
	SDbuffer[3]  = 0xFF;
	SDbuffer[4]  = 0xFF;
	SDbuffer[5]  = 0xFF;
	SDbuffer[6]  = 0xFF;
	SDbuffer[7]  = 0xFF;
	
	for(SD_a = 8; SD_a < 511; SD_a++) {
		SDbuffer[SD_a] = SD_a/2 + 1;
		SDbuffer[SD_a + 1] = (SD_a/2 + 1) >> 8;
		
		SD_a++;
	}
	if(SD_writeSingleBlock(0x0004) != 0) {
	return 1;
	}
	oldSD_a = SD_a;
	for(SD_c = 1; SD_c < (128/SectorPerCluster); SD_c++) {
		
		for(SD_a = 0; SD_a < 510; SD_a++) {
			SDbuffer[SD_a] = SD_a/2 + 1;
			SDbuffer[SD_a + 1] = SD_c;
			
			SD_a++;
		}
		SDbuffer[510] = 0x00;
		SDbuffer[511] = SD_c + 1;
		if(SD_writeSingleBlock(0x0004 + SD_c) != 0) {
			return 1;
		}
	}
	for(SD_a = 0; SD_a < 512; SD_a++) {
		SDbuffer[SD_a] = 0x00;
	}	
	SDbuffer[0]  = 0x01;
	SDbuffer[1]  = 0x10;
	SDbuffer[2]  = 0x02;
	SDbuffer[3]  = 0x10;
	SDbuffer[4]  = 0xFF;
	SDbuffer[5]  = 0xFF;
	SDbuffer[6]  = 0xFF;
	SDbuffer[7]  = 0xFF;
	
	//Записали в FAT1
	
	SDbuffer[0]  = 0xF8;
	SDbuffer[1]  = 0xFF;
	SDbuffer[2]  = 0xFF;
	SDbuffer[3]  = 0xFF;
	SDbuffer[4]  = 0xFF;
	SDbuffer[5]  = 0xFF;
	SDbuffer[6]  = 0xFF;
	SDbuffer[7]  = 0xFF;
	
	for(SD_a = 8; SD_a < 511; SD_a++) {
		SDbuffer[SD_a] = SD_a/2 + 1;
		SDbuffer[SD_a + 1] = (SD_a/2 + 1) >> 8;
		
		SD_a++;
	}
	if(SD_writeSingleBlock(0x00F6) != 0) {
		return 1;
	}
	for(SD_c = 1; SD_c < (128/SectorPerCluster); SD_c++) {
		
		for(SD_a = 0; SD_a < 510; SD_a++) {
			SDbuffer[SD_a] = SD_a/2 + 1;
			SDbuffer[SD_a + 1] = SD_c;
			
			SD_a++;
		}
		SDbuffer[510] = 0x00;		
		SDbuffer[511] = SD_c + 1;
		if(SD_writeSingleBlock(0x00F6 + SD_c) != 0) {
			return 1;
		}
	}
	//Записали в FAT2
	
	return 0;
}

#endif
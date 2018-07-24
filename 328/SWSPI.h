/* Software SPI Library for Atmel AVR v0.3
 *  SWSPI_init();         //Инициализация SPI
 *  SWSPI_byteTxRx(0x**); //Отправка байта 0x**. Возвращает принятые данные
 *  
 *  Не забудьте отконфигурировать выводы и настройки в строках #define
 */

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#ifndef DUBOS_SWSPI
#define DUBOS_SWSPI

#define SWSPI_DELAY		5		//Четверть периода SCK, мкс

#define SPI_PORT		PORTB	//PORT регистр
#define SPI_PIN			PINB	//PIN регистр
#define SPI_DDR			DDRB	//DDR регистр

#define SPI_MOSI		3		//Вывод MOSI
#define SPI_MISO		4		//Вывод MISO
#define SPI_SCK			5		//Вывод SCK
#define SPI_CS			2		//Вывод CS


#define SWSPI_SelectChip()			SPI_PORT &=~ (1<<SPI_CS)
#define SWSPI_DeselectChip()		SPI_PORT |= (1<<SPI_CS)

/*********************************************
 * Функция	: инициализация программного SPI
 *********************************************/
void SWSPI_init(void) {
	SPI_DDR |= (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_CS);
	SPI_DDR &=~ (1<<SPI_MISO);
	
	SPI_PORT |= (1<<SPI_CS)|(1<<SPI_MOSI);
	SPI_PORT &=~ (1<<SPI_SCK);
}

/*********************************************
 * Функция		: отправка и прием байта
 * Аргументы	: байт для отправки
 * Ответ		: принятый байт
 *********************************************/
unsigned char SWSPI_byteTxRx(unsigned char data) {
	unsigned char answ = 0x00;
	for(int SWSPI_i = 0; SWSPI_i < 8; SWSPI_i++) {
		if(data & (1<<7)) {			
			SPI_PORT |= (1<<SPI_MOSI);
		}
		else {			
			SPI_PORT &=~ (1<<SPI_MOSI);
		}
		data = data << 1;
		_delay_us(SWSPI_DELAY);
		_delay_us(SWSPI_DELAY);
		SPI_PORT |= (1<<SPI_SCK);
		_delay_us(SWSPI_DELAY);
		answ += SPI_PIN & SPI_MISO;
		answ = answ << 1;
		_delay_us(SWSPI_DELAY);
		SPI_PORT &=~ (1<<SPI_SCK);
	}
}

#endif
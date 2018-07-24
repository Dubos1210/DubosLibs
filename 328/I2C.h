/* Software I2C (TWI) Library for Atmel AVR (v0.3)
 *  I2C_start();        //Отправка сигнала "СТАРТ"
 *  I2C_transmit(0x**); //Отправка байта 0x**
 *  I2C_receive();      //Прием байта
 *  I2C_send_ack();     //Отправка сигнала ACK
 *  I2C_send_nack();    //Отправка сигнала NACK
 *  I2C_stop();         //Отправка сигнала "СТОП"
 *  
 *  Не забудьте установить резисторы сопротивлением 4.7k-10k между выводами SDA и Vcc, SCL и Vcc
 *  
 *  Не забудьте отконфигурировать выводы и настройки в строках #define и в функции I2C_start  
 */

#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#ifndef DUBOS_I2C
#define DUBOS_I2C

#define SDA_in() DDRC &=~ (1<<4) //SDA - вход (высокий уровень на шине благодаря внешней подтяжке)
#define SDA_out() DDRC |= (1<<4) //SDA - выход
#define SDA_pin() PINC & (1<<4)  //Проверка состояния SDA

#define SCL_in() DDRC &=~ (1<<5) //SCL - вход (высокий уровень на шине благодаря внешней подтяжке)
#define SCL_out() DDRC |= (1<<5) //SCL - выход

/***************************************
 * Функция	 : отправка сигнала "СТАРТ"
 ***************************************/
void I2C_start(void) {
	PORTC &=~ (1<<4);              //Низкий уровень на SDA, если выход
	PORTC &=~ (1<<5);              //Низкий уровень на SCL, если выход
	SDA_in();
	_delay_us(1);
	SCL_in();
	_delay_us(2);
	SDA_out();
	_delay_us(2);
	SCL_out();
}

/***************************************
 * Функция	 : отправка байта
 * Аргументы : байт для отправки
 ***************************************/
void I2C_transmit(unsigned char msg) {
	for(int i = 0; i < 8; i++) {
		if(msg & (1<<7)) {			
			SDA_in();
		}
		else {			
			SDA_out();
		}
		_delay_us(1);
		SCL_in();
		_delay_us(1);
		SCL_out();
		_delay_us(1);
		msg = msg << 1;
	}	
	SCL_in();
  SDA_in();
	_delay_us(1);
	while(SDA_pin()) {
		_delay_us(1);
	};
	_delay_us(1);
	SCL_out();
	_delay_us(1);
	SDA_out();
	_delay_us(1);
}

/***************************************
 * Функция	 : прием байта
 * Ответ	 : полученный байт
 ***************************************/
char I2C_receive(void) {
	char msg = 0;
	SDA_in();
	for(int i = 0; i < 7; i++) {
		SCL_in();
		_delay_us(4);
		if(SDA_pin()) {
			msg = msg + 1;
		}
		SCL_out();
		msg = msg << 1;
	}
	SCL_in();
	_delay_us(4);
	if(SDA_pin()) {
		msg = msg + 1;
	}
	SCL_out();
	return msg;
}

/***************************************
 * Функция	 : отправка сигнала "ACK"
 ***************************************/
void I2C_send_ack(void) {
	SDA_out();
  SCL_in();
	_delay_us(2);
  SCL_out();
  SDA_in();
}

/***************************************
 * Функция	 : отправка сигнала "NACK"
 ***************************************/
void I2C_send_nack(void) {
	SDA_in();
	SCL_in();
	_delay_us(2);
	SCL_out();
}

/***************************************
 * Функция	 : отправка сигнала "СТОП"
 ***************************************/
void I2C_stop(void) {
	SDA_out();
	_delay_us(2);
	SCL_in();
	_delay_us(2);
	SDA_in();
}

#endif
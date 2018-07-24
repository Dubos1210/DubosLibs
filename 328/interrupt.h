/* Interrupts Library for Atmel AVR ATmega328P v0.2
 *  INT0_start(FALLING); //Включение прерывания INT0 по спаду
 *  INT0_stop();         //Выключение прерывания INT0
 *  INT1_start(RISING);  //Включение прерывания INT1 по фронту
 *  INT1_stop();         //Выключение прерывания INT1
 */

#ifndef F_CPU
  #define F_CPU 16000000UL
#endif

#ifndef dubos-int
#define dubos-int

#define LOW_LEVEL 0
#define ANY_CHANGE 1
#define FALLING 2
#define RISING 3

/* ISR(INT0_vect) {
  INT0_stop();
  _delay_ms(150);
  INT0_start(0xFF);
} */

/* ISR(INT1_vect) {
  INT1_stop();
  _delay_ms(150);
  INT1_start(0xFF);
} */

/***************************************************************
 * Функция	 : запуск прерывания INT0
 * Аргументы : событие (LOW_LEVEL, ANY_CHANGE, FALLING, RISING)
 * 			   0xFF - просто запуск без изменения настроек
 ***************************************************************/
void INT0_start(unsigned char event) {
  if(event <= 0x03) {
    EICRA &=~ (1<<ISC01)|(1<<ISC00);
    EICRA |= (event & 0b00000011);
  }
  EIMSK |= (1<<INT0);
  EIFR |= (INTF0);
}

/***************************************************************
 * Функция	 : выключение прерывания INT0
 ***************************************************************/
void INT0_stop(void) {
  EIMSK &=~ (1<<INT0);
}

/***************************************************************
 * Функция	 : запуск прерывания INT1
 * Аргументы : событие (LOW_LEVEL, ANY_CHANGE, FALLING, RISING)
 * 			   0xFF - просто запуск без изменения настроек
 ***************************************************************/
void INT1_start(unsigned char event) {
  if(event <= 0x03) {
    EICRA &=~ (1<<ISC11)|(1<<ISC10);
    EICRA |= ((event & 0b00000011) << 2);
  }
  EIMSK |= (1<<INT1);
  EIFR |= (INTF1);
}

/***************************************************************
 * Функция	 : выключение прерывания INT0
 ***************************************************************/
void INT1_stop(void) {
  EIMSK &=~ (1<<INT1);
}

#endif
DubosLibs
=========

Набор библиотек, написанных (или модифицированных) мной для моих нужд. Некоторые из них используются в цикле уроков ["Arduino по-взрослому. Си и иже с ним"](http://www.dubos.ru/arduino-lessons.html)

------------

### Папка 328
Библиотеки, подходящие для использования с микроконтроллером __Atmel AVR ATmega328 *(Arduino)*__
	
  + __24C0x.h__ - библиотека для работы с микросхемами EEPROM-памяти *T24C0* (AT24C01 и пр.)
  ```c	
  EXTEEPROM_writeByte(0b10101110, 0x00, 0xFF); //Запись значения 0xFF в ячейку 0x00 микросхемы с адресом 0b10101110
  EXTEEPROM_readByte(0b10101110, 0x00);		   //Чтение из ячейки 0x00 микросхемы 0b10101110
  ```
  Адрес микросхемы: старшие четыре бита - 1010, затем уровни на выводах A2, A1 и A0, затем ноль.
  Например: A2 подключен к нулю; A1, A0 подключены к питанию. Тогда адрес - 0b10100110
  
  Вывод WP: ноль - запись разрешена; единица - запись запрещена

  + __ADC.h__ - библиотека для работы с АЦП
  ```c	
  ADC_init();     //Инициализация АЦП
  ADC_read(ADC0); //Измерение сигнала на входе ADC0
  ```
  В качестве источника сигнала могут быть использованы:
  
  *ADC0* - PC0
  
  *ADC1* - PC1
  
  *ADC2* - PC2
  
  *ADC3* - PC3
  
  *ADC4* - PC4
  
  *ADC5* - PC5
  
  *ADC_TEMPERATURE* - встроенный термометр
  
  *ADC_REFERENCE* - источник опороного напряжения
  
  *ADC_GND* - просто 0
  	
  + __DS18B20.h__ - драйвер датчиков температуры от Dallas Semiconductor. 
  
  ```c
  DS18B20_init();        //Reset-импульс (возвращает 0xFF, если датчик ответил)
  DS18B20_write(0x**);   //Отправка байта 0x**
  DS18B20_read();        //Чтение байта с шины
  DS18B20_temperature(); //Чтение температуры
  ```
  Не забудьте установить резистор сопротивлением 4.7k-10k между выводами DQ и Vdd датчика. 
  
  Не забудьте отконфигурировать выводы и настройки в строках #define и в функции DS18B20_init

  Перед началом работы нужно записать конфигурационные байты:
  ```c
  DS18B20_write(SKIP_ROM);
  DS18B20_write(WRITE_SCRATCHPAD);
  DS18B20_write(0x7F);
  DS18B20_write(0x7F);
  DS18B20_write(0x7F);
  ```
  
  
  + __FAT16.h__ - упрощенный драйвер файловой системы FAT16. Требуется библиотека SDcard.h
  ```c
  SD_readMBR();                           //Чтение служебной информации о карте памяти. Считанные данные складываются в соответствующие переменные. Возвращает 0, если успех
  SD_searchFile("0       ", "txt");       //Поиск файла 0.txt в корневом каталоге (имя файла - ровно 8 символов, расширение - 3 символа). Возвращает номер первого кластера файла
  SD_readFilePage("0       ", "txt", 0);  //Чтение нулевой страницы (первые 512 байт) файла 0.txt в корневом каталоге (имя файла - ровно 8 символов, расширение - 3 символа).  Прочитанные данные складываются в буфер SDbuffer.	Возвращает 0, если успех
  SD_writeFilePage("0       ", "txt", 0); //Запись нулевой страницы (первые 512 байт) файла 0.txt в корневом каталоге (имя файла - ровно 8 символов, расширение - 3 символа).  Данные для записи берутся из SDbuffer.	Возвращает 0, если успех
  SD_writeStringToFilePage("0       ", "txt", 0, "Hello World! ", " "); //Запись строки (Hello Word! ) в нулевую страницу файла 0.txt в корневом каталоге (имя файла - ровно 8 символов, расширение - 3 символа). Оставшееся на странице место заполняется символом " ".	Возвращает 0, если успех
  SD_format();                            //Очистка корневого каталога и FAT-таблиц, создание файла размером 8МБ. Возвращает 0, если успех
  ```
  
  + __I2C.h__ - программная реализация протокола связи I2C (IIC, TWI).
  ```c
  I2C_start();        //Отправка сигнала "СТАРТ"
  I2C_transmit(0x**); //Отправка байта 0x**
  I2C_receive();      //Прием байта
  I2C_send_ack();     //Отправка сигнала ACK
  I2C_send_nack();    //Отправка сигнала NACK
  I2C_stop();         //Отправка сигнала "СТОП"
  ```  
  Не забудьте установить резисторы сопротивлением 4.7k-10k между выводами SDA и Vcc, SCL и Vcc
  
  Не забудьте отконфигурировать выводы и настройки в строках #define и в функции I2C_start
  
  + __interrupt.h__ - функции управления внешними прерываниями. *В будущем добавлю обработку прерываний PCINT*
  ```c
  INT0_start(FALLING); //Запуск прерывания INT0 по спаду (по заднему фронту) сигнала
  INT1_start(RISING);  //Запуск прерывания INT0 по переднему фронту сигнала
  INT0_stop();         //Выключение прерывания INT0
  INT1_stop();         //Выключение прерывания INT1
  ```
  Не забудьте разрешить прерывания функцией __sei()__
  	
  + __SDcard.h__ - драйвер для работы с картами памяти SD (до 2ГБ). Требуется библиотека SPI.h
  ```c
  SD_sendCommand();      //Отпрака команды (первый аргумент - команда, второй - аргумент). Возвращает ответ карты.
  SD_init();             //Инициализация карты памяти. Возвращает 0, если успех
  SD_readSingleBlock();  //Чтение одного сектора. Аргумент - номер сектора. Возвращает 0, если успех. Считанные данные складываются в буфер SDbuffer
  SD_writeSingleBlock(); //Запись одного сектора. Аргумент - номер сектора. Возвращает 0, если успех. Данные для записи берутся из SDbuffer
  SD_writeString();      //Запись строки (первый аргумент - номер сектора, второй - строка, третий - символ заполнитель). Строка записывается в один сектор, оставшеется место заполняется байтом-заполнителем. 
  ```
  
  + __SPI.h__ - библиотека для работы с аппаратным SPI.
  ```c
  SPI_init();         //Инициализация SPI
  SPI_byteTxRx(0x**); //Отправка байта 0x**. Возвращает принятые данные
  ```
  Не забудьте отконфигурировать выводы и настройки в строках #define и в функции SPI_init  
 
  + __usart.h__ - библиотека для работы с апаратным UART
  ```c
  USART_init(9600);           //Инициализация USART на скорости 9600 бод
  USART_send(0x**);           //Отправка байта 0x**
  USART_send_BK();            //Отправка сигнала "Возврат каретки" (0x0D 0x0A)
  USART_send_string("Hello"); //Отправка строки Hello
  USART_send_integer(123);    //Отправка числа 0123 в виде ASCII-символов (макс. - 9999)
  USART_send_long(12345);     //Отправка большого числа в виде ASCII-символов
  USART_send_decimal(123, 1); //Отправка десятичной дроби 12,3 в виде ASCII-символов
  ```
  
------------

Обратная связь
------------
- Сайт автора: [www.dubos.ru](http://www.dubos.ru) 
- Электропочта: [dubos1210@yandex.ru](mailto:dubos1210@yandex.ru)
- ВКонтакте: [Владимир Дубишкин](http://vk.com/dubosru)
- Pikabu: [Dubos](https://pikabu.ru/profile/Dubos)
- YouTube: [Vladimir Dubos](https://www.youtube.com/channel/UCkUERCY1I5nALXeckgTSK9w "Vladimir Dubos")
- Instagram: [@vladimirdub](http://instagram.com/vladimirdub)


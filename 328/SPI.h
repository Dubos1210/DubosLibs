/* SPI Library for Atmel AVR (v0.1)
 *  SPI_init();     //������������ SPI
 *  SPI_data(0x**); //�������� ����� 0x**. ���������� �������� ������
 *  
 *  �� �������� ����������������� ������ � ��������� � ������� #define � � ������� SPI_config_pins
*/

#ifndef SPI
#define SPI 1

void SPI_init(void) {            //������������ �������
	DDRB |= (1<<5);                //SCK - �����
	DDRB &=~ (1<<4);               //MISO - ����
	DDRB |= (1<<3);                //MOSI - �����
	DDRB |= (1<<2);                //SS - �����
  
	SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(1<<SPR1)|(0<<SPR0);
	/*
		SPIE � ��������� /��������� ���������� �� ������ SPI. ���� ��� ���������� � 1, ���������� �� SPI ���������.
		SPE � ��������/��������� ������ SPI. ���� ��� ���������� � 1, ������ SPI �������.
		DORD � ���������� ������� �������� ������. ����� ��� ���������� � 1, ���������� �������� ������ ���������� ������� ����� ������. ����� ��� �������, �� ������� ����� ������.
		MSTR � ���������� ����� ������ ����������������. ���� ��� ���������� � 1, ��������������� �������� � ������ Master (�������). ���� ��� ������� � � ������ Slave (�������). ������ ��������������� �������� � ������ master.
		CPOL � CPHA � ���������� � ����� ������ �������� SPI ������. ��������� ����� ������ ������� �� ������������� ������������� ����������.
		 Mode  |  CPOL |  CPHA
		-------|-------|-------
		Mode 0 |   0   |   0
		Mode 1 |   0   |   1
		Mode 2 |   1   |   0
		Mode 3 |   1   |   1
		SPR1 � SPR0 � ���������� ������� ��������� ������� SPI ������, �� ���� �������� ������. ����������� ��������� �������� ������ ������ ����������� � ������������ ������������� ����������.
		    Fsck    |  SPR1 |  SPR0
		------------|-------|-------
		F_CPU/4     |   0   |   0
		F_CPU / 16  |   0   |   1
		F_CPU / 64  |   1   |   0
		F_CPU / 128 |   1   |   1
	*/
	SPSR = (0<<SPIF)|(0<<WCOL)|(0<<SPI2X);
	/*
		���� SPIF ��������� �� ���������� �������� � �������� ������ �� ���������� (���� ���������� ���������)
		���� WCOL ��������������� � ������, ����� �� ����� �������� ������ ��������������� ������� ������ � ������� ������ SPI
		SPI2X - ��������� �������� �������� ������
	*/
}

unsigned char SPI_byteTxRx(unsigned char data) {
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	data = SPDR;
	
	return(data);
}

#endif
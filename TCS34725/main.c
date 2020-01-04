
#define F_CPU   8000000     // 20.MHz


#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "i2c_master.h"

#define LED          PB1
#define DELAY        500

#define FOSC 8000000 /* MCUｸﾛｯｸ周波数 */
#define BAUD 38400 /* 目的USARTﾎﾞｰﾚｰﾄ速度 */
#define MYUBRR FOSC/16/BAUD-1 /* 目的UBRR値 */

#define SCL 400000
#define TCS34725_WRITE 0x52
#define TCS34725_READ 0x53

// see p.21 of datasheet
#define TCS34725_COMMAND 1<<7
#define TCS34725_AUTO_INCREMENT 1<<5

void USART_Init(unsigned int baud)
{
	UBRRH = (unsigned char)(baud>>8); /* ﾎﾞｰﾚｰﾄ設定(上位ﾊﾞｲﾄ) */
	UBRRL = (unsigned char)baud; /* ﾎﾞｰﾚｰﾄ設定(下位ﾊﾞｲﾄ) */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0); /* ﾌﾚｰﾑ形式設定(8ﾋﾞｯﾄ,2停止ﾋﾞｯﾄ) */
	UCSRB = (1<<RXEN)|(1<<TXEN); /* 送受信許可 */
}


void USART_Transmit(unsigned int data)
{
	while ( !(UCSRA & (1<<UDRE)) ); /* 送信緩衝部空き待機 */
	UCSRB &= ~(1<<TXB8); /* TXB8を0に仮設定 */
	if (data & 0x0100) UCSRB |= (1<<TXB8); /* 第9ﾋﾞｯﾄをR17からTXB8へ複写 */
	UDR = data; /* ﾃﾞｰﾀ送信(送信開始) */
}


void USART_puts(char *s)
{
	while(*s){
		USART_Transmit(*s);
		s++;
	}
}

char buffer[6];
int16_t BUFFER_TCS34725;


void init_TCS34725(void)
{
	i2c_start(TCS34725_WRITE);
	i2c_write(TCS34725_COMMAND | 0x00); 
	i2c_write(0x03); 
	i2c_stop();

	i2c_start(TCS34725_WRITE);
	i2c_write(TCS34725_COMMAND | 0x01);
	i2c_write(0xF6);
	i2c_stop();

	i2c_start(TCS34725_WRITE);
	i2c_write(TCS34725_COMMAND | 0x0F); 
	i2c_write(0x00);
	i2c_stop();
}


void read_TCS34725(uint8_t address)
{
	i2c_start(TCS34725_WRITE);
	i2c_write(TCS34725_COMMAND | TCS34725_AUTO_INCREMENT | address); 
	i2c_start(TCS34725_READ);
	BUFFER_TCS34725 = (uint16_t)(i2c_read_nack() << 8);

	i2c_start(TCS34725_WRITE);
	i2c_write(TCS34725_COMMAND | TCS34725_AUTO_INCREMENT | (address - 1)); 
	i2c_start(TCS34725_READ);
	BUFFER_TCS34725 |= (uint16_t)(i2c_read_nack());

	i2c_stop();
}
                   


int main(void)
{
    i2c_init(0x02); // ( 1 / (SCL / F_CPU) - 16 ) / 2
	USART_Init(MYUBRR); /* USART初期化 */;
	init_TCS34725();

	DDRB |= _BV(LED);
	_delay_ms(DELAY);

	while(1)
	{
		read_TCS34725(0x15);
		itoa(BUFFER_TCS34725, buffer, 10);
		USART_puts(buffer);
		USART_puts(",");

		read_TCS34725(0x17);
		itoa(BUFFER_TCS34725, buffer, 10);
		USART_puts(buffer);
		USART_puts(",");

		read_TCS34725(0x19);
		itoa(BUFFER_TCS34725, buffer, 10);
		USART_puts(buffer);
		USART_puts(",");

		read_TCS34725(0x1B);
		itoa(BUFFER_TCS34725, buffer, 10);
		USART_puts(buffer);
		USART_puts("\r\n");
		
		PORTB ^= _BV(LED);
		_delay_ms(DELAY);
	}
}

/*
 * Test1.c
 *
 * Created: 18.01.2017 18:17:18
 * Author : ОлександрСавчук
 */ 

#define F_CPU 8000000UL

#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint8_t rx_data=0, rx_flag, ost=5, rx_data2 = 0;
bool firt_data = true;

#define  MY_UDR UDR; 
static uint8_t UART_RX_BUF[128] ={0}; // Uart RX buffer ;
static uint8_t Uart_Rx_Index = 0; // Uart RX index buffer;

#define LED_YELLOW_PORT PORTC
#define LED_YELLOW_PIN PINC0

#define LED_RED_PORT PORTC
#define LED_RED_PIN PINC1

#define LED_GREEN_PORT PORTB
#define LED_GREEN_PIN PINB0


/*
	Package start bit define
*/
typedef enum
{ 
	START_BIT = 0x55,
}PACKAGE_BEGIN;

#define datasize 16 //max data size, max package size = datasize + 3

/*
	Package structure
*/
typedef struct 
{
	PACKAGE_BEGIN Start_Bit;
	uint8_t cmd;
	uint8_t length;
	uint8_t data[datasize];
}Package;

Package package_tx; //package for tx data
Package package_rx; //package for rx data

void send_UART(uint8_t value) {    // Uart byte send function  
	while(!( UCSRA & (1 << UDRE)));   // Ожидаем когда очистится буфер передачи
	// UDR = value; // Помещаем данные в буфер, начинаем передачу
	UDR = value ;
}

/**
* @brief Transfer packege  
*
* @param package
* @retval true if backage created
*/
bool TX_Package(Package *pack)
{
	if(pack->cmd != 0 && pack->length != 0)
	{
		send_UART(0x55);
		send_UART(pack->length);
		send_UART(pack->cmd);
		for(int i = 0; i < pack->length - 1; i++)
		{
			send_UART(pack->data[i]);
		}
		return true;
	}else
	{
							LED_RED_PORT &= ~(1<<LED_RED_PIN);
							_delay_ms(2000);
							LED_RED_PORT |= (1<<LED_RED_PIN);
							_delay_ms(2000);
		return false;
	}
}
/*
 @brief Get received data to RX buffer
*/
ISR(USART_RXC_vect) {
	uint8_t buff = MY_UDR;
	UART_RX_BUF[Uart_Rx_Index] = buff;
	Uart_Rx_Index++;
}

/**
* @brief Get rx package from rx buffer
*
* @param package RX
* @retval true if package created
*/
bool RX_Package(Package *pack)
{
	if(UART_RX_BUF[0] == 0x55)
	{
		pack->length = UART_RX_BUF[1];
		pack->cmd = UART_RX_BUF[2];
		for(int i = 3, j= 0; j < pack->length - 1; i++, j++)
			pack->data[j] = UART_RX_BUF[i];
		Uart_Rx_Index = 0;
		return true;
	}
	Uart_Rx_Index = 0;
	return false;
}

  void initUART()
  {
	  UBRRH = 0;
	  UBRRL = 51;
	  UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	  UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //8 bit, 1 stop bit
  }


int main(void)
{
	DDRC |=(1<<PINC1) | (1<<PINC0) ;    //Порт C піни 0 і 1 як вихід
	DDRB |=(1<<PINB0);

	initUART();
	PORTC |= (1<<PINC2);   // для кнопок
	PORTC |= (1<<PINC3);
	PORTB |= (1<<PINB1);
	PORTB |= (1<<PINB2);


	PORTB |= (1<<PINB0);
	_delay_ms(100); // Задержка
	PORTB &= ~(1<<PINB0);
	_delay_ms(100); // Задержка

	sei();	

	
	time_t t;
	/* Intializes random number generator */
	srand((unsigned) time(&t));

	int blok = 0x10;
	int word_bloc= 0x0;

	int servise = 0x20;
	int word_servise= 0;

	int perevirca = 0x11;
	int word_perevirca= 0x23;

	int truvoga=0x13;
	int word_truvoga=0x33;

	int chek = 0x33;
	int word_chek = 0x33;


    while(1)
    {
		
	// Обробка кнопки (тривога)
		 if ((PINC&(1 << PC2)) == 0)
		 {
				package_tx.cmd = truvoga;
				package_tx.length = 0x03;
				package_tx.data[0] = word_truvoga;
				package_tx.data[1] = word_truvoga;
				TX_Package(&package_tx); 

				_delay_ms(200); // Задержка
		 }

	// обробка кннопки (перевірка)
		  if ((PINC&(1 << PC3)) == 0)
		  {
				package_tx.cmd = perevirca;
				package_tx.length = 0x03;
				package_tx.data[0] = word_perevirca;
				package_tx.data[1] = word_perevirca;
				TX_Package(&package_tx);
				_delay_ms(200); // Задержка

		  }
		

		// обробка кннопки (сервіс)
		if ((PINB&(1 << PB1)) == 0)
		{

			LED_GREEN_PORT |= (1<<LED_GREEN_PIN);
			_delay_ms(200); // Задержка
			LED_GREEN_PORT &= ~(1<<LED_GREEN_PIN);
			_delay_ms(200); // Задержка
			LED_GREEN_PORT |= (1<<LED_GREEN_PIN);
			_delay_ms(200); // Задержка
			LED_GREEN_PORT &= ~(1<<LED_GREEN_PIN);
			_delay_ms(200); // Задержка
			word_servise=rand() % 50;
			
			package_tx.cmd = servise;
			package_tx.length = 0x03;
			package_tx.data[0] = word_servise;
			package_tx.data[1] = word_servise;
			word_perevirca=package_rx.data[1];	
			TX_Package(&package_tx);

			_delay_ms(200); // Задержка
		}

		// обробка кннопки (блок)
		if ((PINB&(1 << PB2)) == 0)
		{
			LED_GREEN_PORT |= (1<<LED_GREEN_PIN);
			_delay_ms(200); // Задержка
			LED_GREEN_PORT &= ~(1<<LED_GREEN_PIN);
			_delay_ms(200); // Задержка
		

			package_tx.cmd = blok;
			package_tx.length = 0x03;
			package_tx.data[0] = word_truvoga;
			package_tx.data[1] = word_truvoga;
			TX_Package(&package_tx);

			_delay_ms(200); // Задержка
		}

		 // молодша частина
		if(Uart_Rx_Index >= 3)
		{
		 _delay_ms(10);
		 RX_Package(&package_rx);

			if (package_rx.cmd == blok)
			{
				word_truvoga=0;
				word_perevirca=0;
				_delay_ms(90000);
			}

			if (package_rx.cmd == servise)
			{
				word_perevirca=package_rx.data[1];	
			}
			
			if (package_rx.cmd == chek)
			{
				package_tx.cmd = chek;
				package_tx.length = 0x03;
				package_tx.data[0] = word_chek;
				package_tx.data[1] = word_chek;
				TX_Package(&package_tx);
				_delay_ms(200); // Задержка
			}
			
			if (package_rx.cmd == perevirca)
			{

			

				if(package_rx.data[1] == word_perevirca)
				{
					LED_YELLOW_PORT &= ~(1<<LED_YELLOW_PIN);
					_delay_ms(200); // Задержка
					LED_YELLOW_PORT |= (1<<LED_YELLOW_PIN);
					_delay_ms(200); // Задержка

				}
			}			
			if (package_rx.cmd == truvoga)
			{
				if(package_rx.data[1] == word_truvoga)
				{
					LED_RED_PORT &= ~(1<<LED_RED_PIN);
					_delay_ms(200); // Задержка
					LED_RED_PORT |= (1<<LED_RED_PIN);
					_delay_ms(200);	// Задержка
					LED_RED_PORT &= ~(1<<LED_RED_PIN);
					_delay_ms(200); // Задержка
					LED_RED_PORT |= (1<<LED_RED_PIN);
					_delay_ms(200);	// Задержка
				}
			}
		}
	}
}


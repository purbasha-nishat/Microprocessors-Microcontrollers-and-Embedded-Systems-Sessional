/*
 * RFID Module.c
 *
 * Created: 6/18/2021 7:44:12 PM
 * Author : USER
 */ 
#define F_CPU 1000000
#define D4 eS_PORTB4
#define D5 eS_PORTB5
#define D6 eS_PORTB6
#define D7 eS_PORTB7
#define RS eS_PORTB0
#define EN eS_PORTB1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd.h"

int total_id = 7;
int id_len = 10;
unsigned char valid_id[7][10] = {
	"1501020303",
	"1501020304",
	"1501020305",
	"1501020306",
	"1501020307",
	"1501020308",
	"1501020309",
};

uint16_t voted[7] = {0,0,0,0,0,0,0};
uint16_t voteflag = 0;

void UART_init()
{	
	
	UCSRB = 0b00011000;// Turn on the reception and Transmission
	UCSRC = 0b10000110;
	//UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);// Use 8-bit character sizes
	UCSRA = 0b00000000;
	UCSRA |= (1 << RXC) | (1<<TXC);
	// | (0<<TXC)|(0<<UDRE)|(0<<FE)|(0<<DOR)|(0<UPE)|(0<<U2X)|(0<<MPCM)

	//UBRRL = BAUD_PRESCALE;		// Load lower 8-bits of the baud rate
	//UBRRH = (BAUD_PRESCALE >> 8);	// Load upper 8-bits
	UBRRL = 0x19;
	UBRRH = 0x00;
}

void ADC_init()
{
	ADMUX = 0b11000000;
	ADCSRA = 0b10000111;
}

float ADC_Read()
{
	float value;
	float volt;
	ADCSRA |= (1 << ADSC);
	while( ADCSRA & (1 << ADSC));
	value = ADCL  | ( 0b00000011 & ADCH) << 8;
	volt = value*2.56/1024; //V
	volt = volt*100; //temp
	return volt;
}
unsigned char UART_RxChar()
{
	while ((UCSRA & (1 << RXC)) == 0x00);// Wait till data is received
	return UDR;		// Return the byte
}

void UART_TxChar(unsigned char data){
	while((UCSRA & (1 << UDRE)) == 0x00);
	UDR = data;
	
}

volatile int wait = 0;
ISR(INT1_vect){
	
	wait = 0;

}



int main(void)
{
   float temp_F;
   char Fahrenheit[5];	
   DDRD = 0b01110110;
   DDRB = 0xFF;
   DDRA = 0x00;
   
   //INTERRUPT
   GICR = (1<<INT1); 
   MCUCR = MCUCR & 0b11110011;
   sei();
   
   Lcd4_Init();
   UART_init();
   ADC_init();
  
    while (1) 
    {	
		if(wait){
			if(!(PIND & (1 << PIND7))){
				Lcd4_Clear();
				Lcd4_Write_String("Voting Ended");
				
				break;
			}
			continue;
		}
			
		Lcd4_Clear();
		
		Lcd4_Write_String("Checking temperature..");
		_delay_ms(400);
		for(int i = 0; i < 8; i++){
			Lcd4_Shift_Left();
			_delay_ms(200);
		}
		_delay_ms(1000);
		Lcd4_Clear();
		Lcd4_Write_String("Temperature:");
		_delay_ms(1000);
		Lcd4_Set_Cursor(2,0);
		temp_F = ADC_Read();
		dtostrf(temp_F,4,2,Fahrenheit);
		Lcd4_Write_String(Fahrenheit);
		_delay_ms(2000);
		if( temp_F >= 99.50){
			Lcd4_Clear();
			Lcd4_Write_String("Temperature not normal");
			
			for(int i = 0; i < 8; i++){
				Lcd4_Shift_Left();
				_delay_ms(200);
			}
			_delay_ms(1000);
			Lcd4_Clear();
			Lcd4_Write_String("You can't vote now");
			for(int i = 0; i < 8; i++){
				Lcd4_Shift_Left();
				_delay_ms(200);
			}
			Lcd4_Clear();
			_delay_ms(4000);
			continue;
			
		}
		unsigned char id[10];
	    int count = 1;
	    Lcd4_Clear();
	    Lcd4_Write_String("Waiting for  ");
	    _delay_ms(500);
	    Lcd4_Set_Cursor(2, 1);
	    Lcd4_Write_String("RFID Tag");
	    _delay_ms(500);
	    Lcd4_Clear();
	    
		Lcd4_Write_String("Reading Tag...");
	    Lcd4_Set_Cursor(2, 0);
	    
	    int match = 0;
		
		while(1){
			id[count-1] = UART_RxChar();
			Lcd4_Write_Char(id[count-1]);
			if(count < id_len){
				count++;
				continue;
			}
			else if(count == id_len){
				int j;
				for(int i = 0; i<total_id; i++){
					for(j = 0; j<id_len; j++){
						if(valid_id[i][j] != id[j]){
							break;
						}
					}
					if(j == id_len){
						if(!voted[i]){
							voted[i] = 1;
							match = 1;
							break;	
						}
						else if(voted[i]){
							match = 1;
							voteflag = 1;
							break;
						}
					}
				}
			}
			if(match == 0){
				Lcd4_Clear();
				Lcd4_Write_String("ID match: ");
				Lcd4_Set_Cursor(2,0);
				Lcd4_Write_String("Not found. Reset");
				_delay_ms(500);
				break;
			}
			else if(match == 1){
				_delay_ms(300);
				Lcd4_Clear();
				if(voteflag){
					Lcd4_Write_String("This Person Has");
					_delay_ms(1000);
					Lcd4_Set_Cursor(2, 0);
					Lcd4_Write_String("Already Voted");
					_delay_ms(2500);
					Lcd4_Clear();
					voteflag = 0;
					break;
				}
				
				Lcd4_Write_String("ID match found!");
				_delay_ms(2000);
				Lcd4_Clear();
				//Generating Random Lock Code
				int lock = rand() % (9999 + 1 - 1000) + 1000;
				unsigned char locks[4];
				
				int mod = 0;
				while(lock != 0){
					locks[mod] = (lock % 10) + '0';
					lock = lock / 10;
					mod++;
					
				}
				Lcd4_Write_String("Your Lock Code: ");
				Lcd4_Set_Cursor(2, 0);
				for(int i = 3; i!=-1; i--){
					Lcd4_Write_Char(locks[i]);
					_delay_ms(100);
					UART_TxChar(locks[i]);
				}
				
				_delay_ms(1000);
				Lcd4_Clear();
				Lcd4_Write_String("Please Wait...");
				wait = 1;
				break;
				//Lcd4_Write_Char(lock);
				
				
				
			}
		}
		
	}
}


/*
 * LockAndVote.c
 *
 * Created: 6/19/2021 12:05:21 AM
 * Author : USER
 */ 

#define F_CPU 1000000
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

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

unsigned char UART_RxChar()
{
	while ((UCSRA & (1 << RXC)) == 0x00);// Wait till data is received
	return UDR;		// Return the byte
}
void UART_TxChar(unsigned char data){
	while((UCSRA & (1 << UDRE)) == 0x00);
	UDR = data;
	
}
char get_key(){
	
	PORTB |= 1;
	if(PINB & (1 << PINB3)){PORTB &= ~(1);return '1';}
	else if(PINB & (1 << PINB4)){PORTB &= ~(1);return '4';}
	else if(PINB & (1 << PINB5)){PORTB &= ~(1);return '7';}
	else if(PINB & (1 << PINB6)){PORTB &= ~(1);return '*';}
	PORTB &= ~(1);
	
	PORTB |= 2;
	if(PINB & (1 << PINB3)){PORTB &= ~(2); return '2';}
	else if(PINB & (1 << PINB4)){PORTB &= ~(2); return '5';}
	else if(PINB & (1 << PINB5)){PORTB &= ~(2); return '8';}
	else if(PINB & (1 << PINB6)){PORTB &= ~(2); return '0';}
	PORTB &= ~(2);
	
	PORTB |= 4;
	if(PINB & (1 << PINB3)){PORTB &= ~(4); return '3';}
	else if(PINB & (1 << PINB4)){PORTB &= ~(4); return '6';}
	else if(PINB & (1 << PINB5)){PORTB &= ~(4); return '9';}
	else if(PINB & (1 << PINB6)){PORTB &= ~(4); return '#';}
	
	PORTB &= ~(4);
	
	return 0;
}

int count_a = 0;
int count_b = 0;
int count_c = 0;
int count_d = 0;
char* buff;

volatile int end = 0;
ISR(INT1_vect){
	
	end = 1; 

}

int main(void)
{	
	DDRD = 0b11110110;
	DDRB = 0b00000111;
	DDRC = 0xFF;

	//INTERRUPT
	GICR = (1<<INT1); //STEP3
	MCUCR = MCUCR & 0b11110011;//STEP4
	sei();
	
	UART_init();
	unsigned char lock[4];
	//Lcd4_Init();
	unsigned char lock_in[4];
	unsigned char admin_lock[4] = {'8','8','8','8'};
	int count = 0;
	int input_pass = 0;
	Lcd4_Init();
	PORTD |= 0b00000100;
    /* Replace with your application code */
    while (1) 
    {
		Lcd4_Clear();
		if(!end){
		
		while(count < 4){
			lock[count] = UART_RxChar();
			//Lcd4_Write_Char(lock[count]);
			_delay_ms(200);
			count++;
		}
		
		if(PINB & (1<<PINB7)){ // IF MOTION IS DETECTED
			_delay_ms(100);
			
			Lcd4_Write_String("Motion Detected!");
			_delay_ms(1000);
			Lcd4_Clear();
			
			/*Lcd4_Write_String("Lock: ");
			Lcd4_Set_Cursor(2,0);*/
			
			count = 0;
			/*while(count < 4){
				Lcd4_Write_Char(lock[count]);
				_delay_ms(100);
				count++;
			}*/
			_delay_ms(500);
			Lcd4_Clear();
			
			/*Lcd4_Write_String("Input your unique passcode now");
			for(int i = 0; i<16; i++){
				_delay_ms(100);
				Lcd4_Shift_Left();
			}*/
			Lcd4_Clear();
			Lcd4_Write_String("Your Passcode: ");
			Lcd4_Set_Cursor(2, 0);
			_delay_ms(200);
			int i;
			int try = 0;
			while(try < 2){
				
			
				while(input_pass < 4){
					if(get_key() != 0){
						lock_in[input_pass] = get_key();
						Lcd4_Write_Char(lock_in[input_pass]);
						_delay_ms(500);
						input_pass++;
					}
				}
				Lcd4_Clear();
				
				for(i = 0; i<4; i++){
					if(lock[i] != lock_in[i]){
						try++;
						Lcd4_Write_String("Wrong Passcode!");
						_delay_ms(1000);
						
						Lcd4_Clear();
						if(try == 2){
							PORTD &= 0b11111011;
							_delay_ms(100);
							PORTD |= 0b00000100;
							break;
						}
						
						Lcd4_Write_String("Try Again!");
						_delay_ms(1000);
						
						Lcd4_Clear();
						Lcd4_Write_String("Your Passcode: ");
						Lcd4_Set_Cursor(2, 0);
						_delay_ms(200);
						input_pass = 0;
						break;
					}
					
				}
				if(i == 4)break;
			}
			
			if(i == 4){
				Lcd4_Write_String("Correct Passcode.");
				
				 //Rotates Motor Anticlockwise
				 PORTC |= 0b00000001;
				 PORTC &= 0b11111101;
				 _delay_ms(3000);

				 //Stops Motor
				 PORTC &= 0b11111100;
				 _delay_ms(500);
				 
				 Lcd4_Set_Cursor(2, 0);
				 Lcd4_Write_String("Proceed To Vote");
				
				 _delay_ms(800);
				 Lcd4_Clear();
				 Lcd4_Write_String("Vote Now");
				 while(1){
					 
					 if(!(PINA & (1<<PINA0))){
						 count_a++;
						 Lcd4_Clear();
						 Lcd4_Write_String("You voted for Option A");
						  
						 break;
					}
					
					else if(!(PINA & (1<<PINA1))){
						count_b++;
						 Lcd4_Clear();
						 Lcd4_Write_String("You voted for Option B");
						break;
					}
					else if(!(PINA & (1<<PINA2))){
						count_c++;
						 Lcd4_Clear();
						 Lcd4_Write_String("You voted for Option C");
						break;
					}
					else if(!(PINA & (1<<PINA3))){
						count_d++;
						 Lcd4_Clear();
						 Lcd4_Write_String("You voted for Option D");
						break;
					}
					
					
					
				 }
				 
				 _delay_ms(400);
				 for(uint16_t i= 0; i<10; i++){
					 Lcd4_Shift_Left();
					 _delay_ms(100);
				 }
				 _delay_ms(200);
				 Lcd4_Clear();
				 Lcd4_Write_String("Thank You For Voting");
				 _delay_ms(400);
				 
				 for(int i = 0; i<4; i++){
					 _delay_ms(100);
					 Lcd4_Shift_Left();
				 }
				 
				 //Rotates Motor Clockwise
				  PORTC |= 0b00000010;
				  PORTC &= 0b11111110;
				  _delay_ms(3000);

				  //Stops Motor
				  PORTC &= 0b11111100;
				  _delay_ms(500);

				 
			}
			
			Lcd4_Clear();
			count = 0;
			input_pass = 0;
			
		}
		
		
		}

		else if(end){
			Lcd4_Clear();
			Lcd4_Write_String("Voting Ended");
			_delay_ms(1000);
			
			Lcd4_Clear();
			Lcd4_Write_String("Admin Passcode: ");
			Lcd4_Set_Cursor(2, 0);
			_delay_ms(200);
			while(input_pass < 4){
				if(get_key() != 0){
					lock_in[input_pass] = get_key();
					Lcd4_Write_Char(lock_in[input_pass]);
					_delay_ms(500);
					input_pass++;
				}
			}
			Lcd4_Clear();
			int i;
			for(i = 0; i<4; i++){
				if(admin_lock[i] != lock_in[i]){
					Lcd4_Write_String("Wrong Passcode!");
					_delay_ms(1000);
					
					break;
				}
			}
			
			if(i == 4){
				Lcd4_Write_String("Correct Passcode.");
				 //Rotates Motor Anticlockwise
				 PORTC |= 0b00000001;
				 PORTC &= 0b11111101;
				 _delay_ms(3000);

				 //Stops Motor
				 PORTC &= 0b11111100;
				 _delay_ms(500);
					
				Lcd4_Clear();	
				Lcd4_Write_String("Show Votes?");
			while(1){

				if(!(PINA & (1<<PINA4))){
					Lcd4_Clear();
					Lcd4_Write_String("A: ");
					itoa(count_a, buff, 10);
					Lcd4_Write_String(buff);
					Lcd4_Set_Cursor(1,8);
					Lcd4_Write_String("B: ");
					itoa(count_b, buff, 10);
					Lcd4_Write_String(buff);
					Lcd4_Set_Cursor(2, 0);
					Lcd4_Write_String("C: ");
					itoa(count_c, buff, 10);
					Lcd4_Write_String(buff);
					Lcd4_Set_Cursor(2,8);
					Lcd4_Write_String("D: ");
					itoa(count_d, buff, 10);
					Lcd4_Write_String(buff);
					_delay_ms(2000);
							
					break;
				}

			}
			
			//Rotates Motor Clockwise
			PORTC |= 0b00000010;
			PORTC &= 0b11111110;
			_delay_ms(3000);

			//Stops Motor
			PORTC &= 0b11111100;
			_delay_ms(500);
			
			Lcd4_Clear();

		}	

			break;
		}
	}
}

//===============================================================================
//
//	Author				: Cytron Technologies
//	Hardware 			: SK28A
//  	Microcontroller		: 18F2550
//	Project description		: LED Blinking 						 
//                        			  LED will off and stop blinking when SW1 is pressed
//	Compiler			: MPLAB-C18 v3.42 Lite edition
//	IDE				: MPLAB IDE v8.85
//	Osc frequency		: 20MHz
//	Date				: Jun 12
//===============================================================================

#include <p18cxxx.h>	//include the header file for PIC18F, the compiler will select the correct file base on device set in project
#include <delays.h>	//include the header file for delays function

//==========================================================================
//  Configuration bits
//==========================================================================
//  Configuration bits has been configured in bootloader firmware.
//=============================================================================

//===============================================================================
//	Definitions
//===============================================================================
#define LED	     		LATBbits.LATB1		//LED on SK28A is connected to RB1
#define	SW1			PORTBbits.RB0		//Push button labeled as SW1 is connected to RB0

//===============================================================================
//	Function Prototypes
//===============================================================================
void delay_ms(unsigned int ui_data);		//delay in millisecond, maximum value is 65,535; 0 will result in 65536 millisecond of delay
void delay_us(unsigned char uc_data);	// delay in microsecond, maximum value is 255; 0 will result in 256 microsecond of delay

void HighPriorityISRCode();					//interrupt function prototype
void LowPriorityISRCode();
//===============================================================================
//	Main Program
//===============================================================================
void main()
{
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	// Initialize the I/O port direction.
	TRISB = 0b00000001;
	
	while(1)
	{	
		LED ^= 1;		// toggle LED on SK28A
	    	delay_ms(100);	//delay for around 50ms	
			
		while(SW1==0)LED = 0;	//Halt and Off LED when SW1 is press, continue the program when SW1 is released or not pressed
	}

	while(1);	//infinate loop to prevent the program to reset at the end of main
}


//===============================================================================
//	Functions
//===============================================================================
//Simple delay function
// delay in milli second, maximum value for ui_data is 65,535.
void delay_ms(unsigned int ui_data)
{
	for(;ui_data > 0; ui_data--) 
	{
		Delay10KTCYx(1);	//to obtain 1 millisecond, we require 12K cycle of instruction if CPU clock is 48MHz
		Delay1KTCYx(2);	//Delay macro from MCC18 Compiler
	}
}

// delay in milli second, maximum value for ui_data is 65,535.
void delay_us(unsigned char uc_data)
{
	for(;uc_data > 0; uc_data--) 
	{
		Delay10TCYx(1);	//to obtain 1 microsecond delay, we require 12 cycle of instruction if CPU clock is 48MHz.
						// this is approximation 		
	}
}
#pragma interrupt HighPriorityISRCode
void HighPriorityISRCode()
{
	
}
#pragma interruptlow LowPriorityISRCode
void LowPriorityISRCode()
{

}
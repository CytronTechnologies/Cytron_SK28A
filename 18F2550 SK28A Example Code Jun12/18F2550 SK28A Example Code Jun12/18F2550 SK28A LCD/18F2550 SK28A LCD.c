//===============================================================================
//
//	Author				: Cytron Technologies
//	Hardware 			: SK28A
//  	Microcontroller		: 18F2550
//	Project description		: LED Blinking and LCD display on SK28A 
//						  LCD display string "Cytron SK28A"
//                        			  LED will off and stop blinking when SW1 is pressed
//	Compiler			: MPLAB-C18 v3.41 Eval edition
//	IDE				: MPLAB IDE v8.85
//	Osc frequency		: 20MHz
//
//===============================================================================

#include <p18cxxx.h>	//include the header file for PIC18F, the compiler will select the correct file base on device set in project
#include <delays.h>	//include the header file for delays function

//==========================================================================
//  Configuration bits
//=============================================================================
	#pragma config PLLDIV   = 5         // (20 MHz crystal), divide with 5 to get 4MHz and go through 96MHz PLL
        #pragma config CPUDIV   = OSC1_PLL2		//CPU use 96MHZ/ 2 = 48MHz
        #pragma config USBDIV   = 2         //USB clock at 96MHz / 2 = 48 MHz
        #pragma config FOSC     = HSPLL_HS	//High Speed Oscillator, PLL enabled
        #pragma config FCMEN    = OFF
        #pragma config IESO     = OFF
        #pragma config PWRT     = OFF
        #pragma config BOR      = ON
        #pragma config BORV     = 3
        #pragma config VREGEN   = ON		//USB Voltage Regulator
        #pragma config WDT      = OFF
        #pragma config WDTPS    = 32768
        #pragma config MCLRE    = ON
        #pragma config LPT1OSC  = OFF
        #pragma config PBADEN   = OFF	
//      #pragma config CCP2MX   = ON
        #pragma config STVREN   = ON
        #pragma config LVP      = OFF
//      #pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming
        #pragma config XINST    = OFF       // Extended Instruction Set
        #pragma config CP0      = OFF
        #pragma config CP1      = OFF
//      #pragma config CP2      = OFF
//      #pragma config CP3      = OFF
        #pragma config CPB      = OFF
//      #pragma config CPD      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
//      #pragma config WRT2     = OFF
//      #pragma config WRT3     = OFF
        #pragma config WRTB     = OFF       // Boot Block Write Protection
        #pragma config WRTC     = OFF
//      #pragma config WRTD     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
//      #pragma config EBTR2    = OFF
//      #pragma config EBTR3    = OFF
        #pragma config EBTRB    = OFF

//===============================================================================
//	Definitions
//===============================================================================
#define LCD_DB7		LATBbits.LATB7		//LCD data pin 7
#define LCD_DB6		LATBbits.LATB6		//LCD data pin 6
#define LCD_DB5		LATBbits.LATB5		//LCD data pin 5
#define LCD_DB4		LATBbits.LATB4		//LCD data pin 4
#define LCD_DATA		LATB
#define LCD_RS   		LATBbits.LATB2		//LCD Register Select pin is connected to RB2 on SK28A
#define LCD_E   		LATBbits.LATB3		//LCD Enable pin is connected to RB3 on SK28A

#define LED	     		LATBbits.LATB1		//LED on SK28A is connected to RB1
#define	SW1			PORTBbits.RB0		//Push button labeled as SW1 is connected to RB0

//===============================================================================
//	Function Prototypes
//===============================================================================
void lcd_init(void);
void lcd_4bit_write(unsigned char data);
void lcd_config(unsigned char command);
void lcd_putchar(unsigned char data);
void lcd_putstr(rom const char *str);
void lcd_clear(void);
void lcd_home(void);
void lcd_set_cursor(unsigned char uc_column, unsigned char uc_row);	//uc_column is 0 for first character, uc_row is 0 for 1st line
void lcd_goto(unsigned char address);		// move cursor with the address according to datasheet of LCD
void lcd_e_clock(void);

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

	//-----------------------------------
	// LCD in 4-bit mode
	//-----------------------------------
	lcd_init();					//Initialize LCD

	lcd_set_cursor(1,0);	//Set cursor to column 1, line 1
	lcd_putstr("Cytron");	//Display Cytron
	lcd_goto(0x40);		//Set cursor to 2nd line using another function
	lcd_putstr("SK28A :)");	//Display SK28A
	
	while(1)
	{	
		LED ^= 1;		// toggle LED on SK28A
	    	delay_ms(50);	//delay for around 50ms	
			
		while(SW1==0)LED = 0;	//Halt and Off LED when SW1 is press, continue the program when SW1 is released or not pressed
	}

	while(1);	//infinate loop to prevent the program to reset at the end of main
}


//===============================================================================
//	Functions
//===============================================================================
//4-bit mode configuration 
//4-bit mode configuration 
void lcd_init()
{
	delay_ms(30);	//wait for 30ms after power ON for LCD internal controller to initialize itself
	LCD_E = 1;
	//Set lcd to configuration mode
	LCD_RS = 0;		//Selected command register
	delay_us(5);	//macro from HITECH compiler to generate code to delay for 1 microsecond base on _XTAL_FREQ value
	
	LCD_DATA = (LCD_DATA & 0x0F) | 0b00110000;	//make it in 8-bit mode first, for 3 times
	lcd_e_clock();
   	delay_ms(2);
   	LCD_DATA = (LCD_DATA & 0x0F) | 0b00110000;	//make it in 8-bit mode first, for 3 times
	lcd_e_clock();
   	delay_ms(2);
   	LCD_DATA = (LCD_DATA & 0x0F) | 0b00110000;	//make it in 8-bit mode first, for 3 times
	lcd_e_clock();
   	delay_ms(2);
   	
   	LCD_DATA = (LCD_DATA & 0x0F) | 0b00100000;	//make it in 4-bit mode
	lcd_e_clock();
   	delay_ms(2);			
   	   	
	//start sending command in 4 bit mode
   	//Function Set
   	lcd_config(0b00101000);	 //0b 0 0 1 ID N F X X 							
   							//Interface Data Length, ID= 4-bit
   							//Number of line to display, N = 1 is 2 line display, N = 0 is 1 line display
   							//Display Font, F = 0 is 5x 8 dots, F = 1 is 5 x 11 dots
	
	//Command Entry Mode
	lcd_config(0b00000110);	//0b 0 0 0 0 0 1 ID SH
							//ID  = 1, cursor automatic move to right, increase by 1
   							//SH = 0,  shift of entire display is not perform   								
		
	//Display Control
	lcd_config(0b00001111);	//0b 0 0 0 0 1 D C B
							//D  = 1, Display is ON
   							//C = 0,  Cursor is not display
   							//B = 0. Cursor does not blink  					
	
	lcd_clear();	//clear LCD and move the cursor back to home position
}

//Routine to send data to LCD via 2 nibbles in 4-bit mode
void lcd_4bit_write(unsigned char data)			
{										
	LCD_DATA = (LCD_DATA & 0x0F) | (data & 0xF0);	//send out the Most Significant Nibble
	lcd_e_clock();
    
	LCD_DATA = (LCD_DATA & 0x0F) | (data << 4); //send out the Least Significant Nibble
	lcd_e_clock();				
}

//Routine to send command to LCD
void lcd_config(unsigned char command)
{
    LCD_RS = 0;        			//Selected command register
    delay_us(5);
    lcd_4bit_write(command);	//Send command via 2 nibbles 
    delay_ms(1); 
}

//Routine to send display data (single character) to LCD
void lcd_putchar(unsigned char data)
{
    LCD_RS = 1;        			//Selected data register
    delay_us(5);
    lcd_4bit_write(data);		//Send display via 2 nibbles
    delay_ms(1); 
}

//Routine to send string to LCD
void lcd_putstr(rom const char *str)
{
	while(*str != '\0')		//loop till string ends
	{
     	lcd_putchar(*str++);  		//send characters to LCD one by one					
	}		
}

//Routine to clear the LCD
void lcd_clear(void)
{
	lcd_config(0x01);			//command to clear LCD
	delay_ms(1);
}
//function to move LCD cursor to home position
void lcd_home(void)
{
	lcd_config(0x02);			//command to move cursor to home position
  	delay_ms(1);
}
//Rountine to set cursor to the desired position base on coordinate, column and row
/*Place a string in the specified row and column of the screen.
* +--+--+--+--+--+---------------------+
* |0 |1 |2 |3 |4 |5 ...etc             | <- row 0
* +--+--+--+--+--+---------------------+
* |0 |1 |2 |3 |4 |5 ...etc             | <- row 1
* +--+--+--+--+--+---------------------+
*/
void lcd_set_cursor(unsigned char uc_column, unsigned char uc_row)
{
	if(uc_row == 0) lcd_config(0x80 + uc_column);	//command to move cursor to first row/line with offset of column
	else if(uc_row ==1 ) lcd_config(0xC0 + uc_column);	//command to move cursor to 2nd row/line with offset of column
}
	
//Rountine to set cursor to the desired position base on LCD DDRAM address
/*Place a string in the specified row and column of the screen.
* +--+--+--+--+--+---------------------+
* |0x00 |0x01 |0x02 |0x03 |0x04 |0x05  ...etc          |0x0F| <- 1st line
* +--+--+--+--+--+---------------------+
* |0x40 |0x41 |0x42 |0x43 |0x44 |0x45 ...etc           |0x4F| <- 2nd line
* +--+--+--+--+--+---------------------+
*/
void lcd_goto(unsigned char address)
{
	 lcd_config(0x80 + address);	//command to move cursor to desire position base on the LCD DDRAM address	
}

//function to output enable clock pulse to LCD
void lcd_e_clock(void)
{
	delay_us(10);	
	LCD_E = 0;	//create a falling edge for Enable pin of LCD to process data
	delay_us(100);
	LCD_E = 1;	//pull the Enable pin high again
	delay_us(100);	
}	

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
		Delay10TCYx(1);	//to obtain 1 microsecond delay, we require 12 cycle of instruction if CPU clock is 48MHz		
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
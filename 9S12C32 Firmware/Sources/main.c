/*

***********************************************************************

 The objective of this Mini-Project is to create a voice controlled RC Car called BEAVR 
 Bluetooth Enabled Automotive Voice Recognication Car. The car will respond to tasks such as forward, backward, left, right, turn headlights on, play music, etc.
 Everything is done via voice, with the option of using a bluetooth controller app.


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. Arduino to bluetooth SCI connection (HC-06 bluetooth module)

 2. Arduino to 9s12 data connection via SPI

 3. Full powertrain overhaul through implementation of an H-Bridge Motor Driver

 4. Manual vehicle controller via bluetooth controller (Android)

 5. Voice vehicle control via bluetooth (able to switch between voice/controller)
 
 6. Modified car packaging to include custom lights and turn signals
 
 7. Speaker implementation (?)
 
 Peripherals Used: SPI, PWM (Motors), PWM (Lights), Timer (Blinkers), PWM (Speakers ?)
 

***********************************************************************

  Date code started: < 12/3/2017 >

  Update history (add an entry every time a significant change is made):

  Date: < 12/15/2017 >  Name: < Rhythm Syed >   Update: < Proofreading >

  Date: < 12/6/2017 >  Name: < Rhythm Syed >   Update: < Peripheral, Initializations, Logic, Conditionals >

  Date: < ? >  Name: < ? >   Update: < ? >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);


/* Variable declarations */
int timerCounter = 0;	
int timerflag = 0;
int i = 0;   	   			 		  			 		       
int j = 0;

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10		// RS pin mask (PTT[4])
#define RW 0x20		// R/W pin mask (PTT[5])
#define LCDCLK 0x40	// LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 = 0x80	// LCD line 1 cursor position
#define LINE2 = 0xC0	// LCD line 2 cursor position

	 	   		
/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize peripherals */
  
  //SPI Arduino = Master, 9S12 = Slave
  SPICR1 = 0xE0;
  SPICR2 = 0x00;
  SPIBR =  0x10;
  SPIDR = 0x00;
  
  
  //PWM Two motors: Front motor controls steering, back motor controls drive
  MODRR = 0x07;
  
  PWME  = 0x07;
  PWMPOL  = 0x07;
  PWMCTL  = 0x00;         
  PWMCAE  = 0x00;
  
  PWMPER0 = 0x64;  
  PWMPER1 = 0x64;
  PWMPER2 = 0x64;
  PWMDTY0 = 0x00;
  PWMDTY1 = 0x00;
  PWMDTY2 = 0x00;
  
  PWMPRCLK  = 0x07;                  
  PWMCLK  = 0x03;         
  PWMSCLA = 0x64;
           
  #define BackMotor PWMDTY0
  #define FrontMotor PWMDTY1
  #define Headlights PWMDTY2
    
             
/* Initialize interrupts */

	//Enable Timer Interrupt for blinking back lights
	
	TSCR1_TEN = 0;
	TIOS_IOS3 = 1; //OUTPUT COMPARE (PT3)
	TIOS_IOS4 = 1; //OUTPUT COMPARE (PT4)
	TSCR1_TFFCA = 1;
	TSCR2_TOI = 0;
	TSCR2_TCRE = 1;
	TSCR2_PR2 = 1;
	TSCR2_PR1 = 1;
	TSCR2_PR0 = 1;
	
	TCTL2_OM3 = 0;
	TCTL2_OL3 = 1;
	TCTL1_OM4 = 0;
	TCTL1_OL4 = 1;
	
	TCTL3 = 0x00;
	TCTL4 = 0x00;
	TIE = 0x00;
	
	OC7M = 0b00011000; //(Sets pins to output mode)
	
	DDRT = 0b00011000; 
	
	TC3 = 65535;
	TC4 = 65535;      
	      
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
    //DisableInterrupts;
	initializations(); 		  			 		  		
	//EnableInterrupts;

 for(;;) {
  
/* < start of your main loop > */
//test comment 
	while(!SPISR_SPTEF)
	{
	}
	 
    if(SPIDR == 70)		// Move Forward
    {
		BackMotor = 0x08;			//Forward logic provided by Arduino
		FrontMotor = 0x00;
		TSCR1_TEN = 0;
		//TIE_C4I = 1;
		//TIE_C5I = 1;
	}
	
	if(SPIDR == 66)	// Move Reverse
	{
		BackMotor = 0x08;			//Reverse logic provided by Arduino
		FrontMotor = 0x00;
		TSCR1_TEN = 1;
		CFORC_FOC4 = 1;
		CFORC_FOC3 = 1;
		
		//TIE_C4I = 1;
		//TIE_C5I = 1;
	}
	
	if(SPIDR == 76)		// Turn Left
	{
		TSCR1_TEN = 0;
		TSCR1_TEN = 1;
		BackMotor = 0x00;			//Left turn logic provided by Arduino
		FrontMotor = 0x32;
		
		for(j = 0; j < 50; j++)
		{
			CFORC_FOC4 = 1;
			for(i = 0; i < 10000; i++)
			{
				
			}
			TSCR1_TEN = 0;
			for(i = 0; i < 10000; i++)
			{
				
			}
		}
		
		TSCR1_TEN = 0;		
		//TIE_C4I = 1;
		//TIE_C5I = 1;
	}
	
	if(SPIDR == 82)		// Turn Right
	{
		TSCR1_TEN = 0;
		TSCR1_TEN = 1;
		BackMotor = 0x00;			//Right turn logic provided by Arduino
		FrontMotor = 0x32;
		
		for(j = 0; j < 50; j++)
		{
			CFORC_FOC3 = 1;
			for(i = 0; i < 10000; i++)
			{
				
			}
			TSCR1_TEN = 0;
			for(i = 0; i < 10000; i++)
			{
				
			}
		}
		TSCR1_TEN = 0;
			
		//TIE_C4I = 1;
		//TIE_C5I = 1;;
	}
	
	if(SPIDR == 71)		// Move Forward Left
	{	
		TSCR1_TEN = 0;
		TSCR1_TEN = 1;
		BackMotor = 0x08;			//Forward logic provided by Arduino
		FrontMotor = 0x32;			//Left turn logic provided by Arduino
		
		//TIE_C4I = 1;
		//TIE_C5I = 1;	
	}
	
	if(SPIDR == 73)		// Move Forward Right
	{
		TSCR1_TEN = 0;
		TSCR1_TEN = 1;
		BackMotor = 0x08;			//Forward logic provided by Arduino
		FrontMotor = 0x32;			//Right turn logic provided by Arduino
		
		//TIE_C4I = 1;
		//TIE_C5I = 1;	
	}
	
	if(SPIDR == 71)		// Move Reverse Left
	{
	
		BackMotor = 0x08;			//Reverse logic provided by Arduino
		FrontMotor = 0x32;			//Left turn logic provided by Arduino
		TSCR1_TEN = 1;
		CFORC_FOC4 = 1;
		CFORC_FOC3 = 1;
		
		//TIE_C4I = 1;
		//TIE_C5I = 1;
	}
	
	if(SPIDR == 73)		// Move Reverse Right
	{
	
		BackMotor = 0x08;			//Reverse logic provided by Arduino
		FrontMotor = 0x32;			//Right turn logic provided by Arduino
		
		TSCR1_TEN = 1;
		CFORC_FOC4 = 1;
		CFORC_FOC3 = 1;
			
		//TIE_C4I = 1;
		//TIE_C5I = 1;
	}
	
	if(SPIDR == 83) 	// Brake: Stop all PWMs
	{
			
		TSCR1_TEN = 1;
		CFORC_FOC4 = 1;
		CFORC_FOC3 = 1;
		BackMotor = 0x00;		
		FrontMotor = 0x00;			
		
		//TIE_C4I = 1;
		//TIE_C5I = 1;
	}
	
	if(SPIDR == 87)
	{
		Headlights = 0x50;
	}
	
	if(SPIDR == 119)
	{
		Headlights = 0x00;
	}
  
  } /* loop forever */
   
}   /* do not leave main */




/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flagt 
  	CRGFLG = CRGFLG | 0x80; 
 

}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  	// clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
 	
 	timerCounter++;
 	if(timerCounter >= 500)
 	{
 		timerCounter = 0;	
 		timerflag = 1;
 	}
 	

}

/*
***********************************************************************                       
  SCI interrupt service routine		 		  		
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 


}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}
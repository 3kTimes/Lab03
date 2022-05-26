#include "lab03.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"

/*
 * DAC code
 */

#define DAC_CS_TRIS TRISDbits.TRISD8
#define DAC_SDI_TRIS TRISBbits.TRISB10
#define DAC_SCK_TRIS TRISBbits.TRISB11
#define DAC_LDAC_TRIS TRISBbits.TRISB13
    
#define DAC_CS_PORT PORTDbits.RD8
#define DAC_SDI_PORT PORTBbits.RB10
#define DAC_SCK_PORT PORTBbits.RB11
#define DAC_LDAC_PORT PORTBbits.RB13

#define DAC_SDI_AD1CFG AD1PCFGLbits.PCFG10
#define DAC_SCK_AD1CFG AD1PCFGLbits.PCFG11
#define DAC_LDAC_AD1CFG AD1PCFGLbits.PCFG13

#define DAC_SDI_AD2CFG AD2PCFGLbits.PCFG10
#define DAC_SCK_AD2CFG AD2PCFGLbits.PCFG11
#define DAC_LDAC_AD2CFG AD2PCFGLbits.PCFG13

volatile uint8_t counterHalfSecound = 0;

void dac_initialize()
{
    SETBIT(DAC_SDI_AD1CFG);
    SETBIT(DAC_SDI_AD2CFG);  // set AN10 to digital mode
    
    SETBIT(DAC_SCK_AD1CFG);
    SETBIT(DAC_SCK_AD2CFG); // set AN11 to digital mode
    
    SETBIT(DAC_LDAC_AD1CFG);
    SETBIT(DAC_LDAC_AD2CFG); // set AN13 to digital mode
    
    // this means AN10 will become RB10, AN11->RB11, AN13->RB13
    // see datasheet 11.3
    
    CLEARBIT(DAC_CS_TRIS);
    CLEARBIT(DAC_SDI_TRIS);
    CLEARBIT(DAC_SCK_TRIS); 
    CLEARBIT(DAC_LDAC_TRIS);    // set RD8, RB10, RB11, RB13 as output pins
    
    SETBIT(DAC_CS_PORT); // Low to enable 
    CLEARBIT(DAC_SCK_PORT); // wirte on rising edge
    CLEARBIT(DAC_SDI_PORT); // Bit to send Data to DAC
    SETBIT(DAC_LDAC_PORT);  // set default state: CS=1, SCK=0, SDI=undefined, LDAC=1 (Datasheet MCP4822 Page 23)

}

/*
 * Timer code
 */

#define FCY_EXT   32768UL

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

void timer_initialize()
{
    // Enable RTC Oscillator -> this effectively does OSCCONbits.LPOSCEN = 1
    // but the OSCCON register is lock protected. That means you would have to 
    // write a specific sequence of numbers to the register OSCCONL. After that 
    // the write access to OSCCONL will be enabled for one instruction cycle.
    // The function __builtin_write_OSCCONL(val) does the unlocking sequence and
    // afterwards writes the value val to that register. (OSCCONL represents the
    // lower 8 bits of the register OSCCON)
    __builtin_write_OSCCONL(OSCCONL | 2);
    // configure timer
    
    //Timer 1 -- 0.5 secound
    
    T1CONbits.TON = 0; // Disable the Timers
    T1CONbits.TCKPS = 0b11; // Set Prescaler 256
    T1CONbits.TCS = 1; // Set Clock Source (external = 1)
    T1CONbits.TGATE = 0; // Set Gated Timer Mode -> don't use gating //this line can be ignored, if TCS =  1 (have a look at the manual)  
    T1CONbits.TSYNC = 0; // T1: Set External Clock Input Synchronization -> no sync
    PR1 = 64; // Load Timer Periods 
    TMR1 = 0x00; // Reset Timer Values
    IPC0bits.T1IP = 0x01; // Set Interrupt Priority (actually Level 1)
    IFS0bits.T1IF = 0; // Clear Interrupt Flags
    IEC0bits.T1IE = 1; // Enable Interrupts
    T1CONbits.TON = 1; // Enable the Timers
}


// interrupt service routine

void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{   
    counterHalfSecound ++;
    
    if(counterHalfSecound == 1){   //check whether 0.5 s have passed
            TOGGLELED(LED1_PORT);  
        }  
        
    if(counterHalfSecound == 5){   //check whether 2.5 s have passed
            TOGGLELED(LED1_PORT);
        }
        
    if(counterHalfSecound == 7){   //check whether 3.5 s have passed
            TOGGLELED(LED1_PORT);
            counterHalfSecound = 0;
        }
    
    IFS0bits.T1IF = 0;  
}


void setDAC (uint16_t value){
  
    CLEARBIT(DAC_CS_PORT);  // Set CS bit to zero to start conversation
    
    uint8_t i =16;
        
    for(i; i>=1; i--){          // Set Binary for Output signal
            
        CLEARBIT(DAC_SCK_PORT);
        Nop();
        
        DAC_SDI_PORT = value>>(i-1) & BV(0);
            
        Nop();
        SETBIT(DAC_SCK_PORT);
    }
    
    SETBIT(DAC_CS_PORT); // SETBIT(DAC_CS_PORT);  // Clear CS bit
    CLEARBIT(DAC_SDI_PORT); // Clear SDI bit
    Nop();
    
    CLEARBIT(DAC_LDAC_PORT);
    Nop();
    Nop();
    SETBIT(DAC_LDAC_PORT);   
}


/*
 * main loop
 */

void main_loop()
{
    uint16_t oneVoltage = 1000; 
   
    oneVoltage |=  BV(12);  // settings for DAC.... Bit 15 to 0 (write ti DACA); Bit 14 don't care; Bit 13 to 0 (4.096V); Bit 12 to 1
    
    uint16_t twoPointFiveVoltage = 2500;
    twoPointFiveVoltage |= BV(12);  // settings for DAC.... Bit 15 to 0 (write ti DACA); Bit 14 don't care; Bit 13 to 0 (4.096V); Bit 12 to 1
    
    uint16_t threePointFiveVoltage = 3000;
    threePointFiveVoltage |=  BV(12);  // settings for DAC.... Bit 15 to 0 (write ti DACA); Bit 14 don't care; Bit 13 to 0 (4.096V); Bit 12 to 1
    
    
    // print assignment information
    lcd_printf("Lab03: DAC");
    lcd_locate(0, 1);
    lcd_printf("Group: Boyang & Ron");
    
    CLEARBIT(LED1_TRIS);   //set LED1 as output
     
    setDAC(oneVoltage); // set initial 1 voltage
    
    while(TRUE)
    {  // main loop code
        
        setDAC(oneVoltage); // set initial 1 voltage
           
        if(counterHalfSecound == 1){   //check whether 0.5 s have passed
            setDAC(twoPointFiveVoltage); // set Vout to 2.5V  
        }  
        
        if(counterHalfSecound == 5){   //check whether 2.5 s have passed
            setDAC(threePointFiveVoltage); // set Vout to 3.5V
        }
        
        if(counterHalfSecound == 7){   //check whether 3.5 s have passed
           setDAC(oneVoltage); // set Vout to 1V
        }
    
    }
}


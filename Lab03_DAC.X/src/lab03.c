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
    
    SETBIT(DAC_CS_PORT); 
    CLEARBIT(DAC_SCK_PORT);
    CLEARBIT(DAC_SDI_PORT);
    CLEARBIT(DAC_LDAC_PORT);    // set default state: CS=1, SCK=0, SDI=undefined, LDAC=1 (Datasheet MCP4822 Page 23)

    
    // starting conversation
    
    CLEARBIT(DAC_CS_PORT);  // Set CS bit to zero to start conversation
    
    uint8_t i =0;
     
    for(i; i<16; i++){          // Set Binary for Output signal
        CLEARBIT(DAC_SCK_PORT);
        
        Nop();
        SETBIT(DAC_SCK_PORT);
    }
    
    CLEARBIT(DAC_CS_PORT); // Clear CS bit
    CLEARBIT(DAC_SDI_PORT); // Clear SDI bit
    Nop();
    
    SETBIT(DAC_LDAC_PORT);
    Nop();
    Nop();
    CLEARBIT(DAC_LDAC_PORT);
    
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
    
}

// interrupt service routine?

/*
 * main loop
 */

void main_loop()
{
    // print assignment information
    lcd_printf("Lab03: DAC");
    lcd_locate(0, 1);
    lcd_printf("Group: GroupName");
    
    while(TRUE)
    {
        // main loop code
    }
}

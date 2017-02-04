/*
 * File:   main.c
 * Author: True Administrator
 *
 * Created on July 18, 2016, 12:11 PM
 */


#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"

void set_time(void);
void delay(int);
void stand_by(unsigned char[]);


const char keys[] = "123A456B789C*0#D";
const char happynewyear[7] = {  0x00, //45 Seconds 
                            0x34, //59 Minutes
                            0x16, //24 hour mode, set to 23:00
                            0x02, //Saturday 
                            0x31, //31st
                            0x01, //December
                            0x17};//2016

void main(void) {
    
    // <editor-fold defaultstate="collapsed" desc=" STARTUP SEQUENCE ">
    
    TRISA = 0xFF; // Set Port A as all input
    TRISB = 0xFF; 
    TRISC = 0x00;
    TRISD = 0x00; //All output mode for LCD
    TRISE = 0x00;    

    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0xFF;  //Set PORTB to be digital instead of analog default  
    
    nRBPU = 0;

    //</editor-fold>

    initLCD();
    __lcd_clear();
    unsigned char time[7];

    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
//    di(); // Disable all interrupts
    
    //only to configure the time
//    set_time(); 
    
    while (PORTBbits.RB1 == 0 || keys[(PORTB & 0xF0)>>4] != '#') {
        // RB1 is the interrupt pin, so if there is no key pressed, RB1 will be 0
            // the PIC will wait and do nothing until a key press is signaled
        stand_by(time);  
    }
    __lcd_clear();
    initLCD();
    __lcd_home();
    printf("You have begun");
    __lcd_newline();
    printf("the operation!");
    delay(3);
    __lcd_clear();
    initLCD();
    __lcd_home();
    printf("Sorting...");
    __lcd_newline();
    printf("'*' to STOP");
    
//    while (1) {
//        while(PORTBbits.RB1 == 1){
//            // Wait until the key has been released
//        }
//        Nop();  //Apply breakpoint here because of compiler optimizations
//        Nop();
//    }
    while (PORTBbits.RB1 == 0 || keys[(PORTB & 0xF0)>>4] != '*') {
//        operating phase
        
    }
    __lcd_clear()
    return;
}

void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(happynewyear[i]);
    }    
    I2C_Master_Stop(); //Stop condition
    
    
}

void stand_by(unsigned char time[]) {

    //Reset RTC memory pointer 
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition

            //Read Current Time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read
    for(unsigned char i=0;i<0x06;i++){
        time[i] = I2C_Master_Read(1);
    }
    time[6] = I2C_Master_Read(0);       //Final Read without ack
    I2C_Master_Stop();
    __lcd_home();
    printf("%02x/%02x/%02x '#' to", time[6],time[5],time[4]);    //Print date in YY/MM/DD
    __lcd_newline();
    printf("%02x:%02x:%02x Start!", time[2],time[1],time[0]);    //HH:MM:SS
    __delay_ms(10);
}
void delay(int seconds) {
    for (int i = 0; i <= seconds; i ++) {
        __delay_1s();
    }
}

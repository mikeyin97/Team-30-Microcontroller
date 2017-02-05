/*
 * File:   main.c
 * Author: True Administrator
 *
 * Created on July 18, 2016, 12:11 PM
 */


#include <xc.h>
#include <stdio.h>
#include <math.h>
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"


void set_time(void);
void delay(int);
void stand_by(unsigned char[]);
void end_phase(int);
void update_time(unsigned char[]);
void message(char[], char[]);
int time_difference(unsigned char[], unsigned char[]);
int dec_to_hex(int);


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
    unsigned char start_time[7];
    unsigned char end_time[7];

    I2C_Master_Init(10000); //Initialize I2C Master with 100KHz clock
//    di(); // Disable all interrupts
    
    //only to configure the time
//    set_time(); 
    __lcd_clear();
    initLCD();
    __lcd_home();
    while (PORTBbits.RB1 == 0 || keys[(PORTB & 0xF0)>>4] != '#') {
        // RB1 is the interrupt pin, so if there is no key pressed, RB1 will be 0
            // the PIC will wait and do nothing until a key press is signaled
        stand_by(time);  
    }
    update_time(start_time);
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
    
    while (PORTBbits.RB1 == 0 || keys[(PORTB & 0xF0)>>4] != '*') {
//        operating phase
    }
    
    update_time(end_time);
    
    int d;
    d = time_difference(end_time, start_time);
    end_phase(d);
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
    
    update_time(time);
    
    __lcd_home();
    printf("%02x/%02x/%02x '#' to", time[6],time[5],time[4]);    //Print date in YY/MM/DD
    __lcd_newline();
    printf("%02x:%02x:%02x Start!", time[2],time[1],time[0]);    //HH:MM:SS
    __delay_ms(10);
}

void end_phase(int time) {
    int hours, min, sec;
    char pg;
    pg = 0;
    __lcd_clear();
    
    while (1) {
        if (pg == 0) {
            initLCD();
            while (PORTBbits.RB1 == 0 && keys[(PORTB & 0xF0)>>4] != 'A') {
                
                message("Next Page: A", "Count Sorted:10");
                
            }
            
            while(PORTBbits.RB1 == 1){
            // Wait until the key has been released
            }
            __lcd_clear();
            pg = 1;
        }
        
        else if (pg == 1) {
            initLCD();
            while (PORTBbits.RB1 == 0 && keys[(PORTB & 0xF0)>>4] != 'A') {
                
                message("Cat1:3   Cat2:4", "Cat3:1   Cat4:0");
                
            }
            
            while(PORTBbits.RB1 == 1){
            // Wait until the key has been released
            }
            __lcd_clear();
            pg = 2;
        }
        
        else if (pg == 2) {
            initLCD();
            while (PORTBbits.RB1 == 0 && keys[(PORTB & 0xF0)>>4] != 'A') {
                
                message("Cat5:3   Cat6:1", "Cat7:1   Cat8:0");
                
            }
            
            while(PORTBbits.RB1 == 1){
            // Wait until the key has been released
            }
            __lcd_clear();
            pg = 3;
        }
        
        else if (pg == 3) {
            initLCD();
            while (PORTBbits.RB1 == 0 && keys[(PORTB & 0xF0)>>4] != 'A') {
                hours = time/3600;
                min = (time%3600)/60;
                sec = time%60;
                __lcd_home();
                printf("Time: %d:%d:%d", hours, min, sec);
                __lcd_newline();
                printf("End-A to go back");
                
            }
            
            while(PORTBbits.RB1 == 1){
            // Wait until the key has been released
            }
            __lcd_clear();
            pg = 0;
        }
    }


    return;
}

void message(char line1[], char line2[]) {
    __lcd_home();
    printf(line1);
    __lcd_newline();
    printf(line2);
}

void update_time(unsigned char time[]) {
    
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
    
    
}
void delay(int seconds) {
    for (int i = 0; i <= seconds; i ++) {
        __delay_1s();
    }
}

int time_difference(unsigned char time1[], unsigned char time2[]) {
    int hr1, hr2, min1, min2, s1, s2;
    char d1, d2, d3;
    hr1 = time1[2]; hr2 = time2[2]; min1 = time1[1]; min2 = time2[1]; 
    s1 = time1[0]; s2 = time2[0];
    
    d1 = dec_to_hex(hr1) - dec_to_hex(hr2);
    d2 = dec_to_hex(min1) - dec_to_hex(min2);
    d3 = dec_to_hex(s1) - dec_to_hex(s2);
    
    __lcd_clear();
    __lcd_home();
    initLCD();
    return 3600*d1 + 60*d2 + d3;
}

int dec_to_hex(int num) {
    int i = 0, quotient = num, temp, hexnum = 0;
 
    
    while (quotient != 0) {
        temp = quotient % 16;
        
        hexnum += temp*pow(10, i);
        
        quotient = quotient / 16;
        i += 1;
    }
    return hexnum;
}




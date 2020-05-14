// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017
 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Language.h"
#include "Graphics.h"
#include "Player.h"
#include "Sound.h"

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

bool isCaught = false;
bool isPaused = false;
bool isThrown = false;
uint8_t  score = 0;
uint32_t ADCIn = 0;
uint32_t newADC = 0;

struct Football {
	uint32_t x; // x pos
	uint32_t y; // y pos
	const unsigned short *image;
};



SlidePot Slider(181, 23);
Player WR;
Player QB;
Football ball;
//Player CB; // maybe maybe not



// *********** Helper Functions / Misc *******************

void SysTick_Init(unsigned long period){
  //*** students write this ******
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = period - 1;
	NVIC_ST_CURRENT_R = 1;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; 
	NVIC_ST_CTRL_R = 0x07;
	
}

volatile unsigned long FallingEdges = 0;
void EdgeCounter_Init(void){       
	SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port F
	FallingEdges = 0;             // (b) initialize count and wait for clock
	GPIO_PORTF_DIR_R &= ~0x10;    // (c) make PF4 in (built-in button)
	GPIO_PORTF_AFSEL_R &= ~0x10;  //     disable alt funct on PF4
	GPIO_PORTF_DEN_R |= 0x10;     //     enable digital I/O on PF4
	GPIO_PORTF_PCTL_R &= ~0x000F0000; //  configure PF4 as GPIO
	GPIO_PORTF_AMSEL_R &= ~0x10;  //    disable analog functionality on PF4
	GPIO_PORTF_PUR_R |= 0x10;     //     enable weak pull-up on PF4
	GPIO_PORTF_IS_R &= ~0x10;     // (d) PF4 is edge-sensitive
	GPIO_PORTF_IBE_R &= ~0x10;    //     PF4 is not both edges
	GPIO_PORTF_IEV_R &= ~0x10;    //     PF4 falling edge event
	GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
	GPIO_PORTF_IM_R |= 0x10;      // (f) arm interrupt on PF4
	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00000000; // (g) priority 0
	NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC

}

void GPIOPortF_Handler(void){
	GPIO_PORTF_ICR_R = 0x10;      // acknowledge flag4
	FallingEdges = FallingEdges + 1;
	isPaused ^= 1;

}

void Delay(uint32_t n){
    uint32_t volatile time;
    while(n){
        time = 72724*2/91;  // 1msec, tuned at 80 MHz
        while(time){
            time--;
        }
        n--;
    }
}


void PortE_Init(){
    SYSCTL_RCGCGPIO_R |= 0x10;
    volatile unsigned long delay = SYSCTL_RCGCGPIO_R;
    GPIO_PORTE_DIR_R &= ~(0x03); // initialize PB0-1 as inputs
    GPIO_PORTE_DEN_R |= 0x03;
}


void PortF_Init(void){
  //*** students write this ******
	SYSCTL_RCGCGPIO_R |= 0x20;
	volatile unsigned long delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTF_DIR_R |= 0x0E;
	GPIO_PORTF_DEN_R |= 0x0E;
	
}

// Creating a struct for the Sprite.
typedef enum {
    dead, alive
} status_t;
struct sprite {
    uint32_t x;      // x coordinate
    uint32_t y;      // y coordinate
    const unsigned short *image; // ptr->image
    status_t life;            // dead/alive
};


typedef struct sprite sprite_t;

sprite_t bill = {60, 9, SmallEnemy20pointB, alive};

uint32_t time = 0;
volatile uint32_t flag;

void background(void) {
    flag = 1; // semaphore
    if (bill.life == alive) {
        bill.y++;
    }
    if (bill.y > 155) {
        bill.life = dead;
    }
}

void clock(void) {
    time++;
}

/*****************************************************************/

/****************** Wide Receiver Movements **********************/

// football dimensions 11 x 11
//void WR_Init(){ // initialize pos/image
//    WR.image = wr;
//    WR.x = 54;
//    WR.y = 65;
//}
//
//void MoveWR(int level){ // helper func that will move WR at different speeds based on level input
//    if (level == 1) {
//        ST7735_FillRect(18, 41, 108, 25, ST7735_Color565(49, 176, 42)) ;
//        ST7735_DrawBitmap(WR.x, WR.y, wr, 22, 19);
//    }
//}
//
//
//void WR_Move(int level){ // main loop that moves WR up and down @ given speed depending on level
//    if (level == 1) {
//        while (isCaught == false){
//            // level 1, lower speed
//
//            while (WR.x > 18){ // lower bound for screen, move down
//                WR.x -= 4;
//                MoveWR(level);
//                Delay(100);
//            }
//            while (WR.x < 101) { // upper bound for screen, move up
//                WR.x +=4;
//                MoveWR(level);
//                Delay(100);
//            }
//        }
//    }
//}



bool inRange(uint32_t high, uint32_t low, uint32_t input) { // check if input is within a certain range of vals
	if ((input <= high) && (input >= low)){ return true;}
}


void moveBall(){ // use same logic as WR_Move but to move ball across field
	bool left = false;
	if (ball.y > 60) {
		left = true;
	}
	if (left) {
		ball.y-=3;
	}
	else {
		isThrown = true;
	}
}

void throwBall() {
		uint16_t white = ST7735_Color565(255,255,255);
		moveBall();			
		if(isThrown) {return;}
		if (ball.y <= WR.y+4 && ball.y >= WR.y-4 && ball.x <= WR.x+6 && ball.x >= WR.x-6) {
			ST7735_DrawFastHLine(0, 70, 128, white); // 10 yd line
			isCaught = true;
			isThrown = true;
		}
		
		// replace yard lines as football sprite overwrite them
		ST7735_DrawFastHLine(0, 70, 128, white); // 10 yd line
		ST7735_DrawFastHLine(0, 100, 128, white); // 20 yd line
		ST7735_DrawFastHLine(0, 130, 128, white); // 30 yd line 
		
		// draw ball and overwrite prev
		ST7735_DrawBitmap(ball.x, ball.y, football, 8, 12);
		ST7735_FillRect(ball.x-2, ball.y, 10, 5, ST7735_Color565(49, 176, 42));
		
		Delay(100);
}





void levelOne(){ // game engine? s
    WR.Init(54, 65, wr);
		QB.Init(54, 155, qb);
		ball.x = 63;
		ball.y = 135;
    // init QB and whatever else
    while(isPaused == false || isThrown == false) { 
				
		
				if ((GPIO_PORTE_DATA_R&0x01) != 0) { // button 1 (throw)
					
					QB.image = qbthrown; // new qb sprite (thrown)
					ST7735_DrawBitmap(QB.x,QB.y, qbthrown, 29, 15);					
					while (isThrown == false) {   // as ball pos is updated across field, keep moving WR
						throwBall();
						WR.WR_Move(2);
						ST7735_FillRect(WR.x+10, WR.y-18, 15, 15, ST7735_Color565(49, 176, 42));
						ST7735_FillRect(WR.x-3, WR.y-18, 15, 15, ST7735_Color565(49, 176, 42));
						ST7735_DrawBitmap(WR.x, WR.y, WR.image, 22, 19);
					
					}
				}
				
				if ((GPIO_PORTE_DATA_R&0x02) != 0) {
					// if the pause button is pressed
					Delay(500);
					while ((GPIO_PORTE_DATA_R &0x02) == 0) {};
					Delay(500);
				}
				
				if (isCaught == true){ // ball is caught
					score++;
					TDScreen();
					Sound_Touchdown();
					Delay(5000);
					break;
				}
				if (isThrown == true) { // prev condition would break on TD, so must be a miss
				missScreen();	
				Delay(5000);
				break;
				}
				
				Slider.Save(ADC_In());
				newADC = Slider.Distance();
				if ((newADC <= 0x60) && (newADC >= 0x19)){
				QB.x = newADC;
				ball.x = newADC;
				ST7735_FillRect(25, 135, 100 , 25, ST7735_Color565(49,176,42));
				ST7735_DrawBitmap(QB.x, QB.y, qb, 29, 15);
				}
				
        WR.WR_Move(2);
        ST7735_FillRect(WR.x+10, WR.y-18, 15, 15, ST7735_Color565(49, 176, 42));
				ST7735_FillRect(WR.x-3, WR.y-18, 15, 15, ST7735_Color565(49, 176, 42));
        ST7735_DrawBitmap(WR.x, WR.y, WR.image, 22, 19);
        // QB stuff

        // CB stuff

        Delay(100);
    }
}

void levelTwo() {
WR.Init(54, 65, wr);
		QB.Init(54, 155, qb);
		ball.x = 63;
		ball.y = 135;
    // init QB and whatever else
    while(isPaused == false || isThrown == false) { 
		
		
				if ((GPIO_PORTE_DATA_R&0x01) != 0) { // button 1 (throw)
				
					QB.image = qbthrown; // new qb sprite (thrown)
					ST7735_DrawBitmap(QB.x,QB.y, qbthrown, 29, 15);					
					while (isThrown == false) {   // as ball pos is updated across field, keep moving WR
						throwBall();
						WR.WR_Move(3);
						ST7735_FillRect(WR.x+10, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
						ST7735_FillRect(WR.x-5, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
						ST7735_DrawBitmap(WR.x, WR.y, WR.image, 22, 19);
					
					}
				}
				
				if ((GPIO_PORTE_DATA_R&0x02) != 0) {
					// if the pause button is pressed
					Delay(500);
					while ((GPIO_PORTE_DATA_R &0x02) == 0) {};
					Delay(500);
				}
				
				if (isCaught == true){ // ball is caught
					score++;
					TDScreen();
					Sound_Touchdown();
					Delay(5000);
					break;
				}
				if (isThrown == true) { // prev condition would break on TD, so must be a miss
				missScreen();	
				Delay(5000);
				
				break;
				}
				
				Slider.Save(ADC_In());
				newADC = Slider.Distance();
				if ((newADC <= 0x60) && (newADC >= 0x19)){ // define boundaries for QB movement
				QB.x = newADC;
				ball.x = newADC;
				ST7735_FillRect(25, 135, 100 , 25, ST7735_Color565(49,176,42));
				ST7735_DrawBitmap(QB.x, QB.y, qb, 29, 15);
				}
				
        WR.WR_Move(3);
        ST7735_FillRect(WR.x+10, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
				ST7735_FillRect(WR.x-5, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
        ST7735_DrawBitmap(WR.x, WR.y, WR.image, 22, 19);


        Delay(100);
    }
}

void levelThree() {
		WR.Init(54, 65, wr);
		QB.Init(54, 155, qb);
		ball.x = 63;
		ball.y = 135;
    // init QB and whatever else
    while(isPaused == false || isThrown == false) { 
		
		
				if ((GPIO_PORTE_DATA_R&0x01) != 0) { // button 1 (throw)
				
					QB.image = qbthrown; // new qb sprite (thrown)
					ST7735_DrawBitmap(QB.x,QB.y, qbthrown, 29, 15);					
					while (isThrown == false) {   // as ball pos is updated across field, keep moving WR
						throwBall();
						WR.WR_Move(4);
						ST7735_FillRect(WR.x+10, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
						ST7735_FillRect(WR.x-5, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
						ST7735_DrawBitmap(WR.x, WR.y, WR.image, 22, 19);
					
					}
				}
				
				if ((GPIO_PORTE_DATA_R&0x02) != 0) {
					// if the pause button is pressed
					Delay(500);
					while ((GPIO_PORTE_DATA_R &0x02) == 0) {};
					Delay(500);
				}
				
				if (isCaught == true){ // ball is caught
					score++;
					TDScreen();
					Sound_Touchdown();
					Delay(5000);
					break;
				}
				if (isThrown == true) { // prev condition would break on TD, so must be a miss
				missScreen();	
				Delay(5000);
				break;
				}
				
				Slider.Save(ADC_In());
				newADC = Slider.Distance();
				if ((newADC <= 0x60) && (newADC >= 0x19)){ // define boundaries for QB movement
				QB.x = newADC;
				ball.x = newADC;
				ST7735_FillRect(25, 135, 100 , 25, ST7735_Color565(49,176,42));
				ST7735_DrawBitmap(QB.x, QB.y, qb, 29, 15);
				}
				
        WR.WR_Move(4);
        ST7735_FillRect(WR.x+10, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
				ST7735_FillRect(WR.x-5, WR.y-18, 25, 15, ST7735_Color565(49, 176, 42));
        ST7735_DrawBitmap(WR.x, WR.y, WR.image, 22, 19);


        Delay(100);
    }
}
/*******************************************************************/


	int main(void){
    PLL_Init(Bus80MHz);       // Bus clock is 80 MHz
    Random_Init(1);
    Output_Init();
    Timer0_Init(&background,1600000); // 50 Hz
    Timer1_Init(&clock,80000000); // 1 Hz
    PortE_Init();
		PortF_Init();
		ST7735_InitR(INITR_BLACKTAB);
		Sound_Init();
		SysTick_Init(8000000);
		EnableInterrupts();
		ADC_Init();
		
		pickLang();
		loadField(0);	

		
    // begin main game engine
		
    levelOne();
    if(isCaught) {
        isCaught = false;
				isThrown = false;
				loadField(1);
        levelTwo();
        if(isCaught) {
            isCaught = false;
						isThrown = false;
						loadField(2);
            levelThree();
            if(isCaught) {
                gameWin();
            } else {
							score = 2;
							gameOver(score);
                // user lost on level 3
            }
        } else {
					score = 1;
					gameOver(score);
            // user lost on level 2
        }
    } else {
			score = 0;
			gameOver(score);
        // user lost on level 1
    }
		
		


}


void SysTick_Handler(void){ // every sample
    //*** students write this ******
// should call ADC_In() and Sensor.Save
	//newADC = ADC_In();
	Slider.Save(ADC_In());
}

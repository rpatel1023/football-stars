// SlidePot.cpp
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0 and use a slide pot to measure distance
// Created: 3/28/2018 
// Student names: Nishanth Navali & Rushi Patel
// Last modification date: 4/19/2020

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
//*** students write this ******
	uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x08;
	while((SYSCTL_PRGPIO_R&0x08) != 0x08){};
	GPIO_PORTD_DIR_R &= ~(0x04);
	GPIO_PORTD_AFSEL_R |= 0x04; // enable alternate fun
	GPIO_PORTD_DEN_R &= ~(0x04); // disable digital I/O
	GPIO_PORTD_AMSEL_R |= 0x04; // enable analog input
	SYSCTL_RCGCADC_R |= 0x01;   // ADC0 on
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	ADC0_PC_R = 0x01;
	ADC0_SSPRI_R = 0x0123;
	ADC0_ACTSS_R &= ~(0x0008);
	ADC0_EMUX_R &= ~(0xF000);
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 5;
	ADC0_SSCTL3_R = 0x0006;
	ADC0_IM_R &= ~(0x0008);
	ADC0_ACTSS_R |= 0x0008;
//  ADC0_SAC_R = 4;   // 16-point averaging, move this line into your ADC_Init()

}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
//*** students write this ******
	uint32_t input;
	ADC0_PSSI_R = 0x0008; // bit 3 = 1, start sample conv.

	while((ADC0_RIS_R&0x08) == 0) {}; // check RIS bit 3 for flag, 0 = busy converting, 1 = done
	input = ADC0_SSFIFO3_R & 0xFFF; // FIFO buffer, read 12 bits of data
	ADC0_ISC_R = 0x0008; // write 1 to bit 3 to clear flag
		
  return input;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
//*** students write this ******
// initialize all private variables
    this->flag = 0;
    this->distance = 0;
    this->data = 0;
// make slope equal to m and offset equal to b
	this->slope = m;
	this->offset = b;
}

void SlidePot::Save(uint32_t n){
//*** students write this ******
// 1) save ADC sample into private variable
    this->data = n;
// 2) calculate distance from ADC, save into private variable
    this->distance = Convert(this->data);
// 3) set semaphore flag = 1
    this->flag = 1;

}
uint32_t SlidePot::Convert(uint32_t n){
  //*** students write this ******
  // use calibration data to convert ADC sample to distance

  return ((n * this->slope) + this->offset) / 4096;
}

void SlidePot::Sync(void){ // TODO: This is definitely wrong
// 1) wait for semaphore flag to be nonzero
    while(this->flag == 0);
// 2) set semaphore flag to 0
    this->flag = 0;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  //*** students write this ******
  // return last calculated ADC sample
  return this->data;
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
  //*** students write this ******
  // return last calculated distance in 0.001cm
  return this->distance;
}



//
// Created by Nishanth on 4/28/2020.
//

#include "Player.h"
#include "Graphics.h"
#include <stdint.h>



Player::Player() {
    this->x = 50;
    this->y = 50;
}

void DelayPlayer(uint32_t n){
    uint32_t volatile time;
    while(n){
        time = 72724*2/91;  // 1msec, tuned at 80 MHz
        while(time){
            time--;
        }
        n--;
    }
}

void Player::Init(uint32_t x_cord, uint32_t y_cord, const unsigned short *img ) {
    this->x = x_cord;
    this->y = y_cord;
    this->image = img;
}
void Player::WR_Move(int speed) {
    static bool up = false;
    if(this->x < 26) {
        up = true;
    }
    else if(this->x > 99) {
        up = false;
    }
    if(up) {
        this->x += speed;
    } else {
        this->x -= speed;
    }

}


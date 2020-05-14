//
// Created by Nishanth on 4/28/2020.
//

#ifndef LAB10_C_PLAYER_H
#define LAB10_C_PLAYER_H
#include <stdint.h>

class Player {
public:
    uint32_t x;      // x coordinate
    uint32_t y;      // y coordinate
    const unsigned short *image;

    Player();
    void Init(uint32_t x_cord, uint32_t y_cord, const unsigned short *img);
    void WR_Move(int speed);
};


#endif //LAB10_C_PLAYER_H

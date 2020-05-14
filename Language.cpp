//
// Created by Nishanth on 4/26/2020.
//

#include "Language.h"
#include "ST7735.h"

uint16_t text = ST7735_Color565(255,255,255);

Language::Language() {
    selector = -1;
};

void Language::set(int selection) {
    selector = selection;
}

void Language::outTD() {
    if(selector) {
        // print touchdown to console in spanish
    }
    else {
        // print touchdown in english

    }
};

void Language::outInstructions() {
    if(selector) {
        // print instructions to console in spanish
			ST7735_FillScreen(0);
			ST7735_DrawString(3, 0, "¡Bienvenidos a", text);
			ST7735_DrawString(3, 1, "Football Stars!", text);
			ST7735_DrawString(1, 3, "Tu objectivo es ", text);
			ST7735_DrawString(1, 4, "anotar 3 touchdowns.", text);
			ST7735_DrawString(1, 6, "Use el control", text);
			ST7735_DrawString(1, 7, "deslizante para", text);
			ST7735_DrawString(1, 8, "mover su QB e intente", text);
			ST7735_DrawString(1, 9, "apuntar a su ", text);
			ST7735_DrawString(1, 10, "receptor ancho", text);
			ST7735_DrawString(1, 12, "Use el botón 1 para", text);
			ST7735_DrawString(1, 13, "lanzar la pelota", text);
			ST7735_DrawString(1, 14, "y el botón 2", text);
			ST7735_DrawString(1, 15, "para pausar", text);
    }
    else {
        // print instructions in english
			ST7735_FillScreen(0);
			ST7735_DrawString(5, 1, "Welcome to", text);
			ST7735_DrawString(3, 2, "Football Stars!", text);
			ST7735_DrawString(1, 4, "Your goal is to", text);
			ST7735_DrawString(1, 5, "score 3 touchdowns.", text);
			ST7735_DrawString(1, 7, "Use the slider to", text);
			ST7735_DrawString(1, 8, "move your QB and", text);
			ST7735_DrawString(1, 9, "try to target your", text);
			ST7735_DrawString(1, 10, "wide receiver.", text);
			ST7735_DrawString(1, 12, "Use button 1 to ", text);
			ST7735_DrawString(1, 13, "throw the ball and", text);
			ST7735_DrawString(1, 14, "button 2 to pause.", text);
    }
}







/*
Efectos de luces animadas para cuando el programa esté en modo "espera".
*/

#pragma once
#include <Arduino.h>
#include <entorno.h>

// Calcula el desplazamiento de una diagonal
void calculaDiagonal(int vuelta, bool inversa, CRGB color);

// Calcula el desplazamiento de una línea horizontal/vertical
void calculaBarra(int vez, bool horizontal, CRGB color);

// Calcula el tamaño de un "cuadrado".
void calculaCuadrado(int vez, CRGB color);
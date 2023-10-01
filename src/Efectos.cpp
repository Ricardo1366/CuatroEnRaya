#include <Efectos.h>
#include <entorno.h>

// =====================================================================================

void calculaDiagonal(int vuelta, bool inversa, CRGB color)
{
    static int x, y, fila;

    // Dibujamos el punto de inicio de la diagonal
    fila = inversa ? NUMEROFILAS - 1 : 0;

    if (vuelta < NUMEROCOLUMNAS)
    {
        leds[tableroLed.posicion(vuelta, fila)] = color;
#if defined(DEBUG)
        Serial.print(vuelta);
        Serial.print(",");
        Serial.print(fila);
        Serial.print("|");
#endif
    }
    // Resto de puntos de la diagonal.
    for (byte posicion = 1; posicion <= vuelta; posicion++)
    {
        // Punto inicial
        if (inversa)
        {
            x = vuelta - posicion;
            y = NUMEROFILAS - posicion - 1;
        }
        else
        {
            x = vuelta - posicion;
            y = posicion;
        }

        if (x >= 0 && x < NUMEROCOLUMNAS && y >= 0 && y < NUMEROFILAS)
        {
            leds[tableroLed.posicion(x, y)] = color;
#if defined(DEBUG)
            Serial.print(x);
            Serial.print(",");
            Serial.print(y);
            Serial.print("|");
#endif
        }
    }
#if defined(DEBUG)
    Serial.print("\n|");
#endif
}

void calculaBarra(int vez, bool horizontal, CRGB color)
{
    static int numeroLeds;

    numeroLeds = horizontal ? NUMEROCOLUMNAS : NUMEROFILAS;
#if defined(DEBUG)
    Serial.print("\n|");
#endif
    // Si es mayor que el número de columnas volvemos a empezar por la cero.
    vez %= numeroLeds;

    for (int i = 0; i < numeroLeds; i++)
    {
        if (horizontal)
        {
            leds[tableroLed.posicion(vez, i)] = color;
#if defined(DEBUG)
            Serial.print(vez);
            Serial.print(",");
            Serial.print(i);
            Serial.print("|");
#endif
        }
        else
        {
            leds[tableroLed.posicion(i, vez)] = color;
#if defined(DEBUG)
            Serial.print(i);
            Serial.print(",");
            Serial.print(vez);
            Serial.print("|");
#endif
        }
    }
}

void calculaCuadrado(int vez, CRGB color)
{
#if defined(DEBUG_)
    Serial.print("Vez = ");
    Serial.println(vez);
#endif
    // Se rrecorren todos los pixel del cuadrado para calcular si está encendido o apagado.
    for (byte f = 0; f < NUMEROFILAS; f++)
    {
        for (byte c = 0; c < NUMEROCOLUMNAS; c++)
        {
            // Solo se calculan los pixel dentro del area del cuadrado.
            if (f >= vez && f <= (NUMEROFILAS - vez - 1))
            {
                // Si es la primera o última fila del cuadrdo se encienden todos los pixel
                if (f == vez || f == (NUMEROFILAS - vez - 1))
                {
                    if (c >= vez && c <= (NUMEROCOLUMNAS - vez - 1))
                        leds[tableroLed.posicion(c, f)] = color;
                }
                else
                {
                    // Solo se enciende el primer y último pixel de la línea.
                    if (c == vez || c == (NUMEROCOLUMNAS - vez - 1))
                    {
                        leds[tableroLed.posicion(c, f)] = color;
                    }
                }
            }
        }
    }
}
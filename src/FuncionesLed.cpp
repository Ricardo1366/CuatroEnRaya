// Ilumina cada celda del tablero con su color correspondiente.
#include <Arduino.h>
#include <entorno.h>
#include <FuncionesLed.h>

// Enciende el led de la letra seleccionada.
void AsignaLetra()
{
#if defined(DEBUG)
	Serial.print(F("Encendemos letra "));
	Serial.println(letra);
#endif
	// Iluminamos solo la letra selecciomada.
	for (byte i = 0; i < 8; i++)
	{
		leds[pulsadores.posicion(i,ledsLetras)] = i == letra ? color[nivel][turno1 ? 1 : 2] : CRGB::Black;
	}
	LEDS.show();
}

// Enciende el led del número seleccionado.
void AsignaNumero()
{
	//  Iluminamos solo la letra selecciomada.
	for (byte i = 0; i < 8; i++)
	{
		leds[pulsadores.posicion(i,ledsNumeros)] = i == numero ? color[nivel][turno1 ? 1 : 2] : CRGB::Black;
	}
	LEDS.show();
}

// Calculamos el valor del color.
CRGB DameColor(byte _color, byte _intensidad)
{

	CRGB _Color = colores[_color];
#if defined(VERCOLOR)
	Serial.print(F("Color = "));
	Serial.print(_Color.r);
	Serial.print("|");
	Serial.print(_Color.g);
	Serial.print("|");
	Serial.print(_Color.b);
	Serial.println();
	Serial.print(F("Intensidad = "));
	Serial.print(_intensidad);
	Serial.println();
#endif
	_Color.r >>= (8 - _intensidad);
	_Color.g >>= (8 - _intensidad);
	_Color.b >>= (8 - _intensidad);
#if defined(VERCOLOR)
	Serial.print(F("Resultado = "));
	Serial.print(_Color.r);
	Serial.print("|");
	Serial.print(_Color.g);
	Serial.print("|");
	Serial.print(_Color.b);
	Serial.println();
#endif
	return _Color;
}

// Calcula la intensidad del color de fondo.
byte DameIntensidadFondo(byte nivel)
{
	if (nivel < 5)
	{
		return nivel > 1 ? nivel / 2 : 1;
	}
	else
	{
		return nivel / 2 - 1;
	}
}

// Dibuja en el tablero el número de jugador que empieza con su color.
void DibujaTurno(byte turnoActual)
{
	byte posicion = 0;
	CRGB color_ = turnoActual == 1 ? DameColor(configTemp.color1, configTemp.NivelIntensidad) : DameColor(configTemp.color2, configTemp.NivelIntensidad);
	CRGB colorFondo_ = DameColor(0, DameIntensidadFondo(configTemp.NivelIntensidad));
	// Dibujamos el turno en el tablero.
	for (byte f = 0; f < 8; f++)
	{
		posicion = 1;
		for (byte p = 0; p < 8; p++)
		{
			leds[tableroLed.posicion(f, p)] = (numeros[turnoActual][7 - f] && posicion) ? color_ : colorFondo_;
			posicion <<= 1;
		}
	}
}

// Dibuja el estado actual del tablero.
void PintaTablero()
{
	// Pintamos tablero.
	for (byte c = 0; c < columnas; c++)
	{
		for (byte f = 0; f < filas; f++)
		{
			leds[tableroLed.posicion(c, f)] = color[nivel][tablero[c][f]];
		}
	}
	// Pintamos letra.
	AsignaLetra();
}


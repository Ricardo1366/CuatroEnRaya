#include <Procesos.h>
#include <entorno.h>

// Comprueba si hay huecos libres en la columna elegida.
bool Comprobar(byte _columna)
{

#if defined(DEBUG)
	Serial.println(F("Comprobamos hay un hueco libre."));
#endif
	if (_columna >= columnas)
	{
#if defined(DEBUG)
		Serial.print(F("Columna incorrecta "));
		Serial.println(_columna);
#endif

		return false;
	}
	fila = DameHueco(_columna);
	if (fila < filas)
	{
		columna = _columna;
#if defined(DEBUG)
		Serial.print(F("Seleción columna/fila "));
		Serial.print(columna);
		Serial.print("/");
		Serial.println(fila);
#endif
		return true;
	}
	// La columna está llena. Mostramos error.
#if defined(DEBUG)
	Serial.println(F("Error columna llena."));
#endif
	letra = 255;
	return false;
}

// Comprobamos si el jugador actual ha conseguido la victoria.
bool ComprobarGanador()
{
	// Comprobamos Lineas horizontales.
	byte turno = turno1 ? 1 : 2;
	for (byte f = 0; f < 6; f++) // Se comprueban todas las filas.
	{
		for (byte c = 0; c < 4; c++) // En cada fila pueden haber 4 posiciones ganadoras
		{
			if (tablero[c][f] == turno &&
				tablero[c + 1][f] == turno &&
				tablero[c + 2][f] == turno &&
				tablero[c + 3][f] == turno)
			{
				ganador.columna = c;
				ganador.fila = f;
				ganador.jugador = turno;
				ganador.modo = Modo::Horizontal;
				return true;
			}
		}
	}

	// Comprobamos líneas verticales.
	for (byte c = 0; c < 7; c++) // Se comprueban las columnas.
	{
		for (byte f = 0; f < 3; f++) // En cada columna pueden haber 3 posiciones ganadoras
		{
			if (tablero[c][f] == turno &&
				tablero[c][f + 1] == turno &&
				tablero[c][f + 2] == turno &&
				tablero[c][f + 3] == turno)
			{
				ganador.columna = c;
				ganador.fila = f;
				ganador.jugador = turno;
				ganador.modo = Modo::Vertical;
				return true;
			}
		}
	}

	// Comprobamos diagonal derecha
	for (byte f = 0; f < 3; f++) // Se comprueban las filas.
	{
		for (byte c = 0; c < 4; c++) // Se avanza por columna
		{
			if (tablero[c][f] == turno &&
				tablero[c + 1][f + 1] == turno &&
				tablero[c + 2][f + 2] == turno &&
				tablero[c + 3][f + 3] == turno)
			{
				ganador.columna = c;
				ganador.fila = f;
				ganador.jugador = turno;
				ganador.modo = Modo::Diagonal_derecha;
				return true;
			}
		}
	}
	// Comprobamos diagonal izquierda
	for (byte f = 0; f < 3; f++) // Se comprueban las filas.
	{
		for (byte c = 3; c < 7; c++) // Se avanza por columna
		{
			if (tablero[c][f] == turno &&
				tablero[c - 1][f + 1] == turno &&
				tablero[c - 2][f + 2] == turno &&
				tablero[c - 3][f + 3] == turno)
			{
				ganador.columna = c;
				ganador.fila = f;
				ganador.jugador = turno;
				ganador.modo = Modo::Diagonal_izquierda;
				return true;
			}
		}
	}
	return false;
}

// Busca la primera posición libre en una columna.
byte DameHueco(byte columna)
{
	fila = 255;
	// Buscamos la primera fila libre.
	for (byte i = 0; i < filas; i++)
	{
		if (tablero[columna][i] == 0)
		{
			fila = i;
			break;
		}
	}
	return fila;
}

// Resetea y configura el expansor.
bool IniciaExpansor()
{
	byte contador = 0;
	digitalWrite(pinReset, LOW); // Reset inverso. Se resetea al recibir un 0 (LOW)
	delay(1);
	digitalWrite(pinReset, HIGH); // Reset inverso. Se resetea al recibir un 0 (LOW)
	delay(1);

	// Habilitamos el expansor en la dirección indicada. (Lo intentamos 10 veces)
	while (!expansor.begin_I2C(direccionExpansor) && contador < 10)
	{
		delay(10);
		contador++;
	}

	delay(1);
	if (contador != 10)
	{
#ifdef DEBUG
		Serial.println(F("\nExpansor habilitado."));
#endif // DEBUG
	}
	else
	{
		SinExpansor();
		return false;
	}
	return true;
}

// Activa un proceso determinado.
void IniciaProceso()
{
#if defined(DEBUG)	
	Serial.print("Proceso iniciado "); Serial.print(proceso);
	Serial.print("  Animación = "); Serial.println(animacion);
#endif
	contador = 0;
	Procesos.attach_ms(tiempo_PorPaso, ActivaPaso);
}

// Animación inicio al encender el tablero.
void IniciaTablero()
{
	// Configuramos y activamos el proceso de animación.
	proceso = Pr_InicioTablero;
	repeticiones = 9;
	tiempo_PorPaso = 500;
	enProceso = true;
	siguientePaso = true;
	IniciaProceso();
}

// Han pulsado una letra/columna en modo configuración.
void LetraEnConfig()
{
	switch (modoConfig)
	{
	case ModoConfiguracion::Ninguna:
		// Si no hay seleccionado un modo de configuración no hacemos nada.
		break;
	case ModoConfiguracion::Turno:
		// Si no es el pulsador 1 o 2 no hacemos nada.
#if defined(DEBUG)
	Serial.println("Asignamos turno");
#endif
		if (letra > 1)
		{
			break;
		}
		configTemp.Turno = letra == 0 ? true : false;
		MuestraConfiguracion();
		break;
	case ModoConfiguracion::Color1:
		if (letra > 5)
		{
			break;
		}
		if (letra + 1 == configTemp.color2)
		{
			break;
		}
		configTemp.color1 = letra + 1;
		MuestraConfiguracion();
		break;
	case ModoConfiguracion::Color2:
		if (letra > 5)
		{
			break;
		}
		if (letra + 1 == configTemp.color1)
		{
			break;
		}
		configTemp.color2 = letra + 1;
		MuestraConfiguracion();
		break;
	case ModoConfiguracion::Intensidad:
		configTemp.NivelIntensidad = letra + 1;
		MuestraConfiguracion();
		break;
	default:
		break;
	}
}

// Muetra el "valor" del modo de configuración seleccionado.
void MuestraConfiguracion()
{
	// Borramos todos los leds.
	LEDS.showColor(CRGB::Black);

	switch (modoConfig)
	{
	case ModoConfiguracion::Ninguna:
		// Iluminamos todos los leds con el color de fondo.
		LEDS.showColor(DameColor(0, DameIntensidadFondo(configTemp.NivelIntensidad)));
		break;

	case ModoConfiguracion::Turno:

		DibujaTurno(configTemp.Turno ? 1 : 2);

		// Iluminamos el pulsador 1 y 2 con los colores de cada turno.
		leds[pulsadores.posicion(0,ledsLetras)] = DameColor(configTemp.color1, configTemp.NivelIntensidad);
		leds[pulsadores.posicion(1,ledsLetras)] = DameColor(configTemp.color2, configTemp.NivelIntensidad);

		LEDS.show();
		break;
	case ModoConfiguracion::Color1:
		// Es el color del turno 1
		DibujaTurno(1);

		// Iluminamos las teclas con los distintos colores excepto el que tiene el turno 2.
		for (byte i = 1; i < 7; i++)
		{
			leds[pulsadores.posicion(i - 1,ledsLetras)] = i != configTemp.color2 ? DameColor(i, configTemp.NivelIntensidad) : CRGB::Black;
		}

		// Los últimos dos pulsadores no tienen color asignado.
		leds[pulsadores.posicion(6,ledsLetras)] = CRGB::Black;
		leds[pulsadores.posicion(7,ledsLetras)] = CRGB::Black;

		LEDS.show();
		break;
	case ModoConfiguracion::Color2:
		// Es el color del turno 1
		DibujaTurno(2);

		// Iluminamos las teclas con los distintos colores excepto el que tiene el turno 2.
		for (byte i = 1; i < 7; i++)
		{
			leds[pulsadores.posicion(i - 1,ledsLetras)] = i != configTemp.color1 ? DameColor(i, configTemp.NivelIntensidad) : CRGB::Black;
		}

		// Los últimos dos pulsadores no tienen color asignado.
		leds[pulsadores.posicion(6,ledsLetras)] = CRGB::Black;
		leds[pulsadores.posicion(7,ledsLetras)] = CRGB::Black;

		LEDS.show();
		break;

	case ModoConfiguracion::Intensidad:
		CRGB color1, color2, colorFondo, colorFila;
		color1 = DameColor(configTemp.color1, configTemp.NivelIntensidad);
		color2 = DameColor(configTemp.color2, configTemp.NivelIntensidad);
		colorFondo = DameColor(0, DameIntensidadFondo(configTemp.NivelIntensidad));

		// Ya tenemos los colores, ahora dibujamos el tablero mostrando los tres colores.
		for (byte f = 0; f < 8; f++)
		{
			colorFila = f % 2 == 0 ? color1 : color2;
			for (byte c = 0; c < 8; c++)
			{
				leds[tableroLed.posicion(f, c)] = c <= f ? colorFila : colorFondo;
			}
		}

		// Iluminamos la fila de letras con los dos colores cambiando la intensidad.
		for (byte i = 0; i < 8; i++)
		{
			leds[pulsadores.posicion(i,ledsLetras)] = DameColor(i % 2 == 0 ? config.color1 : config.color2, i);
		}
		// Iluminamos la fila de los números con el color de fondo para la intensidad correspondiente.
		for (byte i = 0; i < 8; i++)
		{
			leds[pulsadores.posicion(i,ledsLetras)] = DameColor(0, DameIntensidadFondo(i));
		}

		break;
	default:
		break;
	}
}

// Han pulsado OK estando en modo Juego.
void OkEnJuego()
{
	if (letra < 8)
	{
		if (Comprobar(letra))
		{
			// Hay hueco en la columna. Ponemos la ficha.
			tiempo_PorPaso = tiempo_PonerFicha;
			repeticiones = filas - fila;
			proceso = Pr_PonerFicha;
			enProceso = true;
			siguientePaso = true;
			IniciaProceso();
		}
		else
		{
			// Columna llena.
			// Salvamos los valores de la columna para poder hacerla parpadear;
			for (byte i = 0; i < filas; i++)
			{
				salvaColumna[i] = tablero[columna][i];
			}
			tiempo_PorPaso = tiempo_ColumnaLlena;
			repeticiones = 10;
			proceso = Pr_ColumnaLlena;
			enProceso = true;
			siguientePaso = true;
			IniciaProceso();
		}
	}
	else
	{
#if defined(DEBUG)
		Serial.println(F("No hay letra seleccionada. No hacemos nada."));
#endif
	}
}

// Guardamos la configuración actual.
void SalvaConfiguracion()
{
	config = configTemp;
	EEPROM.begin(sizeEEPROM);
	EEPROM.put(0, config);
	EEPROM.commit();
	EEPROM.end();
}

// No se ha detectado el expansor. Mostramos error en el tablero.
void SinExpansor()
{
	for (byte i = 0; i < 6; i++)
	{
		leds[tableroLed.posicion(i, i)] = CRGB::Red;
		leds[tableroLed.posicion(i, 5 - i)] = CRGB::Red;
	}
	LEDS.show();
}

// Cargamos los valores por defecto de la configuración.
void ValoresxDefecto()
{
	// Cargamos valores por defecto.
	config.color1 = 4;
	config.color2 = 1;
	config.Turno = true;
	config.NivelIntensidad = 6;
	config.configurado = 1;
	// Grabamos configuración.
	EEPROM.put(0, config);
	EEPROM.commit();
}


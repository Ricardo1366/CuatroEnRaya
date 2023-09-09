/*
PINES USADOS
===================================
Altavoz = 			D7;
Interrupciones = 	D6; (Rojo)
Leds = 				D5; (Blanco)
Reset = 			D8;
SCL = 				D1; (Amarillo)
SDA = 				D2; (Azul)

*/
#include <Arduino.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <FastLED.h>
#include <Adafruit_MCP23X17.h>
#include <MatrizLed.h>
#include <WiFiServer.h>

#define DEBUG
#define NOVERCOLOR

void ActivaPaso();
void AsignaLetra();
void AsignaNumero();
bool Comprobar(byte);
bool ComprobarGanador();
void ComprobarLectura();
CRGB DameColor(byte, byte);
byte DameHueco(byte);
byte DameIntensidadFondo(byte);
void DibujaTurno(byte);
bool IniciaExpansor();
void IniciaProceso();
void IniciaTablero();
void LetraEnConfig();
void MuestraConfiguracion();
void OkEnJuego();
void PintaTablero();
void SalvaConfiguracion();
void SinExpansor();
void ValoresxDefecto();

enum class Modo
{
	Horizontal,
	Vertical,
	Diagonal_derecha,
	Diagonal_izquierda
};

// Guarda los datos de la jugada ganadora.
struct Ganador
{
	byte jugador;
	byte fila;
	byte columna;
	Modo modo;
};

// Configuración del juego.
struct Config4eR
{
	byte configurado;
	byte color1;
	byte color2;
	bool Turno;
	byte NivelIntensidad;
};

// Modo configuración
enum ModoConfiguracion
{
	Ninguna,
	Turno,
	Color1,
	Color2,
	Intensidad
};

// Pulsadores
const byte P_OK = 8;
const byte P_START = 9;
const byte P_CONFIG = 10;
const byte P_REINICIO = 11;
const byte P_P1 = 12;
const byte P_P2 = 13;
const byte P_P3 = 14;
const byte P_P4 = 15;
// Procesos
const byte Pr_PonerFicha = 1;
const byte Pr_ColumnaLlena = 2;
const byte Pr_InicioTablero = 3;
const byte Pr_MuestraGanador = 4;

byte numeros[10][8] = {{28, 34, 50, 42, 38, 34, 28, 0}, {8, 12, 8, 8, 8, 8, 28, 0}, {28, 34, 32, 16, 8, 4, 62, 0}, {28, 34, 32, 24, 32, 34, 28, 0}, {16, 24, 20, 18, 62, 16, 16, 0}, {62, 2, 30, 32, 32, 34, 28, 0}, {56, 4, 2, 30, 34, 34, 28, 0}, {62, 32, 32, 16, 8, 8, 8, 0}, {28, 34, 34, 28, 34, 34, 28, 0}, {28, 34, 34, 60, 32, 34, 28, 0}};

// ########### Variables programa #############################
const byte columnas = 7; // Nº de columnas
const byte filas = 6;	 // Nº de filas
byte contador = 0;		 // Contador.
byte fila;				 // Fila seleccionada.
byte columna;			 // Columna seleccionada.
Config4eR config;		 // Cofiguración del programa.
Config4eR configTemp;	 // Valores temporales (mientras el usuario cambia la configuración)
byte salvaColumna[columnas] = {0};
byte salvaFila[filas] = {0};
byte tablero[columnas][filas] = {0};	// Control de las jugadas.
bool turno1 = true;						// Turno jugador 1
bool enProceso = false;					// Indica si hay un proceso activo.
bool comprobarFinProceso = false;		// Realiza las combrobaciones oportunas al finalizar un proceso.
bool enModoConfig = false;				// Indica si estamos en modo configuración.
bool juegoEnMarcha = false;				// Indica si ya ha empezado el juego.
volatile bool siguientePaso = false;	// Activa el siguiente paso de un proceso.
MatrizLed led;							// Clase para cálculo posición led.
Ganador ganador;						// Contiene los datos de la jugada ganadora.
Ticker Procesos;						// Generador de interrupciones para el control de procesos.
unsigned int repeticiones = 0;			// Nº de veces que se ejecuta un proceso.
unsigned int contadorPasos = 0;			// Lleva la cuenta de los pasos ejecutados en un proceso.
byte proceso = 0;						// Proceso activo.
unsigned int tiempo_PorPaso = 0;		// Tiempo entre un paso y el siguiente (en milisegundos)
unsigned int tiempo_PonerFicha = 250;	// Tiempo animación "Poner una ficha".
unsigned int tiempo_ColumnaLlena = 500; // Tiempo animación "Error columna llena".
int avanceFila = 0;
int avanceColumna = 0;
ModoConfiguracion modoConfig = ModoConfiguracion::Ninguna; // indica que opción está configurando el usuario.

// ######## VARIABLES CONTROL LEDS ##################
const int numeroLeds = (8 * 8) + 8 + 8; // Nº total de leds (Filas * columnas + pulsadores)
CRGB leds[numeroLeds];					// Array leds.
CRGB colores[7];						// Posibles colores a utilizar. (El 0 es el de fondo)
CRGB color[2][3];						// Array colores
byte nivel = 0;
byte letra = 255;
byte numero = 255;

// ############# Configuración ESP8266 ###############
const byte pinAltavoz = D7;
const byte pinInterrupciones = D6;
const byte pinLeds = D5;
const byte pinReset = D8;
const int sizeEEPROM = 8;

// ############ Variables control expansor de puertos ##############
Adafruit_MCP23X17 expansor;			 // Configuración expansor de puertos para los pulsadores
const byte direccionExpansor = 0x20; // Dirección del expansor (I2C)
byte pulsador = 255;				 // Nº del último pulsador accionado.
volatile bool teclaPulsada = false;	 // Indica si se ha accionado algún pulsador.

// Interrupción al presionar un pulsador.
IRAM_ATTR void ComprobarLectura()
{
	// Ha saltado la interrupción. El pin 19 y/o 20 (INTA, INTB) se han puesto a cero.
	teclaPulsada = true;
}

void setup()
{
	// put your setup code here, to run once:
	byte intensidad1 = 0;
	byte intensidad2 = 0;
	byte intensidadFondo = 0;

	// Si estamos en modo "Depurar" activamos la comunicación con el monitor serial.
#if defined(DEBUG)
	Serial.begin(115200);
	delay(1);
	Serial.println(F("\nInicio configuración. "));
#endif
	// Leemos la configuración almacenada. Si algun valor no es correcto es porque no se ha configurado
	// y cargamos los valores por defecto.
	EEPROM.begin(sizeEEPROM);

	config = EEPROM.get(0, config);
	if (config.configurado != 1 || config.color1 > 6 || config.color2 > 6 || config.NivelIntensidad > 8)
	{
#ifdef DEBUG
		Serial.println("Valores incorrectos: ");
		Serial.print(config.color1);
		Serial.print(", ");
		Serial.print(config.color2);
		Serial.print(", ");
		Serial.print(config.configurado);
		Serial.print(", ");
		Serial.print(config.NivelIntensidad);
		Serial.print(", ");
		Serial.println(config.Turno);
#endif // DEBUG

		ValoresxDefecto();
	}
#ifdef DEBUG
	else
	{
		Serial.println("Configurado correctamente.");
	}
#endif // DEBUG
	EEPROM.end();
#ifdef DEBUG
	config.NivelIntensidad = 1;
	Serial.println("Valores Leidos: ");
	Serial.print(config.color1);
	Serial.print(", ");
	Serial.print(config.color2);
	Serial.print(", ");
	Serial.print(config.configurado);
	Serial.print(", ");
	Serial.print(config.NivelIntensidad);
	Serial.print(", ");
	Serial.println(config.Turno);
#endif // DEBUG

	// Los leds son de tipo GRB
	colores[0] = 0xFFFFFF; // Fondo (Blanco)
	colores[1] = 0x00FF00; // Rojo
	colores[2] = 0xFFFF00; // Amarillo
	colores[3] = 0xFF0000; // Verde
	colores[4] = 0x0000FF; // Azul
	colores[5] = 0x00FFFF; // Violeta
	colores[6] = 0xFF00FF; // Verde + Azul

	// Definimos los colores;
	intensidad1 = config.NivelIntensidad;
	if (intensidad1 < 5)
	{
		intensidad2 = intensidad1 * 2;
		intensidadFondo = intensidad1 > 1 ? intensidad1 / 2 : 1;
	}
	else
	{
		intensidad2 = intensidad1 / 2;
		intensidadFondo = intensidad2 - 1;
	}
	color[0][0] = DameColor(0, intensidadFondo);		 // Fondo
	color[0][1] = DameColor(config.color1, intensidad1); // Jugador 1
	color[0][2] = DameColor(config.color2, intensidad1); // Jugador 2
	color[1][0] = DameColor(0, intensidadFondo);		 // Fondo
	color[1][1] = DameColor(config.color1, intensidad2); // Jugador 1
	color[1][2] = DameColor(config.color2, intensidad2); // Jugador 2

#if defined(DEBUG)
	Serial.print(color[0][0].r);
	Serial.print("|");
	Serial.print(color[0][0].g);
	Serial.print("|");
	Serial.print(color[0][0].b);
	Serial.print("  -  ");
	Serial.print(color[1][0].r);
	Serial.print("|");
	Serial.print(color[1][0].g);
	Serial.print("|");
	Serial.println(color[1][0].b);

	Serial.print(color[0][1].r);
	Serial.print("|");
	Serial.print(color[0][1].g);
	Serial.print("|");
	Serial.print(color[0][1].b);
	Serial.print("  -  ");
	Serial.print(color[1][1].r);
	Serial.print("|");
	Serial.print(color[1][1].g);
	Serial.print("|");
	Serial.println(color[1][1].b);

	Serial.print(color[0][2].r);
	Serial.print("|");
	Serial.print(color[0][2].g);
	Serial.print("|");
	Serial.print(color[0][2].b);
	Serial.print("  -  ");
	Serial.print(color[1][2].r);
	Serial.print("|");
	Serial.print(color[1][2].g);
	Serial.print("|");
	Serial.println(color[1][2].b);
#endif

	// Asignamos turno.
	turno1 = config.Turno;

	// Habilitamos la matriz de leds.
	LEDS.addLeds<WS2812B, pinLeds, RGB>(leds, numeroLeds);

	// Configuramos los pines I/O del ESP8266
	pinMode(pinAltavoz, OUTPUT);
	pinMode(pinInterrupciones, INPUT);
	pinMode(pinReset, OUTPUT);

	// Iniciamos el expansor. Si no se inicia detenemos el programa.
	if (!IniciaExpansor())
	{
		while (true)
		{
			delay(0);
		}
	}

	// Configuramos el expansor para que genere una interrupción al pulsar cualquier botón de los puertos A o B
	// Parámetro 1: true = Interrupción conjunta | false = interrupciones separadas para PORT A y B
	// Parámetro 2: Indica si los pines configurados para la interrupción estarán "abiertos" o no.
	// Parámetro 3: Indica con que valor se activa la interrupción HIGH o LOW
	expansor.setupInterrupts(true, false, LOW);

	// Habilitamos todas las patillas (16) como entradas y habilitamos la resistencia interna.
	for (byte i = 0; i < 16; i++)
	{
		// Configuramos los 16 pin como entradas y habilitamos el PullUp para que por defecto esten a 1.
		// Al poner un valor 0 pulsando un pulsador se dispara el pin de interrupción.
		expansor.pinMode(i, INPUT_PULLUP);
		expansor.setupInterruptPin(i, LOW);
	}

	// Inicializamos la función que nos va a calcular la posición del led en el tablero.
	led.begin(8, 8, ml_Inicio::Inicio_Inferior_Izquierda, ml_Direccion::Direccion_Vertical);

	// Antes de activar las interrupciones del ESP8266 leemos el expansor por si hay alguna interrupción
	// pendiente, borrarla y empezar "limpios".
	expansor.readGPIOAB();

	// Habilita las interrupciones en el ESP8266
	attachInterrupt(pinInterrupciones, ComprobarLectura, FALLING);

	// Iniciamos el tablero.
	LEDS.clear(true);
	IniciaTablero();
}

void loop()
{

	if (teclaPulsada && !enProceso)
	{
		// La ponemos a false para entrar solo una vez.
		teclaPulsada = false;
#if defined(DEBUG)
		Serial.println(F("Detectada tecla pulsada. "));
#endif
		// Lee el valor del pulsador (0 - 15)
		pulsador = expansor.getLastInterruptPin();
#if defined(DEBUG)
		Serial.print(F("Has pulsado el pulsador n. "));
		Serial.println(pulsador);
#endif
		// El pin 19 y/o 20 no vuelven a ponerse a 1 hasta que volvamos a leer las entradas y no haya
		// ninguna pulsada
		// En esta configuración si no hay ningún interruptor pulsado el valor devuelto
		// es B1111111111111111 (65535)
		while (expansor.readGPIOAB() != 65535)
		{
			// Estamos leyendo de forma continua hasta que el usuario suelte el pulsador.
			delay(20);
		}
		// Comprobamos que la lectura sea buena.
		if (pulsador > 15)
		{
			pulsador = 255;
		}

		if (pulsador < 8)
		{
			// El pulsador corresponde a una letra/columna.
			letra = pulsador;

			// Si estamos jugando vamos a comprobar.
			if (juegoEnMarcha)
			{
				AsignaLetra();
#if defined(DEBUG)
				Serial.print(F("Han seleccionado columna "));
				Serial.println(letra);
#endif
			}
			if (enModoConfig)
			{
				// Estamos en modo configuración.
				LetraEnConfig();
			}
		}

		// Han pulsado una tecla de acción. #############################################
		if (pulsador >= 8 && pulsador < 16)
		{
#if defined(DEBUG)
			Serial.println(F("Asignamos modo. "));
#endif
			switch (pulsador)
			{
			case P_OK:
#if defined(DEBUG)
				Serial.println(F("Han pulsado OK. "));
#endif
				if (juegoEnMarcha)
				{
					OkEnJuego();
				};
				if (enModoConfig)
				{
					SalvaConfiguracion();
					enModoConfig = false;
				}
				break;
			case P_START:
				if (enModoConfig)
				{
					// Activamos el modo juego sin salvar la configuración.
					enModoConfig = false;
				}

				// Solo se pone el juego en marcha si está parado. Si no, no hace nada.
				if (!juegoEnMarcha)
				{
					juegoEnMarcha = true;
#ifdef DEBUG
					Serial.println(F("\nJuego iniciado."));
#endif
					// Borramos el tablero por si este no es el primer juego.
					memset(tablero, 0, sizeof(tablero));
					PintaTablero();
				}
				break;
			case P_CONFIG:
				// Si estamos en una partida no se accede al modo configuración.
				if (!juegoEnMarcha)
				{
					// Si no estamos en modo configuración copiamos los valores a la variable temporal.
					if (!enModoConfig)
					{
						configTemp = config;
					}
					enModoConfig = true;
				}
				break;
			default:
				// Comprobamos el resto de pulsadores.
				if (enModoConfig)
				{
					if (pulsador == P_P1)
					{
						modoConfig = ModoConfiguracion::Turno;
					}
					if (pulsador == P_P2)
					{
						modoConfig = ModoConfiguracion::Color1;
					}
					if (pulsador == P_P3)
					{
						modoConfig = ModoConfiguracion::Color2;
					}
					if (pulsador == P_P4)
					{
						modoConfig = ModoConfiguracion::Intensidad;
					}
				}
				break;
			}
		}
	}

	// Estamos en un proceso.
	if (enProceso)
	{
		if (siguientePaso)
		{
			siguientePaso = false;
			contador++;
			switch (proceso)
			{
			case Pr_PonerFicha:
				tablero[columna][filas - contador] = turno1 ? 1 : 2;
#if defined(DEBUG)
				Serial.print(F("Iluminando columna/fila = valor "));
				Serial.print(columna);
				Serial.print("/");
				Serial.print(filas - contador);
				Serial.print(" = ");
				Serial.println(turno1 ? 1 : 2);
#endif
				if (contador > 1)
				{
					tablero[columna][filas - contador + 1] = 0;
#if defined(DEBUG)
					Serial.print(F("Apagando columna/fila "));
					Serial.print(columna);
					Serial.print("/");
					Serial.println(filas - contador + 1);
#endif
				}
				// Visualizamos el tablero.
				PintaTablero();
				break;
			case Pr_ColumnaLlena:
				for (byte i = 0; i < filas; i++)
				{
					tablero[columna][i] = contador % 2 == 1 ? 0 : salvaColumna[i];
				}
				// Visualizamos el tablero.
				PintaTablero();
				break;
			case Pr_InicioTablero:
				for (byte i = 0; i < 8; i++)
				{
					for (byte j = 0; j < 8; j++)
					{
						leds[led.posicion(i, j)] = contador == (i + 1) ? color[nivel][0] : CRGB::Black;
					}
				}
				LEDS.show();
				break;
			case Pr_MuestraGanador:
				nivel++;
				if (nivel > 1)
				{
					nivel = 0;
				}
				switch (ganador.modo)
				{
				case Modo::Vertical:
					avanceFila = 1;
					avanceColumna = 0;
					break;
				case Modo::Horizontal:
					avanceFila = 0;
					avanceColumna = 1;
					break;
				case Modo::Diagonal_derecha:
					avanceFila = 1;
					avanceColumna = 1;
					break;
				case Modo::Diagonal_izquierda:
					avanceFila = 1;
					avanceColumna = -1;
					break;
				default:
					break;
				}
				for (byte i = 0; i < 4; i++)
				{
					leds[led.posicion(ganador.columna + i * avanceColumna, ganador.fila + i * avanceFila)] = color[nivel][ganador.jugador];
				}
				LEDS.show();
				juegoEnMarcha = false;
				break;
			default:
				break;
			}

			// Comprobamos si es la última repetición.
			if (contador == repeticiones)
			{

				// Informamos "Fin del proceso".
				enProceso = false;
				contador = 0;
				tiempo_PorPaso = 0;
				Procesos.detach();

				// Borramos cualquier pulsación de tecla que pueda haber sucedido durante el proceso.
				teclaPulsada = false;
				expansor.readGPIOAB();

				// Realizamos las comprobaciones de fin de proceso.
				comprobarFinProceso = true;
			}
		}
	}

	if (comprobarFinProceso)
	{
#if defined(DEBUG)
		Serial.print(F("Comprobamos final proceso "));
		Serial.println(proceso);
#endif
		comprobarFinProceso = false;
		switch (proceso)
		{
		case Pr_PonerFicha:
			if (ComprobarGanador())
			{
				// El jugador del turno actual ha ganado.
#if defined(DEBUG)
				Serial.print(F("Ha ganado el jugador: "));
				Serial.println(turno1 ? 1 : 2);
#endif
				tiempo_PorPaso = tiempo_ColumnaLlena;
				repeticiones = 10;
				proceso = Pr_MuestraGanador;
				enProceso = true;
				siguientePaso = true;
				IniciaProceso();
			}
			else
			{
				// Cambiamos de turno.
				turno1 = !turno1;
#if defined(DEBUG)
				Serial.println(F("Cambio de turno "));
#endif
				// Apagamos la letra que esté encendida.
				letra = 255;
				AsignaLetra();
			}
			break;
		case Pr_ColumnaLlena:
			// No se comprueba nada (por ahora).
			break;
		case Pr_InicioTablero:
			LEDS.clear(true);
			PintaTablero();
			break;
		case Pr_MuestraGanador:
			// Borramoa la letra.
			letra = 255;
			AsignaLetra();

			PintaTablero();
			break;
		default:
			break;
		}
	}
	pulsador = 255;
}

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
				ganador.modo = Modo::Horizontal;
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
				ganador.modo = Modo::Horizontal;
				return true;
			}
		}
	}
	return false;
}

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
		leds[led.posicionP1(i)] = i == letra ? color[nivel][turno1 ? 1 : 2] : CRGB::Black;
	}
	LEDS.show();
}

// Enciende el led del número seleccionado.
void AsignaNumero()
{
	// memset(&leds[led.posicionP2(0)], 0, 8);
	//  Iluminamos solo la letra selecciomada.
	for (byte i = 0; i < 8; i++)
	{
		leds[led.posicionP2(i)] = i == numero ? color[nivel][turno1 ? 1 : 2] : CRGB::Black;
	}
	LEDS.show();
}

// No se ha detectado el expansor. Mostramos error en el tablero.
void SinExpansor()
{
	for (byte i = 0; i < 6; i++)
	{
		leds[led.posicion(i, i)] = CRGB::Red;
		leds[led.posicion(i, 5 - i)] = CRGB::Red;
	}
	LEDS.show();
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

// Ilumina cada celda del tablero con su color correspondiente.
void PintaTablero()
{
	// Pintamos tablero.
	for (byte c = 0; c < columnas; c++)
	{
		for (byte f = 0; f < filas; f++)
		{
			leds[led.posicion(c, f)] = color[nivel][tablero[c][f]];
		}
	}
	// Pintamos letra.
	AsignaLetra();
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

// Activa el siguiente paso de un proceso.
IRAM_ATTR void ActivaPaso()
{
	siguientePaso = true;
}

// Activa un proceso determinado.
void IniciaProceso()
{
	contador = 0;
	Procesos.attach_ms(tiempo_PorPaso, ActivaPaso);
}

// Cargamos los valores por defecto de la configuración.
void ValoresxDefecto()
{
	// Cargamos valores por defecto.
	config.color1 = 4;
	config.color2 = 1;
	config.Turno = true;
	config.NivelIntensidad = 4;
	config.configurado = 1;
	// Grabamos configuración.
	EEPROM.put(0, config);
	EEPROM.commit();
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

// Guardamos la configuración actual.
void SalvaConfiguracion()
{
	config = configTemp;
	EEPROM.begin(sizeEEPROM);
	EEPROM.put(0, config);
	EEPROM.commit();
	EEPROM.end();
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
		LEDS.showColor(DameColor(colores[0], DameIntensidadFondo(configTemp.NivelIntensidad)));
		break;

	case ModoConfiguracion::Turno:

		DibujaTurno(configTemp.Turno ? 1 : 2);

		// Iluminamos el pulsador 1 y 2 con los colores de cada turno.
		leds[led.posicionP1(0)] = DameColor(configTemp.color1, configTemp.NivelIntensidad);
		leds[led.posicionP1(1)] = DameColor(configTemp.color2, configTemp.NivelIntensidad);

		LEDS.show();
		break;
	case ModoConfiguracion::Color1:
		// Es el color del turno 1
		DibujaTurno(1);

		// Iluminamos las teclas con los distintos colores excepto el que tiene el turno 2.
		for (byte i = 1; i < 7; i++)
		{
			leds[led.posicionP1(i - 1)] = i != configTemp.color2 ? DameColor(i, configTemp.NivelIntensidad) : CRGB::Black;
		}

		// Los últimos dos pulsadores no tienen color asignado.
		leds[led.posicionP1(6)] = CRGB::Black;
		leds[led.posicionP1(7)] = CRGB::Black;

		LEDS.show();
		break;
	case ModoConfiguracion::Color2:
		// Es el color del turno 1
		DibujaTurno(2);

		// Iluminamos las teclas con los distintos colores excepto el que tiene el turno 2.
		for (byte i = 1; i < 7; i++)
		{
			leds[led.posicionP1(i - 1)] = i != configTemp.color1 ? DameColor(i, configTemp.NivelIntensidad) : CRGB::Black;
		}

		// Los últimos dos pulsadores no tienen color asignado.
		leds[led.posicionP1(6)] = CRGB::Black;
		leds[led.posicionP1(7)] = CRGB::Black;

		LEDS.show();
		break;

	case ModoConfiguracion::Intensidad:
		CRGB color1, color2, colorFondo, colorFila;
		color1 = DameColor(configTemp.color1, configTemp.NivelIntensidad);
		color2 = DameColor(configTemp.color2, configTemp.NivelIntensidad);
		colorFondo = DameColor(colores[0], DameIntensidadFondo(configTemp.NivelIntensidad));

		// Ya tenemos los colores, ahora dibujamos el tablero mostrando los tres colores.
		for (byte f = 0; f < 8; f++)
		{
			colorFila = f % 2 == 0 ? color1 : color2;
			for (byte c = 0; c < 8; c++)
			{
				leds[led.posicion(f, c)] = c <= f ? colorFila : colorFondo;
			}
		}

		// Iluminamos la fila de letras con los dos colores cambiando la intensidad.
		for (byte i = 0; i < 8; i++)
		{
			leds[led.posicionP1(i)] = DameColor(i % 2 == 0 ? color1 : color2, i);
		}
		// Iluminamos la fila de los números con el color de fondo para la intensidad correspondiente.
		for (byte i = 0; i < 8; i++)
		{
			leds[led.posicionP1(i)] = DameColor(colores[0], DameIntensidadFondo(i));
		}

		break;
	default:
		break;
	}
}

// Dibuja en el tablero el número de jugador que empieza con su color.
void DibujaTurno(byte turnoActual)
{
	byte posicion = 0;
	CRGB color_ = turnoActual == 1 ? DameColor(configTemp.color1, configTemp.NivelIntensidad) : DameColor(configTemp.color2, configTemp.NivelIntensidad);
	CRGB colorFondo_ = DameColor(colores[0], DameIntensidadFondo(configTemp.NivelIntensidad));
	// Dibujamos el turno en el tablero.
	for (byte f = 0; f < 8; f++)
	{
		posicion = 1;
		for (byte p = 0; p < 8; p++)
		{
			leds[led.posicion(f, p)] = (numeros[turnoActual][7 - f] && posicion) ? color_ : colorFondo_;
			posicion << 1;
		}
	}
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
#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <MatrizLed.h>
#include <Ticker.h>
#include <Adafruit_MCP23X17.h>
#include <EEPROM.h>

#define NODEBUG

#define NUMEROFILAS 8
#define NUMEROCOLUMNAS 8
#define NUMEROPULSADORES 8
#define TIEMPOANIMACION 5000			// 1 minuto
#define TIEMPOPPARTIDAABANDONADA 300000	// 5 minutos
#define TIEMPOPASOANIMACIONES 125

// Activa el siguiente paso de un proceso.
void ActivaPaso();

// Interrupción al presionar un pulsador.
void ComprobarLectura();


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

// Control de las animaciones.
enum Animacion
{
	ninguna,
	barraVertical,
	barraVerticalInversa,
	dobleBarraVertical,
	barraHorizontal,
	barraHorizontalInversa,
	dobleBarraHorizontal,
	diagonalIzq,
	diagonalDcha,
	diagonalIzqInversa,
	diagonalDchaInversa,
	dobleDiagonalIzquierda,
	dobleDiagonalDerecha,
	diagonalIzdaDcha,
	diagonalIzdaDchaInversa,
	cuadrados,
	cuadradosInversos,
	NumeroAnimaciones
};


// Pulsadores
const byte P_OK = 11;
const byte P_START = 8;
const byte P_CONFIG = 9;
const byte P_REINICIO = 10;
const byte P_P1 = 12;
const byte P_P2 = 13;
const byte P_P3 = 14;
const byte P_P4 = 15;
const byte ledsLetras = 1;
const byte ledsNumeros = 0;

// Procesos
const byte Pr_PonerFicha = 1;
const byte Pr_ColumnaLlena = 2;
const byte Pr_InicioTablero = 3;
const byte Pr_MuestraGanador = 4;
const byte Pr_Animacion = 5;

// ############# Configuración ESP8266 ###############
const byte pinAltavoz = D7;
const byte pinInterrupciones = D6;
const byte pinLeds = D5;
const byte pinReset = D8;
const int sizeEEPROM = 8;

// ########### Variables programa #############################
const byte columnas = 7;                    // Nº de columnas
const byte filas = 6;	                    // Nº de filas
const int numeroLeds = (NUMEROFILAS * NUMEROCOLUMNAS) + NUMEROPULSADORES * 2; // Nº total de leds (Filas * columnas + pulsadores)
const byte direccionExpansor = 0x20;        // Dirección del expansor (I2C)

extern int contador;                		// Contador.
extern byte fila;				            // Fila seleccionada.
extern byte columna;            			// Columna seleccionada.
extern Config4eR config;            		// Cofiguración del programa.
extern Config4eR configTemp;            	// Valores temporales (mientras el usuario cambia la configuración)
extern byte salvaColumna[columnas];
extern byte salvaFila[filas];
extern byte tablero[columnas][filas];	    // Control de las jugadas.
extern bool turno1;						    // Turno jugador 1
extern bool enProceso;					    // Indica si hay un proceso activo.
extern bool comprobarFinProceso;		    // Realiza las combrobaciones oportunas al finalizar un proceso.
extern bool enModoConfig;				    // Indica si estamos en modo configuración.
extern bool juegoEnMarcha;				    // Indica si ya ha empezado el juego.
extern volatile bool siguientePaso;	        // Activa el siguiente paso de un proceso.
extern MatrizLed tableroLed;				// Clase para cálculo posición led.
extern MatrizLed pulsadores;				// Leds asociados a los pulsadores.
extern Ganador ganador;						// Contiene los datos de la jugada ganadora.
extern Ticker Procesos;						// Generador de interrupciones para el control de procesos.
extern unsigned int repeticiones;			// Nº de veces que se ejecuta un proceso.
extern unsigned int contadorPasos;			// Lleva la cuenta de los pasos ejecutados en un proceso.
extern byte proceso;						// Proceso activo.
extern unsigned int tiempo_PorPaso;		    // Tiempo entre un paso y el siguiente (en milisegundos)
extern unsigned int tiempo_PonerFicha;      // Tiempo animación "Poner una ficha".
extern unsigned int tiempo_ColumnaLlena;    // Tiempo animación "Error columna llena".
extern int avanceFila;
extern int avanceColumna;
extern ModoConfiguracion modoConfig;        // indica que opción está configurando el usuario.
extern byte numeroColores;
extern byte numeros[10][8];
extern int veces;							// Nº de veces que se repite una función de animación.
extern MatrizLed tableroLed;				// Clase para cálculo posición led.
extern MatrizLed pulsadores;				// Leds asociados a los pulsadores.

// ######## VARIABLES CONTROL LEDS ##################
extern CRGB leds[numeroLeds];				// Array leds.
extern CRGB colores[7];						// Posibles colores a utilizar. (El 0 es el de fondo)
extern CRGB color[2][3];					// Array colores (3 colores x 2 intensidades)
extern byte nivel;
extern byte letra;
extern byte numero;

// ############ Variables control expansor de puertos ##############
extern Adafruit_MCP23X17 expansor;			 // Configuración expansor de puertos para los pulsadores
extern byte pulsador;				 // Nº del último pulsador accionado.
extern volatile bool teclaPulsada;	 // Indica si se ha accionado algún pulsador.

extern uint32 ultimaPulsacion;
extern uint32 tiempoActivacion;
extern byte animacion;
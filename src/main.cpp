/*
PINES USADOS
===================================

*/
#include <Arduino.h>
#include <entorno.h>
#include <FuncionesLed.h>
#include <Procesos.h>
#include <Efectos.h>

// #define NOVERCOLOR

// ########### Variables programa #############################
int contador = 0;	  // Contador.
byte fila;			  // Fila seleccionada.
byte columna;		  // Columna seleccionada.
Config4eR config;	  // Cofiguración del programa.
Config4eR configTemp; // Valores temporales (mientras el usuario cambia la configuración)
byte salvaColumna[columnas] = {0};
byte salvaFila[filas] = {0};
byte tablero[columnas][filas] = {0};	// Control de las jugadas.
bool turno1 = true;						// Turno jugador 1
bool enProceso = false;					// Indica si hay un proceso activo.
bool comprobarFinProceso = false;		// Realiza las combrobaciones oportunas al finalizar un proceso.
bool enModoConfig = false;				// Indica si estamos en modo configuración.
bool juegoEnMarcha = false;				// Indica si ya ha empezado el juego.
volatile bool siguientePaso = false;	// Activa el siguiente paso de un proceso.
MatrizLed tableroLed;					// Clase para cálculo posición led.
MatrizLed pulsadores;					// Leds asociados a los pulsadores.
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
int veces = 0;
uint32 tiempoActivacion;
ModoConfiguracion modoConfig = ModoConfiguracion::Ninguna; // indica que opción está configurando el usuario.

// ######## VARIABLES CONTROL LEDS ##################
CRGB leds[numeroLeds]; // Array leds.
CRGB colores[7];	   // Posibles colores a utilizar. (El 0 es el de fondo)
CRGB color[2][3];	   // Array colores (3 colores x 2 intensidades)
byte nivel = 0;
byte letra = 255;
byte numero = 255;
byte numeros[10][8] = {{28, 34, 50, 42, 38, 34, 28, 0}, {8, 12, 8, 8, 8, 8, 28, 0}, {28, 34, 32, 16, 8, 4, 62, 0}, {28, 34, 32, 24, 32, 34, 28, 0}, {16, 24, 20, 18, 62, 16, 16, 0}, {62, 2, 30, 32, 32, 34, 28, 0}, {56, 4, 2, 30, 34, 34, 28, 0}, {62, 32, 32, 16, 8, 8, 8, 0}, {28, 34, 34, 28, 34, 34, 28, 0}, {28, 34, 34, 60, 32, 34, 28, 0}};

byte numeroColores = sizeof(colores) / sizeof(CRGB);

// ############ Variables control expansor de puertos ##############
Adafruit_MCP23X17 expansor;			// Configuración expansor de puertos para los pulsadores
byte pulsador = 255;				// Nº del último pulsador accionado.
volatile bool teclaPulsada = false; // Indica si se ha accionado algún pulsador.

byte animacion = Animacion::ninguna;
uint32 ultimaPulsacion = millis();

// Activa el siguiente paso de un proceso.
IRAM_ATTR void ActivaPaso()
{
	siguientePaso = true;
}

// Interrupción al presionar un pulsador.
IRAM_ATTR void ComprobarLectura()
{
	// Ha saltado la interrupción. El pin 19 y/o 20 (INTA, INTB) se han puesto a cero.
	teclaPulsada = true;
	// digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
// uint32 tiempo = millis();

void setup()
{
	// put your setup code here, to run once:
	byte intensidad1 = 0;
	byte intensidad2 = 0;
	byte intensidadFondo = 0;

	// Si estamos en modo "Depurar" activamos la comunicación con el monitor serial.
#if defined(DEBUG_)
	Serial.begin(115200);
#endif

#if defined(DEBUG)
	Serial.begin(115200);
	delay(2000);
	Serial.println(F("\nInicio configuración. "));
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
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
	tableroLed.begin(NUMEROCOLUMNAS, NUMEROFILAS, ml_Inicio::Inicio_Inferior_Izquierda, ml_Direccion::Direccion_Vertical);

	// Los pulsadores están después del tablero. Hay que indicar el offset con el número de leds del tablero.
	pulsadores.begin(NUMEROPULSADORES, 2, ml_Inicio::Inicio_Inferior_Izquierda, ml_Direccion::Direccion_Horizontal, NUMEROFILAS * NUMEROCOLUMNAS);

	// Antes de activar las interrupciones del ESP8266 leemos el expansor por si hay alguna interrupción
	// pendiente, borrarla y empezar "limpios".
	pulsador = expansor.readGPIOAB();

	// Habilita las interrupciones en el ESP8266
	attachInterrupt(pinInterrupciones, ComprobarLectura, FALLING);

	// Iniciamos el tablero.
	LEDS.clear(true);
	IniciaTablero();

	ultimaPulsacion = millis();
	tiempoActivacion = TIEMPOANIMACION;
}

void loop()
{

	if (teclaPulsada)
	{
		ultimaPulsacion = millis();
	}

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
			delay(50);
			Serial.print(expansor.readGPIOAB());
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
					// Hacemos parpadear el tablero para que el usuario sepa que se ha leido su pulsación.
					for (byte i = 0; i < 4; i++)
					{
						/* code */
						LEDS.clear();
						LEDS.show();
						delay(500);
						PintaTablero();
						delay(500);
					}

					// Borramos el tablero por si este no es el primer juego.
					memset(tablero, 0, sizeof(tablero));
					PintaTablero();
				}
				break;
			case P_CONFIG:
				// Si estamos en una partida no se accede al modo configuración.
#if defined(DEBUG)
				Serial.println(F("Han pulsado Config. "));
#endif
				if (!juegoEnMarcha)
				{
					// Si no estamos en modo configuración copiamos los valores a la variable temporal.
					if (!enModoConfig)
					{
#if defined(DEBUG)
						Serial.println(F("cargamos valores. "));
#endif
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
#if defined(DEBUG)
						Serial.println(F("Configurando turno. "));
#endif
						modoConfig = ModoConfiguracion::Turno;
					}
					if (pulsador == P_P2)
					{
#if defined(DEBUG)
						Serial.println(F("Configurando color jugador 1. "));
#endif
						modoConfig = ModoConfiguracion::Color1;
					}
					if (pulsador == P_P3)
					{
#if defined(DEBUG)
						Serial.println(F("Configurando color jugador 1. "));
#endif
						modoConfig = ModoConfiguracion::Color2;
					}
					if (pulsador == P_P4)
					{
#if defined(DEBUG)
						Serial.println(F("Configurando intensidad. "));
#endif
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
						leds[tableroLed.posicion(i, j)] = contador == (i + 1) ? color[nivel][0] : CRGB::Black;
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
					// Alterna el brillo de los leds entre normal y alto de la jugada ganadora.
					leds[tableroLed.posicion(ganador.columna + i * avanceColumna, ganador.fila + i * avanceFila)] = color[nivel][ganador.jugador];
				}
				LEDS.show();
				juegoEnMarcha = false;
				break;
			case Pr_Animacion:
				// Si hemos llegado a la última animación empezamos de nuevo por la primera.
				switch (animacion)
				{
				case Animacion::barraVertical:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaBarra((contador - 1), true, DameColor((contador - 1) / NUMEROCOLUMNAS, config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::barraHorizontal:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaBarra((contador - 1), false, DameColor((contador - 1) / NUMEROFILAS, config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::barraVerticalInversa:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaBarra((repeticiones - contador), true, DameColor((contador - 1) / NUMEROFILAS, config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::barraHorizontalInversa:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaBarra((repeticiones - contador), false, DameColor((contador - 1) / NUMEROFILAS, config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::diagonalIzq:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((contador - 1) % (NUMEROFILAS * 2 - 1), false, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::diagonalDcha:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((contador - 1) % (NUMEROFILAS * 2 - 1), true, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::diagonalIzqInversa:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((repeticiones - contador) % (NUMEROFILAS * 2 - 1), false, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::diagonalDchaInversa:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((repeticiones - contador) % (NUMEROFILAS * 2 - 1), true, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::dobleBarraHorizontal:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaBarra((contador - 1), false, DameColor((contador - 1) / NUMEROFILAS, config.NivelIntensidad));
					calculaBarra((repeticiones - contador), false, DameColor((contador - 1) / NUMEROFILAS, config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::dobleBarraVertical:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaBarra((contador - 1), true, DameColor((contador - 1) / NUMEROCOLUMNAS, config.NivelIntensidad));
					calculaBarra((repeticiones - contador), true, DameColor((contador - 1) / NUMEROCOLUMNAS, config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::dobleDiagonalIzquierda:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((contador - 1) % (NUMEROFILAS * 2 - 1), false, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					calculaDiagonal((repeticiones - contador) % (NUMEROFILAS * 2 - 1), false, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;

				case Animacion::dobleDiagonalDerecha:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((contador - 1) % (NUMEROFILAS * 2 - 1), true, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					calculaDiagonal((repeticiones - contador) % (NUMEROFILAS * 2 - 1), true, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::diagonalIzdaDcha:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((contador - 1) % (NUMEROFILAS * 2 - 1), false, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					calculaDiagonal((contador - 1) % (NUMEROFILAS * 2 - 1), true, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::diagonalIzdaDchaInversa:
					// Contador empieza en 1, pero esta función empieza por cero.
					LEDS.clear(0);
					calculaDiagonal((repeticiones - contador) % (NUMEROFILAS * 2 - 1), false, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					calculaDiagonal((repeticiones - contador) % (NUMEROFILAS * 2 - 1), true, DameColor((contador - 1) / (NUMEROFILAS * 2 - 1), config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::cuadrados:
					LEDS.clear(0);
					calculaCuadrado((contador - 1) % veces, DameColor((contador - 1) / veces, config.NivelIntensidad));
					LEDS.show();
					break;
				case Animacion::cuadradosInversos:
					LEDS.clear(0);
					calculaCuadrado( veces - ((contador - 1) % veces) - 1, DameColor((contador - 1) / veces, config.NivelIntensidad));
					LEDS.show();
					break;
				default:
					break;
				}
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

		// Si pulsan una tecla durante la animación, paramos la animación.
		if (teclaPulsada && (proceso == Pr_Animacion))
		{
			// Salir del proceso animación.
			enProceso = false;
			ultimaPulsacion = millis();
			animacion = Animacion::ninguna;
			// ver que hago.
		}

		// Comprobamos si ha pasado el tiempo necesario para activar una animación.

		// Si pulsan una tecla durante un proceso la ignoramos.
		if (teclaPulsada)
		{
			teclaPulsada = false;
			pulsador = expansor.getLastInterruptPin();
		}
	}

	// Comprobamos si hay que activar las animaciones.
	if (!enProceso && ((millis() - ultimaPulsacion) > tiempoActivacion))
	{
#if defined(DEBUG)
		Serial.println("Activamos animaciones");
#endif
		enProceso = true;
		proceso = Pr_Animacion;
		animacion++;
		if (animacion == Animacion::NumeroAnimaciones)
		{
			animacion = 1;
		}
		enProceso = true;
		siguientePaso = true;
		tiempo_PorPaso = TIEMPOPASOANIMACIONES;

		// Configuramos repeticiones, tiempo por paso y otras características de las animaciones.
		if (animacion >= Animacion::barraVertical && animacion <= Animacion::dobleBarraVertical)
			repeticiones = NUMEROCOLUMNAS * numeroColores;
		
		if (animacion >= Animacion::barraHorizontal && animacion <= Animacion::dobleBarraHorizontal)
			repeticiones = NUMEROFILAS * numeroColores;
		
		if (animacion >= Animacion::diagonalIzq && animacion <= Animacion::diagonalIzdaDchaInversa)
			repeticiones = (NUMEROFILAS * 2 - 1) * numeroColores;
		
		if (animacion >= Animacion::cuadrados && animacion <= Animacion::cuadradosInversos)
		{
			veces = round((double)NUMEROFILAS / (double)2);
			repeticiones =  veces * numeroColores;
			tiempo_PorPaso = TIEMPOPASOANIMACIONES * 2;
		}

		IniciaProceso();
	}

	// Acciones a realizar cuando finaliza un proceso.
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
		case Pr_Animacion:
			// Cuando finaliza un proceso de animación lanzamos el siguiente.
			enProceso = true;
			siguientePaso = false;
			IniciaProceso();
		default:
			break;
		}
	}

	pulsador = 255;
}

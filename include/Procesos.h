#include <Arduino.h>
#include <entorno.h>
#include <FuncionesLed.h>

// Comprueba si hay huecos libres en la columna elegida.
bool Comprobar(byte _columna);

// Comprobamos si el jugador actual ha conseguido la victoria.
bool ComprobarGanador();

// Busca la primera posición libre en una columna.
byte DameHueco(byte columna);

// Resetea y configura el expansor.
bool IniciaExpansor();

// Activa un proceso determinado.
void IniciaProceso();

// Animación inicio al encender el tablero.
void IniciaTablero();

// Han pulsado una letra/columna en modo configuración.
void LetraEnConfig();

// Muetra el "valor" del modo de configuración seleccionado.
void MuestraConfiguracion();

// Han pulsado OK estando en modo Juego.
void OkEnJuego();

// Guardamos la configuración actual.
void SalvaConfiguracion();

// No se ha detectado el expansor. Mostramos error en el tablero.
void SinExpansor();

// Cargamos los valores por defecto de la configuración.
void ValoresxDefecto();


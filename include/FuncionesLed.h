
// Enciende el led de la letra seleccionada.
void AsignaLetra();

// Enciende el led del número seleccionado.
void AsignaNumero();

// Asigna el color con la intensidad solicitada.
CRGB DameColor(byte, byte);

// Calcula la intensidad del color de fondo.
byte DameIntensidadFondo(byte nivel);

// Dibuja en el tablero el número de jugador que empieza con su color.
void DibujaTurno(byte turnoActual);

// Dibuja el estado actual del tablero.
void PintaTablero();

// Asigna una posición contigua al azar.
void Mosca();
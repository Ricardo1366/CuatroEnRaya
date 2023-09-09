#pragma once
/// <summary>
/// Posición física del primer led en la matriz.
/// </summary>
enum class ml_Inicio
{
	Inicio_Superior_Izquierda,
	Inicio_Superior_Derecha,
	Inicio_Inferior_Izquierda,
	Inicio_Inferior_Derecha
};

/// <summary>
/// Dirección de las tiras de leds en la matriz.
/// </summary>
enum class ml_Direccion
{
	Direccion_Horizontal,
	Direccion_Vertical
};


/// <summary>
/// Posición de la matriz de leds.
/// </summary>
enum class ml_Posicion
{
	Antes,
	Despues
};

/// <summary>
/// Orden de conexión de los primeros pulsadores.
/// </summary>
enum class ml_OrdenPulsador
{
	Derecho,
	Reves
};

/// <summary>
/// Controla una matriz de leds compuesta por tiras de leds secuenciales.
/// </summary>
class MatrizLed
{
public:

	MatrizLed() {
		_Columna = 0;
		_Fila = 0;
		_nCols = 0;
		_nRows = 0;
		_offset = 0;
		_offsetP = 0;
		resultado = 0;
	}

	/// <summary>
	/// Guarda las características de la matriz de leds para el cálculo.
	/// </summary>
	/// <param name="Cols">= Número de columnas de la matriz.</param>
	/// <param name="Rows">= Número de filas de la matriz.</param>
	/// <param name="inicio">= Ubicación física del primer led.</param>
	/// <param name="direccion">= Direccón de las tiras de led.</param>
	/// <param name="Posicion">= Si la matriz esta situada antes o despues de los pulsadores.</param>
	/// <param name="OrdenPulsador">= Si los leds de los primeros pulsadores estan conectados al derecho o al reves.
	void begin(int Cols, int Rows, ml_Inicio inicio, ml_Direccion direccion, ml_Posicion posicion = ml_Posicion::Antes, ml_OrdenPulsador ordenPulsador = ml_OrdenPulsador::Derecho) {

		_nCols = Cols;
		_nRows = Rows;
		_direccion = direccion;
		_orden = ordenPulsador;

		// Calculamos los offset en función de como est´nn distribuidos los leds.
		if (posicion == ml_Posicion::Antes) {
			_offset = 0;
			_offsetP = _nCols * _nRows;
			_offsetP2 = _offsetP + 8;
		}
		else
		{
			_offsetP = 0;
			_offsetP2 = _offsetP + 8;
			_offset = _offsetP2 + 8;
		}

		switch (inicio)
		{
		case ml_Inicio::Inicio_Inferior_Derecha:
			FilaInvertida = _direccion == ml_Direccion::Direccion_Horizontal ? false : true;
			ColumnaInvertida = true;
			break;
		case ml_Inicio::Inicio_Inferior_Izquierda:
			FilaInvertida = false;
			ColumnaInvertida = false;
			break;
		case ml_Inicio::Inicio_Superior_Derecha:
			FilaInvertida = _direccion == ml_Direccion::Direccion_Horizontal ? true : false;
			ColumnaInvertida = true;
			break;
		case ml_Inicio::Inicio_Superior_Izquierda:
			FilaInvertida = true;
			ColumnaInvertida = false;
			break;
		default:
			break;
		}
	}

	/// <summary>
	/// Devuelva la posición del led en función de la columna y fila suministradas.
	/// </summary>
	/// <param name="columna"></param>
	/// <param name="fila"></param>
	/// <returns>Posición del led</returns>
	int posicion(int columna, int fila) {

		// 
		_CambioColumna = ColumnaInvertida;
		_CambioFila = FilaInvertida;

		// En función de si la dirección es horizontal/vertical hay que invertir la columna/fila
		if (_direccion == ml_Direccion::Direccion_Horizontal)
		{
			_CambioColumna = fila % 2 == 1 ? !ColumnaInvertida : ColumnaInvertida;
		}
		else
		{
			_CambioFila = columna % 2 == 1 ? !FilaInvertida : FilaInvertida;
		}

		// Calculamos el número de columna/fila en función de si está invertida o no.
		_Columna = _CambioColumna ? _nCols - columna - 1 : columna;
		_Fila = _CambioFila ? _nRows - fila - 1 : fila;

		// Cálculo de la posición lineal del led.
		if (_direccion == ml_Direccion::Direccion_Horizontal)
		{
			resultado = _Fila * _nCols + _Columna;
		}
		else
		{
			resultado = _Columna * _nRows + _Fila;
		}
		return resultado + _offset;
	}

	/// <summary>
	/// Devuelve la posición del pulsador indicado.
	/// </summary>
	/// <param name="posicion">= Nº de pulsador (0 - 7)</param>
	/// <returns>Posición del led.</returns>
	int posicionP1(byte posicion) {
		return posicion + _offsetP;
	}

	/// <summary>
	/// Devuelve la posición del pulsador indicado.
	/// </summary>
	/// <param name="posicion">= Nº de pulsador (0 - 7)</param>
	/// <returns>Posición del led.</returns>
	int posicionP2(byte posicion) {
		return posicion + _offsetP2;
	}

private:
	ml_Direccion _direccion;
	ml_OrdenPulsador _orden;
	bool FilaInvertida = false, ColumnaInvertida = false, _CambioFila = false, _CambioColumna = false;
	int _Columna = 0, _Fila = 0, _nCols = 0, _nRows = 0, resultado = 0, _offset = 0, _offsetP = 0, _offsetP2 = 0;
};
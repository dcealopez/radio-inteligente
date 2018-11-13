//
// Escrito por: Diego Cea. (Doomer) - 2015
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPCION:
//
// Definición de la clase rIntel, objeto principal del programa,
// algunas constantes, enumeradores y otras funciones.
//

#pragma once

#include "r_defs.h"

#define RES_OK												1
#define RES_ERROR											0
#define MAX_IND_UND											4
#define MAX_IND_TAM											32
#define MAX_SND_TAM											64
#define MAX_COMB_MACRO										2
#define MACRO_DESACTIVAR									0
#define MACRO_DESACT_MODIFICADORA							0
#define MACRO_DESACT_TECLA									1
#define SND_OK												1
#define SND_ERR												0
#define CFG_FATAL_ERR										-1
#define CFG_RES_OK											0
#define CFG_RES_NUEVO										1
#define CFG_ERR_CREAR										2
#define CFG_ERR_APERTURA									3
#define CFG_ERR_ANALISIS									4
#define CFG_ERR_VACIO										5
#define COMP_ERR_CONEX									    -1
#define COMP_SERV_OK										1
#define COMP_SERV_NO										0

bool contieneMensajePalabras(const char *mensaje, const char *palabras[]);
void eliminarAcentosCadena(char *cadena);

typedef enum {

    S_DESACTIVADO,
    S_ACTIVADO,
    S_INICIADO,
    S_BEEP,
    S_SOLICITUD,
    S_IMPORTANTE,
    S_REPORTE,
    S_BPANICO_UNO,
    S_BPANICO_DOS,
    S_BPANICO_TRES,
    S_BPANICO_CUATRO,
    S_CENTRAL,
    S_CODIGO_DOS,
    S_CODIGO_TRES,
    S_CODIGO_TAC,
    S_CUSTODIA,
    S_CUSTODIA_VARIOS_UNO,
    S_CUSTODIA_VARIOS_DOS,
    S_HERIDO_UNO,
    S_HERIDO_DOS,
    S_HERIDO_TRES,
    S_HERIDO_CUATRO,
    S_ROBADO_UNO,
    S_ROBADO_DOS,
    S_ROBADO_TRES,
    S_ROBADO_CUATRO,
    S_ROBO_BANCO,
    S_ROBO_CASA,
    S_ROBO_UNO,
    S_ROBO_DOS,
    S_ROBO_TRES,
    S_ROBO_CUATRO,

    NUM_SONIDOS // Número total de sonidos
} l_sonidos;

typedef enum {

    E_STANDBY = -1,
    E_DESACTIVADA,
    E_ACTIVADA,
    E_NOVALIDO, // Servidor no válido

    NUM_ESTADOS

} e_radio;

typedef enum {

    A_REUNION_GENERAL,
    A_SEIS_ADAM,
    A_CODIGO_CINCO,
    A_CENTRALITA,
    A_REPORTES_UNIDADES,
    A_PROPIO, // Aviso propio, cuando nos llaman por unidad o nombre
    A_PEDIDOS,
    A_PEDIDOS_SWAT,
    A_CUSTODIA,
    A_AGENTES_SIN_ASIG,
    A_BOTON_PANICO,
    A_APOYO_REF,
    A_HERIDO,
    A_ROBO_COCHE,
    A_ROBO_CASA,
    A_ROBO_NEG,
    A_PRISION,
    A_ROBO_BANCO,

    MAX_TIPOS_AVISO

} a_radio;

/* Esta clase es la que utilizaremos para crear el
   objeto principal que usaremos a lo largo de la
   aplicación para manejar todos los datos de
   configuración y más
   
   La implentación de esta clase está en "r_radio.cpp" */

class rIntel {

    public:
        rIntel();
        ~rIntel();

        wchar_t *comprobarSonidos();
		int crearConfiguracion(wchar_t *nombre, wchar_t *numOLetra);
        int cargarConfiguracion();

		char *obtenerManeraIndicativo(unsigned const int manera);
        char *obtenerNombreJugador();
        char *obtenerIpServidor();
		int obtenerIdJugador();
        int obtenerEstadoRadio();
		int obtenerValorAviso(unsigned const int aviso);
		int obtenerCodigoTeclaMacro(unsigned const int macro);
		int obtenerCodigoRealMacro(unsigned const int macro);
		wchar_t *obtenerNombreIndicativo();
		int obtenerConfigActualizaciones();
        bool estaAvisoActivado(unsigned const int aviso);
        int esServidorValido();

        void establecerValorAviso(const int aviso, const int valor);
        void establecerNombreJugador(const char *nombre);
        void establecerIpServidor(const char *ip);
		void establecerIdJugador(short int idJugador);
		void establecerNombreIndicativo(const wchar_t *indicativo);
        void establecerEstadoRadio(unsigned const int estado);
		void establecerTeclaMacro(unsigned const int macro, unsigned const int tecla);
		void establecerConfigActualizaciones(int valor);

        void reEstablecer();
    private:
        char nombreJugador[MAX_CAR_NOMBRE_JUGADOR];
		short int idJugador;
        char ipServidor[MAX_CAR_IP_SERVIDOR];
        int estadoRadio;
		wchar_t nombreIndicativo[MAX_IND_TAM];
        char manerasIndicativoUnidad[MAX_IND_UND][MAX_IND_TAM];
        int configuracionAvisos[MAX_TIPOS_AVISO];
		int configuracionMacros[MAX_COMB_MACRO];
		int actualizacionesAutomaticas;
};

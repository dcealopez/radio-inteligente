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
// DESCRIPCIÓN:
//
// Implementación de las funciones para leer la memoria de SAMP.
//

#include <tchar.h>
#include <stdio.h>
#include <ctime>
#include "r_samp.h"
#include "r_defs.h"

/* Esta función borrará todos los códigos de colores de
   SAMP de un mensaje de chat.
   
   Argumento 1 (const char*): Mensaje original.
   Argumento 2 (char*): Variable donde guardar el mensaje
   sin los códigos de colores. */

void eliminarCodigosColores(const char *mensaje, char *destino) {

	unsigned int i = 0, j = 0;

	while(i < strlen(mensaje))
	{
		if((i + 7 < strlen(mensaje)) &&
			(mensaje[i] == '{') &&
			(mensaje[i + 7] == '}'))
		{
			i += 8;
		}
		else
		{
			destino[j++] = mensaje[i++];
		}
	}
	destino[j] = 0;
	return;
}

/* Esta función borrará la marca de la hora de un mensaje
   del chat.

   Argumento 1 (const char*): Mensaje original.
   Argumento 2 (char*): Variable donde guardar el mensaje
   sin los códigos de colores. */

void eliminarMarcaHora(const char *mensaje, char *destino) {

	unsigned int i = 0, j = 0;

	while(i < strlen(mensaje))
	{
		if((i + 9 < strlen(mensaje)) &&
			(mensaje[i] == '[') &&
			(mensaje[i + 9] == ']'))
		{
			i += 10;
		}
		else
		{
			destino[j++] = mensaje[i++];
		}
	}
	destino[j] = 0;
	return;
}

/* Esta función comprobará que SAMP esté ejecutandose.

   Devolverá 1 si se está ejecutando.
   Devolverá 0 si no se está ejecutando. */

int estaSampAbierto() {

    if(FindWindow(NULL, _T(VENTANA_GTA)) == NULL)
        return 0;

    return 1;
}

/* Esta función comprobará que la ventana de SAMP esté
   activa (maximizada).

   Devolverá 1 si está activa.
   Devolverá 0 si está minimizada. */

int estaVentanaSampActiva() {

	wchar_t tituloVentana[256];
	GetWindowText(GetForegroundWindow(), tituloVentana, 256);
	if(wcscmp(tituloVentana, _T(VENTANA_GTA)))
		return 0;

	return 1;
}

/* Esta función obtendra el proceso de GTA, si está abierto.

   Argumento 1 (HANDLE): HANDLE donde guardar el proceso.

   Devolverá 1 si se logró obtener el proceso.
   Devolverá 0 si no se pudo obtener el proceso. */

int obtenerProcesoGTA(HANDLE *procesoGTA) {

    if(!estaSampAbierto())
        return 0;

    DWORD idProcesoGTA;
    GetWindowThreadProcessId(FindWindow(NULL, _T(VENTANA_GTA)), &idProcesoGTA);
    *procesoGTA = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, idProcesoGTA);

    if(procesoGTA == NULL)
        return 0;

    return 1;
}

/* Esta función obtendra la dirección base del modulo "samp.dll".

   Argumento 1 (HANDLE): HANDLE al proceso del GTA.

   Devolverá la dirección de SAMP si todo salió bien.
   Devolverá 0x0 si no se pudo obtener la dirección.  */

DWORD obtenerDireccionSamp(HANDLE *procesoGTA) {

    if(procesoGTA == NULL)
        return DIR_NULA;

    HMODULE modulospGTA[1024];
    DWORD cbNeeded;
    unsigned int i;

    // Conseguimos la lista de todos los modulos que usa SAMP

    if(EnumProcessModules(*procesoGTA, modulospGTA, sizeof(modulospGTA), &cbNeeded))
    {
        for( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
        {
            wchar_t rutaModulo[MAX_PATH];

            // Conseguimos la ruta completa a cada uno de los modulos

            if(GetModuleFileNameEx(*procesoGTA, modulospGTA[i], rutaModulo,
                                      sizeof(rutaModulo) / sizeof(wchar_t)))
            {
                // Buscamos el modulo 'samp.dll'
                wchar_t nombreModulo[MAX_PATH];
                _wsplitpath_s(rutaModulo, NULL, NULL, NULL, NULL, nombreModulo, MAX_PATH, NULL, NULL);
                if(!wcscmp(nombreModulo, _T(MODULO_SAMP)))
                {
                    return (DWORD)modulospGTA[i]; // Yuhu! Ya tenemos la dirección base de SAMP
                }
            }
        }
    }
    return DIR_NULA;
}

/* Esta función obtendra el nombre del jugador.

   Argumento 1 (HANDLE): HANDLE al proceso del GTA.
   Argumento 2 (DWORD): Dirección base de SAMP.
   Argumento 3 (char *): Puntero donde guardar el nombre.

   Devolverá 1 si se logró obtener el nombre.
   Devolverá 0 si no se pudo obtener el nombre. */

int obtenerNombreJugador(HANDLE *procesoGTA, DWORD *direccionBase, char *destino) {

    if(*procesoGTA == NULL || *direccionBase == DIR_NULA || direccionBase == NULL)
        return 0;

    int resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(*direccionBase + DIR_NOMBRE_JUGADOR_SAMP), destino, MAX_CAR_NOMBRE_JUGADOR, NULL);

    if(!resultado || destino == NULL)
        return 0;

    return 1;
}

/* Esta función obtendra la ID del jugador local

	Argumento 1 (HANDLE): HANDLE al proceso del GTA.
	Argumento 2 (DWORD): Dirección base de SAMP.
	Argumento 3 (char *): Puntero donde guardar el nombre.

	Devolverá la ID del jugador local si se logró obtener.
	Devolverá -1 si no se pudo obtener la ID. */

int obtenerIdJugador(HANDLE *procesoGTA, DWORD *direccionBase) {

	if (*procesoGTA == NULL || *direccionBase == DIR_NULA || direccionBase == NULL)
		return -1;

	DWORD dir;
	short int idJugador = -1;

	int resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(*direccionBase + DIR_INFO_SAMP), &dir, sizeof(dir), NULL);

	if (!resultado || dir == DIR_NULA)
		return -1;

	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(dir + DIR_INFO_SAMP_PPOOL), &dir, sizeof(dir), NULL);

	if (!resultado || dir == DIR_NULA)
		return -1;

	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(dir + DIR_INFO_SAMP_PPOOL_LOCAL), &dir, sizeof(dir), NULL);

	if (!resultado || dir == DIR_NULA)
		return -1;

	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(dir + DIR_INFO_SAMP_ID_JUG_OFF), &idJugador, 2, NULL);

	if (!resultado || idJugador == -1)
		return -1;

	return idJugador;
}

/* Esta función obtendra el ultimo mensaje del chat.

   Argumento 1 (HANDLE): HANDLE al proceso del GTA.
   Argumento 2 (DWORD): Dirección base de SAMP.
   Argumento 3 (int): Entrada del chat a obtener. (99 es la última)
   Argumento 4 (char *): Puntero donde guardar el mensaje.

   Devolverá 1 si se logró obtener el nombre.
   Devolverá 0 si no se pudo obtener el nombre. */

int obtenerMensajeChat(HANDLE *procesoGTA, DWORD *direccionBase, int entrada, char *destino) {

    if(*procesoGTA == NULL || *direccionBase == DIR_NULA || direccionBase == NULL)
        return 0;

	DWORD dir;
	int tiempo;
	char prefijo[MAX_CAR_PREFIJO_CHAT_SAMP];
	char mensaje[MAX_CAR_MENSAJE_CHAT_SAMP];
	char mensajeSC[MAX_CAR_MENSAJE_CHAT_SAMP];
    int resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(*direccionBase + DIR_INFO_SAMP_CHAT), &dir, sizeof(dir), NULL);

	if(!resultado || dir == DIR_NULA)
		return 0;

	dir += 0x136; // Escalamos hasta la estructura de la entrada del mensaje
	entrada *= 252; // Cada entrada ocupa 252 bytes
	dir += entrada; // Nos posicionamos en la entrada que especificamos
	// Obtenemos la hora, el prefijo y luego el mensaje y concatenamos todo
	dir -= 4; // Nos movemos hasta la variable que contiene la hora
	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)dir, &tiempo, sizeof(tiempo), NULL);

	if(!resultado)
		return 0;

	dir += 4; // Nos movemos hasta la variable que contiene el prefijo
	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)dir, prefijo, MAX_CAR_PREFIJO_CHAT_SAMP, NULL);

	if(!resultado || prefijo == NULL)
		return 0;

	dir += 28; // Nos movemos hasta la variable que contiene el mensaje en sí
	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)dir, mensaje, MAX_CAR_MENSAJE_CHAT_SAMP, NULL);

	if(!resultado || destino == NULL)
		return 0;

	eliminarCodigosColores(mensaje, mensajeSC); // Eliminamos los codigos de colores
	// Transformamos el tiempo en segundos Unix a formato de hora real
	time_t tiempo_crudo = tiempo;
	struct tm hora;
	localtime_s(&hora, &tiempo_crudo);
	if(!strcmp(prefijo, "") || !strcmp(prefijo, " ")) // Si no hay prefijo, no lo incluimos
	{
		_snprintf_s(destino, MAX_CAR_MENSAJE_CHATLOG, MAX_CAR_MENSAJE_CHATLOG, "[%02d:%02d:%02d] %s", hora.tm_hour,
			hora.tm_min, hora.tm_sec, mensajeSC);
	}
	else
	{
		_snprintf_s(destino, MAX_CAR_MENSAJE_CHATLOG, MAX_CAR_MENSAJE_CHATLOG, "[%02d:%02d:%02d] %s %s", hora.tm_hour,
			hora.tm_min, hora.tm_sec, prefijo, mensajeSC);
	}

    return 1;
}

/* Esta función obtendra el nombre del servidor.

	Argumento 1 (HANDLE): HANDLE al proceso del GTA.
	Argumento 2 (DWORD): Dirección base de SAMP.
	Argumento 3 (char *): Puntero donde guardar el nombre.

	Devolverá 1 si se logró obtener el nombre.
	Devolverá 0 si no se pudo obtener el nombre. */

int obtenerIpServidor(HANDLE *procesoGTA, DWORD *direccionBase, char *destino) {

	if (*procesoGTA == NULL || *direccionBase == DIR_NULA || direccionBase == NULL)
		return 0;

	DWORD dir;
	int resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(*direccionBase + DIR_INFO_SAMP), &dir, sizeof(dir), NULL);

	if (!resultado || dir == DIR_NULA)
		return 0;

	resultado = ReadProcessMemory(*procesoGTA, (LPVOID)(dir + DIR_INFO_SAMP_IP_OFF), destino, MAX_CAR_IP_SERVIDOR, NULL);

	if (!resultado || destino == NULL)
		return 0;

	return 1;
}


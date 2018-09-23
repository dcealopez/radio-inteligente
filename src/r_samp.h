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
// Declaración de funciones y constantes para obtener datos
// desde la memoria de SAMP.
//

#pragma once

#include <windows.h>
#include <psapi.h>

#define MODULO_SAMP										"samp"

// Definiciones de las direcciones de memoria de SAMP

#define DIR_NULA                                        0x0
#define DIR_NOMBRE_JUGADOR_SAMP                         0x219A77
#define DIR_INFO_SAMP_CHAT                              0x21A0EC
#define DIR_INFO_SAMP                                   0x21A100
#define DIR_INFO_SAMP_PPOOL								0x3C5
#define DIR_INFO_SAMP_PPOOL_LOCAL						0x8
#define DIR_INFO_SAMP_IP_OFF                            0x1C
#define DIR_INFO_SAMP_ID_JUG_OFF						0x0

void eliminarCodigosColores(const char *mensaje, char *destino);
void eliminarMarcaHora(const char *mensaje, char *destino);

// Declaración de funciones principales de acceso al modulo

int estaSampAbierto();
int estaVentanaSampActiva();
int obtenerProcesoGTA(HANDLE *procesoGTA);
DWORD obtenerDireccionSamp(HANDLE *procesoGTA);

// Funciones para obtener ciertos datos del modulo en memoria

int obtenerNombreJugador(HANDLE *procesoGTA, DWORD *direccionBase, char *destino);
int obtenerIdJugador(HANDLE *procesoGTA, DWORD *direccionBase);
int obtenerMensajeChat(HANDLE *procesoGTA, DWORD *direccionBase, int entrada, char *destino);
int obtenerIpServidor(HANDLE *procesoGTA, DWORD *direccionBase, char *destino);

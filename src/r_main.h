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
// Definición de las funciones y constantes del corazón del programa.
//

#pragma once

#define MAX_LOADSTRING											100
#define	WM_USER_SHELLICON										WM_USER + 1

#define CRONO_ACTUALIZADOR										1
#define CRONO_ANALIZADOR										2
#define MODO_ACTUAL_SILEN										2
#define MODO_ACTUAL_AUTO										1
#define MODO_ACTUAL_MAN											0
#define TIEMPO_ACTUALIZACION									300000
#define TIEMPO_ANALISIS											250

// Definición de MSDN para comprobar versiones de DLL
#define PACKVERSION(major, minor) MAKELONG(minor, major)

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void reproducirSonido(const wchar_t *sonido);
void analizarMensajes();
BOOL estaProgramaRegistradoIniciarWindows();
BOOL agregarProgramaInicioWindows();
BOOL eliminarProgramaInicioWindows();
DWORD GetVersion(LPCTSTR lpszDllName);
void cambiarIcono(HICON icono);
bool esSistema64bits();
void comprobarActualizaciones(int modo);
void mostrarNotificacion(wchar_t *titulo, wchar_t *mensaje);
void liberarProcesosMemoria();

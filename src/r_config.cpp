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
// DESCRIPCION: Diálogo de configuración del programa.
//

#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include "../res/recursos.h"
#include "r_config.h"
#include "r_radio.h"
#include "r_main.h"

extern rIntel radioInteligente; // Necesitaremos esto para guardar y cargar configuraciones
extern const wchar_t *archivosDeSonido[];
extern int resultadoCargaConfig; // Necesitaremos esto para la notificación de la bandeja del sistema
extern NOTIFYICONDATA aplBandeja; // Necesitaremos esto por la misma razón de arriba
HWND ventanaConfig; // Handle de la ventana

/* Esta función ínicializará el diálogo de la
   ventana de configuración y la mostrará.
   
   Devolverá verdadero si se pudo crear la ventana.
   Devolverá falso si no se pudo crear la ventana. */

bool mostrarVentanaConfiguracion() {

	if(!IsWindow(ventanaConfig))
	{
		ventanaConfig = CreateDialog(NULL, MAKEINTRESOURCE(IDD_DIAG_CONFIG), NULL, VentanaConfiguracion);
		// Configuramos la entrada del control del macro para que solo acepte CTRL y ALT + otra tecla
		SendMessage(GetDlgItem(ventanaConfig, ID_CONF_MACRO), HKM_SETRULES, (WPARAM)HKCOMB_NONE | HKCOMB_CA | HKCOMB_S |
			HKCOMB_SA | HKCOMB_SC | HKCOMB_SCA, MAKELPARAM(HOTKEYF_CONTROL, 0));
		// Mostramos la configuración actual del macro
		SendMessage(GetDlgItem(ventanaConfig, ID_CONF_MACRO), HKM_SETHOTKEY,
			MAKEWORD(radioInteligente.obtenerCodigoTeclaMacro(MACRO_DESACT_TECLA),
			radioInteligente.obtenerCodigoTeclaMacro(MACRO_DESACT_MODIFICADORA)), 0);
		ShowWindow(ventanaConfig, SW_SHOW);
		UpdateWindow(ventanaConfig);
		SetForegroundWindow(ventanaConfig);
		return true;
	}
	SetForegroundWindow(ventanaConfig);
	return false;
}

/* Esta función cerrará el diálogo de la ventana
   de configuración.
   
   Devolverá verdadero si se pudo cerrar la ventana.
   Devolverá falso si no se pudo cerrar la ventana. */

bool cerrarVentanaConfiguracion() {

	if(IsWindow(ventanaConfig))
	{
		CloseWindow(ventanaConfig);
		DestroyWindow(ventanaConfig);
		ventanaConfig = NULL;
		return true;
	}
	return false;
}

/* Esta función es la que recibirá los mensajes de
   del diálogo de la ventana de configuración y hará
   las acciones correspondientes dependiendo del mensaje. 
   
   Esta función siempre devuelve 0. */

INT_PTR CALLBACK VentanaConfiguracion(HWND hwndConf, UINT mensaje, WPARAM wParam, LPARAM lParam) {

	switch (mensaje)
	{
		case WM_INITDIALOG:
			// Cargamos la configuración y la mostramos en el dialogo
			SetWindowText(GetDlgItem(hwndConf, ID_CONF_INDICATIVO), _T("Ej: ADAM-15"));
			if(wcscmp(radioInteligente.obtenerNombreIndicativo(), _T("N/A")))
				SetWindowText(GetDlgItem(hwndConf, ID_CONF_INDICATIVO), radioInteligente.obtenerNombreIndicativo());
			CheckDlgButton(hwndConf, ID_A_REUNION, radioInteligente.obtenerValorAviso(A_REUNION_GENERAL));
			CheckDlgButton(hwndConf, ID_A_SEISADAM, radioInteligente.obtenerValorAviso(A_SEIS_ADAM));
			CheckDlgButton(hwndConf, ID_A_CODIGOCINCO, radioInteligente.obtenerValorAviso(A_CODIGO_CINCO));
			CheckDlgButton(hwndConf, ID_A_CENTRALITA, radioInteligente.obtenerValorAviso(A_CENTRALITA));
			CheckDlgButton(hwndConf, ID_A_REP_UNIDADES, radioInteligente.obtenerValorAviso(A_REPORTES_UNIDADES));
			CheckDlgButton(hwndConf, ID_A_PROPIO, radioInteligente.obtenerValorAviso(A_PROPIO));
			CheckDlgButton(hwndConf, ID_A_PEDIDOS, radioInteligente.obtenerValorAviso(A_PEDIDOS));
			CheckDlgButton(hwndConf, ID_A_PEDIDOS_SWAT, radioInteligente.obtenerValorAviso(A_PEDIDOS_SWAT));
			CheckDlgButton(hwndConf, ID_A_CUSTODIA, radioInteligente.obtenerValorAviso(A_CUSTODIA));
			CheckDlgButton(hwndConf, ID_A_SIN_ASIG, radioInteligente.obtenerValorAviso(A_AGENTES_SIN_ASIG));
			CheckDlgButton(hwndConf, ID_A_PANICO, radioInteligente.obtenerValorAviso(A_BOTON_PANICO));
			CheckDlgButton(hwndConf, ID_A_APOYO, radioInteligente.obtenerValorAviso(A_APOYO_REF));
			CheckDlgButton(hwndConf, ID_A_HERIDO, radioInteligente.obtenerValorAviso(A_HERIDO));
			CheckDlgButton(hwndConf, ID_A_ROBO_COCHE, radioInteligente.obtenerValorAviso(A_ROBO_COCHE));
			CheckDlgButton(hwndConf, ID_A_ROBO_CASA, radioInteligente.obtenerValorAviso(A_ROBO_CASA));
			CheckDlgButton(hwndConf, ID_A_ROBO_NEG, radioInteligente.obtenerValorAviso(A_ROBO_NEG));
			CheckDlgButton(hwndConf, ID_A_PAPAFOX, radioInteligente.obtenerValorAviso(A_PRISION));
			CheckDlgButton(hwndConf, ID_A_ROBO_BANCO, radioInteligente.obtenerValorAviso(A_ROBO_BANCO));
			CheckDlgButton(hwndConf, ID_AVISOS_ACTUAL, radioInteligente.obtenerConfigActualizaciones());
			break;
		case WM_DESTROY:
			cerrarVentanaConfiguracion();
			break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				default:
					break;
				case IDOK:
					// Comprobamos que los datos estén bien
					wchar_t indicativo[32], nombre[32], numOLetra[3];
					GetWindowText(GetDlgItem(hwndConf, ID_CONF_INDICATIVO), indicativo, 32);
					if(swscanf_s(indicativo, _T("%[^-]-%[^-]"), nombre, 32, numOLetra, 3) != 2 || nombre == NULL || numOLetra == NULL || wcslen(numOLetra) > 2)
					{
						MessageBeep(MB_ICONERROR);
						MessageBox(hwndConf, _T("El campo del indicativo de la unidad está vacío o tiene un formato incorrecto.")
							_T(" Recuerde que el campo debe contener el indicativo completo de la unidad.\n\n")
							_T("Ejemplo: ADAM-15."),
							_T("Error"), MB_OK | MB_ICONEXCLAMATION);
						break;
					}
					radioInteligente.establecerNombreIndicativo(indicativo);
					DWORD macro = SendMessage(GetDlgItem(hwndConf, ID_CONF_MACRO), HKM_GETHOTKEY, 0, 0);
					if(macro == NULL)
					{
						MessageBeep(MB_ICONERROR);
						MessageBox(hwndConf, _T("El campo del macro para desactivar el programa no puede dejarse vacío.")
							_T(" Recuerde que para establecer el macro debe hacer click en el campo y pulsar la combinación deseada."),
							_T("Error"), MB_OK | MB_ICONEXCLAMATION);
						break;
					}
					// Registramos el macro para desactivar la radio
					radioInteligente.establecerTeclaMacro(MACRO_DESACT_MODIFICADORA, HIBYTE(macro));
					radioInteligente.establecerTeclaMacro(MACRO_DESACT_TECLA, LOBYTE(macro));
					radioInteligente.establecerValorAviso(A_REUNION_GENERAL, IsDlgButtonChecked(hwndConf, ID_A_REUNION));
					radioInteligente.establecerValorAviso(A_SEIS_ADAM, IsDlgButtonChecked(hwndConf, ID_A_SEISADAM));
					radioInteligente.establecerValorAviso(A_CODIGO_CINCO, IsDlgButtonChecked(hwndConf, ID_A_CODIGOCINCO));
					radioInteligente.establecerValorAviso(A_CENTRALITA, IsDlgButtonChecked(hwndConf, ID_A_CENTRALITA));
					radioInteligente.establecerValorAviso(A_REPORTES_UNIDADES, IsDlgButtonChecked(hwndConf, ID_A_REP_UNIDADES));
					radioInteligente.establecerValorAviso(A_PROPIO, IsDlgButtonChecked(hwndConf, ID_A_PROPIO));
					radioInteligente.establecerValorAviso(A_PEDIDOS, IsDlgButtonChecked(hwndConf, ID_A_PEDIDOS));
					radioInteligente.establecerValorAviso(A_PEDIDOS_SWAT, IsDlgButtonChecked(hwndConf, ID_A_PEDIDOS_SWAT));
					radioInteligente.establecerValorAviso(A_CUSTODIA, IsDlgButtonChecked(hwndConf, ID_A_CUSTODIA));
					radioInteligente.establecerValorAviso(A_AGENTES_SIN_ASIG, IsDlgButtonChecked(hwndConf, ID_A_SIN_ASIG));
					radioInteligente.establecerValorAviso(A_BOTON_PANICO, IsDlgButtonChecked(hwndConf, ID_A_PANICO));
					radioInteligente.establecerValorAviso(A_APOYO_REF, IsDlgButtonChecked(hwndConf, ID_A_APOYO));
					radioInteligente.establecerValorAviso(A_HERIDO, IsDlgButtonChecked(hwndConf, ID_A_HERIDO));
					radioInteligente.establecerValorAviso(A_ROBO_COCHE, IsDlgButtonChecked(hwndConf, ID_A_ROBO_COCHE));
					radioInteligente.establecerValorAviso(A_ROBO_CASA, IsDlgButtonChecked(hwndConf, ID_A_ROBO_CASA));
					radioInteligente.establecerValorAviso(A_ROBO_NEG, IsDlgButtonChecked(hwndConf, ID_A_ROBO_NEG));
					radioInteligente.establecerValorAviso(A_PRISION, IsDlgButtonChecked(hwndConf, ID_A_PAPAFOX));
					radioInteligente.establecerValorAviso(A_ROBO_BANCO, IsDlgButtonChecked(hwndConf, ID_A_ROBO_BANCO));
					radioInteligente.establecerConfigActualizaciones(IsDlgButtonChecked(hwndConf, ID_AVISOS_ACTUAL));
					radioInteligente.crearConfiguracion(nombre, numOLetra); // Guardamos la configuración
					cerrarVentanaConfiguracion();
					// Si fue la primera vez que ejecutamos el programa, notificamos al usuario
					if(resultadoCargaConfig == CFG_RES_NUEVO)
					{
						mostrarNotificacion(_T("¡Bienvenido a Radio Inteligente!"),
							_T("Esta es la primera vez que utiliza Radio Inteligente.\n")
							_T("El programa permanecerá minimizado en la bandeja del sistema. Si hace click derecho")
							_T(" sobre el icono encontrará una serie de opciones para configurar el programa."));
						resultadoCargaConfig = CFG_RES_OK;
					}
					break;
			}
		}
	}
	return 0;
}
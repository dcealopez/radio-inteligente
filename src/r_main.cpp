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
// DESCRIPCION: Corazón del programa.
//
// Radio Inteligente es una utilidad para usar en conjunto con SAMP.
// Añade efectos sonoros para el radiocomunicador de la facción LSPD y SASD.
//

#include <tchar.h>
#include <windows.h>
#include <psapi.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <time.h>
#include "r_main.h"
#include "r_radio.h"
#include "r_samp.h"
#include "r_config.h"
#include "../res/recursos.h"

// Nuestro objeto de radio con todas las funciones
rIntel radioInteligente;

// Variable que determina si nuestro programa está funcionando o no
bool funcionando = true;
// Variable que determina si hay alguna actualización disponible para el programa
bool actualizacionDisponible = false;
int resultadoCargaConfig; // El resultado de la carga de la configuración.
// Necesitaremos esto para la comprobación de los sonidos
extern const wchar_t *archivosDeSonido[];

// Titulo de la aplicación y nombre de la clase
TCHAR tituloApl[] = _T("Radio Inteligente");
TCHAR nombreClaseApl[] = _T("radiointeligente");

/* Estas son las variables que utilizaremos para almacenar los mensajes
   que vayan llegando al chat */

char ultimoMensaje[MAX_CAR_MENSAJE_CHATLOG] = "N/A";
char penultimoMensaje[MAX_CAR_MENSAJE_CHATLOG] = "N/A";
char ultimoMensajeAnterior[MAX_CAR_MENSAJE_CHATLOG] = "N/A";
char mensajeCompleto[(MAX_CAR_MENSAJE_CHATLOG)* 2] = "N/A";

// Declaración externa de la lista que contiene los servidores permitidos
extern const char *servidoresPermitidos[];

/* Todas estas declaraciones externas contienen las palabras clave
   para cada tipo de aviso y palabras clave prohibidas para los
   mensajes que analizaremos */

extern const char *PC_PROHIBIDAS_GENERAL[];
extern const char *PC_REUNION_GENERAL[];
extern const char *PC_SEIS_ADAM[];
extern const char *PC_CODIGO_CINCO[];
extern const char *PC_CENTRALITA[];
extern const char *PC_REP_UNIDADES[];
extern const char *PC_REP_UNIDADES_PROHIB[];
extern const char *PC_PEDIDOS_SWAT[];
extern const char *PC_PEDIDOS_GENERAL[];
extern const char *PC_PEDIDOS_PROHIB[];
extern const char *PC_PEDIDO_URGENTE[];
extern const char *PC_PEDIDO_NOURGENTE[];

// Estructuras para crear la aplicación en la bandeja del sistema
HWND hwndApl;
HMENU menuApl;
HICON iconoPrincipal;
HICON iconoNotificacion;
NOTIFYICONDATA aplBandeja;

// Definiremos algunas variables globales
HANDLE procesoGTA;
DWORD baseSAMP = DIR_NULA;

// Variables para el analizador de pulsaciones del teclado
RAWINPUTDEVICE disCrudo;
DWORD antiguoEvento = WM_KEYUP;
USHORT teclaAnterior = 0;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {

	// Evitamos multiples instancias de nuestra aplicación
	CreateEvent(NULL, FALSE, FALSE, nombreClaseApl);
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		MessageBox(hwndApl, _T("El programa ya se está ejecutando. No puede haber más de")
			_T(" una instancia del programa abierta al mismo tiempo."), _T("Radio Inteligente - Advertencia"), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

    UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpszArgument);

    MSG messages;            // Para recibir los mensajes
	WNDCLASSEX wcex;		 // La clase de nuestra ventana

	// Definimos el estilo de la ventana de la aplicación

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WindowProcedure;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(ID_ICONO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= nombreClaseApl;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(ID_ICONO));

    // Registramos la ventana

    if(!RegisterClassEx (&wcex))
        return 0;

    // Creamos la ventana

    hwndApl = CreateWindowEx(0, nombreClaseApl, tituloApl, WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, HWND_DESKTOP, NULL, hInstance, NULL);

    if(!hwndApl)
        return 0;

	/* Configuramos la estructura para poder obtener la información
	de las pulsaciones del teclado del usuario y comprobar si
	están pulsando un macro del programa. */

	disCrudo.dwFlags	 = RIDEV_INPUTSINK;
	disCrudo.usUsagePage = 0x01; // Aplicación de escritorio
	disCrudo.usUsage	 = 0x06; // Dispositivo teclado
	disCrudo.hwndTarget  = hwndApl;

	// Lo registramos, si falla, notificamos al usuario
	if (RegisterRawInputDevices(&disCrudo, 1, sizeof(disCrudo)) == false)
	{
		SetForegroundWindow(hwndApl);
		MessageBeep(MB_ICONERROR);
		MessageBox(hwndApl, L"Ocurrió un error al registrar el dispositivo de entrada del teclado.\n"
			L"Intente ejecutar la aplicación como administrador para solucionar el problema. Si aún así"
			L" el problema persiste, contacte con el desarrollador de la aplicación y notifíquele el problema"
			L" para intentar solucionarlo.", L"Radio Inteligente - Error", MB_ICONERROR | MB_OK);
		SetForegroundWindow(hwndApl);
		return 0;
	}

	/* Antes de nada realizaremos todas las comprobaciones necesarias
	   antes de iniciar el programa. Comprobaremos que todos los sonidos
	   que tendremos que usar más adelante existan en el disco duro
	   del usuario. Después cargaremos el archivo de configuración, y
	   si no existe o hay algún error se lo notificaremos al usuario
	   y le ayudaremos a solucionarlo */

    wchar_t *sonidos = radioInteligente.comprobarSonidos();
    if(sonidos != NULL)
    {
		wchar_t buffer[256 + MAX_PATH];
        swprintf_s(buffer, 256 + MAX_PATH, L"No se pudo iniciar el programa porque falta: %s\n\n"
			_T("Por favor, reinstale el programa para solucionar el problema. Si aún así el")
			_T(" problema no se soluciona, contacte con el desarrollador de la aplicación para")
			_T(" notificárselo."), sonidos);
		SetForegroundWindow(hwndApl);
        MessageBeep(MB_ICONERROR);
		MessageBox(hwndApl, buffer, _T("Radio Inteligente - Error"), MB_OK | MB_ICONERROR);
		SetForegroundWindow(hwndApl);
        free(sonidos);
        return 0;
    }

	// Cargamos la configuración
	resultadoCargaConfig = radioInteligente.cargarConfiguracion();
    switch(resultadoCargaConfig)
	{
		int res;
		default:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONERROR);
			MessageBox(hwndApl, _T("Ocurrió un error desconocido al intentar cargar la configuración.")
				_T(" Por favor, intente ejecutar el programa de nuevo, y si el problema persiste,")
				_T(" contacte con el desarrollador de la aplicación para notificarle sobre el problema")
				_T(" adjuntando el código de error.\n\n")
				_T("Código de error: error desconocido."), _T("Radio Inteligente - Error"), MB_OK | MB_ICONERROR);
			SetForegroundWindow(hwndApl);
			return 0;
			break;
		case CFG_RES_OK:
			break;
		case CFG_RES_NUEVO:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONINFORMATION);
			MessageBox(hwndApl, _T("Parece que esta es la primera vez que abre Radio Inteligente.")
				_T(" Para empezar, tendrá que ajustar la configuración del programa antes de poder empezar a utilizarlo. ")
				_T("Pulse Aceptar para continuar."), _T("Radio Inteligente - Configuración"), MB_OK | MB_ICONINFORMATION);
			SetForegroundWindow(hwndApl);
			mostrarVentanaConfiguracion();
			break;
		case CFG_ERR_APERTURA:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONERROR);
			MessageBox(hwndApl, _T("Ocurrió un error y no se pudo cargar la configuración porque")
				_T(" el archivo de configuración está en uso o no tiene permisos suficientes para")
				_T(" abrir el archivo. Inténtelo de nuevo ejecutando la aplicación en modo")
				_T(" administrador o reinicie el ordenador. Si el problema persiste, contacte con el")
				_T(" desarrollador de la aplicación para notificarle sobre el problema adjuntando el")
				_T(" código de error.\n\n")
				_T("Código de error: 3."), _T("Radio Inteligente - Error"), MB_OK | MB_ICONERROR);
			SetForegroundWindow(hwndApl);
			return 0;
			break;
		case CFG_ERR_CREAR:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONERROR);
			MessageBox(hwndApl, _T("Ocurrió un error y no se pudo crear el archivo de configuración.")
				_T(" Probablemente no tiene permisos suficientes para crear el archivo.")
				_T(" Inténtelo de nuevo ejecutando la aplicación en modo administrador")
				_T(" o reinicie el ordenador. Si el problema persiste, contacte con el")
				_T(" desarrollador de la aplicación para notificarle sobre el problema adjuntando el")
				_T(" código de error.\n\n")
				_T("Código de error: 2."), _T("Radio Inteligente - Error"), MB_OK | MB_ICONERROR);
			SetForegroundWindow(hwndApl);
			return 0;
			break;
		case CFG_ERR_ANALISIS:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONERROR);
			res = MessageBox(hwndApl, _T("Ocurrió un error al cargar el archivo de configuración porque")
				_T(" una de las entradas o registros del archivo tiene un formato incorrecto.")
				_T(" Para solucionar este problema, puede eliminar el archivo de configuración actual y")
				_T(" dejar que el programa cree uno nuevo la próxima vez que lo ejecute.\n\n")
				_T("Pulse Aceptar para volver a crear la configuración desde cero.\n")
				_T("Pulse Cancelar para cerrar el programa y solucionar el problema manualmente.\n\n")
				_T("Si el problema persiste, contacte con el desarrollador de la aplicación para notificarle")
				_T(" sobre el problema adjuntando el código de error.\n\n")
				_T("Código de error: 4."), _T("Radio Inteligente - Error"), MB_OKCANCEL | MB_ICONERROR);
			SetForegroundWindow(hwndApl);
			if(res == IDCANCEL)
				return 0;
			else
				mostrarVentanaConfiguracion();
			break;
		case CFG_ERR_VACIO:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONERROR);
			res = MessageBox(hwndApl, _T("Ocurrió un error al cargar el archivo de configuración porque")
				_T(" el archivo está vacío. Para solucionar este problema, puede eliminar el archivo")
				_T("  de configuración actual y dejar que el programa cree uno nuevo la próxima vez que lo ejecute.\n\n")
				_T("Pulse Aceptar para volver a crear la configuración desde cero.\n")
				_T("Pulse Cancelar para cerrar el programa y solucionar el problema manualmente.\n\n")
				_T("Si el problema persiste, contacte con el desarrollador de la aplicación para notificarle")
				_T(" sobre el problema adjuntando el código de error.\n\n")
				_T("Código de error: 5."), _T("Radio Inteligente - Error"), MB_OKCANCEL | MB_ICONERROR);
			SetForegroundWindow(hwndApl);
			if(res == IDCANCEL)
				return 0;
			else
				mostrarVentanaConfiguracion();
			break;
		case CFG_FATAL_ERR:
			SetForegroundWindow(hwndApl);
			MessageBeep(MB_ICONERROR);
			res = MessageBox(hwndApl, _T("Ocurrió un error al cargar el archivo de configuración porque")
				_T(" el archivo está corrupto. Para solucionar este problema, puede eliminar el archivo")
				_T("  de configuración actual y dejar que el programa cree uno nuevo la próxima vez que lo ejecute.\n\n")
				_T("Pulse Aceptar para volver a crear la configuración desde cero.\n")
				_T("Pulse Cancelar para cerrar el programa y solucionar el problema manualmente.\n\n")
				_T("Si el problema persiste, contacte con el desarrollador de la aplicación para notificarle")
				_T(" sobre el problema adjuntando el código de error.\n\n")
				_T("Código de error: -1."), _T("Radio Inteligente - Error"), MB_OKCANCEL | MB_ICONERROR);
			SetForegroundWindow(hwndApl);
			if(res == IDCANCEL)
				return 0;
			else
				mostrarVentanaConfiguracion();
			break;
	}

    // Preparamos nuestra estructura para la aplicación de la bandeja del sistema

    iconoPrincipal = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(ID_ICONO));
	iconoNotificacion = LoadIcon(hInstance, (LPTSTR)MAKEINTRESOURCE(ID_ICONO_NOTIF));

	/* Comprobamos la versión de Shell32.dll para que
	   las notificaciones de la aplicación de la bandeja
	   del sistema funcionen en versiones anteriores de
	   los sistemas operativos de Windows. */

	DWORD versionDll = GetVersion(L"C:\\Windows\\System32\\Shell32.dll");

	if(versionDll > PACKVERSION(6, 0))
		aplBandeja.cbSize = sizeof(NOTIFYICONDATA);
	else if(versionDll == PACKVERSION(6, 0))
		aplBandeja.cbSize = NOTIFYICONDATA_V3_SIZE;
	else if(versionDll == PACKVERSION(5, 0))
		aplBandeja.cbSize = NOTIFYICONDATA_V2_SIZE;
	else if(versionDll < PACKVERSION(5, 0))
		aplBandeja.cbSize = NOTIFYICONDATA_V1_SIZE;
	else
		aplBandeja.cbSize = sizeof(NOTIFYICONDATA);

	aplBandeja.hWnd = (HWND)hwndApl;          // Ventana que procesará los mensajes de nuestra aplicación
	aplBandeja.uID = ID_ICONO_TRAY;           // ID del icono
	aplBandeja.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	aplBandeja.hIcon = iconoPrincipal;
	aplBandeja.uCallbackMessage = WM_USER_SHELLICON;
	wcscpy_s((wchar_t *)aplBandeja.szTip, 128, (wchar_t *)tituloApl);
    Shell_NotifyIcon(NIM_ADD, &aplBandeja); // Iniciamos la aplicación
	// Iniciamos el Timer para las comprobaciones de las notificaciones de las actualizaciones
	SetTimer(hwndApl, CRONO_ACTUALIZADOR, TIEMPO_ACTUALIZACION, NULL);
	SetTimer(hwndApl, CRONO_ANALIZADOR, TIEMPO_ANALISIS, NULL);

	if(resultadoCargaConfig != CFG_RES_NUEVO)
		comprobarActualizaciones(MODO_ACTUAL_AUTO);

	/* Eliminamos la anterior entrada del registro para que el
	   programa inicie con Windows y la volvemos a agregar por si
	   el usuario movió el ejecutable de lugar. */

	if(estaProgramaRegistradoIniciarWindows())
	{
		eliminarProgramaInicioWindows();
		agregarProgramaInicioWindows();
	}

	/* A partir de este punto ya estaremos listos para continuar con
	   la ejecución del programa.
	   
	   Este es el bucle principal, el corazón del programa. */
    do
    {
        // Siempre atentos a los mensajes de la aplicacion
		if(GetMessage(&messages, NULL, 0, 0))
		{
			TranslateMessage(&messages);
			DispatchMessage(&messages);
		}

        if(!estaSampAbierto()) // Esperamos a que se inicie SAMP
        {
            liberarProcesosMemoria();
            if(strcmp(radioInteligente.obtenerNombreJugador(), "N/A"))
                radioInteligente.reEstablecer();

            continue;
        }

		if(!estaVentanaSampActiva()) // Si estamos minimizados, no hacemos nada
			continue;

        /* Si el servidor no es válido o la radio está desactivada,
           detenemos el análisis hasta que se vuelva a activar la radio
           o hasta que se cambie de servidor. En el caso de que esté
           desactivada, escuchamos también las teclas por si el usuario
           quiere volver a activar la radio más tarde. */

        if(radioInteligente.obtenerEstadoRadio() == E_NOVALIDO)
            continue;

        /* Si estamos aqui es porque SAMP está abierto
           Obtenemos el proceso de GTA la dirección base de SAMP */

        if(procesoGTA == NULL && baseSAMP == (DWORD)DIR_NULA)
        {
            obtenerProcesoGTA(&procesoGTA);
            baseSAMP = obtenerDireccionSamp(&procesoGTA);
            continue;
        }

        /* Establecemos los datos básicos necesarios para poder funcionar.
           Mientras no estén establecidos no continuamos. */

        if(!strcmp(radioInteligente.obtenerNombreJugador(), "N/A"))
        {
            char nombreJugador[MAX_CAR_NOMBRE_JUGADOR];
            if(obtenerNombreJugador(&procesoGTA, &baseSAMP, nombreJugador) == 0)
                continue;

			// Lo pasamos a minúsculas
			for(unsigned int i = 0; i < strlen(nombreJugador); i++)
				nombreJugador[i] = tolower(nombreJugador[i]);

            radioInteligente.establecerNombreJugador(nombreJugador);
			continue;
        }

		if (radioInteligente.obtenerIdJugador() == -1)
		{
			radioInteligente.establecerIdJugador(obtenerIdJugador(&procesoGTA, &baseSAMP));
			continue;
		}

        if(!strcmp(radioInteligente.obtenerIpServidor(), "N/A"))
        {
            char ipServidor[MAX_CAR_IP_SERVIDOR];
            if(obtenerIpServidor(&procesoGTA, &baseSAMP, ipServidor) == 0)
                continue;

            radioInteligente.establecerIpServidor(ipServidor);
			continue;
        }

        /* Una vez que tengamos la información necesaria para funcionar,
           comprobaremos que el servidor en el que estamos es válido si
           todavía no lo habíamos comprobado anteriormente. */

        if(radioInteligente.obtenerEstadoRadio() == E_STANDBY)
        {
			// Si no se pudo comprobar, continuamos intentándolo
			if(radioInteligente.esServidorValido() == COMP_ERR_CONEX)
				continue;

            if(radioInteligente.esServidorValido() == COMP_SERV_OK)
            {
                reproducirSonido(archivosDeSonido[S_ACTIVADO]);
                radioInteligente.establecerEstadoRadio(E_ACTIVADA); // Activamos la radio
            }
            else
            {
                reproducirSonido(archivosDeSonido[S_DESACTIVADO]);
                radioInteligente.establecerEstadoRadio(E_NOVALIDO); // Deshabilitamos la radio
				continue;
            }
        }
    }
    while(funcionando); // Mientras el programa esté funcionado, continuamos el bucle

    return (int) messages.wParam;
}

/* Función llamada por el sistema operativo cada vez que
   la aplicación reciba algún mensaje. Analizaremos estos
   mensajes y realizaremos las acciones correspondientes
   dependiendo del mensaje. */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

    POINT lugarClick;
    switch(message) 
    {
        case WM_USER_SHELLICON:
            switch(LOWORD(lParam))
            {
				case WM_LBUTTONDBLCLK: // Al pulsar doble click en el icono, mostramos la ventana de configuración
					mostrarVentanaConfiguracion();
					break;
                case WM_RBUTTONDOWN:
                    GetCursorPos(&lugarClick); // Atentos a donde el usuario hace click
                    menuApl = CreatePopupMenu();
                    // Creamos las entradas del menú principal de la aplicación
					InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, 3, _T("Configuración"));
					SetMenuDefaultItem(menuApl, 3, FALSE); // Ponemos el menú "Configuración" como por defecto, para que aparezca en negrita
					if(actualizacionDisponible)
					{
						InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, 5, _T("1 actualización disponible"));
						SetMenuDefaultItem(menuApl, 5, FALSE);
					}
					else
						InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, 5, _T("Buscar actualizaciones"));
					if(estaProgramaRegistradoIniciarWindows())
						InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING | MF_CHECKED, 4, _T("Iniciar con Windows"));
					else
						InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, 4, _T("Iniciar con Windows"));
					InsertMenu(menuApl, 0xFFFFFFFF, MF_SEPARATOR, -1, _T("SEP"));
					InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, 2, _T("Acerca de..."));
					InsertMenu(menuApl, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, 1, _T("Salir"));
                    SetForegroundWindow(hwnd); // Ponemos la ventana como activa, si no lo estaba ya
					TrackPopupMenu(menuApl, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lugarClick.x, lugarClick.y, 0, hwnd, NULL);
                    return TRUE;
                    break;
				case NIN_BALLOONUSERCLICK:
					if(actualizacionDisponible)
						comprobarActualizaciones(MODO_ACTUAL_MAN);
					break;
				default:
					return DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case 1: // Salir
					liberarProcesosMemoria();
					Shell_NotifyIcon(NIM_DELETE, &aplBandeja);
					DestroyWindow(hwnd);
					cerrarVentanaConfiguracion();
					// Eliminamos el dispositivo que creamos anteriormente
					disCrudo.dwFlags = RIDEV_REMOVE;
					RegisterRawInputDevices(&disCrudo, 1, sizeof(disCrudo));
					funcionando = false;
                    PostQuitMessage(0);
                    break;
                case 2: // Acerca de...
					MessageBeep(MB_ICONINFORMATION);
					MessageBox(hwnd, _T("Radio Inteligente es una utilidad creada para usarse en conjunto con SAMP ")
						_T("que añade efectos sonoros para el radiocomunicador de la facción LSPD y SASD de la comunidad ")
						_T("Los Santos Juego de Rol.\n\n")
						_T("Versión: ") _T(RINTEL_VERSION) _T(" (") _T(VERSION_SAMP) _T(")\n")
                        _T("Programador: Diego Cea (Doomer)"),
                        _T("Acerca de Radio Inteligente"), MB_OK | MB_ICONINFORMATION);
                    break;
				case 3: // Configuración
					mostrarVentanaConfiguracion();
					break;
				case 4: // Iniciar con windows
					if(!estaProgramaRegistradoIniciarWindows())
						agregarProgramaInicioWindows();
					else
						eliminarProgramaInicioWindows();
					break;
				case 5: // Buscar actualizaciones
					comprobarActualizaciones(MODO_ACTUAL_MAN);
					break;
                default:
                    return DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;

		/* Aquí recibimos la entrada del usuario desde el dispositivo
		   que registramos para poder comprobar si el usuario está pulsando
		   una macro del programa. */

		case WM_INPUT:
		{
			if (!estaVentanaSampActiva())
				break;

			UINT dwSize;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

			LPBYTE lpb = new BYTE[dwSize];

			if (lpb == NULL)
				break;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;
			DWORD evento;

			evento = raw->data.keyboard.Message;

			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				// Macro para desactivar el programa
				if (radioInteligente.obtenerEstadoRadio() != E_NOVALIDO)
				{
					if (raw->data.keyboard.VKey == radioInteligente.obtenerCodigoRealMacro(MACRO_DESACT_TECLA) &&
						teclaAnterior == radioInteligente.obtenerCodigoRealMacro(MACRO_DESACT_MODIFICADORA) &&
						evento == WM_KEYDOWN && antiguoEvento == WM_KEYDOWN)
					{
						if (radioInteligente.obtenerEstadoRadio() == E_ACTIVADA)
						{
							radioInteligente.establecerEstadoRadio(E_DESACTIVADA);
							reproducirSonido(archivosDeSonido[S_DESACTIVADO]);
						}
						else
						{
							radioInteligente.establecerEstadoRadio(E_ACTIVADA);
							reproducirSonido(archivosDeSonido[S_ACTIVADO]);
						}
					}
				}
			}
			antiguoEvento = evento;
			teclaAnterior = raw->data.keyboard.VKey;
			break;
		}
		case WM_TIMER:
			switch (wParam)
			{
				/* Si las notificaciones automáticas para las actualizaciones
				están activadas, las comprobamos cada cinco minutos. Si no,
				las comprobamos silenciosamente sin notificar al usuario. */

				case CRONO_ACTUALIZADOR:
					if(radioInteligente.obtenerConfigActualizaciones() == 1)
						comprobarActualizaciones(MODO_ACTUAL_AUTO);
					else
						comprobarActualizaciones(MODO_ACTUAL_SILEN);
					break;
				case CRONO_ANALIZADOR:
					if(funcionando && estaSampAbierto() && estaVentanaSampActiva() &&
						radioInteligente.obtenerEstadoRadio() == E_ACTIVADA)
					{
						analizarMensajes();
					}
					break;
				default:
					break;
			}
			break;
		case WM_QUIT:
			liberarProcesosMemoria();
			Shell_NotifyIcon(NIM_DELETE, &aplBandeja);
			DestroyWindow(hwnd);
			cerrarVentanaConfiguracion();
			// Eliminamos el dispositivo que creamos anteriormente
			disCrudo.dwFlags = RIDEV_REMOVE;
			RegisterRawInputDevices(&disCrudo, 1, sizeof(disCrudo));
			funcionando = false;
			PostQuitMessage(0);
			break;
        case WM_DESTROY:
            liberarProcesosMemoria();
            Shell_NotifyIcon(NIM_DELETE, &aplBandeja);
            DestroyWindow(hwnd);
			cerrarVentanaConfiguracion();
			// Eliminamos el dispositivo que creamos anteriormente
			disCrudo.dwFlags = RIDEV_REMOVE;
			RegisterRawInputDevices(&disCrudo, 1, sizeof(disCrudo));
            funcionando = false;
			PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

/* Esta función reproducirá el sonido especificado
   en el argumento.
   
   Argumento 1 (wchar_t*): Archivo de sonido. */

void reproducirSonido(const wchar_t *sonido) {

	wchar_t dirProg[MAX_PATH];
	wchar_t letraDisco[3];
	wchar_t directorio[MAX_PATH];
	wchar_t rutaFinal[MAX_PATH];
	GetModuleFileNameW(NULL, dirProg, MAX_PATH);
	_wsplitpath_s(dirProg, letraDisco, 3, directorio, MAX_PATH, NULL, NULL, NULL, NULL);
	swprintf(rutaFinal, MAX_PATH, L"%s%s", letraDisco, directorio);
	wcscat_s(rutaFinal, MAX_PATH, _T("sonidos\\"));
	wcscat_s(rutaFinal, MAX_PATH, sonido);
	PlaySound(rutaFinal, NULL, SND_FILENAME | SND_SYNC);
	return;
}

/* Esta función es llamada por el cronómetro de
   la aplicación cada 250 milisegundos y se
   encargará de analizar los mensajes recibidos
   en el chat de SAMP. Si son reconocidos por el
   programa, notificará al usuario con el sonido
   correspondiente dependiendo de la configuración
   del programa. */

void analizarMensajes() {

	strcpy_s(ultimoMensajeAnterior, MAX_CAR_MENSAJE_CHATLOG, ultimoMensaje);

	if(!obtenerMensajeChat(&procesoGTA, &baseSAMP, 99, ultimoMensaje))
		return;
	if(!obtenerMensajeChat(&procesoGTA, &baseSAMP, 98, penultimoMensaje))
		return;

	// Lo ponemos todo en minusculas
	for(unsigned int i = 0; i < strlen(ultimoMensaje); i++)
		ultimoMensaje[i] = tolower(ultimoMensaje[i]);
	for(unsigned int i = 0; i < strlen(penultimoMensaje); i++)
		penultimoMensaje[i] = tolower(penultimoMensaje[i]);

	// Antes de nada, eliminamos todos los acentos de los mensajes
	eliminarAcentosCadena(ultimoMensaje);
	eliminarAcentosCadena(penultimoMensaje);

	/* Luego, evitamos analizar todos aquellos mensajes que no sean
	realmente de radio o avisos */

	if(contieneMensajePalabras(ultimoMensaje, PC_PROHIBIDAS_GENERAL))
		ultimoMensaje[0] = '\0'; // Vacíamos el mensaje

	if(contieneMensajePalabras(penultimoMensaje, PC_PROHIBIDAS_GENERAL))
		penultimoMensaje[0] = '\0';

	// Una vez que las líneas estén listas para su análisis, las unimos
	strcpy_s(mensajeCompleto, (MAX_CAR_MENSAJE_CHATLOG)* 2, penultimoMensaje);
	strcat_s(mensajeCompleto, (MAX_CAR_MENSAJE_CHATLOG)* 2, ultimoMensaje);

	// Evitamos analizar una misma linea dos veces
	if(!strcmp(ultimoMensajeAnterior, ultimoMensaje))
		return;

	if(!strcmp(penultimoMensaje, ultimoMensajeAnterior))
		strcpy_s(mensajeCompleto, (MAX_CAR_MENSAJE_CHATLOG)* 2, ultimoMensaje);

	/* Aquí comenzamos a analizar todos los mensajes que vayan llegando.
	De ser el mensaje reconocido por la aplicación, notificará al usuario
	según su configuración.

	Primero analizaremos todos los mensajes de radio, es decir, aquellos que
	empiecen con "**[" */

	if (strstr(mensajeCompleto, "**[id: ") != NULL || strstr(mensajeCompleto, "**[rj ") != NULL || strstr(mensajeCompleto, ", c: ") != NULL || strstr(mensajeCompleto, "[radio id: ") != NULL || strstr(mensajeCompleto, "| ch: ") != NULL)
	{
		// Avisos de reunión general (10-80)
		if(radioInteligente.obtenerValorAviso(A_REUNION_GENERAL))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_REUNION_GENERAL))
			{
				reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
				return;
			}
		}
		// Avisos de código seis adam
		if(radioInteligente.obtenerValorAviso(A_SEIS_ADAM))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_SEIS_ADAM))
			{
				reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
				return;
			}
		}
		// Avisos de código cinco
		if(radioInteligente.obtenerValorAviso(A_CODIGO_CINCO))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_CODIGO_CINCO))
			{
				reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
				return;
			}
		}
		// Avisos de código cuatro
		if (strstr(mensajeCompleto, "codigo cuatro") != NULL || strstr(mensajeCompleto, "codigo 4") != NULL)
		{
			Sleep(250);
			reproducirSonido(archivosDeSonido[S_CODIGO_CUATRO]);
			return;
		}
		// Avisos roleados de centralita
		if(radioInteligente.obtenerValorAviso(A_CENTRALITA))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_CENTRALITA))
			{
				reproducirSonido(archivosDeSonido[S_CENTRAL]);
				return;
			}
		}
		// Avisos de petición de reporte de unidades
		if(radioInteligente.obtenerValorAviso(A_REPORTES_UNIDADES))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_REP_UNIDADES) && !contieneMensajePalabras(mensajeCompleto, PC_REP_UNIDADES_PROHIB))
			{
				reproducirSonido(archivosDeSonido[S_REPORTE]);
				return;
			}
		}
		/* Avisos a nuestra unidad o nombre. Este aviso no usa palabras
		   clave porque su análisis es distinto. */

		if(radioInteligente.obtenerValorAviso(A_PROPIO))
		{
			// Si no somos nosotros mismos los que hemos escrito el mensaje de radio...

			// Volvemos a obtener la ID del jugador en caso de que hayamos relogeado con SAMP abierto (reinicio del server)
			radioInteligente.establecerIdJugador(obtenerIdJugador(&procesoGTA, &baseSAMP));

			char buffer[32], _buffer[32];
			sprintf_s(buffer, 32, "id: %d, c:", radioInteligente.obtenerIdJugador());
			sprintf_s(_buffer, 32, "id: %d]", radioInteligente.obtenerIdJugador());

			if (strstr(mensajeCompleto, buffer) == NULL && strstr(mensajeCompleto, _buffer) == NULL)
			{
				char nombre[MAX_CAR_NOMBRE_JUGADOR];
				char apellido[MAX_CAR_NOMBRE_JUGADOR];
				sscanf_s(radioInteligente.obtenerNombreJugador(), "%[^_]_%s", nombre, MAX_CAR_NOMBRE_JUGADOR, apellido, MAX_CAR_NOMBRE_JUGADOR);

				bool sono = false;
				// Nos están llamando por el nombre o el apellido...
				if (strstr(mensajeCompleto, nombre) != NULL || strstr(mensajeCompleto, apellido) != NULL)
				{
					reproducirSonido(archivosDeSonido[S_BEEP]);
					sono = true;
				}
				// ... o a nuestra unidad...
				for (int i = 0; i < MAX_IND_UND; i++)
				{
					if (radioInteligente.obtenerManeraIndicativo(i) == NULL)
						continue;

					if (strstr(mensajeCompleto, radioInteligente.obtenerManeraIndicativo(i)) != NULL)
					{
						/* Calculamos la posición del indicativo en el mensaje y comprobamos
							que delante del número de la unidad no haya otro número. Si lo hay,
							será porque están llamando a otra unidad, y por lo tanto no debemos
							notificar al usuario. Si no lo hay, será porque nos están llamando
							a nosotros. */

						int posicion = (strstr(mensajeCompleto, radioInteligente.obtenerManeraIndicativo(i)) - mensajeCompleto) +
							strlen(radioInteligente.obtenerManeraIndicativo(i));
						// ... si era a nosotros y no notificamos antes al usuario...
						if ((mensajeCompleto[posicion] > '9' || mensajeCompleto[posicion] < '0') && !sono)
						{
							reproducirSonido(archivosDeSonido[S_BEEP]);
							break;
						}
					}
				}
			}
		}
		// Pedidos SWAT
		if(radioInteligente.obtenerValorAviso(A_PEDIDOS_SWAT))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_PEDIDOS_SWAT))
			{
				/* Notificamos a las posibles unidades metro activas. */

				if(wcsstr(radioInteligente.obtenerNombreIndicativo(), L"DAVID") != NULL ||
					wcsstr(radioInteligente.obtenerNombreIndicativo(), L"LINCOLN") != NULL ||
					wcsstr(radioInteligente.obtenerNombreIndicativo(), L"ADAM") != NULL ||
					wcsstr(radioInteligente.obtenerNombreIndicativo(), L"ROBERT") != NULL ||
					wcsstr(radioInteligente.obtenerNombreIndicativo(), L"K9") != NULL)
				{
					reproducirSonido(archivosDeSonido[S_CODIGO_TAC]);
					return;
				}
			}
		}
		// Pedidos generales (10-32, 10-37, preguntas...)
		if(radioInteligente.obtenerValorAviso(A_PEDIDOS))
		{
			if(contieneMensajePalabras(mensajeCompleto, PC_PEDIDOS_GENERAL))
			{
				if(strstr(mensajeCompleto, radioInteligente.obtenerNombreJugador()) == NULL)
				{
					if(!contieneMensajePalabras(mensajeCompleto, PC_PEDIDOS_PROHIB))
					{
						if(contieneMensajePalabras(mensajeCompleto, PC_PEDIDO_URGENTE))
						{
							reproducirSonido(archivosDeSonido[S_CODIGO_TRES]);
							return;
						}
						if(contieneMensajePalabras(mensajeCompleto, PC_PEDIDO_NOURGENTE))
						{
							reproducirSonido(archivosDeSonido[S_CODIGO_DOS]);
							return;
						}
						reproducirSonido(archivosDeSonido[S_SOLICITUD]);
						return;
					}
				}
				else
				{
					reproducirSonido(archivosDeSonido[S_SOLICITUD]);
					return;
				}
			}
		}
		/* Estos avisos ya no usan arreglos 2D de palabras clave porque utilizan
		muy pocas palabras clave, algunos incluso solamente una. De ser necesario
		en un futuro, se podrán crear los arreglos para las palabras clave y
		utilizar las funciones correspondientes para analizar los mensajes con
		dichos arreglos. */

		// Avisos de sospechoso/a(s) bajo custodia
		if(radioInteligente.obtenerValorAviso(A_CUSTODIA))
		{
			srand((unsigned int)time(NULL));
			int sonidoAleatorio = rand() % (S_CUSTODIA_VARIOS_DOS - S_CUSTODIA_VARIOS_UNO) + S_CUSTODIA_VARIOS_UNO;
			if(strstr(mensajeCompleto, "sospechosos") != NULL || strstr(mensajeCompleto, "sospechosas") != NULL)
			{
				if(strstr(mensajeCompleto, "?") != NULL || strstr(mensajeCompleto, "¿") != NULL)
				{
					reproducirSonido(archivosDeSonido[S_SOLICITUD]);
					return;
				}
				if(strstr(mensajeCompleto, "custodia") != NULL || strstr(mensajeCompleto, "bajo custodia") != NULL || strstr(mensajeCompleto, "en custodia") != NULL)
				{
					Sleep(250);
					reproducirSonido(archivosDeSonido[sonidoAleatorio]);
					return;
				}
			}
			if(strstr(mensajeCompleto, "sospechoso") != NULL || strstr(mensajeCompleto, "sospechosa") != NULL)
			{
				if(strstr(mensajeCompleto, "?") != NULL || strstr(mensajeCompleto, "¿") != NULL)
				{
					reproducirSonido(archivosDeSonido[S_SOLICITUD]);
					return;
				}
				if(strstr(mensajeCompleto, "custodia") != NULL || strstr(mensajeCompleto, "bajo custodia") != NULL || strstr(mensajeCompleto, "en custodia") != NULL)
				{
					Sleep(250);
					reproducirSonido(archivosDeSonido[S_CUSTODIA]);
					return;
				}
			}
		}
		// Avisos de agentes sin asignación
		if(radioInteligente.obtenerValorAviso(A_AGENTES_SIN_ASIG))
		{
			if(strstr(mensajeCompleto, "sin asignacion") != NULL)
			{
				reproducirSonido(archivosDeSonido[S_SOLICITUD]);
				return;
			}
		}
	}
	/* Los siguientes avisos ya no son de radio, por lo tanto,
	si algún mensaje de radio contiene estos mensajes, no
	los analizamos porque no son avisos reales. */

	// Avisos de botón de pánico
	if(radioInteligente.obtenerValorAviso(A_BOTON_PANICO))
	{
		srand((unsigned int)time(NULL));
		int sonidoAleatorio = rand() % (S_BPANICO_CUATRO - S_BPANICO_UNO) + S_BPANICO_UNO;
		if(strstr(mensajeCompleto, "[central][tuls]") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
			return;
		}
		if(strstr(mensajeCompleto, "[central][rtpls]") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
			return;
		}
		if(strstr(mensajeCompleto, "[central][lsfd]") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
			return;
		}
		if(strstr(mensajeCompleto, "[central][estado]") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
			return;
		}
		if(strstr(mensajeCompleto, "ha pulsado el boton de panico") != NULL)
		{
			reproducirSonido(archivosDeSonido[sonidoAleatorio]);
			return;
		}
	}
	// Avisos de apoyo (/ref)
	if(radioInteligente.obtenerValorAviso(A_APOYO_REF))
	{
		if(strstr(mensajeCompleto, "solicita apoyo, esta en el barrio de") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_CODIGO_TRES]);
			return;
		}
	}

	// Avisos de apoyo conjunto (/marcar 6)
	if (radioInteligente.obtenerValorAviso(A_APOYO_REF))
	{
		if (strstr(mensajeCompleto, "solicita apoyo conjunto, esta en el barrio de") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_CODIGO_TRES]);
			return;
		}
	}

	// Avisos de herido
	if(radioInteligente.obtenerValorAviso(A_HERIDO))
	{
		srand((unsigned int)time(NULL));
		int sonidoAleatorio = rand() % (S_HERIDO_CUATRO - S_HERIDO_UNO) + S_HERIDO_UNO;
		if(strstr(mensajeCompleto, "d] aviso de herido en") != NULL)
		{
			reproducirSonido(archivosDeSonido[sonidoAleatorio]);
			return;
		}
	}
	// Avisos de robo de coche
	if(radioInteligente.obtenerValorAviso(A_ROBO_COCHE))
	{
		srand((unsigned int)time(NULL));
		int sonidoAleatorio = rand() % (S_ROBADO_CUATRO - S_ROBADO_UNO) + S_ROBADO_UNO;
		if(strstr(mensajeCompleto, "esta siendo robado en la zona de") != NULL)
		{
			reproducirSonido(archivosDeSonido[sonidoAleatorio]);
			return;
		}
	}
	// Avisos de robo a una vivienda
	if(radioInteligente.obtenerValorAviso(A_ROBO_CASA))
	{
		if(strstr(mensajeCompleto, "[alarma vivienda]:") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_ROBO_CASA]);
			return;
		}
	}
	// Avisos de asalto a mano armada
	if(radioInteligente.obtenerValorAviso(A_ROBO_NEG))
	{
		srand((unsigned int)time(NULL));
		int sonidoAleatorio = rand() % (S_ROBO_CUATRO - S_ROBO_UNO) + S_ROBO_UNO;
		if(strstr(mensajeCompleto, "[centralita] ¡a todas las unidades! ¡alarma de asalto armado") != NULL)
		{
			reproducirSonido(archivosDeSonido[sonidoAleatorio]);
			return;
		}
	}
	// Avisos de alarma de prisión federal
	if(radioInteligente.obtenerValorAviso(A_PRISION))
	{
		if(strstr(mensajeCompleto, "[centralita] activada alarma de la prision federal") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_IMPORTANTE]);
			return;
		}
	}
	// Avisos de robo al banco
	if(radioInteligente.obtenerValorAviso(A_ROBO_BANCO))
	{
		if(strstr(mensajeCompleto, "[centralita]") != NULL && strstr(mensajeCompleto, "banco de temple") != NULL)
		{
			reproducirSonido(archivosDeSonido[S_ROBO_BANCO]);
			return;
		}
	}
}

/* Esta función comprobará que la clave del registro 
   para que el programa arranque con Windows existe o no.
   
   Argumento 1 (wchar_t*): Nombre del programa (nombre de la clave)
   
   Devolverá verdadero si existe.
   Devolverá falso si no existe. */

BOOL estaProgramaRegistradoIniciarWindows() {

	HKEY claveReg = NULL;
	LONG resultadoOperacion = 0;
	BOOL resultadoFinal = TRUE;
	DWORD tipoRegistro = REG_SZ;
	wchar_t rutaAlExe[MAX_PATH] = {};
	DWORD tamReg = sizeof(rutaAlExe);

	resultadoOperacion = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &claveReg);

	resultadoFinal = (resultadoOperacion == 0);

	if(resultadoFinal)
	{
		resultadoOperacion = RegGetValueW(claveReg, NULL, nombreClaseApl, RRF_RT_REG_SZ, &tipoRegistro, rutaAlExe, &tamReg);
		resultadoFinal = (resultadoOperacion == 0);
	}

	if(resultadoFinal)
	{
		resultadoFinal = (wcslen(rutaAlExe) > 0) ? TRUE : FALSE;
	}

	if(claveReg != NULL)
	{
		RegCloseKey(claveReg);
		claveReg = NULL;
	}

	return resultadoFinal;
}

/* Esta función se encarga de agregar la entrada del registro
   correspondiente para que el programa arranque con Windows.
   
   Devolverá verdadero si se pudo crear la entrada.
   Devolverá falso si no se pudo crear. */

BOOL agregarProgramaInicioWindows() {

	HKEY claveReg = NULL;
	LONG resultadoOperacion = 0;
	BOOL resultadoFinal = TRUE;
	size_t tamReg;
	wchar_t rutaAlExe[MAX_PATH];
	GetModuleFileNameW(NULL, rutaAlExe, MAX_PATH);

	resultadoOperacion = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &claveReg, NULL);

	resultadoFinal = (resultadoOperacion == 0);

	if(resultadoFinal)
	{
		tamReg = (wcslen(rutaAlExe) + 1) * 2;
		resultadoOperacion = RegSetValueExW(claveReg, nombreClaseApl, 0, REG_SZ, (BYTE*)rutaAlExe, tamReg);
		resultadoFinal = (resultadoOperacion == 0);
	}

	if(claveReg != NULL)
	{
		RegCloseKey(claveReg);
		claveReg = NULL;
	}

	return resultadoFinal;
}

/* Esta función se encarga de eliminar la entrada del registro
   correspondiente para que el programa arranque con Windows.

   Devolverá verdadero si se pudo borrar la entrada.
   Devolverá falso si no se pudo borrar. */

BOOL eliminarProgramaInicioWindows() {

	HKEY claveReg = NULL;
	LONG resultadoOperacion = 0;
	BOOL resultadoFinal = TRUE;
	DWORD tipoRegistro = REG_SZ;

	resultadoOperacion = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE | KEY_READ, &claveReg);

	resultadoFinal = (resultadoOperacion == 0);

	if(resultadoFinal)
	{
		resultadoOperacion = RegDeleteValue(claveReg, nombreClaseApl);
		resultadoFinal = (resultadoOperacion == 0);
	}

	if(claveReg != NULL)
	{
		RegCloseKey(claveReg);
		claveReg = NULL;
	}

	return resultadoFinal;
}

/* Esta función fue obtenida de MSDN y servirá para
   obtener la versión de una DLL del ordenador del
   usuario. 
   
   Devolverá un DWORD con la versión de la DLL */

DWORD GetVersion(LPCTSTR lpszDllName) {

	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	/* For security purposes, LoadLibrary should be provided with a fully qualified
	path to the DLL. The lpszDllName variable should be tested to ensure that it
	is a fully qualified path before it is used. */

	hinstDll = LoadLibrary(lpszDllName);

	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

		/* Because some DLLs might not implement this function, you must test for
		it explicitly. Depending on the particular DLL, the lack of a DllGetVersion
		function can be a useful indicator of the version. */

		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}
		FreeLibrary(hinstDll);
	}
	return dwVersion;
}

/* Esta función cambiará el icono de la aplicación 
   al especificado en el argumento.
   
   Argumento 1 (HICON): Icono. */

void cambiarIcono(HICON icono) {

	aplBandeja.hIcon = icono;
	Shell_NotifyIcon(NIM_MODIFY, &aplBandeja);
	return;
}

/* Esta función comprobará si el sistema del usuario
   es de 64 bits. 
   
   Devolverá verdadero si el sistema es de 64 bits.
   Devolverá falso si el sistema no es de 64 bits. */

bool esSistema64bits() {

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	if((si.wProcessorArchitecture & PROCESSOR_ARCHITECTURE_IA64) || (si.wProcessorArchitecture & PROCESSOR_ARCHITECTURE_AMD64) == 64)
		return true;

	return false;
}

/* Esta función comprobará si existe alguna versión
   más reciente del programa.
   
   Argumento 1 (bool): Especificar si estamos
   comprobando las actualizaciones de manera automática.

   Si el argumento es 1, se hará de manera automática.
   Si el argumento es 2, se hará de manera silenciosa.
   Si el argumento es 0, será porque el usuario ha
   pulsado el botón de buscar actualizaciones. */

void comprobarActualizaciones(int modo) {

	WSADATA datosWsa;
	if(WSAStartup(MAKEWORD(2, 2), &datosWsa) != NO_ERROR)
	{
		if (modo == MODO_ACTUAL_MAN)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			MessageBox(hwndApl, L"No se pudo contactar con el servidor para obtener la información sobre"
				L" la versión. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
		}
		return;
	}

	char bufferLectura[10000];
	char *separadorHtml;
	char *separadorSiguiente;
	char analisisHtml[128][256];
	int numeroBytesTotales;
	struct hostent *host;

	if (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == INVALID_SOCKET)
	{
		if (modo == MODO_ACTUAL_MAN)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			MessageBox(hwndApl, L"No se pudo contactar con el servidor para obtener la información sobre"
				L" la versión. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
		}
		WSACleanup();
		return;
	}

	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	host = gethostbyname(WEB_VERSION);

	if (host == NULL)
	{
		if (modo == MODO_ACTUAL_MAN)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			MessageBox(hwndApl, L"No se pudo contactar con el servidor para obtener la información sobre"
				L" la versión. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
		}
		closesocket(Socket);
		WSACleanup();
		return;
	}

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(80);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	if(connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) == SOCKET_ERROR)
	{
		if(modo == MODO_ACTUAL_MAN)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			MessageBox(hwndApl, L"No se pudo contactar con el servidor para obtener la información sobre"
				L" la versión. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
		}
		closesocket(Socket);
		WSACleanup();
		return;
	}

	if (send(Socket, "GET /version HTTP/1.1\r\nHost: " WEB_VERSION "\r\nConnection: close\r\n\r\n",
		(size_t)strlen("GET /version HTTP/1.1\r\nHost: " WEB_VERSION "\r\nConnection: close\r\n\r\n"), 0) == SOCKET_ERROR)
	{
		if (modo == MODO_ACTUAL_MAN)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			MessageBox(hwndApl, L"No se pudo contactar con el servidor para obtener la información sobre"
				L" la versión. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
		}
		closesocket(Socket);
		WSACleanup();
		return;
	}

	while ((numeroBytesTotales = recv(Socket, bufferLectura, 10000, 0)) > 0)
	{
		if (numeroBytesTotales == SOCKET_ERROR)
		{
			if (modo == MODO_ACTUAL_MAN)
			{
				MessageBeep(MB_ICONEXCLAMATION);
				MessageBox(hwndApl, L"No se pudo contactar con el servidor para obtener la información sobre"
					L" la versión. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
			}
			closesocket(Socket);
			WSACleanup();
			return;
		}
	}

	closesocket(Socket);
	WSACleanup();
	
	/* Analizamos la respuesta HTTP del servidor para obtener
	   el número de la versión. */

	separadorHtml = strtok_s(bufferLectura, "\r\n", &separadorSiguiente);
	int i = 0;
	while(separadorHtml != NULL)
	{
		strcpy_s(analisisHtml[i++], 256, separadorHtml);
		separadorHtml = strtok_s(NULL, "\r\n", &separadorSiguiente);
	}

	/* Buscamos y comparamos la versión actual con la del servidor, y si
	   es distinta, notificamos al usuario. */

	for (int j = 0; j < i; j++) // Recorremos el array con el resultado del servidor
	{
		if (strstr(analisisHtml[j], "rintel.exe") != NULL)  // Si encontramos una referencia al programa, el resultado fue bueno
		{
			if (strstr(analisisHtml[j], RINTEL_VERSION) == NULL) // Si hay una actualización nueva...
			{
				switch (modo)
				{
					case MODO_ACTUAL_SILEN:
						actualizacionDisponible = true;
						cambiarIcono(iconoNotificacion);
						break;
					case MODO_ACTUAL_AUTO:
						cambiarIcono(iconoNotificacion);
						mostrarNotificacion(_T("Actualización de Radio Inteligente"),
							_T("Hay una nueva actualización disponible para Radio Inteligente.\n")
							_T("Puede pulsar en este mismo mensaje para dirigirse a la página web de")
							_T(" de descarga, o puede descargar la actualización más tarde desde el menú")
							_T(" del programa."));
						actualizacionDisponible = true;
						break;
					case MODO_ACTUAL_MAN:
						if (actualizacionDisponible)
						{
							if (esSistema64bits())
								ShellExecute(NULL, _T("open"), _T(WEB_DESCARGA_64BITS), NULL, NULL, SW_SHOWNORMAL);
							else
								ShellExecute(NULL, _T("open"), _T(WEB_DESCARGA_32BITS), NULL, NULL, SW_SHOWNORMAL);
							actualizacionDisponible = false;
							cambiarIcono(iconoPrincipal);
							return;
						}
						SetForegroundWindow(hwndApl);
						MessageBeep(MB_ICONINFORMATION);
						if ((MessageBox(hwndApl, _T("Hay una nueva actualización disponible para Radio Inteligente.\n")
							_T("Pulse Aceptar para proceder a descagar la nueva actualización.\n") 
							_T("Pulse Cancelar si no quiere actualizar todavía."),
							_T("Radio Inteligente - Actualización"), MB_OKCANCEL | MB_ICONINFORMATION)) == IDCANCEL)
						{
							return;
						}
						else
						{
							actualizacionDisponible = false;
							cambiarIcono(iconoPrincipal);
							if (esSistema64bits())
								ShellExecute(NULL, _T("open"), _T(WEB_DESCARGA_64BITS), NULL, NULL, SW_SHOWNORMAL);
							else
								ShellExecute(NULL, _T("open"), _T(WEB_DESCARGA_32BITS), NULL, NULL, SW_SHOWNORMAL);
						}
						break;
					default:
						break;
				}
				break;
			}
			else
			{
				switch (modo)
				{
					case MODO_ACTUAL_MAN:
						SetForegroundWindow(hwndApl);
						MessageBeep(MB_ICONINFORMATION);
						MessageBox(hwndApl, _T("No hay ninguna actualización más reciente disponible.\n"),
							_T("Radio Inteligente - Actualización"), MB_OK | MB_ICONINFORMATION);
						break;
					default:
						break;
				}
				actualizacionDisponible = false;
				cambiarIcono(iconoPrincipal);
				break;
			}
		}

		// Si llegamos aquí es porque no se pudo obtener la información de la versión desde el servidor

		if ((strstr(analisisHtml[j], "rintel.exe") == NULL) && j == i - 1)  // Si no encontramos una referencia al programa y ya terminamos de analizar la respuesta...
		{
			if (modo == MODO_ACTUAL_MAN)
			{
				MessageBeep(MB_ICONEXCLAMATION);
				MessageBox(hwndApl, L"No se pudo obtener la información sobre la versión desde el"
					L" servidor. Inténtelo de nuevo más tarde.", L"Radio Inteligente - Error", MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}

	return;
}

/* Esta función mostrará un mensaje informativo en el icono de
   la bandeja del sistema.
   
   Argumento 1 (wchar_t*): Título del mensaje.
   Argumento 2 (wchat_t*): Contenido del mensaje. */

void mostrarNotificacion(wchar_t *titulo, wchar_t *mensaje) {

	Shell_NotifyIcon(NIM_ADD, &aplBandeja);
	aplBandeja.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	aplBandeja.dwInfoFlags = NIIF_INFO;
	aplBandeja.uTimeout = 15000;
	_tcscpy_s(aplBandeja.szInfoTitle, titulo);
	_tcscpy_s(aplBandeja.szInfo, mensaje);
	Shell_NotifyIcon(NIM_MODIFY, &aplBandeja); // La mostramos
	reproducirSonido(archivosDeSonido[S_INICIADO]); // Sonido
	// Devolvemos todo a su valor por defecto y eliminamos la notificación
	_tcscpy_s(aplBandeja.szInfoTitle, _T(""));
	_tcscpy_s(aplBandeja.szInfo, _T(""));
	aplBandeja.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	aplBandeja.dwInfoFlags = 0;
	Shell_NotifyIcon(NIM_MODIFY, &aplBandeja);

}

/* Esta función se encargará de cerrar el handle del
   proceso del GTA cuando cerremos la aplicación */

void liberarProcesosMemoria() {

    if(procesoGTA != NULL || baseSAMP != (DWORD)DIR_NULA)
    {
        CloseHandle(procesoGTA);
        baseSAMP = DIR_NULA;
        procesoGTA = NULL;
    }
	return;
}

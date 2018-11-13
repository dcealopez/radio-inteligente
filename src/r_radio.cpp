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
// Implementación de las funciones de la clase rIntel.
//

#include <tchar.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "r_radio.h"
#include "r_main.h"

// Lista de IPs de los servidores permitidos

const char *servidoresPermitidos[] = {

	"server.ls-rp.es",
	"server1.ls-rp.es",
	"server2.ls-rp.es",
	"server3.ls-rp.es",
	"practicas.ls-rp.es",

	NULL
};

// Lista de los archivos de sonido, enlazados con el enumerador

const wchar_t *archivosDeSonido[] = {

    L"desactivado.wav", // S_DESACTIVADO
    L"activado.wav", // S_ACTIVADO
    L"iniciado.wav", // S_INICIADO
    L"beep.wav", // S_BEEP
    L"solicitud.wav", // S_SOLICITUD
    L"importante.wav", // S_IMPORTANTE
    L"reporte.wav", // S_REPORTE
    L"bpanico1.wav", // S_BPANICO_UNO
    L"bpanico2.wav", // S_BPANICO_DOS
    L"bpanico3.wav", // S_BPANICO_TRES
    L"bpanico4.wav", // S_BPANICO_CUATRO
    L"central.wav", // S_CENTRAL
    L"code2.wav", // S_CODIGO_DOS
    L"code3.wav", // S_CODIGO_TRES
    L"codigo_tac.wav", // S_CODIGO_TAC
    L"custody.wav", // S_CUSTODIA
    L"custodia_varios1.wav", // S_CUSTODIA_VARIOS_UNO
    L"custodia_varios2.wav", // S_CUSTODIA_VARIOS_DOS
    L"herido1.wav", // S_HERIDO_UNO
    L"herido2.wav", // S_HERIDO_DOS
    L"herido3.wav", // S_HERIDO_TRES
    L"herido4.wav", // S_HERIDO_CUATRO
    L"robado1.wav", // S_ROBADO_UNO
    L"robado2.wav", // S_ROBADO_DOS
    L"robado3.wav", // S_ROBADO_TRES
    L"robado4.wav", // S_ROBADO_CUATRO
    L"robo_banco.wav", // S_ROBO_BANCO
    L"robo_casa.wav", // S_ROBO_CASA
    L"robo1.wav", // S_ROBO_UNO
    L"robo2.wav", // S_ROBO_DOS
    L"robo3.wav", // S_ROBO_TRES
    L"robo4.wav" // S_ROBO_CUATRO
};

// Entradas de configuración para los avisos, enlazados con el enumerador

const char *avisosConfiguracion[] = {

    "a_reunion_general", // A_REUNION_GENERAL
    "a_seis_adam", // A_SEIS_ADAM
    "a_codigo_cinco", // A_CODIGO_CINCO
    "a_centralita", // A_CENTRALITA
    "a_reportes_unidades", // A_REPORTES_UNIDADES
    "a_propio", // A_PROPIO
    "a_pedidos", // A_PEDIDOS
    "a_pedidos_swat", // A_PEDIDOS_SWAT
    "a_custodia", // A_CUSTODIA
    "a_agentes_sin_asig", // A_AGENTES_SIN_ASIG
    "a_boton_panico", // A_BOTON_PANICO
    "a_apoyo_ref", // A_APOYO_REF
    "a_herido", // A_HERIDO
    "a_robo_coche", // A_ROBO_COCHE
    "a_robo_casa", // A_ROBO_CASA
    "a_robo_neg", // A_ROBO_NEG
	"a_prision", // A_PRISION
	"a_robo_banco", // A_ROBO_BANCO
};

/* Todas estas declaraciones de arreglos multidimensionales
   contendrán todas las palabras clave para el analizador
   de mensajes. También las palabras prohibidas */

const char *PC_PROHIBIDAS_GENERAL[] = {

	"mens. privado", "susurra:", "grita:",
	") te ha susurrado al oido:", "whatsapp",
	"sms", "por telefono", "administracion",
	"(( [", "[anuncio]", "[telefono",
	"[megafono", "[[extraño",
	"privado enviado a", "[altavoz tfno. id:", "dice:",
	"[info]", "por telefono ajeno",

	NULL
};

const char *PC_REUNION_GENERAL[] = {

	"1080 general en", "'80 general en", "80' general en",
	"sala de asignaciones", "10-80 general en", "80 general en",

	NULL
};

const char *PC_SEIS_ADAM[] = {

	"codigo 6adam", "6adam", "6-adam",
	"codigo 6-adam", "6 adam", "codigo 6 adam",
	"codigo seis adam",

	NULL
};

const char *PC_CODIGO_CINCO[] = {

	"codigo 5", "codigo cinco", "codigo5",

	NULL
};

const char *PC_CENTRALITA[] = {

	"[central]", "[centralita]", "[despacho]",
	"[dispatch]", "central:", "-central-",
	"-despacho-", "despacho:", "-centralita-",
	"-dispatch-", "[dispatch]",

	NULL
};

const char *PC_REP_UNIDADES[] = {

	"reporten", "asignaciones", "componen",
	"miembros", "reportense", "reportar",
	"unidades activas", "actuales",

	NULL
};

const char *PC_REP_UNIDADES_PROHIB[] = {

	"actualmente", "actualidad", "actualizado",
	"actualizados", "actualicen", "actualizada",
	"actualizadas", "actualizar", "reportarse",

	NULL
};

const char *PC_PEDIDOS_SWAT[] = {

	"codigo 77", "codigo seteta y siete", "se requiere metro", "solicita metro",
	"solicitamos metro", "solicitando metro", "necesito metro",
	"necesitamos metro", "necesita metro", "solicito metro",
	"requiere metro", "requerimos metro", "requiriendo metro",
	"requiero metro", "unidades metro", "unidades david",

	NULL
};

const char *PC_PEDIDOS_GENERAL[] = {

	"'32", "10-32", "32'", "1032", "'37", "10-37", "37'",
	"1037", "solicitando", "solicitamos", "solicito",
	"necesito", "necesitamos", "requiere", "requerimos",
	"requiriendo", "requiero", "solicita", "necesita",
	"¿", "?",

	NULL
};

const char *PC_PEDIDOS_PROHIB[] = {

	"acude", "'26", "10-26", "26'", "1026", "responde",
	"respondemos", "respondiendo", "acudiendo",
	"en camino", "en ruta", "acudimos", "eta",

	NULL
};

const char *PC_PEDIDO_URGENTE[] = {

	"codigo 3", "codigo tres", "codigo3", "urgente",

	NULL
};

const char *PC_PEDIDO_NOURGENTE[] = {

	"codigo 2", "codigo dos", "codigo2", "no urgente",

	NULL
};

/* Esta función comprobará si en un mensaje existe
   otra cadena de un arreglo multidimensional. Esta
   función la usaremos para analizar los mensajes
   que lleguen. Los arreglos multidimensionales
   tendrán todas las palabras clave y son las que
   hemos declarado arriba.
   
   Argumento 1 (const char*): Mensaje a comprobar.
   Argumento 2 (const char*): Arreglo 2D con las palabras clave. 
   
   Devolverá verdadero si se encontraron coincidencias.
   Devolverá falso si no se encontraron coincidencias. */

bool contieneMensajePalabras(const char *mensaje, const char *palabras[]) {

	for(int i = 0; palabras[i] != NULL; i++)
	{
		if(strstr(mensaje, palabras[i]) != NULL)
			return true;
	}
	return false;
}

/* Esta función eliminará todos los acentos
   de la cadena especificada en el argumento.
   
   Argumento 1 (char*): Cadena a la que se
   le eliminarán los acentos. */

void eliminarAcentosCadena(char *cadena) {

	for(unsigned int i = 0; i < strlen(cadena); i++)
	{
		switch (cadena[i])
		{
			case 'á':
				cadena[i] = 'a';
				break;
			case 'é':
				cadena[i] = 'e';
				break;
			case 'í':
				cadena[i] = 'i';
				break;
			case 'ó':
				cadena[i] = 'o';
				break;
			case 'ú':
				cadena[i] = 'u';
				break;
			case 'Á':
				cadena[i] = 'a';
				break;
			case 'É':
				cadena[i] = 'e';
				break;
			case 'Í':
				cadena[i] = 'i';
				break;
			case 'Ó':
				cadena[i] = 'o';
				break;
			case 'Ú':
				cadena[i] = 'u';
				break;
			case 'Ñ':
				cadena[i] = 'ñ';
				break;
			default:
				break;
		}
	}
	return;
}

/* Constructor de nuestra clase rIntel.
   Establecerá algunas variables a su estado por defecto. */

rIntel::rIntel() {

    for(int i = 0; i < MAX_TIPOS_AVISO; i++)
        this->configuracionAvisos[i] = 1; // Activamos todos los avisos por defecto

    this->establecerNombreJugador("N/A");
	this->idJugador = -1;
    this->establecerIpServidor("N/A");
	this->establecerNombreIndicativo(_T("N/A"));
	this->establecerTeclaMacro(MACRO_DESACT_MODIFICADORA, 0x02); // CTRL por defecto
	this->establecerTeclaMacro(MACRO_DESACT_TECLA, 0x50); // TECLA P por defecto
	this->establecerConfigActualizaciones(false); // Por defecto desactivadas
    this->establecerEstadoRadio(E_STANDBY);
}

/* Destructor de nuestra clase rIntel.
   No hará nada porque no hay memoria que liberar. */

rIntel::~rIntel() {

}

/* Esta función comprobará que los archivos de sonido existen.

   Devolverá el nombre del archivo faltante si es que falta alguno.
   Devolverá NULL si no falta ningún archivo. */

wchar_t* rIntel::comprobarSonidos() {

	wchar_t *salida;
	wchar_t dirProg[MAX_PATH];
	wchar_t letraDisco[3];
	wchar_t directorio[MAX_PATH];
	wchar_t rutaFinal[MAX_PATH];
    unsigned int i;

    for(i = 0; i < NUM_SONIDOS; i++)
    {
		GetModuleFileNameW(NULL, dirProg, MAX_PATH);
		_wsplitpath_s(dirProg, letraDisco, 3, directorio, MAX_PATH, NULL, NULL, NULL, NULL);
		swprintf(rutaFinal, MAX_PATH, L"%s%s", letraDisco, directorio);
		wcscat_s(rutaFinal, MAX_PATH, _T("sonidos\\"));
		wcscat_s(rutaFinal, MAX_PATH, archivosDeSonido[i]);
        if(_waccess(rutaFinal, 0) != -1)
            continue;

		salida = (wchar_t*)malloc((wcslen(rutaFinal) + 1) * sizeof(wchar_t));
		wcscpy_s(salida, (wcslen(rutaFinal) + 1) * sizeof(wchar_t), rutaFinal);
        return salida;
    }
    return NULL;
}

/* Esta función creará o guardará el archivo de configuración con
   los valores especificados en los argumentos. Si no se especifican
   valores para los argumentos (es decir, NULL), se creará el archivo
   pero no se guardará ningún valor para el registro del indicativo.

   Argumento 1 (int): cuadrante de la unidad.
   Argumento 2 (char *): nombre de la unidad.
   Argumento 3 (int): número de la unidad.

   Devolverá 0 si el archivo se creó satisfactoriamente.
   Devolverá 2 si no se pudo crear el archivo. */

int rIntel::crearConfiguracion(wchar_t *nombre, wchar_t *numOLetra) {

    FILE *archivoConfiguracion;
	wchar_t dirProg[MAX_PATH];
	wchar_t letraDisco[3];
	wchar_t directorio[MAX_PATH];
	wchar_t rutaFinal[MAX_PATH];

	GetModuleFileNameW(NULL, dirProg, MAX_PATH);
	_wsplitpath_s(dirProg, letraDisco, 3, directorio, MAX_PATH, NULL, NULL, NULL, NULL);
	swprintf_s(rutaFinal, MAX_PATH, L"%s%s", letraDisco, directorio);
	wcscat_s(rutaFinal, MAX_PATH, _T(ARCHIVO_CONF));

    _wfopen_s(&archivoConfiguracion, rutaFinal, L"w");
    if(archivoConfiguracion == NULL)
        return CFG_ERR_CREAR;

    fprintf(archivoConfiguracion, "indicativo=");
    if(nombre != NULL && numOLetra != NULL)
        fwprintf(archivoConfiguracion, _T("%s-%s"), nombre, numOLetra);

    fprintf(archivoConfiguracion, "\n");

    for(int i = 0; i < MAX_TIPOS_AVISO; i++)
        fprintf(archivoConfiguracion, "%s=%d\n", avisosConfiguracion[i], this->configuracionAvisos[i]);

	fprintf(archivoConfiguracion, "macro_desact_modif=%X\n", this->obtenerCodigoTeclaMacro(MACRO_DESACT_MODIFICADORA));
	fprintf(archivoConfiguracion, "macro_desact_tecla=%X\n", this->obtenerCodigoTeclaMacro(MACRO_DESACT_TECLA));

	fprintf(archivoConfiguracion, "avisos_actualizaciones=%d\n", this->obtenerConfigActualizaciones());

    fclose(archivoConfiguracion);
	this->cargarConfiguracion(); // Después de crearla, la cargamos
    return CFG_RES_OK;
}

/* Esta función cargará la configuración desde el archivo de
   configuración y generará además todas las maneras posibles
   de escribir el indicativo de la unidad.

   Devolverá -1 si el archivo de configuración está corrupto.
   Devolverá 0 si la configuración se cargó satisfactoriamente.
   Devolverá 1 si se creó una nueva configuración desde cero.
   Devolverá 2 si el archivo de configuración no se pudo crear.
   Devolverá 3 si el archivo de configuración no se pudo abrir.
   Devolverá 4 si hubo algún error en el análisis de algún valor o registro.
   Devolverá 5 si se encontró un registro desconocido.
   Devolverá 6 si el archivo de configuración está vacío. */

int rIntel::cargarConfiguracion() {

    FILE *archivoConfiguracion;
	wchar_t dirProg[MAX_PATH];
	wchar_t letraDisco[3];
	wchar_t directorio[MAX_PATH];
	wchar_t rutaFinal[MAX_PATH];
    char buffer[128];
    char registro[32];
    char valor[64];
    int cargado = 0;

	GetModuleFileNameW(NULL, dirProg, MAX_PATH);
	_wsplitpath_s(dirProg, letraDisco, 3, directorio, MAX_PATH, NULL, NULL, NULL, NULL);
	swprintf_s(rutaFinal, MAX_PATH, L"%s%s", letraDisco, directorio);
	wcscat_s(rutaFinal, MAX_PATH, _T(ARCHIVO_CONF));

    // Comprobamos que el archivo de configuración exista. Si no existe, lo creamos
	if(_waccess(rutaFinal, 0) == -1)
    {
        if(crearConfiguracion(NULL, NULL) == CFG_ERR_CREAR)
            return CFG_ERR_CREAR;

        return CFG_RES_NUEVO;
    }

    // Si existe la configuración, la intentaremos cargar
	_wfopen_s(&archivoConfiguracion, rutaFinal, L"r");
    if(archivoConfiguracion == NULL) // Si no se pudo leer, devolvemos un error
        return CFG_ERR_APERTURA;

    // Comprobamos que el archivo de configuración no esté vacío
    fseek(archivoConfiguracion, 0, SEEK_END);
    if(ftell(archivoConfiguracion) == 0)
	{
		fclose(archivoConfiguracion);
        return CFG_ERR_VACIO;
	}

    // Comenzamos a leer el archivo de configuración linea a linea
    fseek(archivoConfiguracion, 0, SEEK_SET); // Nos movemos al principio
    while(!ferror(archivoConfiguracion) && !feof(archivoConfiguracion))
    {
        fgets(buffer, sizeof(buffer), archivoConfiguracion);
        if(sscanf_s(buffer, "%[^=]=%s", registro, 32, valor, 64) != 2 || registro == NULL || valor == NULL)
		{
			fclose(archivoConfiguracion);
            return CFG_ERR_ANALISIS;
		}

        // Ahora que tenemos el registro y su valor, guardamos el valor dependiendo del registro
        if(!strcmp(registro, "indicativo"))
        {
            /* Ahora tendremos que generar todas las maneras posibles de escribir el indicativo
               Para ello, dividiremos el indicativo en tres partes: cuadrante, nombre y numero...
               ... y generaremos todas las maneras a partir de las tres partes */

            char nombre[32], numOLetra[3];
			wchar_t nombreUnicode[32];

            if(sscanf_s(valor, "%[^-]-%[^-]", nombre, 32, numOLetra, 3) != 2 || nombre == NULL || numOLetra == NULL || strlen(numOLetra) > 2)
			{
				fclose(archivoConfiguracion);
                return CFG_ERR_ANALISIS;
			}

			// Primero, pasamos todo el valor a mayúsculas
			for(unsigned int i = 0; i < strlen(valor); i++)
				valor[i] = toupper(valor[i]);

			eliminarAcentosCadena(valor); // Por si acaso

			/* Pasamos el valor del registro a Unicode para poder guardar
			   el nombre del indicativo en nuestra variable */

			size_t tamRequerido = 0;
			mbstowcs_s(&tamRequerido, NULL, 0, valor, 0);
			mbstowcs_s(&tamRequerido, nombreUnicode, tamRequerido + 1, valor, tamRequerido);
			wcscpy_s(this->nombreIndicativo, MAX_IND_TAM, nombreUnicode);

            /* Ya tenemos las tres partes, ahora generamremos todas las maneras y las guardamos
			   Primero, pasamos el nombre a minúsculas */

			for(unsigned int i = 0; i < strlen(nombre); i++)
				nombre[i] = tolower(nombre[i]);

			eliminarAcentosCadena(nombre);

			_snprintf_s(this->manerasIndicativoUnidad[0], MAX_IND_TAM, MAX_IND_TAM, "%s-%s", nombre, numOLetra);
			_snprintf_s(this->manerasIndicativoUnidad[1], MAX_IND_TAM, MAX_IND_TAM, "%c-%s", nombre[0], numOLetra);
			_snprintf_s(this->manerasIndicativoUnidad[2], MAX_IND_TAM, MAX_IND_TAM, "%c%s", nombre[0], numOLetra);
			_snprintf_s(this->manerasIndicativoUnidad[3], MAX_IND_TAM, MAX_IND_TAM, "%s%s", nombre, numOLetra);
            cargado++;
        }
        // Cargamos todas las entradas de los avisos
        for(int i = 0; i < MAX_TIPOS_AVISO; i++)
        {
            if(!strcmp(registro, avisosConfiguracion[i]))
            {
                int valorAviso;
                if(sscanf_s(valor, "%d", &valorAviso) != 1 || valorAviso < 0 || valorAviso > 1)
				{
					fclose(archivoConfiguracion);
                    return CFG_ERR_ANALISIS;
				}
                this->establecerValorAviso(i, valorAviso);
            }
        }
		// Cargamos la configuración de los macros
		if(!strcmp(registro, "macro_desact_modif"))
		{
			int codigo_tecla;
			if(sscanf_s(valor, "%X", &codigo_tecla) != 1 || codigo_tecla <= -1)
			{
				fclose(archivoConfiguracion);
				return CFG_ERR_ANALISIS;
			}
			this->establecerTeclaMacro(MACRO_DESACT_MODIFICADORA, codigo_tecla);
		}
		if(!strcmp(registro, "macro_desact_tecla"))
		{
			int codigo_tecla;
			if(sscanf_s(valor, "%X", &codigo_tecla) != 1 || codigo_tecla <= -1)
			{
				fclose(archivoConfiguracion);
				return CFG_ERR_ANALISIS;
			}
			this->establecerTeclaMacro(MACRO_DESACT_TECLA, codigo_tecla);
		}
		if(!strcmp(registro, "avisos_actualizaciones"))
		{
			int valorAvisos;
			if(sscanf_s(valor, "%d", &valorAvisos) != 1 || valorAvisos < 0 || valorAvisos > 1)
			{
				fclose(archivoConfiguracion);
				return CFG_ERR_ANALISIS;
			}
			this->establecerConfigActualizaciones(valorAvisos);
		}
    }

    if(cargado <= 0) // Si lo hemos abierto pero no cargamos ningún dato, algo va muy mal
	{
		fclose(archivoConfiguracion);
        return CFG_FATAL_ERR;
	}

	fclose(archivoConfiguracion);
    return CFG_RES_OK;
}

/* Esta serie de funciones sirven para obtener el valor
   de algunas de las variables privadas de nuestro
   objeto. */

char* rIntel::obtenerManeraIndicativo(unsigned const int manera) {

	if(manera < 0 || manera > MAX_IND_UND)
		return NULL;

	return this->manerasIndicativoUnidad[manera];
}

char* rIntel::obtenerNombreJugador() {

    return this->nombreJugador;
}

int rIntel::obtenerIdJugador() {

	return this->idJugador;
}

char* rIntel::obtenerIpServidor() {

    return this->ipServidor;
}

int rIntel::obtenerEstadoRadio() {

    return this->estadoRadio;
}

int rIntel::obtenerValorAviso(unsigned const int aviso) {

	return this->configuracionAvisos[aviso];
}

int rIntel::obtenerCodigoTeclaMacro(unsigned const int macro) {

	return this->configuracionMacros[macro];
}

int rIntel::obtenerCodigoRealMacro(unsigned const int macro) {

	/* En caso de que la tecla sea una modificadora (CTRL o ALT),
	   tendremos que convertir su valor del control HOTKEY
	   de la API de Windows a su equivalente valor virtual.
	
	   Por alguna razón cuando se combinan teclas extendidas, CTRL
	   se convierte en 0xA (11) y ALT en 0xC (13), así que hay que
	   tener esto en cuenta también. Hay que investigar esto. De
	   todas maneras, este método actual funciona como es de esperar. */

	if(this->configuracionMacros[macro] == 0x02 || this->configuracionMacros[macro] == 0xA) // CTRL
		return VK_CONTROL;

	if(this->configuracionMacros[macro] == 0x04 || this->configuracionMacros[macro] == 0xC) // ALT
		return VK_MENU;

	/* Para las teclas de la 'A' a la 'Z' y los números
	   del '0' al '9', el código que hay que devolver es
	   el código ASCII. Para todas las demás teclas hay que
	   devolver el código virtual de la tecla */

	int caracter = MapVirtualKey(this->configuracionMacros[macro], MAPVK_VK_TO_CHAR);
	if((caracter <= 'z' && caracter >= 'a') || (caracter <= 'Z' && caracter >= 'A') || (caracter <= '9' && caracter >= '0'))
		return caracter;

	return this->configuracionMacros[macro];
}

wchar_t* rIntel::obtenerNombreIndicativo() {

	return this->nombreIndicativo;
}

int rIntel::obtenerConfigActualizaciones() {

	return this->actualizacionesAutomaticas;
}

/* Esta función sirve para comprobar si un aviso está activado
   en la configuración de la radio.

   Argumento 1 (int): ID del aviso a comprobar.

   Devolverá verdadero si está activado.
   Devolverá falso si no está activado. */

bool rIntel::estaAvisoActivado(unsigned const int aviso) {

    if(this->configuracionAvisos[aviso] == 1)
        return true;

    return false;
}

/* Esta función sirve para comprobar si el servidor en el que
   estamos conectados es válido.

   Devolverá -1 si no se pudo comprobar.
   Devolverá 1 si es válido.
   Devolverá 0 si no es valido. */

int rIntel::esServidorValido() {

	for (int i = 0; servidoresPermitidos[i] != NULL; i++)
	{
		WSADATA datosWsa;
		if (WSAStartup(MAKEWORD(2, 2), &datosWsa) != NO_ERROR)
			return COMP_ERR_CONEX;

		struct hostent *host;
		struct in_addr addr;
		host = gethostbyname(servidoresPermitidos[i]);

		if (host == NULL)
			return COMP_ERR_CONEX;

		addr.s_addr = *(u_long *)host->h_addr;
		WSACleanup();

		if (!strcmp(inet_ntoa(addr), this->obtenerIpServidor()))
			return COMP_SERV_OK;
	}

    return COMP_SERV_NO;
}

/* Esta serie de funciones sirven para establecer el nombre
   de algunas de las variables privadas del objeto.

   El argumento de cada función es el valor a establecer
   a la variable correspondiente de la que se encarga cada
   función. */

void rIntel::establecerValorAviso(int aviso, int valor) {

    this->configuracionAvisos[aviso] = valor;
	return;
}

void rIntel::establecerNombreJugador(const char *nombre) {

    strcpy_s(this->nombreJugador, MAX_CAR_NOMBRE_JUGADOR, nombre);
	return;
}

void rIntel::establecerIdJugador(short int idJugador) {

	this->idJugador = idJugador;
	return;
}

void rIntel::establecerIpServidor(const char *ip) {

    strcpy_s(this->ipServidor, MAX_CAR_IP_SERVIDOR, ip);
	return;
}

void rIntel::establecerNombreIndicativo(const wchar_t *indicativo) {

	wcscpy_s(this->nombreIndicativo, MAX_IND_TAM, indicativo);
	return;
}

void rIntel::establecerEstadoRadio(unsigned const int estado) {

    this->estadoRadio = estado;
	return;
}

void rIntel::establecerTeclaMacro(unsigned const int macro, unsigned const int tecla) {

	this->configuracionMacros[macro] = tecla;
	return;
}

void rIntel::establecerConfigActualizaciones(int valor) {

	this->actualizacionesAutomaticas = valor;
}

/* Esta función se encarga de reestablecer las variables
   esenciales del objeto a sus valores por defecto para
   cuando se cambie de servidor.

   Hace lo mismo que el constructor del objeto. */

void rIntel::reEstablecer() {

    this->establecerNombreJugador("N/A");
	this->idJugador = -1;
    this->establecerIpServidor("N/A");
    this->establecerEstadoRadio(E_STANDBY);
	return;
}

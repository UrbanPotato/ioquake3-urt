/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifndef DEDICATED
#ifdef USE_LOCAL_HEADERS
#       include "SDL.h"
#       include "SDL_cpuinfo.h"
#else
#       include <SDL.h>
#       include <SDL_cpuinfo.h>
#endif
#endif

#include "sys_local.h"
#include "sys_loadlib.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

//@r00t: Need these for crash dump reporting
#include "../qcommon/vm_local.h"
#ifndef DEDICATED
#include "../client/client.h"
#endif
#ifdef USE_CURL
cvar_t *report_url;
#endif

static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

#include "../ioq3-urt/ioq3-urt.h"
/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
        Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void)
{
        return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path)
{
        Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void)
{
        if (*installPath)
                return installPath;
        else
                return Sys_Cwd();
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void)
{
        return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void )
{
        IN_Restart( );
}

/*
=================
Sys_ConsoleInput

Handle new console input
=================
*/
char *Sys_ConsoleInput(void)
{
        return CON_Input( );
}

#ifdef DEDICATED
#       define PID_FILENAME PRODUCT_NAME "_server.pid"
#else
#       define PID_FILENAME PRODUCT_NAME ".pid"
#endif

/*
=================
Sys_PIDFileName
=================
*/
static char *Sys_PIDFileName( void )
{
        return va( "%s/%s", Sys_TempPath( ), PID_FILENAME );
}

/*
=================
Sys_WritePIDFile

Return qtrue if there is an existing stale PID file
=================
*/
qboolean Sys_WritePIDFile( void )
{
        char      *pidFile = Sys_PIDFileName( );
        FILE      *f;
        qboolean  stale = qfalse;

        // First, check if the pid file is already there
        if( ( f = fopen( pidFile, "r" ) ) != NULL )
        {
                char  pidBuffer[ 64 ] = { 0 };
                int   pid;
                int   rbytes;
                rbytes = fread( pidBuffer, sizeof( char ), sizeof( pidBuffer ) - 1, f );
                fclose( f );

                pid = atoi( pidBuffer );
                if( !Sys_PIDIsRunning( pid ) )
                        stale = qtrue;
        }

        if( ( f = fopen( pidFile, "w" ) ) != NULL )
        {
                fprintf( f, "%d", Sys_PID( ) );
                fclose( f );
        }
        else
                Com_Printf( S_COLOR_YELLOW "Couldn't write %s.\n", pidFile );

        return stale;
}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
static void Sys_Exit( int exitCode )
{
        CON_Shutdown( );
#ifndef DEDICATED
        SDL_Quit( );
#endif

        if( exitCode < 2 )
        {
                // Normal exit
                remove( Sys_PIDFileName( ) );
        }

        fprintf(stderr,"*** FINAL: Sys_Exit(%d)\n",exitCode);

        exit( exitCode );
}

/*
=================
Sys_Quit
=================
*/
void Sys_Quit( void )
{
        Sys_Exit( 0 );
}

/*
=================
Sys_GetProcessorFeatures
=================
*/
cpuFeatures_t Sys_GetProcessorFeatures( void )
{
        cpuFeatures_t features = 0;

#ifndef DEDICATED
        if( SDL_HasRDTSC( ) )    features |= CF_RDTSC;
        if( SDL_HasMMX( ) )      features |= CF_MMX;
        if( SDL_HasMMXExt( ) )   features |= CF_MMX_EXT;
        if( SDL_Has3DNow( ) )    features |= CF_3DNOW;
        if( SDL_Has3DNowExt( ) ) features |= CF_3DNOW_EXT;
        if( SDL_HasSSE( ) )      features |= CF_SSE;
        if( SDL_HasSSE2( ) )     features |= CF_SSE2;
        if( SDL_HasAltiVec( ) )  features |= CF_ALTIVEC;
#endif

        return features;
}

/*
=================
Sys_Init
=================
*/
void Sys_Init(void)
{
        Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
        Cvar_Set( "arch", OS_STRING " " ARCH_STRING );
        Cvar_Set( "username", Sys_GetCurrentUser( ) );
}

/*
=================
Sys_AnsiColorPrint

Transform Q3 colour codes to ANSI escape sequences
=================
*/
void Sys_AnsiColorPrint( const char *msg )
{
        static char buffer[ MAXPRINTMSG ];
        int         length = 0;
        static int  q3ToAnsi[ 8 ] =
        {
                30, // COLOR_BLACK
                31, // COLOR_RED
                32, // COLOR_GREEN
                33, // COLOR_YELLOW
                34, // COLOR_BLUE
                36, // COLOR_CYAN
                35, // COLOR_MAGENTA
                0   // COLOR_WHITE
        };

        while( *msg )
        {
                if( Q_IsColorString( msg ) || *msg == '\n' )
                {
                        // First empty the buffer
                        if( length > 0 )
                        {
                                buffer[ length ] = '\0';
                                fputs( buffer, stderr );
                                length = 0;
                        }

                        if( *msg == '\n' )
                        {
                                // Issue a reset and then the newline
                                fputs( "\033[0m\n", stderr );
                                msg++;
                        }
                        else
                        {
                                // Print the color code
                                Com_sprintf( buffer, sizeof( buffer ), "\033[%dm",
                                                q3ToAnsi[ ColorIndex( *( msg + 1 ) ) ] );
                                fputs( buffer, stderr );
                                msg += 2;
                        }
                }
                else
                {
                        if( length >= MAXPRINTMSG - 1 )
                                break;

                        buffer[ length ] = *msg;
                        length++;
                        msg++;
                }
        }

        // Empty anything still left in the buffer
        if( length > 0 )
        {
                buffer[ length ] = '\0';
                fputs( buffer, stderr );
        }
}

/*
=================
Sys_Print
=================
*/
void Sys_Print( const char *msg )
{
        CON_LogWrite( msg );
        CON_Print( msg );
}

/*
=================
Sys_Error
=================
*/
void Sys_Error( const char *error, ... )
{
        va_list argptr;
        char    string[1024];

        va_start (argptr,error);
        Q_vsnprintf (string, sizeof(string), error, argptr);
        va_end (argptr);

        CL_Shutdown( string );
        Sys_ErrorDialog( string );

        Sys_Exit( 3 );
}

/*
=================
Sys_Warn
=================
*/
void Sys_Warn( char *warning, ... )
{
        va_list argptr;
        char    string[1024];

        va_start (argptr,warning);
        Q_vsnprintf (string, sizeof(string), warning, argptr);
        va_end (argptr);

        CON_Print( va( "Warning: %s", string ) );
}

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime( char *path )
{
        struct stat buf;

        if (stat (path,&buf) == -1)
                return -1;

        return buf.st_mtime;
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void *dllHandle )
{
        if( !dllHandle )
        {
                Com_Printf("Sys_UnloadDll(NULL)\n");
                return;
        }

        Sys_UnloadLibrary(dllHandle);
}

/*
=================
Sys_TryLibraryLoad
=================
*/
static void* Sys_TryLibraryLoad(const char* base, const char* gamedir, const char* fname, char* fqpath )
{
        void* libHandle;
        char* fn;

        *fqpath = 0;

        fn = FS_BuildOSPath( base, gamedir, fname );
        Com_Printf( "Sys_LoadDll(%s)... \n", fn );

        libHandle = Sys_LoadLibrary(fn);

        if(!libHandle) {
                Com_Printf( "Sys_LoadDll(%s) failed:\n\"%s\"\n", fn, Sys_LibraryError() );
                return NULL;
        }

        Com_Printf ( "Sys_LoadDll(%s): succeeded ...\n", fn );
        Q_strncpyz ( fqpath , fn , MAX_QPATH ) ;

        return libHandle;
}

/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
#1 look in fs_homepath
#2 look in fs_basepath
=================
*/
void *Sys_LoadDll( const char *name, char *fqpath ,
        intptr_t (**entryPoint)(int, ...),
        intptr_t (*systemcalls)(intptr_t, ...) )
{
        void  *libHandle;
        void  (*dllEntry)( intptr_t (*syscallptr)(intptr_t, ...) );
        char  fname[MAX_OSPATH];
        char  *basepath;
        char  *homepath;
        char  *gamedir;

        assert( name );

        Q_snprintf (fname, sizeof(fname), "%s" ARCH_STRING DLL_EXT, name);

        // TODO: use fs_searchpaths from files.c
        basepath = Cvar_VariableString( "fs_basepath" );
        homepath = Cvar_VariableString( "fs_homepath" );
        gamedir = Cvar_VariableString( "fs_game" );

        libHandle = Sys_TryLibraryLoad(homepath, gamedir, fname, fqpath);

        if(!libHandle && basepath)
                libHandle = Sys_TryLibraryLoad(basepath, gamedir, fname, fqpath);

        if(!libHandle) {
                Com_Printf ( "Sys_LoadDll(%s) failed to load library\n", name );
                return NULL;
        }

        dllEntry = Sys_LoadFunction( libHandle, "dllEntry" );
        *entryPoint = Sys_LoadFunction( libHandle, "vmMain" );

        if ( !*entryPoint || !dllEntry )
        {
                Com_Printf ( "Sys_LoadDll(%s) failed to find vmMain function:\n\"%s\" !\n", name, Sys_LibraryError( ) );
                Sys_UnloadLibrary(libHandle);

                return NULL;
        }

        Com_Printf ( "Sys_LoadDll(%s) found vmMain function at %p\n", name, *entryPoint );
        dllEntry( systemcalls );

        return libHandle;
}

/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs( int argc, char **argv )
{
        if( argc == 2 )
        {
                if( !strcmp( argv[1], "--version" ) ||
                                !strcmp( argv[1], "-v" ) )
                {
                        const char* date = __DATE__ " " __TIME__;
#ifdef DEDICATED
                        fprintf( stdout, Q3_VERSION " dedicated server (%s)\n", date );
#else
                        fprintf( stdout, Q3_VERSION " client (%s)\n", date );
#endif
                        Sys_Exit( 0 );
                }
        }
}

#ifndef DEFAULT_BASEDIR
#       ifdef MACOS_X
#               define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
#       else
#               define DEFAULT_BASEDIR Sys_BinaryPath()
#       endif
#endif

/*
=================
Sys_SigHandler
=================
*/

#if 0
#include "../qcommon/vm_local.h"
extern vm_t *currentVM;
void DumpVM()
{
 FILE *f;
 if (!currentVM) return;
 fprintf(stderr,"VM: %s\nStack: %X\nEntry point: %X\nIs running: %d\n",
  currentVM->name,currentVM->programStack,currentVM->entryPoint,currentVM->currentlyInterpreting);
 fprintf(stderr,"Code Addr: %X len %X\n",currentVM->codeBase,currentVM->codeLength);
 fprintf(stderr,"Data Addr: %X len %X\n",currentVM->dataBase,currentVM->dataMask+1);
 fprintf(stderr,"Pointers Addr: %X num %X\n",currentVM->instructionPointers,currentVM->instructionCount);
 fprintf(stderr,"Jumps Addr: %X num %X\n",currentVM->jumpTableTargets,currentVM->numJumpTableTargets);
 fprintf(stderr,"Stack bottom: %X\nCall level: %X\n",currentVM->stackBottom,currentVM->callLevel);
 if (currentVM->codeBase) {
  f = fopen("code.bin","wb");
  fwrite(currentVM->codeBase,currentVM->codeLength,1,f);
  fclose(f);
  fprintf(stderr,"Saved code.bin\n");
 }
 if (currentVM->dataBase) {
  f = fopen("data.bin","wb");
  fwrite(currentVM->dataBase,currentVM->dataMask+1,1,f);
  fclose(f);
  fprintf(stderr,"Saved data.bin\n");
 }
 if (currentVM->codeBase) {
  f = fopen("insts.bin","wb");
  fwrite(currentVM->instructionPointers,currentVM->instructionCount,sizeof(currentVM->instructionPointers[0]),f);
  fclose(f);
  fprintf(stderr,"Saved insts.bin\n");
 }
 if (currentVM->jumpTableTargets) {
  f = fopen("jumps.bin","wb");
  fwrite(currentVM->jumpTableTargets,currentVM->numJumpTableTargets,sizeof(currentVM->jumpTableTargets[0]),f);
  fclose(f);
  fprintf(stderr,"Saved jumps.bin\n");
 }
}
#endif




int Sys_DumpPacks(char *dst, int dstlen)
{
 int len = 0;
 int i;
 len+=Q_snprintf(dst+len, dstlen-len,
  "\n*** PK3 dump ***\n Game pure checksum: %s\n"
  " Loaded packs: %s\n Loaded pack sums: %s\n"
  " Referenced packs: %s\n Referenced pack sums: %s\n",
  FS_GamePureChecksum(),
  FS_LoadedPakNames(),FS_LoadedPakChecksums(),
  FS_ReferencedPakNames(),FS_ReferencedPakChecksums()
 );
 return len;
}


static char* dump_cvars[] = {
 "version",
 "arch",
 "com_gamename",
 "protocol",
 "fs_game",
 "ui_modversion",

 "cl_running",
 "sv_running",
 "dedicated",


 "cl_auth_status",
 "cl_guid",
 "username",
 "cl_master",

 "sv_auth_engine",
 "cl_currentServerAddress",
 "sv_referencedPakNames",
 "sv_referencedPaks",
 "sv_pakNames",
 "sv_paks",
 "sv_cheats",
 "mapname",

 "name",
 "gear",
 "raceblue",
 "racered",
 "racejump",
 "racefree",
 "funblue",
 "funred",

 "sv_hostname",
 "net_port",
 "sv_pure",
 "sv_maxclients",
 "g_gametype",
 "g_matchmode",
 "g_allowPosSaving",
 "g_regainStamina",
 "g_noStamina",
 "g_noDamage",
 "g_ghostPlayers",
 "bot_enable",


 "r_lastValidRenderer",
 "r_mode",
 "com_maxfps",
 "com_hunkmegs",
 "com_zonemegs",

};

int Sys_CrashDumpVars(char *dst, int dstlen)
{
 int len = 0;
 int i;
 len+=Q_snprintf(dst+len, dstlen-len, "\n*** Cvars dump ***\n");
 for(i=0;i<sizeof(dump_cvars)/sizeof(dump_cvars[0]);i++) {
  cvar_t *fs = Cvar_Get(dump_cvars[i],"",0);
  len+=Q_snprintf(dst+len, dstlen-len, " %s = \"%s\"\n",dump_cvars[i],fs?fs->string:"<<no value>>");
 }
 return len;
}


#ifndef DEDICATED
int Sys_DumpConsole(char *dst, int dstlen)
{
 int len = 0;
 int i;
 char tmp[256];
 len+=Q_snprintf(dst+len, dstlen-len, "\n*** Console dump ***\n");
 for(i=0;i<10;i++) {
  dst[len++]=' ';
  len+=Con_GetLine(-9+i,dst+len,dstlen-len);
  dst[len++]='\n';
 }
 return len;
}
#endif

#ifdef USE_CURL
void Sys_SendCrashLog(char *crashbuf)
{
 CURL *curl;
 CURLcode res;

 struct curl_httppost *formpost=NULL;
 struct curl_httppost *lastptr=NULL;
 struct curl_slist *headerlist=NULL;
 static const char buf[] = "Expect:";

 if (!report_url || !report_url->string[0]) {
  Com_Printf("No crash report URL specified, crash report not sent\n");
  return;
 }

 #ifdef DEDICATED
 qcurl_global_init(CURL_GLOBAL_ALL);
 #endif

 qcurl_formadd(&formpost,&lastptr,CURLFORM_PTRNAME,"type",CURLFORM_PTRCONTENTS,"debug",CURLFORM_END);
 qcurl_formadd(&formpost,&lastptr,CURLFORM_PTRNAME,"token",CURLFORM_PTRCONTENTS,"00112233445566778899AABBCCDDEEFF",CURLFORM_END);
 qcurl_formadd(&formpost,&lastptr,CURLFORM_PTRNAME,"data",CURLFORM_PTRCONTENTS,crashbuf,CURLFORM_END);

 curl = qcurl_easy_init();
 headerlist = qcurl_slist_append(headerlist, buf);
 if(curl) {
  qcurl_easy_setopt(curl, CURLOPT_URL, report_url->string);
  qcurl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
  qcurl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
  Com_Printf("Sending crash report...\n");
  res = qcurl_easy_perform(curl);
  if(res != CURLE_OK) {
   Com_Printf("Sending crash dump failed: %s\n", qcurl_easy_strerror(res));
  } else {
   Com_Printf("Crash report sent OK\n");
  }
  qcurl_easy_cleanup(curl);
 }
 qcurl_formfree(formpost);
 qcurl_slist_free_all (headerlist);
}
#endif

void Sys_CrashDump(int signal)
{
 const int crashlen = 1024*1024;
 char *crashbuf = malloc(crashlen);
 if (!crashbuf) return;
 int txtlen = Q_snprintf(crashbuf, crashlen,"*** Caught signal: %d ***\n",signal);
 txtlen+=Sys_CrashInfo(crashbuf+txtlen,crashlen-txtlen);
 txtlen+=VM_CrashDump(crashbuf+txtlen,crashlen-txtlen);
 txtlen+=Sys_CrashDumpVars(crashbuf+txtlen,crashlen-txtlen);
 txtlen+=Sys_DumpPacks(crashbuf+txtlen,crashlen-txtlen);
#ifndef DEDICATED
 txtlen+=Sys_DumpConsole(crashbuf+txtlen,crashlen-txtlen);
#endif
 fprintf( stderr, "------------- Crash report: -------------\n%s\n------------- End of crash report -------------\n",crashbuf);
#ifdef USE_CURL
 Sys_SendCrashLog(crashbuf);
#else
 fprintf( stderr, "No CURL support to send crash report\n");
#endif
 free(crashbuf);
}

void Sys_SigHandler( int signal )
{
        static qboolean signalcaught = qfalse;

        if( signalcaught )
        {
                fprintf( stderr, "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n",
                        signal );
        }
        else
        {
                signalcaught = qtrue;
                Sys_CrashDump(signal);
#ifndef DEDICATED
                CL_Shutdown( va( "Received signal %d", signal ) );
#endif
                SV_Shutdown( va( "Received signal %d", signal ) );
        }

        if( signal == SIGTERM || signal == SIGINT )
                Sys_Exit( 1 );
        else
                Sys_Exit( 2 );
}
void Sys_InitTimers(void);
/*
=================
main
=================
*/
int main( int argc, char **argv )
{
        int   i;
        char  commandLine[ MAX_STRING_CHARS ] = { 0 };

#ifndef DEDICATED
        // SDL version check

        // Compile time
#       if !SDL_VERSION_ATLEAST(MINSDL_MAJOR,MINSDL_MINOR,MINSDL_PATCH)
#               error A more recent version of SDL is required
#       endif

        // Run time
        const SDL_version *ver = SDL_Linked_Version( );

#define MINSDL_VERSION \
        XSTRING(MINSDL_MAJOR) "." \
        XSTRING(MINSDL_MINOR) "." \
        XSTRING(MINSDL_PATCH)

        if( SDL_VERSIONNUM( ver->major, ver->minor, ver->patch ) <
                        SDL_VERSIONNUM( MINSDL_MAJOR, MINSDL_MINOR, MINSDL_PATCH ) )
        {
                Sys_Dialog( DT_ERROR, va( "SDL version " MINSDL_VERSION " or greater is required, "
                        "but only version %d.%d.%d was found. You may be able to obtain a more recent copy "
                        "from http://www.libsdl.org/.", ver->major, ver->minor, ver->patch ), "SDL Library Too Old" );

                Sys_Exit( 1 );
        }
#endif

        Sys_PlatformInit( );

        // Set the initial time base
        //Sys_Milliseconds( );
        Sys_InitTimers(); // ioq3-urt: Attempting to reduce impact

        Sys_ParseArgs( argc, argv );
        Sys_SetBinaryPath( Sys_Dirname( argv[ 0 ] ) );
        Sys_SetDefaultInstallPath( DEFAULT_BASEDIR );

        // Concatenate the command line for passing to Com_Init
        for( i = 1; i < argc; i++ )
        {
                const qboolean containsSpaces = strchr(argv[i], ' ') != NULL;
                if (containsSpaces)
                        Q_strcat( commandLine, sizeof( commandLine ), "\"" );

                Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );

                if (containsSpaces)
                        Q_strcat( commandLine, sizeof( commandLine ), "\"" );

                Q_strcat( commandLine, sizeof( commandLine ), " " );
        }

        Com_Init( commandLine );
        NET_Init( );

        CON_Init( );

#ifndef WINDOWS
        signal( SIGILL, Sys_SigHandler );
        signal( SIGFPE, Sys_SigHandler );
        signal( SIGSEGV, Sys_SigHandler );
        signal( SIGTERM, Sys_SigHandler );
        signal( SIGINT, Sys_SigHandler );
#endif

#ifdef  USE_CURL
        report_url = Cvar_Get("CrashReportURL","",0);
#endif

        while( 1 )
        {
                IN_Frame( );
                Com_Frame( );
        }

        return 0;
}


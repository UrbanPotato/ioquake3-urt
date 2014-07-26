// under GPL v2

#ifdef WIN32
    #include <windows.h>
#endif
#include "../client/client.h"

/*Execute a system command
For windows, we hand-hold it to not make ioq3 lose focus when on fullscreen (avoiding a userland loader to do it).
For non-windows, we assume system()'s cmd will cater for it with some userland magic.
Some programs misbehave and steal focus anyway on windows which is .."normal" (e.g. ioq3 does it:D).*/
void Com_Sysexec_f (void) {
    int  num, i;
    char     cmd[1024];

    #ifdef WIN32
        char errormsg[1024];
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) ); //up to here almost carbon copy of microsoft's example
        si.dwFlags = STARTF_USESHOWWINDOW; //needed to use the next one
        si.wShowWindow = SW_SHOWNOACTIVATE; //needed to *not activate, i.e. not focus*
    #endif //WIN32

    num = Cmd_Argc();
    if (num < 2) {
        Com_Printf ("sysexec [command and parameters] : run an external system command/program with its parameters, it can be in the path or be entered in full path\n");
        return;
    }

    cmd[0] = 0; //required to start with a null string
    for (i = 1; i < num; i++) {
        strcat (cmd, Cmd_Argv(i));
        if (i != (num-1))
            strcat (cmd, " ");
    }
    Com_Printf("Issuing external command: %s\n",cmd);
    #ifdef WIN32
        if(!CreateProcess( NULL,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi )) {
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,GetLastError(),0,errormsg,1024,NULL);
            Com_Printf("Issuing of %s failed: %s.\n",cmd, errormsg);
                if (GetLastError() == 2)
                    Com_Printf("a command can be in the path or full path can be used\n");
        }
    #else
    i = system(cmd);
    #endif //WIN32
}

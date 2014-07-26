// under GPL v2

#if !defined(DEDICATED) 

	#include "../client/client.h"

	/*	teleportation:
		bind a key for use with rcon's /teleport (SV_Teleport_f())	*/
	void CL_BindTeleport_f (void) {
		char cmd[256];
		int b;

		if (Cmd_Argc() != 2) 	{ 
			Com_Printf ("bindteleport <key>: bind a <key> for use with rcon's /teleport\n");
			return;
		}
	
		if ( cls.state != CA_ACTIVE ) {
			Com_Printf ("Game view is required to use bindteleport.\n");
			return;
		}
		
		b = Key_StringToKeynum (Cmd_Argv(1));

		if (b==-1){
			Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
			return;
		}

		sprintf(cmd,"teleport %i %f %f %f",clc.clientNum, cl.snap.ps.origin[0],cl.snap.ps.origin[1], cl.snap.ps.origin[2]);

		Key_SetBinding(b, cmd);
	}	

#endif

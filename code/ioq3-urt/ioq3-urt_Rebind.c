// under GPL v2

#if !defined(DEDICATED) 

	#include "ioq3-urt.h"
	#include "../client/client.h"

	//bind a key to a cvar and its current value; based on Key_Bind_f() and Key_Unbind_f()
	void Key_Rebind_f (void) {
		int		c, b;
		cvar_t	*cvar_tmp;
		char		cmd[2048];

		c = Cmd_Argc();

		if (c < 2 || c > 3)
		{
			Com_Printf ("rebind <key> [cvar] : bind a <key> to a [cvar] and its current value\n");
			return;
		}
		b = Key_StringToKeynum (Cmd_Argv(1));
		if (b==-1)
		{
			Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
			return;
		}

		if (c == 2)
		{
			if (keys[b].binding)
				Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keys[b].binding );
			else
				Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
			return;
		}

		if ( ( cvar_tmp = Cvar_FindVar(Cmd_Argv(2)) ) ) {
			strcat(cmd, Cmd_Argv(2));strcat(cmd, " ");strcat(cmd, cvar_tmp->string);
			Key_SetBinding(b, cmd);
		} else {
			Com_Printf("cvar %s does not exist\n", Cmd_Argv(2));
		}
	}

#endif

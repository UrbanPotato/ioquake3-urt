// under GPL v2

#if !defined(DEDICATED) 
	
	#include "../client/client.h"

	/*	
		urbanterror.net DNS issue workaround 
		(UI code overrides sv_master1 to something that doesn't work, override back) 
	*/
	void DNS_workaround (void) {
		cvar_t *tmp;
			
		tmp = Cvar_Get ("sv_master1", MASTER_SERVER_NAME, 0 );
			
		if (!strncmp(tmp->string, "master.urbanterror.net", strlen("master.urbanterror.net")))
			Cvar_Set ("sv_master1", "master.urbanterror.info");
	}

#endif

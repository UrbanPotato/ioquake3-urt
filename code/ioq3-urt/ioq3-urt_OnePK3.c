// under GPL v2

#if !defined(DEDICATED) 

	#include "../client/client.h"
	
	/*
		one_pk3() hack for autodownloading from iourt + no z.. names.

		Two string functions are backported.
	*/

	char* Q_strnchr( const char* string, int c, int n )
	{
			char *s;

		if( string == 0 ) return (char *)0;

		for( s = (char *)string; *s; s++ ) {
				if( *s == c ) {
					n--;
					if(!n)
        		return s;
				}
			}

		return (char *)0;
	}
	char* Q_strnrchr( const char *string, int c, int n )
	{
		char *s;

		if( string == 0 ) return (char *)0;

			for( s = (char *)string+strlen(string)-1; s>=string; s-- ) {
				if( *s == c ) {
					n--;
					if(!n) 
						return s;
				}
			}

		return (char *)0;
	}
	void one_pk3(void) {
		char *s;
		//sprintf(clc.downloadList, "@q3ut4/%s.pk3@q3ut4/%s.pk3", clc.mapname);
		
		if( cl_autodownload->integer & DLF_ENABLE ) {
		// check whether there is a <bspname>.pk3; if so, download, else proceed as normal
			if (!(((int)clc.mapname[0] >= (int)('a') && (int)clc.mapname[0] <= (int)('y')) || ((int)clc.mapname[0] >= (int)('A') && (int)clc.mapname[0] <= (int)('Y')))) {
				clc.downloadList[0] = '\0';
			} else {
				s=strstr(clc.downloadList, va("/%s.pk3@", clc.mapname));
				if (s) {
					// remove stuff after current map
					s=Q_strnchr(s, '@', 2);
					if(s)
						s[0] = '\0';
					// remove stuff before current map
					s=Q_strnrchr(clc.downloadList, '@', 2);
					if(s)
						memmove( clc.downloadList, s, strlen(s) + 1);
				} else {
					clc.downloadList[0] = '\0';
				}
			}
		}
	}


#endif

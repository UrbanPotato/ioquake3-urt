//under GPL v2

#if defined(USE_CURL) && !defined(DEDICATED) /* We could have it on a USE_TRANSLATION, however, that'd be largely pointless
                                                since cURL is the only special thing it needs. */
    /*
        On 12/01/2011, Google decided to disable the free version of their translation API.
        Unfortunately for us, somewhere during November 2011, the auto-translate capability of
        Mitsubishi's patch stopped working.

        The following code has been hacked up to use Microsoft's Bing Translator API.
        Hopefully "EXTREME ABUSE" won't give them a reason them to shut theirs down too.

        The TOS for Microsoft Translator API didn't cover anything of concern in relation to
        usage of the AppId. Therefore I'm assuming this implementation doesn't violate any TOS.
        If, for any reason, my developer account is closed, you change the App Id.

        Instructions for changing App Id:
        1. Go to http://msdn.microsoft.com/en-us/library/ff512386.aspx to find out how to create
           a Windows Live account and generate your own App Id.
        2. Create your App, then change the following define to reflect your new App Id.
    */
#define BING_API_APPID "EB4125A3C0A0988B013C010B6EE6A3F239D60A9E"
#define BING_API_URL   "http://api.microsofttranslator.com/v2/Http.svc/"
#define TR_METHOD_UNKNOWN 255
#define TR_METHOD_GETAPPIDTOKEN 0
#define TR_METHOD_DETECT 1
#define TR_METHOD_TRANSLATE 2
    /*
        P.S. The model of the original translation code was based on a single request vs. a
             single response. Microsoft's API requires seperate requests to get an AppIdToken
             and detect languages. The XML response does not indicate what was requested so I
             changed the usage of CURL easy handle private data to allow requests to persist
             so responses can thus be linked to the original request made.

        strata@dropswitch.net Jan-2012
    */
    /*
        The routine uses a multi cURL handle; while at first glance it may be considered slower than multithreading
        of easy handles, in fact it may be better since it utilizes already open connections.

        However, it uses both the multi approach and a thread for its main loop, hence it gets both the advantages.

        Before multithreading, it required c-ares support on libcurl on non-windows to avoid blocking on DNS resolution
        (now it is there but not noticeable). There were other rare cases of (undocumented by libcurl) blocking.

        The main looping function runs in a thread on its own (in a single instance) hence it's safe to call this and
        thread-safe functions in it without immediate locks.

        However:

        Com_Printf is massively thread-unsafe, hence we have to indirectly call it with a protected 'to print' var on the
        main thread. *THIS APPLIES TO EVERYTHING ON THE LOOPING FUNCTION AND EVERYTHING IT CALLS*

        Plus, we have to protect shared vars even recursively to functions this calls.

        Any cvars have to be read only at initialization since they are globally used. If changed, the thread should be
        restarted.

        Any timer functions have to be avoided unless they are protected themselves. This may be undesireable since
        they are called a lot by the rest of the engine.

    */

    #include "ioq3-urt.h"
    #include "../client/cl_curl.h"
    #include "SDL_thread.h"
    #include "SDL_mutex.h"
    #include     "../client/client.h"

    char TranslateAppIdToken[256];

    // Memory Struct for Microsoft Translator received strings
    struct BingMem {
        char *memory;
        size_t size;
    };

    struct BingMem TrChunk;

    // copies of cvars for thread safety
    char *translateIn_c;
    char *translateOut_c;
    int cl_translationSleep_c;

    // We have to protect: char** TranslationURLS and char** TranslationToPrint:
    SDL_mutex *TranslationURLS_mutex, *TranslationToPrint_mutex;

    // 'Messages to print' buffer for thread safety and avoiding missing messages.
    // XXX: CHANGE BACK TO 3 DAMNIT?
    #define TRANSLATION_TOPRINT_SLOTS 9
    char TranslationToPrint[TRANSLATION_TOPRINT_SLOTS + 1][2048]; // + 1 is reserved for sending 'it's full' message.

    void Com_Translate_ToPrint(const char* msg, ...) {
        int i;
        va_list args;

        SDL_LockMutex(TranslationToPrint_mutex);
            for (i = 0; i < TRANSLATION_TOPRINT_SLOTS; i++) {
                if (TranslationToPrint[i][0] == '\0') { // Empty slot found
                    va_start(args, msg);
                        vsnprintf(TranslationToPrint[i], 2048, msg, args);
                    va_end(args);
                    SDL_UnlockMutex(TranslationToPrint_mutex);
                    return;
                }

            }
            sprintf(TranslationToPrint[TRANSLATION_TOPRINT_SLOTS], "Translation: The %i printing slots are full.\n", TRANSLATION_TOPRINT_SLOTS);
            va_end(args);
        SDL_UnlockMutex(TranslationToPrint_mutex);
    }

    void Com_Translate_DispatchMessages (void) {
        int i;
        SDL_LockMutex(TranslationToPrint_mutex);
            for (i = 0; i < TRANSLATION_TOPRINT_SLOTS + 1; i++) {
                if (TranslationToPrint[i][0] != '\0') {
                    Com_Printf(TranslationToPrint[i]);
                    #ifdef BUILD_FREETYPE
                        /*  Send the same message to Unicode output if requested.
                            We only run in the main thread here so no need to protect anything there
                            (since it's also single threaded there) */
                        if (clu.Unicode_TranslationOutput->integer && strstr(TranslationToPrint[i], "^2Translated^7["))
                            clu.Unicode_FindSlot(TranslationToPrint[i], clu.Unicode_MessageTime->integer);
                    #endif
                    TranslationToPrint[i][0] = '\0';
                }
            }
        SDL_UnlockMutex(TranslationToPrint_mutex);
    }

    static size_t Com_Translate_Mem(void *ptr, size_t size, size_t nmemb, void *data) {
        size_t realsize = size * nmemb;
        struct BingMem *mem = (struct BingMem *)data;

        mem->memory = realloc(mem->memory, mem->size + realsize + 1);
        if (mem->memory == NULL)
            return 0;
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }

    // URL encoding routine, based on shared by http://www.geekhideout.com/urlcode.shtml
    static char to_hex(char code) {     // Converts an integer value to its hex character
        static char hex[] = "0123456789abcdef";
        return hex[code & 15];
    }
    static char *url_encode(char *str) {
        char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf;
        if (!buf) return 0;
        pbuf = buf;

        while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
            *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
        pstr++;
        }
        *pbuf = '\0';
        return buf;
    }

    /*
        The Max cURL easy handles we're going to be looping through for available slots.
    */
    #define MAX_EASY_HANDLES 12

    CURLM   *TrMultiHandle;

    /*
        Max text messages/URLs looping in memory looking for available cURL easy handles.
        This will often help if the translation server is unreachable for a while.

        It also serves as a common buffer.
    */
    #define MAX_URLS_IN_MEM 24

    char TranslationURLS[MAX_URLS_IN_MEM][1024];
    char TranslationNames[MAX_URLS_IN_MEM][1216];

    void Com_Translate_CheckVarsForChange(void);
    /*
        Add a URL to the list.
        This function is called in the main thread only hence it can be allowed to be
        thread unsafe, where it doesn't touch variables shared with the translation thread.
    */
    void Com_Translate_AddURL (char* name, char* msg, char* detected_lang) {
        char url[1024], inlang[8], outlang[8], *encoded;
        char new_private[1216];
        int i, tr_method = TR_METHOD_UNKNOWN;

        /*  This is a good time to check vars for change since running it in the global loop
            is a waste of CPU time */
        Com_Translate_CheckVarsForChange();

        // If "auto", we can skip the Detect method
        if (!strcmp(clu.translateIn->string, "auto"))
            strcpy(inlang, "");
        else
            strcpy(inlang, clu.translateIn->string);

        if (!strcmp(clu.translateOut->string, "auto"))
            strcpy(outlang, "en"); // Change this if you wish.
        else
            strcpy(outlang, clu.translateOut->string);

        if(detected_lang) strcpy(inlang, detected_lang);

        // If the In/Out languages are the same, we don't need to translate.
        if(!strcmp(inlang, outlang)) return;

        url[0] = '\0';

        if ( !( encoded = url_encode(msg) ) ) {
            Com_Printf("Translation: URL encoding error\n");
            return;
        }

        if(!TranslateAppIdToken[0]) { // No AppIdToken? Let's request one.
            snprintf(url, 1024, "%sGetAppIdToken?appId=%s&expireSeconds=86400",
                    BING_API_URL, BING_API_APPID);
            tr_method = TR_METHOD_GETAPPIDTOKEN;
            strcpy(inlang, "!!\0");
        } else if(!inlang[0]) { // Detect language
            snprintf(url, 1024, "%sDetect?appId=%s&text=%s", BING_API_URL, TranslateAppIdToken,
                     encoded);
            tr_method = TR_METHOD_DETECT;
            strcpy(inlang, "!!\0");
        } else { // Translate
            snprintf(url, 1024, "%sTranslate?appId=%s&from=%s&to=%s&text=%s", BING_API_URL,
                    TranslateAppIdToken, inlang, outlang, encoded);
            tr_method = TR_METHOD_TRANSLATE;
        }

        free(encoded);

        if(name) {
            snprintf(new_private, 1216, "%d/%s%d/%s%s", tr_method, inlang, (int) strlen(name),
                     name, msg);
        } else {
            snprintf(new_private, 1216, "%d/%s0/%s", tr_method, inlang, msg);
        }

        // Find a URL slot:
        SDL_LockMutex(TranslationURLS_mutex);
            for (i = 0; i < MAX_URLS_IN_MEM; i++) {
                if (!strlen(TranslationURLS[i])) { // Found an available one
                    strcpy(TranslationURLS[i], url);
                    strcpy(TranslationNames[i], new_private);
                    SDL_UnlockMutex(TranslationURLS_mutex);
                    return;
                }
            }
        SDL_UnlockMutex(TranslationURLS_mutex);

        Com_Printf("Translation: The %i message slots are full\n", MAX_URLS_IN_MEM);
    }

    /*
        Populate an easy handle
    */
    void Com_Translate_PopulateEasyHandle (CURL* easy_handle, char *url, void *name_ptr) {

        // give it the url
        qcurl_easy_setopt(easy_handle, CURLOPT_URL, url);

        // add the name if any
        if (strlen((char*) name_ptr))
            qcurl_easy_setopt(easy_handle, CURLOPT_PRIVATE, name_ptr);
        else
            qcurl_easy_setopt(easy_handle, CURLOPT_PRIVATE, NULL);

        // add it to the multi
        qcurl_multi_add_handle(TrMultiHandle, easy_handle);

    }

    /*
        Perform any operations on multi
    */
    void Com_Translate_MultiPerform (void) {
        CURLMcode multi_code;
        int running_handles, i = 0;

        /*
            Before multithreading, curl_multi_perform would block the client on
            non-windows when first resolving the translation domain unless libcurl
            is compiled with c-ares support.

            Other blocking conditions may exist (e.g. when the internet connection
            goes down? select()?) but that's not documented by libcurl.

            A solution/workaround to all of this is the threaded approach.
        */
        multi_code = qcurl_multi_perform(TrMultiHandle, &running_handles);

        while(multi_code == CURLM_CALL_MULTI_PERFORM && i < 100) { // this apparently occurs  4 or 5 times each job on win64
            multi_code = qcurl_multi_perform(TrMultiHandle, &running_handles);
            i++;
        }

    }

    CURL *UnusedExistingHandles[MAX_EASY_HANDLES];

    /*
        Add an existing easy handle as unused
    */
    void Com_Translate_AddUnusedEasyHandle (CURL *easy_handle) {
        int i;
        for (i = 0; i < MAX_EASY_HANDLES; i++) {
            if (!UnusedExistingHandles[i]) {
                UnusedExistingHandles[i] = easy_handle;
                return;
            }
        }
        Com_Translate_ToPrint("UnusedExistingHandles is full.\n");
    }

    /*
        Pop an easy handle out of the list of unused ones
    */
    CURL* Com_Translate_PopUnusedHandle (void) {
        int i;
        CURL *ptr;
        for (i = 0; i < MAX_EASY_HANDLES; i++) {
            if (UnusedExistingHandles[i]) {
                ptr = UnusedExistingHandles[i];
                UnusedExistingHandles[i] = NULL;
                return ptr;
            }
        }
        return 0;
    }

    /*
        Remove a handle from the multi and add it to the unused
    */
    void Com_Translate_RemoveHandle (CURL* handle) {

        // ************* IMPORTANT: remove_handle must come second since it destroys curl_msg:
        Com_Translate_AddUnusedEasyHandle(handle); // add it to the list of idle handles
        qcurl_multi_remove_handle(TrMultiHandle, handle); // remove it from the multi
    }

    /*
        Loop through the system. See comments on the top of the file.
    */
    void Com_Translate_Loop (void) {
        int i = 0, msgs_in_queue, tr_method, received_len, name_len;
        CURLMsg *curl_msg;
        CURL* unused_handle;
        char *pos1, *pos2;
        char final[1024], final_ToPrint[1024], name[1216], msg[1216], tmp[1216];
        char *new_private, lang[3];

        // Perform here
        Com_Translate_MultiPerform();

        // See if multi has any messages
        while ( ( curl_msg = qcurl_multi_info_read(TrMultiHandle, &msgs_in_queue)) ) {

            if (curl_msg->msg == CURLMSG_DONE) { // An easy has completed its performance

                if (!TrChunk.size) {

                    Com_Translate_ToPrint("Translation: no data received from Microsoft Translator\n");

                    if(TrChunk.memory) free(TrChunk.memory);
                    TrChunk.memory = NULL;
                    TrChunk.size = 0;
                    Com_Translate_RemoveHandle(curl_msg->easy_handle);
                    return;
                }

                // The Translator API spits out a single line of XML instead of "JSON gibberish"
                // As such, the parsing is much easier than it was under Google API.
                // Error handling is done by checking to see if the first five bytes are
                // the string: "<html" (which seems to indicate some sort of error)
                if(!strncmp(TrChunk.memory, "<html", 5)) {
                    if(TrChunk.memory) free(TrChunk.memory);
                    TrChunk.memory = NULL;
                    TrChunk.size = 0;

                    Com_Translate_RemoveHandle(curl_msg->easy_handle);
                    return;
                }

                // Only to be replaced by my ghetto parsing skillz.
                qcurl_easy_getinfo(curl_msg->easy_handle, CURLINFO_PRIVATE, &new_private);

                pos1 = new_private;
                pos2 = strchr(pos1, '/');
                strncpy(tmp, pos1, pos2 - pos1); tmp[pos2 - pos1] = '\0';
                tr_method = atoi(tmp);

                pos1 = pos2 + 1;
                strncpy(lang, pos1, 2); lang[2] = '\0';

                pos1 += 2;
                pos2 = strchr(pos1, '/');
                strncpy(tmp, pos1, pos2 - pos1); tmp[pos2 - pos1] = '\0';
                name_len = atoi(tmp);

                if(name_len < 1) {
                    name[0] = '\0';
                    pos1 += 2;
                } else {
                    pos1 = pos2 + 1;
                    strncpy(name, pos1, name_len); name[name_len] = '\0';
                    pos1 += name_len;
                }

                strcpy(msg, pos1);

                pos1 = strchr(TrChunk.memory, '>');

                if(pos1) pos2 = strchr(pos1, '<');
                pos1++;

                if(pos2) pos2--;

                received_len = 0;
                if(pos1 && pos2) {
                    received_len = pos2 - pos1;
                    received_len = (received_len >= 1024) ? 1023 : received_len;
                    received_len++;

                    if(received_len < 0) received_len = 0;

                    if(received_len) {
                        strncpy(final, pos1, received_len);
                        final[received_len] = '\0';
                    }
                }

                if(!received_len) {
                    if(TrChunk.memory) free(TrChunk.memory);
                    TrChunk.memory = NULL;
                    TrChunk.size = 0;

                    Com_Translate_RemoveHandle(curl_msg->easy_handle);
                    return;
                }

                switch(tr_method) {
                    case TR_METHOD_GETAPPIDTOKEN: // Got an AppIdToken.
                        strcpy(TranslateAppIdToken, final);
                        Com_Translate_AddURL(name, msg, NULL);
                        break;

                    case TR_METHOD_DETECT: // Detected a language.
                        Com_Translate_AddURL(name, msg, final);

                        break;

                    case TR_METHOD_TRANSLATE: // Finished translation
                        if(!strlen(final)) {
                            Com_Translate_ToPrint("Translation: recieved an empty string.\n");
                        } else {
                            if(strlen(name) == 1 && name[0] == '$') {
                                snprintf(final_ToPrint, 1024,
                                         "^2Translated^7[^4%s^7]: ^3%s\n", lang, final);
                            } else {
                                snprintf(final_ToPrint, 1024,
                                         "^2Translated^7[^4%s^7]: ^7%s^3%s\n",
                                         lang, name, final);
                            }

                            Com_Translate_ToPrint(final_ToPrint);
                        }

                        break;

                    default:
                        break;
                }

                // Free allocated memory
                if(TrChunk.memory) free(TrChunk.memory);

                TrChunk.memory = NULL;
                TrChunk.size = 0;

                Com_Translate_RemoveHandle(curl_msg->easy_handle);

            }
        }

        // Find if there are new messages(URLS)
        SDL_LockMutex(TranslationURLS_mutex);
        for (i = 0; i < MAX_URLS_IN_MEM; i++) {

            if (strlen(TranslationURLS[i])) { // A new message is found:

                // Find an available cURL handle and populate it with it
                if ( (unused_handle = Com_Translate_PopUnusedHandle()) ) {
                    Com_Translate_PopulateEasyHandle(unused_handle, TranslationURLS[i], TranslationNames[i]);
                    TranslationURLS[i][0] = '\0';
                    // Don't clean the name yet because we passed it its pointer
                } // no else, just keep looping trying to post them

            }

        }
        SDL_UnlockMutex(TranslationURLS_mutex);

    }

    int Com_Translate_Thread (void* nothing) {

        while(1) {

            Com_Translate_Loop();

            // Sleep if requested
            if ( cl_translationSleep_c)
                    Sys_Sleep (cl_translationSleep_c);

        }

        return 0;
    }

    /*
        Initialization
    */
    SDL_Thread *Translation_thread;
    CURL *all_handles[MAX_EASY_HANDLES]; // used for cleaning up on shutdown
    void Com_Translate_Init(void) {
        int i;

        // Initialize the multi handle
        if (! ( TrMultiHandle = qcurl_multi_init() ) ) {
            Com_Printf (S_COLOR_YELLOW "Translation: multi handle not initialized, aborting\n");
            return;
        }
        /*  set the max connections to 8
            commented for now since it's supported on only 7.16.3+
            Notice each easy can have by default up to 5 cached connections */
        // qcurl_multi_setopt(TrMultiHandle, CURLMOPT_MAXCONNECTS, 8);

        if(TrChunk.memory) { // in case we are restarting
            free(TrChunk.memory);
            TrChunk.memory = NULL;
        }
        TrChunk.size = 0;    // if initially, no data yet

        // Initialize unused easy handles:
        for (i = 0; i < MAX_EASY_HANDLES; i++) {

            CURL *easy = qcurl_easy_init(); // Initialize the easy handle

            /* Set easy options: */
            // Commented ones remain here in case SSL or threading are in need in the future
            // qcurl_easy_setopt(easy, CURLOPT_SSLVERSION, 3); // it was needed on https:// (v2 API) attempts when using GnuTLS
            // qcurl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L); // skip peer verification
            qcurl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, Com_Translate_Mem); // send all data to this function
            qcurl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *)&TrChunk); // pass 'chunk' struct to callback function
            qcurl_easy_setopt(easy, CURLOPT_USERAGENT, "libcurl-agent/1.0"); // required on some web servers
            qcurl_easy_setopt(easy, CURLOPT_REFERER, "ioq3-urt");
            // logging for debugging:
            // qcurl_easy_setopt(easy, CURLOPT_VERBOSE, 1);
            // qcurl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1); // uncertain if it is needed or if it affects stability; see docs

            // Add the handle to the list of unused ones
            Com_Translate_AddUnusedEasyHandle(easy);
            all_handles[i] = easy; // for cleaning up on shutdown

        }

        // Create the mutexes
        TranslationURLS_mutex = SDL_CreateMutex();
        TranslationToPrint_mutex = SDL_CreateMutex();

        // Copy cvars for thread safety:
        if ( !(translateIn_c = malloc (sizeof(char) * (strlen(clu.translateIn->string)+1)))) return;
        strcpy (translateIn_c, clu.translateIn->string);
        if ( !(translateOut_c = malloc (sizeof(char) * (strlen(clu.translateOut->string)+1)))) return;
        strcpy (translateOut_c, clu.translateOut->string);
        cl_translationSleep_c = clu.cl_translationSleep->integer;

        // Spawn the thread
        if ( !( Translation_thread =  SDL_CreateThread(Com_Translate_Thread, NULL)) ) {
                Com_Printf(S_COLOR_YELLOW "Translation: Unable to create thread\n");
                return;
        }

        clu.TranslationInitialized = qtrue;
    }

    void Com_Translate_Shutdown (void) {
        int i;

        // Inform the engine to not use the system anymore:
        clu.TranslationInitialized = qfalse;

        /*  Kill the thread gracelessly (PS. cURL will be very ungry and start killing engines
            if two threads run its same vars). fixme? Ideally communicate with the thread about it. */
        SDL_KillThread(Translation_thread);

        // Destroy its mutexes:
        SDL_DestroyMutex(TranslationURLS_mutex);
        SDL_DestroyMutex(TranslationToPrint_mutex);

        // Remove all handles from multi and clean them
        for (i = 0; i < MAX_EASY_HANDLES; i++) {
            qcurl_multi_remove_handle(TrMultiHandle, all_handles[i]);
            qcurl_easy_cleanup(all_handles[i]);
            Com_Translate_PopUnusedHandle(); // keep popping
        }

        // Clean the multi
        qcurl_multi_cleanup(TrMultiHandle);

        // Free saved variables
        free(translateIn_c);
        free(translateOut_c);
        cl_translationSleep_c = 0;

        if(TrChunk.memory) {
            free(TrChunk.memory);
            TrChunk.memory = NULL;
        }
        TrChunk.size = 0;

    }

    /*
        For thread safety and live functionality, check if vars used in the Translation thread have
        changed and restart the system to re-copy them.

        We don't run it in the global loop to save some cycles.
    */
    void Com_Translate_CheckVarsForChange(void) {
        if (
            strcmp(translateIn_c, clu.translateIn->string) ||
            strcmp(translateOut_c, clu.translateOut->string) ||
            cl_translationSleep_c != clu.cl_translationSleep->integer
            ) {

            Com_Printf("Thread safety related cvar had changed, restarting the Translation system..\n");
            Com_Translate_Shutdown();
            Com_Translate_Init();
        }
    }

    // Translate something explicitly with /translate <text>
    void Com_Translate_f (void) {

        if (Cmd_Argc() < 2) {
            Com_Printf("translate <text>: translate text according to translateIn/Out languages.\n");
            return;
        }

        if(!clu.cl_translation->integer && !clu.TranslationInitialized) { // since they may be changing it on the fly
            Com_Printf("Translation is not enabled, /cl_translation 1 and restart client.\n");
            return;
        }

        if(!clu.TranslationInitialized) {
            Com_Printf("Translation is not initialized, probably because cURL wasn't found.\n");
            return;
        }

        Com_Translate_AddURL(NULL, Cmd_Args(), NULL);

    }

    // Take input automatically from console
    void Com_Translate_Auto (char *input) {
        char copy[256], *pointer, name[128], name_loc[128];
        static char saved1[256], saved2[256];
        static qboolean Seconds_turn;
        uint8_t i, y;
        size_t ignore = 0;
        const char dead[] = "(DEAD) ", spec[] = "(SPEC) ", dead_spec[] = "(DEAD) (SPEC) ";
        qboolean no_letters = qtrue, proceed = qfalse;

        // remove '^' and the next character (q3 coloring)
        for (i = 0, y =0; i < strlen(input); i++) {
            if ( input[i] == '^') {
                i++;
                continue;
            }
            copy[y++] = (input)[i];
        }
        copy[y] = '\0';

        // Make sure the message came from one of the players
        for (i = 0; i < MAX_CLIENTS; i++) {
            // credit to Roman Tetelman from the mailing list:
            pointer = Info_ValueForKey( cl.gameState.stringData + cl.gameState.stringOffsets[ CS_PLAYERS +i ], "n" );
            if (strlen(pointer)) {

                // Append the ': '; this avoids quirks on kill messages and with names ending with ':'
                strcpy(name, pointer);
                strcat(name, ": ");

                // Location names like UrT's:
                strcpy(name_loc, pointer);
                strcat(name_loc, " (");

                // If it has '(DEAD) ' or '(SPEC) ' of UrT ignore it
                if (
                    (
                        (pointer = strstr(copy, dead)) &&
                        strlen(pointer) == strlen(copy) // It was in the beginning
                    ) ||
                    (
                        (pointer = strstr(copy, spec))  &&
                        strlen(pointer) == strlen(copy)
                    )
                ) ignore = strlen(dead);

                if ( // No 'else if' because dead will catch dead_spec
                    (
                        (pointer = strstr(copy, dead_spec))  && /*  a dead that went spec prints '(DEAD) (SPEC)'
                                                                    or a buggy condition */
                        strlen(pointer) == strlen(copy)
                    )
                ) ignore = strlen(dead_spec);

                if  (
                        (
                            (pointer = strstr(copy + ignore, name)) &&
                            (strlen(pointer) == strlen(copy + ignore))
                        )
                        ||
                        (
                            (pointer = strstr(copy + ignore, name_loc)) &&
                            (strlen(pointer) == strlen(copy + ignore))
                        )
                    ) {
                        proceed = qtrue;
                        break;
                }
            }
        }

        // Allow some server messages if requested
        if (
                !clu.translateAutoFilterServer->integer
                && (
                    ((pointer = strstr(copy, "console: ")) && strlen(pointer) == strlen(copy)) ||
                    ((pointer = strstr(copy, "server: ")) && strlen(pointer) == strlen(copy))
                )
        ) {
            proceed = qtrue;
            if (strstr(copy, "console: "))
                strcpy (name, "console: ");
            else
                strcpy (name, "server: ");
        }

        if (!proceed) return;

        // remove newline towards the end if it exists (e.g. baseq3 sends one, q3ut3 doesn't):
        if( (pointer = strchr(copy, '\n')) != NULL) *pointer = '\0';

        // point after the name
        pointer = copy + strlen(name) + ignore; // It already has ': ' appended in it, plus we ignore '(DEAD)/(SPEC)'.

        // Check if the message has any letters at all
        for (i = 0; i < strlen(pointer); i++)
            if (isalpha((int)pointer[i])) no_letters = qfalse;

        // Disallow messages from translation, such as 'lol' alone. Also saved ones described below
        if (
            no_letters                  ||
            !strcmp(saved1, pointer)        ||
            !strcmp(saved2, pointer)        ||
            !strcmp("lol",  pointer)        ||  // fixme?: to be a gentleman(or lady) implement a proper parser
            !strcmp("LOL",  pointer)        ||
            !strcmp("Lol",  pointer)        ||
            pointer[0] == '!'           || // One of those bot commands most probably
            strlen(pointer) < clu.translateAutoFilterShorterThan->integer ||     // good for smilies and "gg"
            (strchr(pointer, ':') && strlen(pointer) == 3) // it's a smiley, most likely. ||

        ) {
            return;
        }

        /*  Do not send the same message twice in a row and for two interchanging values.
            e.g. two people spamming different messages each but each spamming their own.  */
        if (!Seconds_turn) { // Is it the second variable's turn to be saved?
            strcpy(saved1, pointer);
            Seconds_turn = qtrue;
        } else {
            strcpy(saved2, pointer);
            Seconds_turn = qfalse;
        }

        Com_Translate_AddURL(name, pointer, NULL);

    }

#endif

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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>

/*
=============================================================
tty console routines

NOTE: if the user is editing a line when something gets printed to the early
console then it won't look good so we provide CON_Hide and CON_Show to be
called before and after a stdout or stderr output
=============================================================
*/

extern qboolean stdinIsATTY;
static qboolean stdin_active;
// general flag to tell about tty console mode
static qboolean ttycon_on = qfalse;
static int ttycon_hide = 0;
static int ttycon_show_overdue = 0; // used only by CON_Print()

// some key codes that the terminal may be using, initialised on start up
static int TTY_erase;
static int TTY_eof;

static struct termios TTY_tc;

static field_t TTY_con;

// This is somewhat of aduplicate of the graphical console history
// but it's safer more modular to have our own here
#define CON_HISTORY 32
static field_t ttyEditLines[ CON_HISTORY ];
static int hist_current = -1, hist_count = 0;

// Note: TTY_CONSOLE_PROMPT can contain any number of characters.
#ifdef DEDICATED
#define TTY_CONSOLE_PROMPT "]"
#else
// Right now the in-game console has the "]" prompt.
// We choose a different prompt for the tty console for the following reasons:
// 1. Both tty and in-game console commands get printed to tty output, so it may be
//    confusing to tell where a command came from.
// 2. tty and in-game consoles have an independent history buffers.
// 3. When connected to a server, a command in the in-game console not preceded
//    by a slash is a chat (not so for tty).
#define TTY_CONSOLE_PROMPT "ioq3-tty]"
// Note: In order to cause tty commands to be visible in the in-game console, you
// must define TTY_COMMANDS_VISIBLE_IN_GAME.
#define TTY_COMMANDS_VISIBLE_IN_GAME
#endif

/*
==================
CON_FlushIn

Flush stdin, I suspect some terminals are sending a LOT of shit
FIXME relevant?
==================
*/
static void CON_FlushIn( void )
{
	char key;
	while (read(STDIN_FILENO, &key, 1)!=-1);
}

/*
==================
CON_Back

Output a backspace

NOTE: it seems on some terminals just sending '\b' is not enough so instead we
send "\b \b"
(FIXME there may be a way to find out if '\b' alone would work though)
==================
*/
static void CON_Back( void )
{
	char key;
	size_t size;

	key = '\b';
	size = write(STDOUT_FILENO, &key, 1);
	key = ' ';
	size = write(STDOUT_FILENO, &key, 1);
	key = '\b';
	size = write(STDOUT_FILENO, &key, 1);
}

/*
==================
CON_Hide

Clear the display of the line currently edited
bring cursor back to beginning of line
==================
*/
static void CON_Hide( void )
{
	if( ttycon_on )
	{
		int i;
		if (ttycon_hide)
		{
			ttycon_hide++;
			return;
		}
		if (TTY_con.cursor>0)
		{
			for (i=0; i<TTY_con.cursor; i++)
			{
				CON_Back();
			}
		}
		for (i = strlen(TTY_CONSOLE_PROMPT); i > 0; i--) {
			CON_Back();
		}
		ttycon_hide++;
	}
}

/*
==================
CON_Show

Show the current line
FIXME need to position the cursor if needed?
==================
*/
static void CON_Show( void )
{
	if( ttycon_on )
	{
		int i;

		assert(ttycon_hide>0);
		ttycon_hide--;
		if (ttycon_hide == 0)
		{
			size_t size;
			size = write(STDOUT_FILENO, TTY_CONSOLE_PROMPT, strlen(TTY_CONSOLE_PROMPT));
			if (TTY_con.cursor)
			{
				for (i=0; i<TTY_con.cursor; i++)
				{
					size = write(STDOUT_FILENO, TTY_con.buffer+i, 1);
				}
			}
		}
	}
}

/*
==================
CON_Shutdown

Never exit without calling this, or your terminal will be left in a pretty bad state
==================
*/
void CON_Shutdown( void )
{
	int i;
	if (ttycon_on)
	{
		for (i = strlen(TTY_CONSOLE_PROMPT); i > 0; i--) {
			CON_Back();
		}
		tcsetattr (STDIN_FILENO, TCSADRAIN, &TTY_tc);
	}

	// Restore blocking to stdin reads
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
}

/*
==================
Hist_Add
==================
*/
void Hist_Add(field_t *field)
{
	int i;
	if (!field->cursor) { return; }	// Don't save blank lines in history.
					// We could have checked field->buffer[0] instead.
	assert(hist_count <= CON_HISTORY);
	assert(hist_count >= 0);
	assert(hist_current >= -1);
	assert(hist_current <= hist_count);
	// make some room
	for (i=CON_HISTORY-1; i>0; i--)
	{
		ttyEditLines[i] = ttyEditLines[i-1];
	}
	ttyEditLines[0] = *field;
	if (hist_count<CON_HISTORY)
	{
		hist_count++;
	}
	hist_current = -1; // re-init
}

/*
==================
Hist_Prev
==================
*/
field_t *Hist_Prev( void )
{
	int hist_prev;
	assert(hist_count <= CON_HISTORY);
	assert(hist_count >= 0);
	assert(hist_current >= -1);
	assert(hist_current <= hist_count);
	hist_prev = hist_current + 1;
	if (hist_prev >= hist_count)
	{
		return NULL;
	}
	hist_current++;
	return &(ttyEditLines[hist_current]);
}

/*
==================
Hist_Next
==================
*/
field_t *Hist_Next( void )
{
	assert(hist_count <= CON_HISTORY);
	assert(hist_count >= 0);
	assert(hist_current >= -1);
	assert(hist_current <= hist_count);
	if (hist_current >= 0)
	{
		hist_current--;
	}
	if (hist_current == -1)
	{
		return NULL;
	}
	return &(ttyEditLines[hist_current]);
}

/*
==================
CON_SigCont
Reinitialize console input after receiving SIGCONT, as on Linux the terminal seems to lose all
set attributes if user did CTRL+Z and then does fg again.
==================
*/

void CON_SigCont(int signum)
{
	CON_Init();
}

/*
==================
CON_Init

Initialize the console input (tty mode if possible)
==================
*/
void CON_Init( void )
{
	struct termios tc;

	// If the process is backgrounded (running non interactively)
	// then SIGTTIN or SIGTOU is emitted, if not caught, turns into a SIGSTP
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	// If SIGCONT is received, reinitialize console
	signal(SIGCONT, CON_SigCont);

	// Make stdin reads non-blocking
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK );

	if (!stdinIsATTY)
	{
		//Com_Printf("tty console mode disabled\n"); //ioq3-urt: too spammy
		ttycon_on = qfalse;
		stdin_active = qtrue;
		return;
	}

	Field_Clear(&TTY_con);
	tcgetattr (STDIN_FILENO, &TTY_tc);
	TTY_erase = TTY_tc.c_cc[VERASE];
	TTY_eof = TTY_tc.c_cc[VEOF];
	tc = TTY_tc;

	/*
	ECHO: don't echo input characters
	ICANON: enable canonical mode.  This  enables  the  special
	characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
	STATUS, and WERASE, and buffers by lines.
	ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
	DSUSP are received, generate the corresponding sig­
	nal
	*/
	tc.c_lflag &= ~(ECHO | ICANON);

	/*
	ISTRIP strip off bit 8
	INPCK enable input parity checking
	*/
	tc.c_iflag &= ~(ISTRIP | INPCK);
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSADRAIN, &tc);
	ttycon_on = qtrue;
	CON_Hide(); // These two lines cause the tty prompt to
	CON_Show(); // appear right away.  It's a problem on clients.
}

/*
==================
CON_Input
==================
*/
char *CON_Input( void )
{
	// we use this when sending back commands
	static char text[MAX_EDIT_LINE];
	int avail;
	char key;
	field_t *history;
	size_t size;

	if(ttycon_on)
	{
		avail = read(STDIN_FILENO, &key, 1);
		if (avail != -1)
		{
			// we have something
			// backspace?
			// NOTE TTimo testing a lot of values .. seems it's the only way to get it to work everywhere
			if ((key == TTY_erase) || (key == 127) || (key == 8))
			{
				if (TTY_con.cursor > 0)
				{
					TTY_con.cursor--;
					TTY_con.buffer[TTY_con.cursor] = '\0';
					CON_Back();
				}
				return NULL;
			}
			// check if this is a control char
			if ((key) && (key) < ' ')
			{
				if (key == '\n')
				{
#ifndef DEDICATED
					if (TTY_con.cursor &&	// We can either check TTY_con.cursor or
								// TTY_con.buffer[0].  I'm not sure which one
								// is cleaner.  You can decide that.  Same
								// goes for the last boolean below - we could
								// check strlen(TTY_con.buffer) there.
							TTY_con.buffer[0] != '/' &&
							TTY_con.buffer[0] != '\\' &&
							TTY_con.cursor < MAX_EDIT_LINE - 1) {
						// Add backslash to beginning of command to make it
						// clear that only commands are accepted.  Also this is consistent
						// with the autocomplete feature, which adds a backslash.  We don't
						// add the backslash if the command buffer is at capacity because
						// that would truncate the command (poor user experience).  Prepending
						// the backslash is not necessary; it's only to improve user experience.
						text[0] = '\\';
						Q_strncpyz(text + 1, TTY_con.buffer, sizeof(text) - 1);
						Q_strncpyz(TTY_con.buffer, text, sizeof(TTY_con.buffer));
						TTY_con.cursor++;
					}
					else {
						Q_strncpyz(text, TTY_con.buffer, sizeof(text));
					}
					Hist_Add(&TTY_con);
#ifdef TTY_COMMANDS_VISIBLE_IN_GAME
					CON_Hide();
					Com_Printf("%s%s\n", TTY_CONSOLE_PROMPT, TTY_con.buffer);
					Field_Clear(&TTY_con);
					CON_Show();
#else
					CON_Hide(); // make sure the added
					CON_Show(); // backslash shows up
					Field_Clear(&TTY_con);
					key = '\n';
					size = write(STDOUT_FILENO, &key, 1);
					size = write(STDOUT_FILENO, TTY_CONSOLE_PROMPT, strlen(TTY_CONSOLE_PROMPT));
#endif
					if (text[0] == '/' || text[0] == '\\') { // explicit command
						return text + 1;
					}
					else {
						return text;
					}
#else // DEDICATED - Below is the original code before Rambetter's fixes.
					Hist_Add(&TTY_con);
					Q_strncpyz(text, TTY_con.buffer, sizeof(text));
					Field_Clear(&TTY_con);
					key = '\n';
					size = write(STDOUT_FILENO, &key, 1);
					size = write(STDOUT_FILENO, TTY_CONSOLE_PROMPT, strlen(TTY_CONSOLE_PROMPT));
					return text;
#endif
				}
				if (key == '\t')
				{
					CON_Hide();
					Field_AutoComplete( &TTY_con );
					CON_Show();
					return NULL;
				}
				avail = read(STDIN_FILENO, &key, 1);
				if (avail != -1)
				{
					// VT 100 keys
					if (key == '[' || key == 'O')
					{
						avail = read(STDIN_FILENO, &key, 1);
						if (avail != -1)
						{
							switch (key)
							{
								case 'A':
									history = Hist_Prev();
									if (history)
									{
										CON_Hide();
										TTY_con = *history;
										CON_Show();
									}
									CON_FlushIn();
									return NULL;
									break;
								case 'B':
									history = Hist_Next();
									CON_Hide();
									if (history)
									{
										TTY_con = *history;
									} else
									{
										Field_Clear(&TTY_con);
									}
									CON_Show();
									CON_FlushIn();
									return NULL;
									break;
								case 'C':
									return NULL;
								case 'D':
									return NULL;
							}
						}
					}
				}
				Com_DPrintf("droping ISCTL sequence: %d, TTY_erase: %d\n", key, TTY_erase);
				CON_FlushIn();
				return NULL;
			}
			if (TTY_con.cursor >= sizeof(text) - 1)
				return NULL;
			// push regular character
			TTY_con.buffer[TTY_con.cursor] = key;
			TTY_con.cursor++; // next char will always be '\0'
			// print the current line (this is differential)
			size = write(STDOUT_FILENO, &key, 1);
		}

		return NULL;
	}
	else if (stdin_active)
	{
		int     len;
		fd_set  fdset;
		struct timeval timeout;

		FD_ZERO(&fdset);
		FD_SET(STDIN_FILENO, &fdset); // stdin
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if(select (STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(STDIN_FILENO, &fdset))
			return NULL;

		len = read(STDIN_FILENO, text, sizeof(text));
		if (len == 0)
		{ // eof!
			stdin_active = qfalse;
			return NULL;
		}

		if (len < 1)
			return NULL;
		text[len-1] = 0;    // rip off the /n and terminate

		return text;
	}
	return NULL;
}

/*
==================
CON_Print
==================
*/
void CON_Print( const char *msg )
{
	if (!msg[0]) return;
	
	CON_Hide( );

	if( com_ansiColor && com_ansiColor->integer )
		Sys_AnsiColorPrint( msg );
	else
		fputs( msg, stderr );

	// The special logic below prevents printing the tty prompt
	// when this CON_Print() doesn't end with a newline.  The problem
	// with printing the prompt in this case is that the console
	// might get garbled when output does not fit on one line,
	// especially if the prompt is more than one character.
	// Note that regardless of the hide status of the console,
	// a user will always be able to input and execute commands [when the
	// console is turned on].  So even if the newline never arrives,
	// the user will be able to type commands and see them typed
	// after the text on the current line (although not pretty).
	if (!ttycon_on) {
		// Our CON_Hide() above didn't do anything so we can just do nothing here.
		return;
	}
	// From here on it's guaranteed that CON_Hide() did increment ttycon_hide.
	if (msg[strlen(msg) - 1] == '\n') { // ends with newline
		CON_Show();
		while (ttycon_show_overdue > 0) {
			CON_Show();
			ttycon_show_overdue--;
		}
	}
	else {	// This means that ttycon_hide was incremented by CON_Hide() above
		// so we owe the system one CON_Show() later on.
		ttycon_show_overdue++;
	}
}

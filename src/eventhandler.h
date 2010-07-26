#ifndef HAVE_EVENTHANDLER_H
#define HAVE_EVENTHANDLER_H
#include <glib.h>
#include "SDL_mixer.h"
#include <stdio.h>

typedef struct event {
	char* pattern;
	GList* music;
	GList* sfx;
#ifdef TTS
	GList* speech_lines;
#endif
	int stateful;
	//char* music_path;
	//char* sfx_path;
	char* source_file; // Will eventually be used to allow removal of all events
	                   // that come from a single file - unused right now.
} EVENT;
extern GList* events;
extern EVENT* current_stateful_event;
extern EVENT* default_event;
extern FILE* gamelog;
extern Mix_Music* music;
extern int sfx_volume;
extern int music_volume;
void event_iterator(gpointer data,gpointer user_data);
char* gamelog_iterate();
int load_events();
void load_gamelog();

#endif

#ifndef HAVE_EVENTHANDLER_H
#define HAVE_EVENTHANDLER_H
#include <glib.h>
#include "SDL_mixer.h"
#include <stdio.h>

typedef struct event {
	char* pattern;
	GList* music;
	GList* sfx;
	int stateful;
} EVENT;
extern GList* events;
extern EVENT* current_stateful_event;
extern EVENT* default_event;
extern FILE* gamelog;
extern Mix_Music* music;
extern char* music_path;
extern char* sfx_path;
extern int sfx_volume;
extern int music_volume;
void event_iterator(gpointer data,gpointer user_data);
char* gamelog_iterate();
int load_events();
void load_gamelog();

#endif
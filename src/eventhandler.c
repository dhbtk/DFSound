#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <libxml/parser.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "eventhandler.h"
GList* events = NULL;
EVENT* current_stateful_event = NULL;
EVENT* default_event = NULL;
FILE* gamelog = NULL;
Mix_Music* music = NULL;
char* music_path = NULL;
char* sfx_path = NULL;
int sfx_volume = 128;
int music_volume = 128;

void event_iterator(gpointer data,gpointer user_data) {
	EVENT* event = (EVENT*)data;
	char*  line  = (char*)user_data;
	if(g_regex_match_simple(event->pattern,line,0,0)) {
		printf("Found event\n");
		if(event->stateful) {
			current_stateful_event = event;
		}
		if(event->music != NULL) {
			printf("Playing music\n");
			if(Mix_PlayingMusic()) {
				Mix_HaltMusic();
				fprintf(stderr,"Error code: %s",Mix_GetError());
				Mix_FreeMusic(music);
				fprintf(stderr,"Error code: %s",Mix_GetError());
				music = NULL;
			}
			char* fname = (char*)g_list_nth_data(event->music,g_random_int_range(0,g_list_length(event->music)));
			char* path  = g_strdup_printf("%s%s",music_path,fname);
			printf("%s\n",path);
			free(fname);
			music = Mix_LoadMUS(path);
			fprintf(stderr,"Error code: %s",Mix_GetError());
			Mix_VolumeMusic(music_volume);
			fprintf(stderr,"Error code: %s",Mix_GetError());
			Mix_PlayMusic(music,0);
			fprintf(stderr,"Error code: %s",Mix_GetError());
		}
		if(event->sfx != NULL) {
			printf("Playing sfx\n");
			char* fname = (char*)g_list_nth_data(event->sfx,g_random_int_range(0,g_list_length(event->sfx)));
			Mix_Chunk* sfx = Mix_LoadWAV(g_strdup_printf("%s%s",sfx_path,fname));
			Mix_VolumeChunk(sfx,sfx_volume);
			fprintf(stderr,"Error code: %s",Mix_GetError());
			Mix_PlayChannel(-1,sfx,0);
			fprintf(stderr,"Error code: %s",Mix_GetError());
		}
	}
}
char* gamelog_iterate() {
	if(gamelog == NULL) return NULL;
	char* buf = (char*)malloc(sizeof(char)*1024);
	memset(buf,0,sizeof(char)*1024);
	if(fgets(buf,1024,gamelog) != NULL) {
		printf("%s",buf);
		g_list_foreach(events,&event_iterator,buf);
	}
	if(!Mix_PlayingMusic() && current_stateful_event != NULL) {
		if(current_stateful_event->music != NULL) {
			Mix_FreeMusic(music);
			music = NULL;
			char* fname = (char*)g_list_nth_data(current_stateful_event->music,g_random_int_range(0,g_list_length(current_stateful_event->music)));
			music = Mix_LoadMUS(g_strdup_printf("%s%s",music_path,fname));
			Mix_PlayMusic(music,0);
		}
	} else if(!Mix_PlayingMusic() && default_event != NULL) {
		if(default_event->music != NULL) {
			Mix_FreeMusic(music);
			music = NULL;
			char* fname = (char*)g_list_nth_data(default_event->music,g_random_int_range(0,g_list_length(default_event->music)));
			music = Mix_LoadMUS(g_strdup_printf("%s%s",music_path,fname));
			Mix_PlayMusic(music,0);
		}
	}
	return buf;
}

int load_events(char* file) {
	printf("Loading events\n");
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;
	xmlNodePtr child;
	doc = xmlParseFile(file);
	if(doc == NULL) {
		fprintf(stderr,"Events file could not be parsed. Please verify.\n");
		return 1;
	}
	root = xmlDocGetRootElement(doc);
	if(root == NULL) {
		fprintf(stderr,"Events file is empty!\n");
		return 2;
	}
	if(!xmlStrcmp(root->name,(const xmlChar*)"events")) { // DFSound native
		for(node = root->xmlChildrenNode;node != NULL;node = node->next) {
			if(!xmlStrcmp(node->name,(const xmlChar*)"event")) {
				EVENT* curr_event    = malloc(sizeof(EVENT));
				if(xmlGetProp(node,"default") != NULL) {
					default_event = curr_event;
				}
				curr_event->pattern  = (char*)xmlGetProp(node,"pattern");
				curr_event->music    = NULL;
				curr_event->sfx      = NULL;
				curr_event->stateful = 0;
				for(child = node->xmlChildrenNode;child != NULL;child = child->next) {
					if(!xmlStrcmp(child->name,(const xmlChar*)"music")) {
						curr_event->music = g_list_append(curr_event->music,(char*)xmlNodeListGetString(doc,child->xmlChildrenNode,1));
					} else if(!xmlStrcmp(child->name,(const xmlChar*)"sfx")) {
						curr_event->sfx   = g_list_append(curr_event->sfx,(char*)xmlNodeListGetString(doc,child->xmlChildrenNode,1));
					} else if(!xmlStrcmp(child->name,(const xmlChar*)"stateful")) {
						curr_event->stateful = 1;
					}
				}
				events = g_list_append(events,curr_event);
			} else if(!xmlStrcmp(node->name,(const xmlChar*)"music_path")) {
				if(xmlNodeListGetString(doc,node->xmlChildrenNode,1) != NULL) {
					music_path = (char*)xmlNodeListGetString(doc,node->xmlChildrenNode,1);
				}
			} else if(!xmlStrcmp(node->name,(const xmlChar*)"sfx_path")) {
				if(xmlNodeListGetString(doc,node->xmlChildrenNode,1) != NULL) {
					sfx_path = (char*)xmlNodeListGetString(doc,node->xmlChildrenNode,1);
				}
			}
		}
	} else if(!xmlStrcmp(root->name,(const xmlChar*)"sounds")) { // SoundSense file
		for(node = root->xmlChildrenNode;node != NULL;node = node->next) {
			if(!xmlStrcmp(node->name,(const xmlChar*)"sound")) {
				EVENT* curr_event = malloc(sizeof(EVENT));
				// No default events for SoundSense
				curr_event->pattern  = (char*)xmlGetProp(node,"logPattern"); // camelCase sucks
				curr_event->music    = NULL; // No native music support
				curr_event->sfx      = NULL;
				curr_event->stateful = 0; // No stateful events as well
				for(child = node->xmlChildrenNode;child != NULL;child = child->next) {
					if(!xmlStrcmp(child->name,(const xmlChar*)"soundFile")) {
						curr_event->sfx = g_list_append(curr_event->sfx,(char*)xmlGetProp(child,"fileName"));
					}
				}
				events = g_list_append(events,curr_event);
			}
		}
	} else { // Unsupported
		fprintf(stderr,"Unsupported events file.\n");
		return 3;
	}
	char* tmp1 = strdup(file);
	char* path = dirname(tmp1);
	
#ifndef WINDOWS
	music_path = g_strdup_printf("%s/music/",path);
	printf("Music path: %s\n",music_path);
	sfx_path   = g_strdup_printf("%s/sfx/",path);
	printf("SFX path: %s\n",sfx_path);
#else
	music_path = g_strdup_printf("%s\\music\\",path);
	printf("Music path: %s\n",music_path);
	sfx_path   = g_strdup_printf("%s\\sfx\\",path);
	printf("SFX path: %s\n",sfx_path);
#endif
	return 0;
}

void load_gamelog(char* path) {
	printf("Loading game log\n");
	gamelog = fopen(path,"r");
	if(gamelog == NULL) {
		fprintf(stderr,"There was an error opening the game log. Try checking the path.\n");
		exit(EXIT_FAILURE);
	}
	fseek(gamelog,0,SEEK_END);
}

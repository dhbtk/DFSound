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
FILE* gamelog = NULL;
Mix_Music* music = NULL;
char* music_path = "music/";
char* sfx_path = "sfx/";

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
				Mix_FreeMusic(music);
				music = NULL;
			}
			char* fname = (char*)g_list_nth_data(event->music,g_random_int_range(0,g_list_length(event->music)));
			music = Mix_LoadMUS(g_strdup_printf("%s%s",music_path,fname));
			Mix_PlayMusic(music,0);
		}
		if(event->sfx != NULL) {
			printf("Playing sfx\n");
			char* fname = (char*)g_list_nth_data(event->sfx,g_random_int_range(0,g_list_length(event->sfx)));
			Mix_Chunk* sfx = Mix_LoadWAV(g_strdup_printf("%s%s",sfx_path,fname));
			Mix_PlayChannel(-1,sfx,0);
		}
	}
}
void gamelog_iterate() {
	char* buf = (char*)malloc(sizeof(char)*1024);
	if(fgets(buf,1024,gamelog) != NULL) {
		printf("%s",buf);
		g_list_foreach(events,&event_iterator,buf);
	}
	free(buf);
	if(!Mix_PlayingMusic() && current_stateful_event != NULL) {
		if(current_stateful_event->music != NULL) {
			Mix_FreeMusic(music);
			music = NULL;
			char* fname = (char*)g_list_nth_data(current_stateful_event->music,g_random_int_range(0,g_list_length(current_stateful_event->music)));
			music = Mix_LoadMUS(g_strdup_printf("%s%s",music_path,fname));
			Mix_PlayMusic(music,0);
		}
	}
}

void load_events(char* file) {
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;
	xmlNodePtr child;
	doc = xmlParseFile(file);
	if(doc == NULL) {
		fprintf(stderr,"Events file could not be parsed. Please verify.\n");
		exit(EXIT_FAILURE);
	}
	root = xmlDocGetRootElement(doc);
	if(root == NULL) {
		fprintf(stderr,"Events file is empty!\n");
		exit(EXIT_FAILURE);
	}
	for(node = root->xmlChildrenNode;node != NULL;node = node->next) {
		if(!xmlStrcmp(node->name,(const xmlChar*)"event")) {
			EVENT* curr_event    = malloc(sizeof(EVENT));
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
}

void load_gamelog(char* path) {
	gamelog = fopen(path,"r");
	if(gamelog == NULL) {
		fprintf(stderr,"There was an error opening the game log. Try checking the path.\n");
		exit(EXIT_FAILURE);
	}
	fseek(gamelog,0,SEEK_END);
}
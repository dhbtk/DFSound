#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <libxml/parser.h>
#include <errno.h>
#include "SDL.h"
#include "SDL_mixer.h"

GList* events = NULL;
typedef struct event {
	char* pattern;
	GList* music;
	GList* sfx;
	int stateful;
} EVENT;
EVENT* current_stateful_event = NULL;
FILE* gamelog = NULL;
Mix_Music* music = NULL;

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
			music = Mix_LoadMUS(fname);
			Mix_PlayMusic(music,0);
		}
		if(event->sfx != NULL) {
			printf("Playing sfx\n");
			char* fname = (char*)g_list_nth_data(event->sfx,g_random_int_range(0,g_list_length(event->sfx)));
			Mix_Chunk* sfx = Mix_LoadWAV(fname);
			Mix_PlayChannel(-1,sfx,0);
		}
	}
}

void main_loop() {
	while(1) {
		char* buf = (char*)malloc(sizeof(char)*1024);
		if(fgets(buf,1024,gamelog) != NULL) {
			printf("%s",buf);
			g_list_foreach(events,&event_iterator,buf);
		}
		usleep((unsigned int)100);
		free(buf);
		if(!Mix_PlayingMusic() && current_stateful_event != NULL) {
			if(current_stateful_event->music != NULL) {
				Mix_FreeMusic(music);
				music = NULL;
				char* fname = (char*)g_list_nth_data(current_stateful_event->music,g_random_int_range(0,g_list_length(current_stateful_event->music)));
				music = Mix_LoadMUS(fname);
				Mix_PlayMusic(music,0);
			}
		}
	}
}

int main(int argc,char** argv) {
	printf("DFSound 0.1\n");
	printf("Copyright (c) 2010 NieXS - All rights reserved.\n");
	printf("See the file LICENSE for licensing information.\n");
	printf("\n");
	printf("Starting up...\n");
	printf("Starting up SDL...\n");
	SDL_Init(SDL_INIT_AUDIO);
	if(Mix_OpenAudio(44010,AUDIO_S16,2,4096)) {
		fprintf(stderr,"Could not open audio!\n");
		exit(EXIT_FAILURE);
	}
	printf("Loading events...\n");
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;
	xmlNodePtr child;
	doc = xmlParseFile("events.xml");
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
		if(xmlStrcmp(node->name,(const xmlChar*)"event")) continue;
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
	}
	printf("Opening game log...\n");
	gamelog = fopen(argv[1],"r");
	if(gamelog == NULL) {
		fprintf(stderr,"There was an error opening the game log. Try checking the path.\n");
		exit(EXIT_FAILURE);
	}
	fseek(gamelog,0,SEEK_END);
	printf("Ready.\n");
	main_loop();
	return 0;
}
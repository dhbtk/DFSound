#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef GUI_ENABLED
#include <gtk/gtk.h>
#endif
#include "dfsound.h"
#include "eventhandler.h"
#include "SDL.h"
#include "SDL_mixer.h"
#ifdef GUI_ENABLED
GtkWidget* event_file_chooser;
GtkWidget* game_log_chooser;
int destroyed = 0;

static void destroy(GtkWidget* window,gpointer data) {
	destroyed = 1;
}
static void toggle_state(GtkWidget* button,gpointer data) {
	printf("%d\n",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
	if(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(game_log_chooser)) != NULL && gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(event_file_chooser)) != NULL) {
		if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
			if(music != NULL) {
				Mix_HaltMusic();
				Mix_FreeMusic(music);
				music = NULL;
			}
			if(events != NULL) {
				g_list_free(events); // FIXME -- won't free the events themselves
				events = NULL;
			}
			if(gamelog != NULL) {
				fclose(gamelog);
				gamelog = NULL;
			}
		} else {
			if(!load_events(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(event_file_chooser)))) {
				load_gamelog(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(game_log_chooser)));
			} else {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
			}
		}
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
	}
}
static void music_vol_change(GtkWidget* spinner,gpointer data) {
	
}
static void sfx_vol_change(GtkWidget* spinner,gpointer data) {
	
}
static void log_file_changed(GtkWidget* button,gpointer data) {
	
}
static void event_file_changed(GtkWidget* button,gpointer data) {
	
}
static void reload_button_pressed(GtkWidget* button,gpointer data) {
	
}
static void loop_func(gpointer data) {
	GtkWidget* view = g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(data)),0);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	char* string;
	if((string = gamelog_iterate()) != NULL) {
		GtkAdjustment* tmp1 = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(data));
		int scrolled = (int)gtk_adjustment_get_value(tmp1) == (int)gtk_adjustment_get_upper(tmp1) - (int)gtk_adjustment_get_page_size(tmp1);
		GtkTextIter end_iter;
		gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer),&end_iter);
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&end_iter,string,-1);
		if(scrolled) {
			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(view),&end_iter,0,TRUE,0,1);
		}
		free(string);
	}
}

int main(int argc,char** argv) {
	// Starting SDL etc.
	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio(44010,AUDIO_S16,2,4096);
	// Starting up the GUI
	GtkWidget* main_window;
	GtkWidget* table;
	GtkWidget* status_label;
	GtkWidget* status_button;
	GtkWidget* music_volume_spinner;
	GtkWidget* music_volume_label;
	GtkWidget* sfx_volume_spinner;
	GtkWidget* sfx_volume_label;
	GtkWidget* game_log_label;
	GtkWidget* event_file_label;
	GtkWidget* reload_files_button;
	GtkWidget* game_log_view;
	GtkWidget* game_log_scrollwin;
	
	gtk_init(&argc,&argv);
	
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(main_window,240,480);
	g_signal_connect(main_window,"destroy",G_CALLBACK(destroy),NULL);
	gtk_window_set_title(GTK_WINDOW(main_window),g_strdup_printf("DFSound %s",
	                     VERSION));
	
	table = gtk_table_new(9,2,FALSE);
	gtk_container_add(GTK_CONTAINER(main_window),table);
	
	// Status things
	status_label  = gtk_label_new("Status:");
	status_button = gtk_check_button_new();
	g_signal_connect(status_button,"toggled",G_CALLBACK(toggle_state),NULL);
	gtk_table_attach_defaults(GTK_TABLE(table),status_label,0,1,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),status_button,1,2,0,1);
	// SPACER SPACER SPACER
	gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(""),0,2,1,2);
	// Music volume
	music_volume_label = gtk_label_new("Music volume:");
	music_volume_spinner = gtk_spin_button_new_with_range(0.0,128,1.0);
	g_signal_connect(music_volume_spinner,"value-changed",G_CALLBACK(music_vol_change),NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(music_volume_spinner),128);
	gtk_table_attach_defaults(GTK_TABLE(table),music_volume_label,0,1,2,3);
	gtk_table_attach_defaults(GTK_TABLE(table),music_volume_spinner,1,2,2,3);
	// SFX volume
	sfx_volume_label = gtk_label_new("SFX volume:");
	sfx_volume_spinner = gtk_spin_button_new_with_range(0.0,128,1.0);
	g_signal_connect(sfx_volume_spinner,"value-changed",G_CALLBACK(sfx_vol_change),NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(sfx_volume_spinner),128);
	gtk_table_attach_defaults(GTK_TABLE(table),sfx_volume_label,0,1,3,4);
	gtk_table_attach_defaults(GTK_TABLE(table),sfx_volume_spinner,1,2,3,4);
	// Spacer
	gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(""),0,2,4,5);
	// Game log chooser
	game_log_label = gtk_label_new("Game log file:");
	game_log_chooser = gtk_file_chooser_button_new("Select the game log file",GTK_FILE_CHOOSER_ACTION_OPEN);
	GtkFileFilter* game_log_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(game_log_filter,"gamelog.txt");
	gtk_file_filter_set_name(game_log_filter,"gamelog.txt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(game_log_chooser),game_log_filter);
	g_signal_connect(game_log_chooser,"file-set",G_CALLBACK(log_file_changed),NULL);
	gtk_table_attach_defaults(GTK_TABLE(table),game_log_label,0,1,5,6);
	gtk_table_attach_defaults(GTK_TABLE(table),game_log_chooser,1,2,5,6);
	// Events file chooser
	event_file_label = gtk_label_new("Event file:");
	event_file_chooser = gtk_file_chooser_button_new("Select the event file",GTK_FILE_CHOOSER_ACTION_OPEN);
	GtkFileFilter* event_file_filter = gtk_file_filter_new();
	gtk_file_filter_add_mime_type(event_file_filter,"text/xml");
	gtk_file_filter_add_mime_type(event_file_filter,"application/xml");
	gtk_file_filter_set_name(event_file_filter,"XML files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(event_file_chooser),event_file_filter);
	g_signal_connect(event_file_chooser,"file-set",G_CALLBACK(event_file_changed),NULL);
	gtk_table_attach_defaults(GTK_TABLE(table),event_file_label,0,1,6,7);
	gtk_table_attach_defaults(GTK_TABLE(table),event_file_chooser,1,2,6,7);
	// Reload files
	reload_files_button = gtk_button_new_with_label("Reload files");
	g_signal_connect(reload_files_button,"clicked",G_CALLBACK(reload_button_pressed),NULL);
	gtk_table_attach_defaults(GTK_TABLE(table),reload_files_button,0,2,7,8);
	// Game log view
	game_log_view = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(game_log_view),FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(game_log_view),FALSE);
	game_log_scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(game_log_scrollwin),game_log_view);
	gtk_table_attach_defaults(GTK_TABLE(table),game_log_scrollwin,0,2,8,9);
	
	gtk_widget_show_all(main_window);
	
	while(!destroyed) {
		while(gtk_events_pending()) {
			gtk_main_iteration();
			loop_func(game_log_scrollwin);
			usleep(10);
		}
	}
	
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}
#else
int main(int argc,char** argv) {
	if(argc != 3) {
		fprintf(stderr,"Usage: %s [gamelog.txt] [events.xml]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio(44010,AUDIO_S16,2,4096);
	load_gamelog(argv[1]);
	int result;
	if(!(result = load_events(argv[2]))) {
		while(1) {
			char* line = gamelog_iterate();
			if(line != NULL) printf("%s",line);
			free(line);
			usleep(10);
		}
	} else {
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "config.h"
#ifdef WINDOWS
#include <windows.h>
#endif
#include "dfsound.h"
#include "eventhandler.h"
#include "SDL.h"
#include "SDL_mixer.h"

GtkWidget *logfile,*events_file;
gboolean destroyed = FALSE;

static void destroy(GtkWidget* window,gpointer data) {
	destroyed = 1;
}
static void toggle_state(GtkWidget* button,GtkWidget* data) {
	printf("%d\n",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
	if(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(logfile)) != NULL && gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(events_file)) != NULL) {
		if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
			if(music != NULL) {
				Mix_HaltMusic();
				fprintf(stderr,"Error code: %s",Mix_GetError());
				Mix_FreeMusic(music);
				fprintf(stderr,"Error code: %s",Mix_GetError());
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
			gtk_statusbar_pop(GTK_STATUSBAR(data),0);
			gtk_statusbar_push(GTK_STATUSBAR(data),0,"Idle.");
		} else {
			int worked = -1;
			if(!(worked = load_events(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(events_file))))) {
				load_gamelog(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(logfile)));
				gtk_statusbar_pop(GTK_STATUSBAR(data),0);
				gtk_statusbar_push(GTK_STATUSBAR(data),0,"Active.");
			} else {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
				gtk_statusbar_pop(GTK_STATUSBAR(data),0);
				switch(worked) {
					case 1:
						gtk_statusbar_push(GTK_STATUSBAR(data),0,"Events file could not be parsed. Please verify.");
						break;
					case 2:
						gtk_statusbar_push(GTK_STATUSBAR(data),0,"Events file is empty!");
						break;
					case 3:
						gtk_statusbar_push(GTK_STATUSBAR(data),0,"Unsupported events file.");
						break;
				}
			}
		}
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),FALSE);
		gtk_statusbar_pop(GTK_STATUSBAR(data),0);
		gtk_statusbar_push(GTK_STATUSBAR(data),0,"No files are selected!");
	}
}
static void music_vol_change(GtkWidget* spinner,gpointer data) {
	music_volume = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner));
	Mix_VolumeMusic(music_volume);
	fprintf(stderr,"Error code: %s",Mix_GetError());
}
static void sfx_vol_change(GtkWidget* spinner,gpointer data) {
	sfx_volume   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner));
}
static void loop_func(gpointer data) {
	if(destroyed) return;
	GtkWidget* view = g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(data)),0);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	char* string;
	if((string = gamelog_iterate()) != NULL) {
		GtkAdjustment* tmp1 = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(data));
		int scrolled = (int)gtk_adjustment_get_value(tmp1) == (int)gtk_adjustment_get_upper(tmp1) - (int)gtk_adjustment_get_page_size(tmp1);
		GtkTextIter end_iter;
		char* inserted = g_convert(string,-1,"UTF-8","ISO-8859-1",NULL,NULL,NULL);
		gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer),&end_iter);
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&end_iter,inserted,-1);
		//if(scrolled) {
			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(view),&end_iter,0,TRUE,0,1);
		//}
		free(inserted);
		free(string);
	}
}

int main(int argc,char** argv) {
	GtkWidget *window,*main_vbox,*main_hbox,*left_vbox,*right_vbox,*statusbar;
	GtkWidget *music_vol,*sfx_vol,*log_view,*log_scroll,*status_button;
	
	gtk_init(&argc,&argv);
	SDL_Init(SDL_INIT_AUDIO);
#ifdef WINDOWS
	Mix_Init(MIX_INIT_MP3|MIX_INIT_OGG);
	fprintf(stderr,"Error code: %s",Mix_GetError());
#endif
	Mix_OpenAudio(44010,AUDIO_S16,2,4096);
	fprintf(stderr,"Error code: %s",Mix_GetError());
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window,640,320);
	gtk_window_set_title(GTK_WINDOW(window),g_strdup_printf("DFSound %s",VERSION));
	g_signal_connect(window,"destroy",G_CALLBACK(destroy),NULL);
	// Boxen
	main_vbox = gtk_vbox_new(FALSE,0);
	main_hbox = gtk_hbox_new(FALSE,0);
	left_vbox = gtk_vbox_new(FALSE,0);
	right_vbox = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(main_vbox),main_hbox,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(main_hbox),left_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(main_hbox),right_vbox,TRUE,TRUE,0);
	gtk_container_add(GTK_CONTAINER(window),main_vbox);
	
	// Left pane
	gtk_box_pack_start(GTK_BOX(left_vbox),gtk_label_new("Volumes"),FALSE,FALSE,0);
	GtkWidget* music_box = gtk_hbox_new(FALSE,0);
	music_vol = gtk_spin_button_new_with_range(0,128,1);
	g_signal_connect(music_vol,"value-changed",G_CALLBACK(music_vol_change),NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(music_vol),128);
	gtk_box_pack_start(GTK_BOX(music_box),music_vol,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(music_box),gtk_label_new("Music"),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(left_vbox),music_box,FALSE,FALSE,0);
	GtkWidget* sfx_box   = gtk_hbox_new(FALSE,0);
	sfx_vol = gtk_spin_button_new_with_range(0,128,1);
	g_signal_connect(sfx_vol,"value-changed",G_CALLBACK(sfx_vol_change),NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(sfx_vol),128);
	gtk_box_pack_start(GTK_BOX(sfx_box),sfx_vol,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(sfx_box),gtk_label_new("Effects"),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(left_vbox),sfx_box,FALSE,FALSE,0);
	
	gtk_box_pack_start(GTK_BOX(left_vbox),gtk_label_new("Log file:"),FALSE,FALSE,0);
	logfile = gtk_file_chooser_button_new("Select the game log file",GTK_FILE_CHOOSER_ACTION_OPEN);
	GtkFileFilter* game_log_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(game_log_filter,"gamelog.txt");
	gtk_file_filter_set_name(game_log_filter,"gamelog.txt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(logfile),game_log_filter);
	//g_signal_connect(logfile,"file-set",G_CALLBACK(log_file_changed),NULL);
	gtk_box_pack_start(GTK_BOX(left_vbox),logfile,FALSE,FALSE,0);
	
	gtk_box_pack_start(GTK_BOX(left_vbox),gtk_label_new("Events:"),FALSE,FALSE,0);
	events_file = gtk_file_chooser_button_new("Select the event file",GTK_FILE_CHOOSER_ACTION_OPEN);
	GtkFileFilter* event_file_filter = gtk_file_filter_new();
	gtk_file_filter_add_mime_type(event_file_filter,"text/xml");
	gtk_file_filter_add_mime_type(event_file_filter,"application/xml");
	gtk_file_filter_set_name(event_file_filter,"XML files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(events_file),event_file_filter);
	gtk_box_pack_start(GTK_BOX(left_vbox),events_file,FALSE,FALSE,0);
	//g_signal_connect(events_file,"file-set",G_CALLBACK(event_file_changed),NULL);
	
	// Log viewer thing
	log_view = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(log_view),FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view),FALSE);
	log_scroll = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(log_scroll),log_view);
	gtk_box_pack_start(GTK_BOX(right_vbox),log_scroll,TRUE,TRUE,0);
	
	// Toggle button
	status_button = gtk_toggle_button_new_with_label("Active");
	gtk_box_pack_start(GTK_BOX(right_vbox),status_button,FALSE,FALSE,0);
	
	// Status bar
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(main_vbox),statusbar,FALSE,FALSE,0);
	
	g_signal_connect(status_button,"toggled",G_CALLBACK(toggle_state),statusbar);
	// Go!
	gtk_widget_show_all(window);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),0,"Idle.");
	
	while(!destroyed) {
		while(gtk_events_pending()) {
			gtk_main_iteration();
		}
		loop_func(log_scroll);
		usleep(10);
	}
	Mix_CloseAudio();
	fprintf(stderr,"Error code: %s",Mix_GetError());
	SDL_Quit();
	return 0;
}

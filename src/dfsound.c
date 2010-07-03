#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "dfsound.h"
#include "eventhandler.h"
#include "config.h"

static void destroy(GtkWidget* window,gpointer data) {
	gtk_main_quit();
}

int main(int argc,char** argv) {
	// Starting up the GUI
	 GtkWidget* main_window;
	 GtkWidget* main_vbox;
	 GtkWidget* status_label;
	 GtkWidget* toggle_button;
	 GtkWidget* music_volume_spinner;
	 GtkWidget* music_volume_label;
	 GtkWidget* sfx_volume_spinner;
	 GtkWidget* sfx_volume_label;
	 GtkWidget* game_log_label;
	 GtkWidget* game_log_chooser;
	 GtkWidget* event_file_label;
	 GtkWidget* event_file_chooser;
	 GtkWidget* reload_file_button;
	 GtkWidget* view_game_log_button;
	 
	 gtk_init(&argc,&argv);
	 
	 main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	 g_signal_connect(main_window,"destroy",G_CALLBACK(destroy),NULL);
	 gtk_window_set_title(GTK_WINDOW(main_window),g_strdup_printf("DFSound %s",VERSION));
	 gtk_widget_show(main_window);
	 
	 gtk_main();
	 
	 return 0;
}
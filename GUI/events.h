#ifndef EVENT_H
#define EVENT_H

#include <gtk/gtk.h>

#define MARK_RADIUS 4
#define SENSITIVITY 5 
#define MOVE_PACE 5
#define ZOOM_SCALE 2 
#define PLAY_SCALE 4 
#define PLAY_SPEED 10 
#define FIND_SCALE 64 

gboolean forward(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean fast_forward(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean backward(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean fast_backward(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean replay(GtkWidget *widget, GdkEventButton *event, gpointer data);

void enter_from_time(GtkWidget *widget, GtkWidget *entry);
void enter_until_time(GtkWidget *widget, GtkWidget *entry);
void enter_x(GtkWidget *widget, GtkWidget *entry);
void enter_y(GtkWidget *widget, GtkWidget *entry);

void route_selected(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void routelist_tree_selection_changed(GtkTreeSelection *selection, gpointer data);
gboolean select_all_route(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean select_coverage(GtkWidget *widget, GdkEventButton *event, gpointer data);

void trace_selected(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void contact_selected(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);

void roadlist_tree_selection_changed(GtkTreeSelection *selection, gpointer data);
void crosslist_tree_selection_changed(GtkTreeSelection *selection, gpointer data);

void tracelist_tree_selection_changed(GtkTreeSelection *selection, gpointer data);

gboolean select_all_contact(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean select_all_trace(GtkWidget *widget, GdkEventButton *event, gpointer data);

gboolean draw_rsu_topology(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean draw_graph_layout(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean draw_storage_nodes(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean relocate_graph(GtkWidget *widget, GdkEventButton *event, gpointer data);

gboolean draw_forward(gpointer data);
gboolean draw_backward(gpointer data);

gboolean drawing_area_motion_event (GtkWidget  *widget, GdkEventMotion *event, gpointer data);
gboolean drawing_area_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean drawing_area_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data);
gboolean drawing_area_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data);

#endif

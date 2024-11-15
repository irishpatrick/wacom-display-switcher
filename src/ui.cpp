#include "ui.hpp"

namespace ui {
    GtkWidget *MonitorChooser(int width, int height, GtkDrawingAreaDrawFunc drawCallback, GCallback clickCallback) {
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        GtkWidget *canvas = gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(canvas), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(canvas), height);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(canvas), drawCallback, nullptr, nullptr);
        gtk_box_append(GTK_BOX(box), canvas);

        GtkWidget *label = gtk_label_new("Press 'w' to move this window");
        gtk_box_append(GTK_BOX(box), label);

        GtkGesture *click = gtk_gesture_click_new();
        g_signal_connect_object(click, "released", G_CALLBACK(clickCallback), canvas, G_CONNECT_SWAPPED);
        gtk_widget_add_controller(canvas, GTK_EVENT_CONTROLLER(click));

        return box;
    }
} // namespace ui

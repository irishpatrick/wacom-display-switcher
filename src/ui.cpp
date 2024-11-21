#include <iostream>
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

    void errorPopupCallback(GObject *sourceObject, GAsyncResult *result, gpointer data) {
        GError *error;
        gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(sourceObject), result, &error);
        exit(1);
    }

    void ErrorPopup(GtkWindow *parent, const std::string &message) {
        auto dialog = gtk_alert_dialog_new("Error:\n\n%s", message.c_str());
        const char *const labels[] = {"Ok", nullptr};
        gtk_alert_dialog_set_buttons(dialog, labels);
        gtk_alert_dialog_choose(dialog, parent, nullptr, errorPopupCallback, nullptr);
    }
} // namespace ui

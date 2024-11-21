#ifndef UI_HPP
#define UI_HPP

#include <string>

#include <gtk/gtk.h>

namespace ui {
    GtkWidget *MonitorChooser(int, int, GtkDrawingAreaDrawFunc, GCallback);

    void ErrorPopup(GtkWindow *parent, const std::string &message);
}

#endif /* UI_HPP */
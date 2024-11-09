#ifndef UI_HPP
#define UI_HPP

#include <gtk/gtk.h>

namespace ui
{
    GtkWidget *MonitorChooser(int, int, GtkDrawingAreaDrawFunc, GCallback);
}

#endif /* UI_HPP */
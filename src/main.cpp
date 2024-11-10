#include "aabb.hpp"
#include "displays.hpp"
#include "instancemutex.hpp"
#include "wacom.hpp"
#include "ui.hpp"

#ifdef _WDS_ADWAITA
#include <adwaita.h>
#else
#include <gtk/gtk.h>
#endif
#include <gdk/x11/gdkx.h>

#include <cstdio>
#include <format>
#include <iostream>
#include <vector>

static std::vector<std::pair<displays::DisplayMetrics, AABB>> displaysButtons;
static int curDisplay = 0;

static void drawMonitors(cairo_t *cr, int width, int height)
{
    const auto margin = 50;
    const auto padding = 10;
    const auto monitors = displays::GetDisplays();
    const auto spacing = 4;
    std::pair<int, int> min;
    std::pair<int, int> max;
    for (const auto &monitor : monitors)
    {
        min = std::pair<int, int>(std::min(min.first, monitor.offsetX), std::min(min.second, monitor.offsetY));
        max = std::pair<int, int>(std::max(max.first, monitor.offsetX + monitor.width), std::max(max.second, monitor.offsetY + monitor.height));
    }
    const int bbox[4] = {min.first, min.second, max.first - min.first, max.second - min.second};
    const double scaleX = double(width - padding * 2 - spacing * (monitors.size() - 1)) / double(bbox[2]);
    const double scaleY = scaleX;

    displaysButtons.clear();
    int i = 0;
    for (const auto &monitor : monitors)
    {
        const auto x = padding + monitor.offsetX * scaleX + spacing * (i > 0);
        const auto y = padding + monitor.offsetY * scaleY;
        const auto w = monitor.width * scaleX;
        const auto h = monitor.height * scaleY;
        const auto margin = 10;

        displaysButtons.emplace_back(monitor, AABB(x, y, w, h));

        cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
        cairo_rectangle(cr, x, y, w, h);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_rectangle(cr, x + margin, y + margin, w - margin * 2, h - margin * 2);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
        cairo_select_font_face(cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 16);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, monitor.name.c_str(), &extents);
        cairo_move_to(cr, x + padding + extents.x_bearing + spacing, y + padding - extents.y_bearing + spacing);
        cairo_show_text(cr, monitor.name.c_str());

        ++i;
    }
}

static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
    drawMonitors(cr, width, height);
}

static gboolean handle_canvas_click(GtkWidget *widget, int n_press, double x, double y, GtkGestureClick *gesture)
{
    for (const auto &nameBtnPair : displaysButtons)
    {
        const auto name = nameBtnPair.first;
        const auto btn = nameBtnPair.second;
        if (btn.intersects(x, y))
        {
            wacom::SetDisplay(name.GetName());
            gtk_window_close(GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(widget))));
            break;
        }
    }

    return G_SOURCE_CONTINUE;
}

static gboolean handle_keypress(GtkWidget *widget, guint keyval, guint keycode, GdkModifierType state, GtkEventControllerKey *event_controller)
{
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(widget)));
    if (keyval == GDK_KEY_Escape)
    {
        gtk_window_close(window);
        return G_SOURCE_CONTINUE;
    }
    if (keyval != GDK_KEY_w)
    {
        return G_SOURCE_CONTINUE;
    }

    const auto displays = displays::GetDisplays();
    curDisplay = (curDisplay + 1) % displays.size();

    GdkSurface *native = gtk_native_get_surface(GTK_NATIVE(window));
    Window xw = gdk_x11_surface_get_xid(GDK_SURFACE(gtk_native_get_surface(GTK_NATIVE(window))));
    Display *xd = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    XMoveWindow(xd, xw, displays[curDisplay].offsetX, 0);

        return G_SOURCE_CONTINUE;
}

static void activate(GtkApplication *app, gpointer user_data)
{
    const auto monitors = displays::GetDisplays();
    const auto minWidth = 200;
    const auto monitorWidth = monitors.size() * 100;
    const auto width = minWidth + monitorWidth;
    const auto height = displays::EstimateHeight(width);

    GtkWidget *window;

    auto monitorChooser = ui::MonitorChooser(width, height, draw_function, G_CALLBACK(handle_canvas_click));

// window
#ifdef _WDS_ADWAITA
    window = adw_application_window_new(app);
#else
    window = gtk_application_window_new(app);
#endif
    gtk_window_set_title(GTK_WINDOW(window), "Wacom Monitor Mapping Switcher");
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_window_set_resizable(GTK_WINDOW(window), false);

#ifdef _WDS_ADWAITA
    GtkWidget *toolbar_view = adw_toolbar_view_new();
    GtkWidget *header_bar = adw_header_bar_new();
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar_view), header_bar);
    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), monitorChooser);
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), toolbar_view);
#else
    gtk_window_set_child(GTK_WINDOW(window), monitorChooser);
#endif

    gtk_window_present(GTK_WINDOW(window));
    GdkSurface *native = gtk_native_get_surface(GTK_NATIVE(window));
    Window xw = gdk_x11_surface_get_xid(GDK_SURFACE(gtk_native_get_surface(GTK_NATIVE(window))));
    Display *xd = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    const auto mouse = displays::QueryMousePosition();
    XMoveWindow(xd, xw, mouse.first - 100, mouse.second - 100);

    GtkEventController *event_controller = gtk_event_controller_key_new();
    g_signal_connect_object(event_controller, "key-released", G_CALLBACK(handle_keypress), monitorChooser, G_CONNECT_SWAPPED);
    gtk_widget_add_controller(window, GTK_EVENT_CONTROLLER(event_controller));
}

void sig_handler(int sig)
{
    std::cout << "term" << std::endl;
}

int main(int argc, char **argv)
{
    InstanceMutex instance;
    if (!instance.IsHeld())
    {
        return 0;
    }

#ifdef _WDS_ADWAITA
    g_autoptr(AdwApplication) app = nullptr;
    app = adw_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
#else
    g_autoptr(GtkApplication) app = nullptr;
    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
#endif

    signal(SIGTERM, sig_handler);

    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    return g_application_run(G_APPLICATION(app), argc, argv);
}

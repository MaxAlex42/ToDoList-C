#include <gtk/gtk.h>

void on_main_window_destroy() {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, "todo.glade", NULL) == 0) {
        g_printerr("Error loading file: interface.glade\n");
        return 1;
    }

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    if (!window) {
        g_printerr("Unable to find object with id 'main_window'\n");
        return 1;
    }

    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
// gcc `pkg-config --cflags gtk+-3.0` -o ToDoList test.c `pkg-config --libs gtk+-3.0` -rdynamic

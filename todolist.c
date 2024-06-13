#include <gtk/gtk.h>

/*
    Enum to define the columns in the GtkListStore (ListView)
    and holds the number of total columns
*/
enum
{
    COLUMN_DONE,
    COLUMN_TASK,
    COLUMN_DUE,
    NUM_COLUMNS
};

// Callback function for the "destroy" signal of the main window
void on_main_window_destroy()
{
    gtk_main_quit(); // Quits the app
}

// Function to add a new todo item to the GtkListStore
void add_todo_item(GtkListStore *list, gboolean done, const gchar *task, const gchar *due)
{
    GtkTreeIter iter;                     // Iterator to access the row in the List
    gtk_list_store_append(list, &iter);   // Adds a new row to the GtkListStore and sets iter to that row
    gtk_list_store_set(list, &iter,       // Set Values in the row where iter points to
                       COLUMN_DONE, done, // Sets the "done" status
                       COLUMN_TASK, task, // Sets the task description
                       COLUMN_DUE, due,   // Sets the due date of the task
                       -1);               // Marks the end of the variable arguments list
}

/*
    Callback function for the "toggled" signal of the GtkCellRendererToggle.
    In easier words ... this is the function that gets executed, if someone clicks a
    checkbox
*/
void on_toggled(GtkCellRendererToggle *renderer, gchar *path, gpointer user_data)
{
    GtkListStore *store = GTK_LIST_STORE(user_data);
    GtkTreeIter iter;
    gboolean active;

    // Gets the GtkTreeIter corresponding to the given path
    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(store), &iter, path);
    // Gets the current value of the "done" column
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COLUMN_DONE, &active, -1);

    // Toggles the "done" status
    active = !active;
    // Updates the "done" status in the GtkListStore
    gtk_list_store_set(store, &iter, COLUMN_DONE, active, -1);
}

// Main function
int main(int argc, char *argv[])
{
    GtkBuilder *builder;       // Builder object to load the UI from the Glade file
    GtkWidget *window;         // Main application window
    GtkTreeView *tree_view;    // TreeView to display the todo list
    GtkListStore *list_store;  // ListStore to hold the todos
    GtkTreeViewColumn *column; // Variable for the columns in the TreeView
    GtkCellRenderer *renderer; // Variable for the cell renderers

    gtk_init(&argc, &argv); // Initializes the GTK library

    builder = gtk_builder_new(); // Creates a new GtkBuilder object
    // Loads the UI definition from the Glade file
    if (gtk_builder_add_from_file(builder, "todo.glade", NULL) == 0)
    {
        g_printerr("Error loading file: todo.glade\n");
        return 1;
    }

    window = GTK_WIDGET(gtk_builder_get_object(builder, "application")); // Gets the main window object from the builder
    if (!window)
    {
        g_printerr("Unable to find object with id 'application'\n");
        return 1;
    }

    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); // Sets the default size of the window to 800x600

    gtk_builder_connect_signals(builder, NULL); // Connects the signal handlers defined in the Glade file

    tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "items_view")); // Gets the TreeView object from the builder

    // Creates a new GtkListStore with two columns: one for booleans and one for strings
    list_store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);

    /*
        Sets the model for the TreeView meaning that the TreeView will use the list_store as it's "data model"
    */
    gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(list_store));
    // g_object_unref(list_store);                                     // Decreases the reference count of the list store (really nessecary?)

    // Add the checkbox column
    renderer = gtk_cell_renderer_toggle_new();                                                        // Creates a new toggle cell renderer
    g_signal_connect(renderer, "toggled", G_CALLBACK(on_toggled), list_store);                        // Connects the "toggled" signal to the callback function
    column = gtk_tree_view_column_new_with_attributes("Done", renderer, "active", COLUMN_DONE, NULL); // Creates a new column with the toggle renderer
    gtk_tree_view_append_column(tree_view, column);                                                   // Adds the column to the TreeView

    // Add the task column
    renderer = gtk_cell_renderer_text_new();                                                        // Creates a new text cell renderer
    column = gtk_tree_view_column_new_with_attributes("Task", renderer, "text", COLUMN_TASK, NULL); // Creates a new column with the text renderer
    gtk_tree_view_append_column(tree_view, column);                                                 // Adds the column to the TreeView

    // Add the due column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Due", renderer, "text", COLUMN_DUE, NULL);
    gtk_tree_view_append_column(tree_view, column);

    // Add sample todos
    add_todo_item(list_store, FALSE, "Projekt für Programming in C", "July 1, 7:30");
    add_todo_item(list_store, FALSE, "Abgabe für Programming in C vorbereiten", "July 1, 19:00");

    gtk_widget_show_all(window); // Shows the main window and all its child widgets

    gtk_main(); // Enters the GTK main loop

    return 0; // Exits the program
}

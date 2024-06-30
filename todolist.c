#include <gtk/gtk.h>
#include <jansson.h>
#include <time.h>
#include <unistd.h>

#define DATA_FILE "res/json/tasks.json"
#define UI_FILE "res/ui/todo.glade"
#define MAX_DESCRIPTION_LENGTH 255

// Platform-specific macro for exporting symbols
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

/*
    Enum to define the columns in the GtkListStore (ListView)
    and holds the number of total columns
*/
enum
{
    COLUMN_DONE,
    COLUMN_TASK,
    COLUMN_DUE,
    COLUMN_DESCRIPTION,
    NUM_COLUMNS
};

// Function declarations
void save_tasks();
void load_tasks();

// Global variables
GtkWidget *checkbox;               // Checkbox to filter completed todos  
GtkWidget *edit_button;           //  Button to edit a todo                  
GtkWidget *delete_button;        //   Button to delete a todo
GtkListStore *unfinished_list;  // ListStore for unfinished tasks
GtkListStore *finished_list;   // ListStore for finished tasks
GtkBuilder *builder;          // Builder object to load the UI from the Glade file


// Callback function for the "destroy" signal of the main window
void EXPORT on_main_window_destroy()
{
    save_tasks();    // Save tasks before quitting the app
    gtk_main_quit(); // Quits the app
}

gboolean get_selected_task(GtkTreeView *tree_view, GtkTreeModel **model, GtkTreeIter *iter)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    if (gtk_tree_selection_get_selected(selection, model, iter)) {
        return TRUE;
    }
    return FALSE;
}

void EXPORT on_update_cancel_button_clicked()
{
    GtkWidget *update_task_window = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_window"));
    gtk_widget_hide(update_task_window);
}

void EXPORT on_edit_destroy(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *update_task_window = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_window"));
    gtk_widget_hide(update_task_window);
}

void EXPORT on_edit_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!get_selected_task(tree_view, &model, &iter)) {
        return; // No task selected
    }

    gchar *task;
    gchar *due;
    gchar *description;
    gtk_tree_model_get(model, &iter, COLUMN_TASK, &task, COLUMN_DUE, &due, COLUMN_DESCRIPTION, &description, -1);

    GtkWidget *task_window = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_window"));

    g_signal_connect(task_window, "destroy", G_CALLBACK(on_edit_destroy), NULL);

    GtkWidget *task_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_name_entry"));
    gtk_entry_set_text(GTK_ENTRY(task_name_entry), task);

    GtkWidget *due_date_calendar = GTK_WIDGET(gtk_builder_get_object(builder, "update_due_date_calendar"));
    guint year, month, day;
    sscanf(due, "%u-%u-%u", &year, &month, &day);
    gtk_calendar_select_month(GTK_CALENDAR(due_date_calendar), month - 1, year);
    gtk_calendar_select_day(GTK_CALENDAR(due_date_calendar), day);

    GtkWidget *description_view = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_description_entry"));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(description_view));
    gtk_text_buffer_set_text(buffer, description, -1);

    g_free(task);
    g_free(due);
    g_free(description);

    gtk_widget_show_all(task_window);
}

void EXPORT on_delete_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (get_selected_task(tree_view, &model, &iter)) {
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
}

// Function to add a new todo item to the GtkListStore
void add_todo_item(GtkListStore *list, gboolean done, const gchar *task, const gchar *due, const gchar *description)
{
    GtkTreeIter iter;
    gtk_list_store_append(list, &iter);
    gtk_list_store_set(list, &iter,
                       COLUMN_DONE, done,
                       COLUMN_TASK, task,
                       COLUMN_DUE, due,
                       COLUMN_DESCRIPTION, description,
                       -1);
}

void EXPORT on_update_button_clicked(GtkButton *button, gpointer user_data)
{    
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model;
    GtkTreeIter iter;

    // Remove old task from the list store
    if (get_selected_task(tree_view, &model, &iter)) {
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }

    // Get updated task details from the input fields
    GtkWidget *task_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_name_entry"));
    GtkWidget *due_date_calendar = GTK_WIDGET(gtk_builder_get_object(builder, "update_due_date_calendar"));
    GtkWidget *description_view = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_description_entry"));

    const gchar *task_name = gtk_entry_get_text(GTK_ENTRY(task_name_entry));

    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(due_date_calendar), &year, &month, &day);
    gchar due_date[256];
    snprintf(due_date, sizeof(due_date), "%04u-%02u-%02u", year, month + 1, day);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(description_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    gboolean done = FALSE;

    // Update the task in the list store
    //update_todo_item(GTK_LIST_STORE(model), done, task_name, due_date, description);
    add_todo_item(unfinished_list, done, task_name, due_date, description);

    g_free(description);

    // Hide the update task window
    GtkWidget *update_task_window = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_window"));
    gtk_widget_hide(update_task_window);
}

void on_insert_text(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, gpointer data) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *current_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    if (g_utf8_strlen(current_text, -1) + g_utf8_strlen(text, -1) > MAX_DESCRIPTION_LENGTH) {
        g_signal_stop_emission_by_name(buffer, "insert-text");
    }

    g_free(current_text);
}

// Callback function for the "toggled" signal of the checkbox
void EXPORT on_checkbox_toggled(GtkToggleButton *toggle_button, gpointer user_data)
{
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    gboolean show_done_only = gtk_toggle_button_get_active(toggle_button);

    if (show_done_only)
    {
        gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(finished_list));
    }
    else
    {
        gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(unfinished_list));
    }
}

/*
    Callback function for the "toggled" signal of the GtkCellRendererToggle.
    This is the function that gets executed if someone clicks a checkbox
*/
void EXPORT on_toggled(GtkCellRendererToggle *renderer, gchar *path, gpointer user_data)
{
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
    gboolean active;

    // Gets the GtkTreeIter corresponding to the given path
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    // Gets the current value of the "done" column
    gtk_tree_model_get(model, &iter, COLUMN_DONE, &active, -1);

    // Toggles the "done" status
    active = !active;
    // Updates the "done" status in the GtkListStore
    gtk_list_store_set(store, &iter, COLUMN_DONE, active, -1);

    // Move the task to the appropriate list store
    gchar *task;
    gchar *due;
    gchar *description;
    gtk_tree_model_get(model, &iter, COLUMN_TASK, &task, COLUMN_DUE, &due, COLUMN_DESCRIPTION, &description, -1);
    if (active)
    {
        add_todo_item(finished_list, active, task, due, description);
        gtk_list_store_remove(store, &iter);
    }
    else
    {
        add_todo_item(unfinished_list, active, task, due, description);
        gtk_list_store_remove(store, &iter);
    }
    g_free(task);
    g_free(due);
    g_free(description);
}

// Function to handle the response of the task window buttons
void EXPORT on_task_ok_button_clicked(GtkButton *button, gpointer user_data) 
{
    // Get the task name and due date from the window
    GtkWidget *task_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task_name_entry"));
    GtkWidget *due_date_calendar = GTK_WIDGET(gtk_builder_get_object(builder, "due_date_calendar"));
    GtkWidget *description_view = GTK_WIDGET(gtk_builder_get_object(builder, "task_description_entry"));

    const gchar *task_name = gtk_entry_get_text(GTK_ENTRY(task_name_entry));

    guint year, month, day;
    gtk_calendar_get_date(GTK_CALENDAR(due_date_calendar), &year, &month, &day);

    gchar due_date[256];
    snprintf(due_date, sizeof(due_date), "%04u-%02u-%02u", year, month + 1, day);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(description_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *description = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Add the new task to the unfinished list
    add_todo_item(unfinished_list, FALSE, task_name, due_date, description);

    g_free(description);
    GtkWidget *task_window = GTK_WIDGET(gtk_builder_get_object(builder, "task_window"));
    gtk_widget_hide(task_window);
}

void EXPORT on_task_cancel_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *task_window = GTK_WIDGET(gtk_builder_get_object(builder, "task_window"));
    gtk_widget_hide(task_window);
}

// Function to show the task window
void EXPORT on_floating_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *task_window = GTK_WIDGET(gtk_builder_get_object(builder, "task_window"));

    // Reset the task name entry
    GtkWidget *task_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task_name_entry"));
    gtk_entry_set_text(GTK_ENTRY(task_name_entry), "");

    // Reset the calendar to the current date
    GtkWidget *due_date_calendar = GTK_WIDGET(gtk_builder_get_object(builder, "due_date_calendar"));
    GDateTime *now = g_date_time_new_now_local();
    gtk_calendar_select_month(GTK_CALENDAR(due_date_calendar), g_date_time_get_month(now) - 1, g_date_time_get_year(now));
    gtk_calendar_select_day(GTK_CALENDAR(due_date_calendar), g_date_time_get_day_of_month(now));
    g_date_time_unref(now);

    // Get the text view and its buffer
    GtkWidget *description_view = GTK_WIDGET(gtk_builder_get_object(builder, "task_description_entry"));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(description_view));
    
    // Disconnect any existing handler to avoid multiple connections
    g_signal_handlers_disconnect_by_func(buffer, G_CALLBACK(on_insert_text), NULL);
    
    // Connect the "insert-text" signal to the buffer
    g_signal_connect(buffer, "insert-text", G_CALLBACK(on_insert_text), NULL);

    // Clear the description buffer
    gtk_text_buffer_set_text(buffer, "", -1);

    gtk_widget_show_all(task_window);
}

// Function to save the tasks to a JSON file
void save_tasks() {
    json_t *root = json_object();
    json_t *unfinished_tasks = json_array();
    json_t *finished_tasks = json_array();
    GtkTreeIter iter;

    // Save unfinished tasks
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(unfinished_list), &iter);
    while (valid) {
        gboolean done;
        gchar *task;
        gchar *due;
        gchar *description;
        gtk_tree_model_get(GTK_TREE_MODEL(unfinished_list), &iter,
                           COLUMN_DONE, &done,
                           COLUMN_TASK, &task,
                           COLUMN_DUE, &due,
                           COLUMN_DESCRIPTION, &description,
                           -1);
        json_t *task_obj = json_object();
        json_object_set_new(task_obj, "done", json_boolean(done));
        json_object_set_new(task_obj, "task", json_string(task));
        json_object_set_new(task_obj, "due", json_string(due));
        json_object_set_new(task_obj, "description", json_string(description));
        json_array_append_new(unfinished_tasks, task_obj);
        g_free(task);
        g_free(due);
        g_free(description);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(unfinished_list), &iter);
    }
    json_object_set_new(root, "unfinished_tasks", unfinished_tasks);

    // Save finished tasks
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(finished_list), &iter);
    while (valid) {
        gboolean done;
        gchar *task;
        gchar *due;
        gchar *description;
        gtk_tree_model_get(GTK_TREE_MODEL(finished_list), &iter,
                           COLUMN_DONE, &done,
                           COLUMN_TASK, &task,
                           COLUMN_DUE, &due,
                           COLUMN_DESCRIPTION, &description,
                           -1);
        json_t *task_obj = json_object();
        json_object_set_new(task_obj, "done", json_boolean(done));
        json_object_set_new(task_obj, "task", json_string(task));
        json_object_set_new(task_obj, "due", json_string(due));
        json_object_set_new(task_obj, "description", json_string(description));
        json_array_append_new(finished_tasks, task_obj);
        g_free(task);
        g_free(due);
        g_free(description);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(finished_list), &iter);
    }
    json_object_set_new(root, "finished_tasks", finished_tasks);

    // Save to file
    if (json_dump_file(root, DATA_FILE, JSON_INDENT(2)) != 0) {
    g_warning("Failed to save tasks to file");
    }
    json_decref(root);
}

// Function to load tasks from a JSON file
void load_tasks()
{
    json_error_t error;
    json_t *root = json_load_file(DATA_FILE, 0, &error);
    if (!root)
    {
        g_warning("Unable to load tasks from file: %s", error.text);
        return;
    }

    json_t *unfinished_tasks = json_object_get(root, "unfinished_tasks");
    for (size_t i = 0; i < json_array_size(unfinished_tasks); i++)
    {
        json_t *task_obj = json_array_get(unfinished_tasks, i);
        gboolean done = json_boolean_value(json_object_get(task_obj, "done"));
        const gchar *task = json_string_value(json_object_get(task_obj, "task"));
        const gchar *due = json_string_value(json_object_get(task_obj, "due"));
        const gchar *description = json_string_value(json_object_get(task_obj, "description"));
        add_todo_item(unfinished_list, done, task, due, description);
    }

    json_t *finished_tasks = json_object_get(root, "finished_tasks");
    for (size_t i = 0; i < json_array_size(finished_tasks); i++)
    {
        json_t *task_obj = json_array_get(finished_tasks, i);
        gboolean done = json_boolean_value(json_object_get(task_obj, "done"));
        const gchar *task = json_string_value(json_object_get(task_obj, "task"));
        const gchar *due = json_string_value(json_object_get(task_obj, "due"));
        const gchar *description = json_string_value(json_object_get(task_obj, "description"));
        add_todo_item(finished_list, done, task, due, description);
    }

    json_decref(root);
}

int main(int argc, char *argv[])
{
    GtkWidget *window;         // Main application window
    GtkTreeView *tree_view;    // TreeView to display the todo list
    GtkTreeViewColumn *column; // Variable for the columns in the TreeView
    GtkCellRenderer *renderer; // Variable for the cell renderers

    gtk_init(&argc, &argv); // Initializes the GTK library

    builder = gtk_builder_new(); // Creates a new GtkBuilder object
    // Loads the UI definition from the Glade file
    if (gtk_builder_add_from_file(builder, UI_FILE, NULL) == 0)
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

    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 800); // Sets the default size of the window to 1000x800

    gtk_builder_connect_signals(builder, NULL); // Connects the signal handlers defined in the Glade file

    tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "items_view")); // Gets the TreeView object from the builder
    if (!tree_view)
    {
        g_printerr("Unable to find object with id 'items_view'\n");
        return 1;
    }

    // Creates two new GtkListStores: one for unfinished tasks and one for finished tasks
    // Create the GtkListStore with the correct column types
    unfinished_list = gtk_list_store_new(NUM_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    finished_list = gtk_list_store_new(NUM_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);


    // Initially set the tree view model to the unfinished tasks list
    gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(unfinished_list));

    // Add the checkbox column
    renderer = gtk_cell_renderer_toggle_new();                                                        // Creates a new toggle cell renderer
    g_signal_connect(renderer, "toggled", G_CALLBACK(on_toggled), tree_view);                         // Connects the "toggled" signal to the callback function
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

    // Load tasks from the file
    load_tasks();

    // Get the checkbox and connect its toggled signal
    checkbox = GTK_WIDGET(gtk_builder_get_object(builder, "radio_button"));
    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_checkbox_toggled), tree_view);

    // Connect the edit Button with its window
    GtkWidget *edit_button = GTK_WIDGET(gtk_builder_get_object(builder, "edit_button"));
    g_signal_connect(edit_button, "clicked", G_CALLBACK(on_edit_button_clicked), tree_view);

    GtkWidget *update_button = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_button"));
    g_signal_connect(update_button, "clicked", G_CALLBACK(on_update_button_clicked), tree_view);

    GtkWidget *update_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder, "update_task_cancel_button"));
    g_signal_connect(update_cancel_button, "clicked", G_CALLBACK(on_update_cancel_button_clicked), NULL);

    GtkWidget *delete_button = GTK_WIDGET(gtk_builder_get_object(builder, "delete_button"));
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_button_clicked), tree_view);

    // Connect the floating button signal to show the task window
    GtkWidget *floating_button = GTK_WIDGET(gtk_builder_get_object(builder, "floating_button"));
    g_signal_connect(floating_button, "clicked", G_CALLBACK(on_floating_button_clicked), NULL);

    // Connect the task window buttons
    GtkWidget *task_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "task_ok_button"));
    g_signal_connect(task_ok_button, "clicked", G_CALLBACK(on_task_ok_button_clicked), NULL);

    GtkWidget *task_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder, "task_cancel_button"));
    g_signal_connect(task_cancel_button, "clicked", G_CALLBACK(on_task_cancel_button_clicked), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(on_main_window_destroy), NULL);

    gtk_widget_show_all(window); // Shows the main window and all its child widgets

    gtk_main(); // Enters the GTK main loop

    return 0; // Exits the program
}

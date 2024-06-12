# Todo App with GTK and Glade
# Programming in C (700.015, 24S)
Course Taken at AAU-University

## Contributors
- **Arbeitstein Philipp**
- **Sellner Maximilian**

## Project Description
This project is a simple todo application written using the GTK library and Glade for the user interface design. The app allows users to add tasks to a list, mark them as done, and keep track of their todo items in a straightforward graphical interface.

### Features
- **Add Todo Items**: Users can add new tasks to the todo list.
- **Mark Tasks as Done**: Users can mark tasks as completed by toggling a checkbox.
- **Task List View**: Tasks are displayed in a list view, showing both the task description and its completion status.

### Technologies Used
- **[GTK](https://www.gtk.org/)**: A multi-platform toolkit for creating graphical user interfaces.
- **[Glade](https://glade.gnome.org/)**: A RAD tool to enable quick and easy development of user interfaces for the GTK toolkit.

### Files in the Project
- `todo.glade`: The Glade file defining the UI layout for the todo app.
- `todo.c`: The C source file containing the logic for the todo application.

### How to Compile and Run
1. Ensure you have GTK 3 and Glade installed on your system.
2. Clone the repository or download the source files.
3. Open a terminal and navigate to the directory containing `todo.c` and `todo.glade`.
4. Compile the program using the following command:
   ```sh
   gcc `pkg-config --cflags gtk+-3.0` -o ToDoList todo.c `pkg-config --libs gtk+-3.0` -rdynamic 
   ```
   or by running the Make file with
   ```sh
   make
   ```
5. Executing the Program with
   ```sh
   ./ToDoList
   ```

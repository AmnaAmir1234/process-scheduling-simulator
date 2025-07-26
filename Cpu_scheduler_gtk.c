#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_PROCESSES 50
#define MAX_NAME_LEN 20

typedef struct {
    char name[MAX_NAME_LEN];
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    int process_id;
    GdkRGBA color;
} Process;

typedef struct {
    char process_name[MAX_NAME_LEN];
    int start_time;
    int end_time;
    GdkRGBA color;
} GanttBlock;

typedef enum {
    FCFS = 1,
    SJF,
    SRTF,
    PRIORITY,
    ROUND_ROBIN,
    PREEMPTIVE_PRIORITY
} SchedulingAlgorithm;

// Global variables
Process processes[MAX_PROCESSES];
GanttBlock gantt_chart[MAX_PROCESSES * 10];
int process_count = 0;
int gantt_count = 0;
int time_quantum = 2;
int current_time = 0;

// GTK widgets
GtkWidget* main_window;
GtkWidget* notebook;
GtkWidget* process_list_view;
GtkWidget* gantt_drawing_area;
GtkWidget* performance_drawing_area;
GtkWidget* statistics_text_view;
GtkWidget* algorithm_info_text_view;
GtkWidget* comparison_text_view;
GtkListStore* process_list_store;

// Function prototypes
void setup_gui();
void on_add_process_clicked(GtkButton* button, gpointer user_data);
void on_delete_process_clicked(GtkButton* button, gpointer user_data);
void on_run_algorithm_clicked(GtkButton* button, gpointer user_data);
void on_reset_clicked(GtkButton* button, gpointer user_data);
void on_load_sample_clicked(GtkButton* button, gpointer user_data);
void on_show_info_clicked(GtkButton* button, gpointer user_data);
void on_compare_algorithms_clicked(GtkButton* button, gpointer user_data);
gboolean on_gantt_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
gboolean on_performance_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data);
void update_process_list();
void update_statistics();
void simulate_scheduling(SchedulingAlgorithm algo);
void fcfs_scheduling();
void sjf_scheduling();
void srtf_scheduling();
void priority_scheduling();
void round_robin_scheduling();
void preemptive_priority_scheduling();
void calculate_times();
void reset_simulation();
void load_sample_processes();
void assign_process_colors();
void show_algorithm_info(SchedulingAlgorithm algo);
void compare_algorithms();

// Color palette for processes
GdkRGBA process_colors[] = {
    {0.8, 0.2, 0.2, 1.0}, // Red
    {0.2, 0.8, 0.2, 1.0}, // Green
    {0.2, 0.2, 0.8, 1.0}, // Blue
    {0.8, 0.8, 0.2, 1.0}, // Yellow
    {0.8, 0.2, 0.8, 1.0}, // Magenta
    {0.2, 0.8, 0.8, 1.0}, // Cyan
    {0.8, 0.4, 0.2, 1.0}, // Orange
    {0.4, 0.2, 0.8, 1.0}, // Purple
    {0.2, 0.6, 0.4, 1.0}, // Teal
    {0.6, 0.4, 0.2, 1.0}  // Brown
};

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL));

    setup_gui();
    assign_process_colors();

    gtk_widget_show_all(main_window);
    gtk_main();

    return 0;
}

void setup_gui() {
    // Main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "CPU Scheduling Simulator");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1200, 800);
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Main container
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(main_window), main_box);

    // Title
    GtkWidget* title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label),
        "<span size='x-large' weight='bold'>CPU Scheduling Simulator</span>");
    gtk_box_pack_start(GTK_BOX(main_box), title_label, FALSE, FALSE, 10);

    // Control buttons
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 5);

    GtkWidget* add_btn = gtk_button_new_with_label("Add Process");
    GtkWidget* delete_btn = gtk_button_new_with_label("Delete Process");
    GtkWidget* sample_btn = gtk_button_new_with_label("Load Sample");
    GtkWidget* reset_btn = gtk_button_new_with_label("Reset");
    GtkWidget* info_btn = gtk_button_new_with_label("Algorithm Information");
    GtkWidget* compare_btn = gtk_button_new_with_label("Compare Algorithms");
    // Set button colors to grey
    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "button { background-color: #d3d3d3; }", -1, NULL);

    GtkStyleContext* context;
    context = gtk_widget_get_style_context(add_btn);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    context = gtk_widget_get_style_context(delete_btn);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    context = gtk_widget_get_style_context(sample_btn);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    context = gtk_widget_get_style_context(reset_btn);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    context = gtk_widget_get_style_context(info_btn);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    context = gtk_widget_get_style_context(compare_btn);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_box_pack_start(GTK_BOX(button_box), add_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box), delete_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box), sample_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box), reset_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box), info_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box), compare_btn, FALSE, FALSE, 5);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_process_clicked), NULL);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_process_clicked), NULL);
    g_signal_connect(sample_btn, "clicked", G_CALLBACK(on_load_sample_clicked), NULL);
    g_signal_connect(reset_btn, "clicked", G_CALLBACK(on_reset_clicked), NULL);
    g_signal_connect(info_btn, "clicked", G_CALLBACK(on_show_info_clicked), NULL);
    g_signal_connect(compare_btn, "clicked", G_CALLBACK(on_compare_algorithms_clicked), NULL);

    // Algorithm selection
    GtkWidget* algo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), algo_box, FALSE, FALSE, 5);

    GtkWidget* algo_label = gtk_label_new("Algorithm:");
    gtk_box_pack_start(GTK_BOX(algo_box), algo_label, FALSE, FALSE, 5);

    const char* algorithms[] = { "FCFS", "SJF", "SRTF", "Priority", "Round Robin", "Preemptive Priority" };
    for (int i = 0; i < 6; i++) {
        GtkWidget* algo_btn = gtk_button_new_with_label(algorithms[i]);
        // Set algorithm button color to grey
        context = gtk_widget_get_style_context(algo_btn);
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

        gtk_box_pack_start(GTK_BOX(algo_box), algo_btn, FALSE, FALSE, 2);
        g_signal_connect(algo_btn, "clicked", G_CALLBACK(on_run_algorithm_clicked), GINT_TO_POINTER(i + 1));
    }

    // Notebook for tabs
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), notebook, TRUE, TRUE, 5);

    // Process List Tab
    GtkWidget* process_tab = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(process_tab),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    process_list_store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT,
        G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
    process_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(process_list_store));

    const char* column_titles[] = { "Process", "Arrival", "Burst", "Priority", "Start", "Complete", "TAT" };
    for (int i = 0; i < 7; i++) {
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
            column_titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);
    }

    gtk_container_add(GTK_CONTAINER(process_tab), process_list_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), process_tab, gtk_label_new("Processes"));

    // Gantt Chart Tab
    gantt_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(gantt_drawing_area, 800, 400);
    g_signal_connect(gantt_drawing_area, "draw", G_CALLBACK(on_gantt_draw), NULL);

    GtkWidget* gantt_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gantt_scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(gantt_scroll), gantt_drawing_area);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gantt_scroll, gtk_label_new("Gantt Chart"));

    // Performance Matrix Tab
    performance_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(performance_drawing_area, 800, 500);
    g_signal_connect(performance_drawing_area, "draw", G_CALLBACK(on_performance_draw), NULL);

    GtkWidget* perf_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(perf_scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(perf_scroll), performance_drawing_area);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), perf_scroll, gtk_label_new("Performance Matrix"));

    // Statistics Tab
    statistics_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(statistics_text_view), FALSE);
    GtkWidget* stats_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(stats_scroll), statistics_text_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stats_scroll, gtk_label_new("Statistics"));

    // Algorithm Information Tab
    algorithm_info_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(algorithm_info_text_view), FALSE);
    GtkWidget* info_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(info_scroll), algorithm_info_text_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), info_scroll, gtk_label_new("Algorithm Info"));

    // Comparison Tab
    comparison_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(comparison_text_view), FALSE);
    GtkWidget* compare_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(compare_scroll), comparison_text_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), compare_scroll, gtk_label_new("Comparison"));
}

void on_show_info_clicked(GtkButton* button, gpointer user_data) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons("Select Algorithm",
        GTK_WINDOW(main_window), GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Show Info", GTK_RESPONSE_OK, NULL);

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    GtkWidget* label = gtk_label_new("Select an algorithm to view information:");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 5);

    const char* algorithms[] = { "FCFS", "SJF", "SRTF", "Priority", "Round Robin", "Preemptive Priority" };
    GtkWidget* combo = gtk_combo_box_text_new();
    for (int i = 0; i < 6; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), algorithms[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int algo = gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) + 1;
        show_algorithm_info(algo);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 4); // Switch to Algorithm Info tab
    }

    gtk_widget_destroy(dialog);
}

void on_compare_algorithms_clicked(GtkButton* button, gpointer user_data) {
    compare_algorithms();
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 5); // Switch to Comparison tab
}

void show_algorithm_info(SchedulingAlgorithm algo) {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(algorithm_info_text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    const char* title = NULL;
    const char* description = NULL;
    const char* characteristics = NULL;
    const char* advantages = NULL;
    const char* disadvantages = NULL;

    switch (algo) {
    case FCFS:
        title = "First-Come, First-Served (FCFS) Scheduling\n\n";
        description = "Description:\n"
            "FCFS is the simplest CPU scheduling algorithm. The process that requests the CPU first "
            "gets the CPU first. It is implemented using a FIFO queue. When a process enters the "
            "ready queue, its PCB is linked to the tail of the queue. When the CPU is free, it is "
            "allocated to the process at the head of the queue.\n\n";
        characteristics = "Characteristics:\n"
            "- Non-preemptive algorithm\n"
            "- Simple to understand and implement\n"
            "- Uses FIFO queue\n"
            "- Performance is poor as average waiting time is high\n\n";
        advantages = "Advantages:\n"
            "- Simple to understand and implement\n"
            "- No starvation as every process gets chance to execute\n\n";
        disadvantages = "Disadvantages:\n"
            "- Not suitable for time-sharing systems\n"
            "- Poor performance as average waiting time is high\n"
            "- Not optimal for minimizing turnaround time\n";
        break;

    case SJF:
        title = "Shortest Job First (SJF) Scheduling\n\n";
        description = "Description:\n"
            "This algorithm associates with each process the length of its next CPU burst. When the CPU "
            "is available, it is assigned to the process that has the smallest next CPU burst. If two "
            "processes have the same length next CPU burst, FCFS scheduling is used to break the tie.\n\n";
        characteristics = "Characteristics:\n"
            "- Can be either preemptive or non-preemptive\n"
            "- Optimal for minimizing average waiting time\n"
            "- Requires knowledge of next CPU burst length\n"
            "- Difficult to implement in practice\n\n";
        advantages = "Advantages:\n"
            "- Optimal for minimizing average waiting time\n"
            "- Better than FCFS in terms of turnaround time\n\n";
        disadvantages = "Disadvantages:\n"
            "- Difficult to know the length of next CPU request\n"
            "- May starve long processes\n"
            "- Not suitable for interactive systems\n";
        break;

    case SRTF:
        title = "Shortest Remaining Time First (SRTF) Scheduling\n\n";
        description = "Description:\n"
            "This is the preemptive version of SJF scheduling. In SRTF, the execution of the process "
            "can be stopped after certain amount of time. At arrival of every process, the short term "
            "scheduler schedules the process with the least remaining burst time among the list of "
            "available processes and the running process.\n\n";
        characteristics = "Characteristics:\n"
            "- Preemptive version of SJF\n"
            "- More context switches than SJF\n"
            "- Optimal for minimizing average waiting time\n"
            "- Requires knowledge of remaining CPU burst length\n\n";
        advantages = "Advantages:\n"
            "- Optimal for minimizing average waiting time\n"
            "- Better than SJF for interactive systems\n\n";
        disadvantages = "Disadvantages:\n"
            "- Difficult to predict remaining burst time\n"
            "- High overhead due to frequent context switches\n"
            "- May starve long processes\n";
        break;

    case PRIORITY:
        title = "Priority Scheduling\n\n";
        description = "Description:\n"
            "A priority is associated with each process, and the CPU is allocated to the process with "
            "the highest priority. Equal priority processes are scheduled in FCFS order. Priority can "
            "be defined internally (by system) or externally (by user). Lower numbers typically "
            "represent higher priority.\n\n";
        characteristics = "Characteristics:\n"
            "- Can be either preemptive or non-preemptive\n"
            "- Processes with same priority are executed in FCFS order\n"
            "- Priority can be static or dynamic\n"
            "- May suffer from starvation\n\n";
        advantages = "Advantages:\n"
            "- Simple to understand and implement\n"
            "- Good for systems with varying process priorities\n\n";
        disadvantages = "Disadvantages:\n"
            "- May suffer from starvation of low priority processes\n"
            "- Indefinite blocking (starvation) can occur\n"
            "- Not optimal for minimizing turnaround time\n";
        break;

    case ROUND_ROBIN:
        title = "Round Robin (RR) Scheduling\n\n";
        description = "Description:\n"
            "Each process gets a small unit of CPU time (time quantum), usually 10-100 milliseconds. "
            "After this time has elapsed, the process is preempted and added to the end of the ready "
            "queue. If there are n processes in the ready queue and the time quantum is q, then each "
            "process gets 1/n of the CPU time in chunks of at most q time units at once.\n\n";
        characteristics = "Characteristics:\n"
            "- Preemptive algorithm\n"
            "- Performance depends heavily on time quantum\n"
            "- No starvation as each process gets regular CPU time\n"
            "- High context switch overhead if quantum is small\n\n";
        advantages = "Advantages:\n"
            "- Fair allocation of CPU across processes\n"
            "- No starvation as each process gets regular CPU time\n"
            "- Good for time-sharing systems\n\n";
        disadvantages = "Disadvantages:\n"
            "- Performance depends on time quantum size\n"
            "- High context switch overhead if quantum is small\n"
            "- Not optimal for minimizing turnaround time\n";
        break;

    case PREEMPTIVE_PRIORITY:
        title = "Preemptive Priority Scheduling\n\n";
        description = "Description:\n"
            "This is the preemptive version of priority scheduling. In this case, the scheduler will "
            "preempt the CPU if the priority of the newly arrived process is higher than the priority "
            "of the currently running process. Upon preemption, the current process is placed back in "
            "the ready queue (at the head if using same priority).\n\n";
        characteristics = "Characteristics:\n"
            "- Preemptive version of priority scheduling\n"
            "- Higher priority processes get immediate attention\n"
            "- May suffer from starvation of low priority processes\n"
            "- More context switches than non-preemptive version\n\n";
        advantages = "Advantages:\n"
            "- High priority processes get immediate attention\n"
            "- Good for real-time systems\n\n";
        disadvantages = "Disadvantages:\n"
            "- May suffer from starvation of low priority processes\n"
            "- High overhead due to frequent context switches\n"
            "- Not optimal for minimizing turnaround time\n";
        break;
    }

    gtk_text_buffer_insert(buffer, &iter, title, -1);
    gtk_text_buffer_insert(buffer, &iter, description, -1);
    gtk_text_buffer_insert(buffer, &iter, characteristics, -1);
    gtk_text_buffer_insert(buffer, &iter, advantages, -1);
    gtk_text_buffer_insert(buffer, &iter, disadvantages, -1);
}

void compare_algorithms() {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(comparison_text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    const char* comparison_text =
        "Comparison of CPU Scheduling Algorithms\n"
        "======================================\n\n"

        "1. First-Come, First-Served (FCFS)\n"
        "   - Simple to implement\n"
        "   - Non-preemptive\n"
        "   - Poor performance (high average waiting time)\n"
        "   - No starvation\n"
        "   - Not suitable for time-sharing systems\n\n"

        "2. Shortest Job First (SJF)\n"
        "   - Optimal for minimizing average waiting time\n"
        "   - Non-preemptive version\n"
        "   - Difficult to predict next CPU burst\n"
        "   - May starve long processes\n\n"

        "3. Shortest Remaining Time First (SRTF)\n"
        "   - Preemptive version of SJF\n"
        "   - Optimal for minimizing average waiting time\n"
        "   - High context switch overhead\n"
        "   - May starve long processes\n\n"

        "4. Priority Scheduling\n"
        "   - Can be preemptive or non-preemptive\n"
        "   - May suffer from starvation\n"
        "   - Good for systems with varying priorities\n"
        "   - Not optimal for turnaround time\n\n"

        "5. Round Robin (RR)\n"
        "   - Preemptive\n"
        "   - Fair allocation of CPU time\n"
        "   - Performance depends on time quantum\n"
        "   - No starvation\n"
        "   - Good for time-sharing systems\n\n"

        "6. Preemptive Priority Scheduling\n"
        "   - Preemptive version of priority scheduling\n"
        "   - High priority processes get immediate attention\n"
        "   - May suffer from starvation\n"
        "   - Good for real-time systems\n\n"

        "Summary Table:\n"
        "+-------------------+------------+------------+-------------------+----------------+\n"
        "| Algorithm         | Preemptive | Starvation | Average Wait Time | Suitable For   |\n"
        "+-------------------+------------+------------+-------------------+----------------+\n"
        "| FCFS              | No         | No         | High              | Batch systems  |\n"
        "| SJF               | No         | Yes        | Lowest            | Batch systems  |\n"
        "| SRTF              | Yes        | Yes        | Lowest            | Interactive    |\n"
        "| Priority          | Optional   | Yes        | Medium            | Real-time      |\n"
        "| Round Robin       | Yes        | No         | Medium            | Time-sharing   |\n"
        "| Preemptive Prio.  | Yes        | Yes        | Medium            | Real-time      |\n"
        "+-------------------+------------+------------+-------------------+----------------+\n\n"

        "Key Points:\n"
        "- FCFS is simplest but has poor performance\n"
        "- SJF/SRTF give optimal waiting times but hard to implement\n"
        "- Round Robin is fair and good for time-sharing\n"
        "- Priority scheduling is good for real-time systems\n"
        "- Preemptive algorithms have more overhead but better response\n";

    gtk_text_buffer_insert(buffer, &iter, comparison_text, -1);
}

void assign_process_colors() {
    for (int i = 0; i < process_count; i++) {
        processes[i].color = process_colors[i % 10];
    }
}

void on_add_process_clicked(GtkButton* button, gpointer user_data) {
    if (process_count >= MAX_PROCESSES) {
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Maximum number of processes reached!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkWidget* dialog = gtk_dialog_new_with_buttons("Add Process",
        GTK_WINDOW(main_window), GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Add", GTK_RESPONSE_OK, NULL);

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    GtkWidget* name_entry = gtk_entry_new();
    GtkWidget* arrival_entry = gtk_entry_new();
    GtkWidget* burst_entry = gtk_entry_new();
    GtkWidget* priority_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process Name:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Arrival Time:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), arrival_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Burst Time:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), burst_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Priority (1-10):"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), priority_entry, 1, 3, 1, 1);

    // Set default values
    char default_name[20];
    snprintf(default_name, sizeof(default_name), "P%d", process_count + 1);
    gtk_entry_set_text(GTK_ENTRY(name_entry), default_name);
    gtk_entry_set_text(GTK_ENTRY(arrival_entry), "0");
    gtk_entry_set_text(GTK_ENTRY(burst_entry), "5");
    gtk_entry_set_text(GTK_ENTRY(priority_entry), "5");

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        Process* p = &processes[process_count];
        strncpy(p->name, gtk_entry_get_text(GTK_ENTRY(name_entry)), MAX_NAME_LEN - 1);
        p->arrival_time = atoi(gtk_entry_get_text(GTK_ENTRY(arrival_entry)));
        p->burst_time = atoi(gtk_entry_get_text(GTK_ENTRY(burst_entry)));
        p->priority = atoi(gtk_entry_get_text(GTK_ENTRY(priority_entry)));

        if (p->priority < 1) p->priority = 1;
        if (p->priority > 10) p->priority = 10;

        p->remaining_time = p->burst_time;
        p->process_id = process_count + 1;
        p->color = process_colors[process_count % 10];
        p->start_time = -1;
        p->completion_time = 0;
        p->waiting_time = 0;
        p->turnaround_time = 0;
        p->response_time = -1;

        process_count++;
        update_process_list();
    }

    gtk_widget_destroy(dialog);
}

void on_delete_process_clicked(GtkButton* button, gpointer user_data) {
    if (process_count == 0) {
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "No processes to delete!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(process_list_view));
    GtkTreeModel* model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
        int index = gtk_tree_path_get_indices(path)[0];

        // Remove process
        for (int i = index; i < process_count - 1; i++) {
            processes[i] = processes[i + 1];
        }
        process_count--;

        update_process_list();
        gtk_tree_path_free(path);
    }
    else {
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "Please select a process to delete!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_run_algorithm_clicked(GtkButton* button, gpointer user_data) {
    int algo = GPOINTER_TO_INT(user_data);

    if (process_count == 0) {
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "No processes available! Add some processes first.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    if (algo == ROUND_ROBIN) {
        GtkWidget* dialog = gtk_dialog_new_with_buttons("Time Quantum",
            GTK_WINDOW(main_window), GTK_DIALOG_MODAL,
            "Cancel", GTK_RESPONSE_CANCEL,
            "OK", GTK_RESPONSE_OK, NULL);

        GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_container_add(GTK_CONTAINER(content_area), box);

        GtkWidget* label = gtk_label_new("Time Quantum:");
        GtkWidget* entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), "2");

        gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);

        gtk_widget_show_all(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
            time_quantum = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
            if (time_quantum <= 0) time_quantum = 2;
        }
        else {
            gtk_widget_destroy(dialog);
            return;
        }
        gtk_widget_destroy(dialog);
    }

    simulate_scheduling(algo);
    update_process_list();
    update_statistics();
    gtk_widget_queue_draw(gantt_drawing_area);
    gtk_widget_queue_draw(performance_drawing_area);
}

void on_reset_clicked(GtkButton* button, gpointer user_data) {
    reset_simulation();
    update_process_list();
    update_statistics();
    gtk_widget_queue_draw(gantt_drawing_area);
    gtk_widget_queue_draw(performance_drawing_area);
}

void on_load_sample_clicked(GtkButton* button, gpointer user_data) {
    load_sample_processes();
    assign_process_colors();
    update_process_list();
}

void update_process_list() {
    gtk_list_store_clear(process_list_store);

    for (int i = 0; i < process_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(process_list_store, &iter);
        gtk_list_store_set(process_list_store, &iter,
            0, processes[i].name,
            1, processes[i].arrival_time,
            2, processes[i].burst_time,
            3, processes[i].priority,
            4, processes[i].start_time,
            5, processes[i].completion_time,
            6, processes[i].turnaround_time,
            -1);
    }
}

void update_statistics() {
    if (process_count == 0) return;

    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(statistics_text_view));

    char stats_text[2048];
    strcpy(stats_text, "SCHEDULING STATISTICS\n");
    strcat(stats_text, "====================\n\n");

    float total_tat = 0, total_wt = 0, total_rt = 0;

    strcat(stats_text, "Process Details:\n");
    strcat(stats_text, "Process\tAT\tBT\tCT\tTAT\tWT\tRT\n");
    strcat(stats_text, "-------\t--\t--\t--\t---\t--\t--\n");

    for (int i = 0; i < process_count; i++) {
        Process* p = &processes[i];
        char line[128];
        snprintf(line, sizeof(line), "%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
            p->name, p->arrival_time, p->burst_time, p->completion_time,
            p->turnaround_time, p->waiting_time, p->response_time);
        strcat(stats_text, line);

        total_tat += p->turnaround_time;
        total_wt += p->waiting_time;
        total_rt += p->response_time;
    }

    char averages[256];
    snprintf(averages, sizeof(averages),
        "\nAVERAGES:\n"
        "Average Turnaround Time: %.2f\n"
        "Average Waiting Time: %.2f\n"
        "Average Response Time: %.2f\n\n"
        "Legend:\n"
        "AT = Arrival Time\n"
        "BT = Burst Time\n"
        "CT = Completion Time\n"
        "TAT = Turnaround Time\n"
        "WT = Waiting Time\n"
        "RT = Response Time\n",
        total_tat / process_count,
        total_wt / process_count,
        total_rt / process_count);

    strcat(stats_text, averages);

    gtk_text_buffer_set_text(buffer, stats_text, -1);
}

gboolean on_gantt_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    int width = allocation.width;
    int height = allocation.height;

    // Clear background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    if (gantt_count == 0) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 20, height / 2);
        cairo_show_text(cr, "No simulation data available. Run an algorithm first.");
        return FALSE;
    }

    // Find total time
    int total_time = 0;
    for (int i = 0; i < gantt_count; i++) {
        if (gantt_chart[i].end_time > total_time) {
            total_time = gantt_chart[i].end_time;
        }
    }

    // Draw title
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, 20, 25);
    cairo_show_text(cr, "Gantt Chart");

    // Calculate dimensions
    int chart_start_y = 50;
    int chart_height = 40;
    int chart_width = width - 100;
    double time_scale = (double)chart_width / total_time;

    // Draw time axis
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 50, chart_start_y + chart_height);
    cairo_line_to(cr, 50 + chart_width, chart_start_y + chart_height);
    cairo_stroke(cr);

    // Draw Gantt blocks
    for (int i = 0; i < gantt_count; i++) {
        GanttBlock* block = &gantt_chart[i];

        double start_x = 50 + (block->start_time * time_scale);
        double block_width = (block->end_time - block->start_time) * time_scale;

        // Draw colored rectangle
        cairo_set_source_rgba(cr, block->color.red, block->color.green,
            block->color.blue, block->color.alpha);
        cairo_rectangle(cr, start_x, chart_start_y, block_width, chart_height);
        cairo_fill(cr);

        // Draw border
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1);
        cairo_rectangle(cr, start_x, chart_start_y, block_width, chart_height);
        cairo_stroke(cr);

        // Draw process name
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, start_x + 2, chart_start_y + chart_height / 2 + 3);
        cairo_show_text(cr, block->process_name);

        // Draw time labels
        cairo_set_font_size(cr, 8);
        char time_str[10];
        snprintf(time_str, sizeof(time_str), "%d", block->start_time);
        cairo_move_to(cr, start_x, chart_start_y + chart_height + 15);
        cairo_show_text(cr, time_str);
    }

    // Draw final time
    if (gantt_count > 0) {
        cairo_set_font_size(cr, 8);
        char time_str[10];
        snprintf(time_str, sizeof(time_str), "%d", total_time);
        cairo_move_to(cr, 50 + chart_width - 10, chart_start_y + chart_height + 15);
        cairo_show_text(cr, time_str);
    }

    return FALSE;
}

gboolean on_performance_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    int width = allocation.width;
    int height = allocation.height;

    // Clear background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    if (process_count == 0) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 20, height / 2);
        cairo_show_text(cr, "No process data available.");
        return FALSE;
    }

    // Draw title
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, 20, 25);
    cairo_show_text(cr, "Performance Matrix");

    // Calculate chart dimensions
    int chart_start_x = 80;
    int chart_start_y = 60;
    int chart_width = width - 150;
    int chart_height = height - 120;
    int bar_height = chart_height / (process_count * 3 + 2); // 3 metrics per process

    // Find maximum values for scaling
    int max_tat = 0, max_wt = 0, max_rt = 0;
    for (int i = 0; i < process_count; i++) {
        if (processes[i].turnaround_time > max_tat) max_tat = processes[i].turnaround_time;
        if (processes[i].waiting_time > max_wt) max_wt = processes[i].waiting_time;
        if (processes[i].response_time > max_rt) max_rt = processes[i].response_time;
    }
    int max_value = (max_tat > max_wt) ? ((max_tat > max_rt) ? max_tat : max_rt) :
        ((max_wt > max_rt) ? max_wt : max_rt);

    if (max_value == 0) max_value = 1; // Avoid division by zero

    double scale = (double)chart_width / max_value;

    int y_pos = chart_start_y;

    // Draw bars for each process
    for (int i = 0; i < process_count; i++) {
        Process* p = &processes[i];

        // Process name label
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_font_size(cr, 10);
        cairo_move_to(cr, 10, y_pos + bar_height / 2 + 3);
        cairo_show_text(cr, p->name);

        // Turnaround Time bar
        cairo_set_source_rgb(cr, 0.8, 0.2, 0.2); // Red
        cairo_rectangle(cr, chart_start_x, y_pos, p->turnaround_time * scale, bar_height - 2);
        cairo_fill(cr);

        // Value label
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_font_size(cr, 8);
        char val_str[20];
        snprintf(val_str, sizeof(val_str), "TAT: %d", p->turnaround_time);
        cairo_move_to(cr, chart_start_x + p->turnaround_time * scale + 5, y_pos + bar_height / 2 + 3);
        cairo_show_text(cr, val_str);

        y_pos += bar_height;

        // Waiting Time bar
        cairo_set_source_rgb(cr, 0.2, 0.8, 0.2); // Green
        cairo_rectangle(cr, chart_start_x, y_pos, p->waiting_time * scale, bar_height - 2);
        cairo_fill(cr);

        snprintf(val_str, sizeof(val_str), "WT: %d", p->waiting_time);
        cairo_move_to(cr, chart_start_x + p->waiting_time * scale + 5, y_pos + bar_height / 2 + 3);
        cairo_show_text(cr, val_str);

        y_pos += bar_height;

        // Response Time bar
        cairo_set_source_rgb(cr, 0.2, 0.2, 0.8); // Blue
        cairo_rectangle(cr, chart_start_x, y_pos, p->response_time * scale, bar_height - 2);
        cairo_fill(cr);

        snprintf(val_str, sizeof(val_str), "RT: %d", p->response_time);
        cairo_move_to(cr, chart_start_x + p->response_time * scale + 5, y_pos + bar_height / 2 + 3);
        cairo_show_text(cr, val_str);

        y_pos += bar_height + 5;
    }

    // Draw legend
    int legend_y = height - 40;
    cairo_set_font_size(cr, 10);

    cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
    cairo_rectangle(cr, 20, legend_y, 15, 10);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 40, legend_y + 8);
    cairo_show_text(cr, "Turnaround Time");

    cairo_set_source_rgb(cr, 0.2, 0.8, 0.2);
    cairo_rectangle(cr, 150, legend_y, 15, 10);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 170, legend_y + 8);
    cairo_show_text(cr, "Waiting Time");

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.8);
    cairo_rectangle(cr, 260, legend_y, 15, 10);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 280, legend_y + 8);
    cairo_show_text(cr, "Response Time");

    return FALSE;
}

void simulate_scheduling(SchedulingAlgorithm algo) {
    reset_simulation();
    current_time = 0;
    gantt_count = 0;

    switch (algo) {
    case FCFS:
        fcfs_scheduling();
        break;
    case SJF:
        sjf_scheduling();
        break;
    case SRTF:
        srtf_scheduling();
        break;
    case PRIORITY:
        priority_scheduling();
        break;
    case ROUND_ROBIN:
        round_robin_scheduling();
        break;
    case PREEMPTIVE_PRIORITY:
        preemptive_priority_scheduling();
        break;
    }

    calculate_times();
}

void fcfs_scheduling() {
    // Sort processes by arrival time
    for (int i = 0; i < process_count - 1; i++) {
        for (int j = 0; j < process_count - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    current_time = 0;
    for (int i = 0; i < process_count; i++) {
        Process* p = &processes[i];

        if (current_time < p->arrival_time) {
            current_time = p->arrival_time;
        }

        p->start_time = current_time;
        p->completion_time = current_time + p->burst_time;

        // Add to Gantt chart
        strcpy(gantt_chart[gantt_count].process_name, p->name);
        gantt_chart[gantt_count].start_time = current_time;
        gantt_chart[gantt_count].end_time = p->completion_time;
        gantt_chart[gantt_count].color = p->color;
        gantt_count++;

        current_time = p->completion_time;
    }
}

void sjf_scheduling() {
    int completed = 0;
    current_time = 0;

    // Mark all processes as not completed
    int is_completed[MAX_PROCESSES] = { 0 };

    while (completed != process_count) {
        int shortest = -1;
        int min_burst = INT_MAX;

        // Find shortest job among arrived processes
        for (int i = 0; i < process_count; i++) {
            if (!is_completed[i] && processes[i].arrival_time <= current_time) {
                if (processes[i].burst_time < min_burst) {
                    min_burst = processes[i].burst_time;
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            current_time++;
            continue;
        }

        Process* p = &processes[shortest];
        p->start_time = current_time;
        p->completion_time = current_time + p->burst_time;

        // Add to Gantt chart
        strcpy(gantt_chart[gantt_count].process_name, p->name);
        gantt_chart[gantt_count].start_time = current_time;
        gantt_chart[gantt_count].end_time = p->completion_time;
        gantt_chart[gantt_count].color = p->color;
        gantt_count++;

        current_time = p->completion_time;
        is_completed[shortest] = 1;
        completed++;
    }
}

void srtf_scheduling() {
    int completed = 0;
    current_time = 0;
    int is_completed[MAX_PROCESSES] = { 0 };

    // Reset remaining times
    for (int i = 0; i < process_count; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
    }

    while (completed != process_count) {
        int shortest = -1;
        int min_remaining = INT_MAX;

        // Find process with shortest remaining time
        for (int i = 0; i < process_count; i++) {
            if (!is_completed[i] && processes[i].arrival_time <= current_time) {
                if (processes[i].remaining_time < min_remaining) {
                    min_remaining = processes[i].remaining_time;
                    shortest = i;
                }
            }
        }

        if (shortest == -1) {
            current_time++;
            continue;
        }

        Process* p = &processes[shortest];

        // Set start time if first execution
        if (p->start_time == -1) {
            p->start_time = current_time;
        }

        // Execute for 1 time unit
        p->remaining_time--;

        // Add/update Gantt chart entry
        if (gantt_count == 0 || strcmp(gantt_chart[gantt_count - 1].process_name, p->name) != 0) {
            strcpy(gantt_chart[gantt_count].process_name, p->name);
            gantt_chart[gantt_count].start_time = current_time;
            gantt_chart[gantt_count].end_time = current_time + 1;
            gantt_chart[gantt_count].color = p->color;
            gantt_count++;
        }
        else {
            gantt_chart[gantt_count - 1].end_time = current_time + 1;
        }

        current_time++;

        // Mark as completed if finished
        if (p->remaining_time == 0) {
            p->completion_time = current_time;
            is_completed[shortest] = 1;
            completed++;
        }
    }
}

void priority_scheduling() {
    int completed = 0;
    current_time = 0;
    int is_completed[MAX_PROCESSES] = { 0 };

    while (completed != process_count) {
        int highest_priority = -1;
        int min_priority = INT_MAX;

        // Find highest priority process (lower number = higher priority)
        for (int i = 0; i < process_count; i++) {
            if (!is_completed[i] && processes[i].arrival_time <= current_time) {
                if (processes[i].priority < min_priority) {
                    min_priority = processes[i].priority;
                    highest_priority = i;
                }
            }
        }

        if (highest_priority == -1) {
            current_time++;
            continue;
        }

        Process* p = &processes[highest_priority];
        p->start_time = current_time;
        p->completion_time = current_time + p->burst_time;

        // Add to Gantt chart
        strcpy(gantt_chart[gantt_count].process_name, p->name);
        gantt_chart[gantt_count].start_time = current_time;
        gantt_chart[gantt_count].end_time = p->completion_time;
        gantt_chart[gantt_count].color = p->color;
        gantt_count++;

        current_time = p->completion_time;
        is_completed[highest_priority] = 1;
        completed++;
    }
}

void round_robin_scheduling() {
    int completed = 0;
    current_time = 0;
    int queue[MAX_PROCESSES];
    int front = 0, rear = 0;
    int in_queue[MAX_PROCESSES] = { 0 };

    // Reset remaining times
    for (int i = 0; i < process_count; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
    }

    // Add processes that arrive at time 0
    for (int i = 0; i < process_count; i++) {
        if (processes[i].arrival_time == 0) {
            queue[rear++] = i;
            in_queue[i] = 1;
        }
    }

    while (completed != process_count) {
        if (front == rear) {
            // Find next arriving process
            int next_arrival = INT_MAX;
            for (int i = 0; i < process_count; i++) {
                if (processes[i].arrival_time > current_time &&
                    processes[i].arrival_time < next_arrival &&
                    processes[i].remaining_time > 0) {
                    next_arrival = processes[i].arrival_time;
                }
            }
            if (next_arrival != INT_MAX) {
                current_time = next_arrival;
                for (int i = 0; i < process_count; i++) {
                    if (processes[i].arrival_time <= current_time &&
                        !in_queue[i] && processes[i].remaining_time > 0) {
                        queue[rear++] = i;
                        in_queue[i] = 1;
                    }
                }
            }
            continue;
        }

        int current_process = queue[front++];
        Process* p = &processes[current_process];

        // Set start time if first execution
        if (p->start_time == -1) {
            p->start_time = current_time;
        }

        // Execute for time quantum or remaining time, whichever is smaller
        int execution_time = (p->remaining_time < time_quantum) ? p->remaining_time : time_quantum;

        // Add to Gantt chart
        strcpy(gantt_chart[gantt_count].process_name, p->name);
        gantt_chart[gantt_count].start_time = current_time;
        gantt_chart[gantt_count].end_time = current_time + execution_time;
        gantt_chart[gantt_count].color = p->color;
        gantt_count++;

        current_time += execution_time;
        p->remaining_time -= execution_time;

        // Add newly arrived processes to queue
        for (int i = 0; i < process_count; i++) {
            if (processes[i].arrival_time <= current_time &&
                !in_queue[i] && processes[i].remaining_time > 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (p->remaining_time == 0) {
            p->completion_time = current_time;
            in_queue[current_process] = 0;
            completed++;
        }
        else {
            // Add back to queue if not completed
            queue[rear++] = current_process;
        }
    }
}

void preemptive_priority_scheduling() {
    int completed = 0;
    current_time = 0;
    int is_completed[MAX_PROCESSES] = { 0 };

    // Reset remaining times
    for (int i = 0; i < process_count; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
    }

    while (completed != process_count) {
        int highest_priority = -1;
        int min_priority = INT_MAX;

        // Find highest priority process among arrived processes
        for (int i = 0; i < process_count; i++) {
            if (!is_completed[i] && processes[i].arrival_time <= current_time) {
                if (processes[i].priority < min_priority) {
                    min_priority = processes[i].priority;
                    highest_priority = i;
                }
            }
        }

        if (highest_priority == -1) {
            current_time++;
            continue;
        }

        Process* p = &processes[highest_priority];

        // Set start time if first execution
        if (p->start_time == -1) {
            p->start_time = current_time;
        }

        // Execute for 1 time unit
        p->remaining_time--;

        // Add/update Gantt chart entry
        if (gantt_count == 0 || strcmp(gantt_chart[gantt_count - 1].process_name, p->name) != 0) {
            strcpy(gantt_chart[gantt_count].process_name, p->name);
            gantt_chart[gantt_count].start_time = current_time;
            gantt_chart[gantt_count].end_time = current_time + 1;
            gantt_chart[gantt_count].color = p->color;
            gantt_count++;
        }
        else {
            gantt_chart[gantt_count - 1].end_time = current_time + 1;
        }

        current_time++;

        // Mark as completed if finished
        if (p->remaining_time == 0) {
            p->completion_time = current_time;
            is_completed[highest_priority] = 1;
            completed++;
        }
    }
}

void calculate_times() {
    for (int i = 0; i < process_count; i++) {
        Process* p = &processes[i];
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;

        // Response time is start time - arrival time
        if (p->start_time != -1) {
            p->response_time = p->start_time - p->arrival_time;
        }
        else {
            p->response_time = 0;
        }

        // Ensure non-negative values
        if (p->waiting_time < 0) p->waiting_time = 0;
        if (p->response_time < 0) p->response_time = 0;
    }
}

void reset_simulation() {
    gantt_count = 0;
    current_time = 0;

    for (int i = 0; i < process_count; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
        processes[i].completion_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].response_time = -1;
    }

    // Clear statistics
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(statistics_text_view));
    gtk_text_buffer_set_text(buffer, "Run a scheduling algorithm to see statistics.", -1);
}

void load_sample_processes() {
    process_count = 5;

    // Sample process data
    strcpy(processes[0].name, "P1");
    processes[0].arrival_time = 0;
    processes[0].burst_time = 6;
    processes[0].priority = 3;

    strcpy(processes[1].name, "P2");
    processes[1].arrival_time = 1;
    processes[1].burst_time = 4;
    processes[1].priority = 1;

    strcpy(processes[2].name, "P3");
    processes[2].arrival_time = 2;
    processes[2].burst_time = 3;
    processes[2].priority = 4;

    strcpy(processes[3].name, "P4");
    processes[3].arrival_time = 3;
    processes[3].burst_time = 2;
    processes[3].priority = 2;

    strcpy(processes[4].name, "P5");
    processes[4].arrival_time = 4;
    processes[4].burst_time = 5;
    processes[4].priority = 5;

    // Initialize other fields
    for (int i = 0; i < process_count; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].process_id = i + 1;
        processes[i].start_time = -1;
        processes[i].completion_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].response_time = -1;
        processes[i].color = process_colors[i % 10];
    }
}

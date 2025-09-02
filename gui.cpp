// gui.cpp
#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <array>
#include <cstdio>
#include <vector>   // <<--- Add this line

// Structure to hold pointers for callbacks
struct CallbackData {
    GtkWidget* text_view;
    GtkEntry* url_entry;
    std::string tool_name;
};

// Add this after the existing CallbackData struct
struct RunAllData {
    GtkWidget* text_view;
    GtkEntry* url_entry;
    std::vector<std::string> tools;
};

// Function to run an external binary and capture output (stderr + stdout)
std::string run_tool(const std::string& tool, const std::string& url) {
    // Redirect stderr (2) to stdout (1) with 2>&1
    std::string command = "./" + tool + " " + url + " 2>&1";

    std::array<char, 256> buffer;
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "Failed to run " + tool;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);
    return result;
}


// Callback for tool buttons
static void on_tool_clicked(GtkButton* button, gpointer user_data) {
    CallbackData* data = static_cast<CallbackData*>(user_data);
    const gchar* url = gtk_entry_get_text(data->url_entry);

    // Clear text buffer
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    // Run the tool and display output
    std::string output = run_tool(data->tool_name, url);
    gtk_text_buffer_set_text(buffer, output.c_str(), -1);
}

// New callback for “Run All”
static void on_run_all_clicked(GtkButton* button, gpointer user_data) {
    RunAllData* data = static_cast<RunAllData*>(user_data);
    const gchar* url = gtk_entry_get_text(data->url_entry);

    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    std::string all_output;

    for (const auto& tool : data->tools) {
        // Header
        all_output += "// " + tool + " =================================================\n\n";
        // Tool output
        all_output += run_tool(tool, url);
        // Six newlines after each tool
        all_output += "\n\n\n\n\n\n";
    }

    gtk_text_buffer_set_text(buffer, all_output.c_str(), -1);
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    std::cout.setf(std::ios::unitbuf); // disables buffering


    // Main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Web Dive Toolset GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // Vertical box
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // URL entry
    GtkWidget* url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(url_entry), "Enter URL here...");
    gtk_box_pack_start(GTK_BOX(vbox), url_entry, FALSE, FALSE, 5);

    // Button box
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 5);


    // Scrollable text view
    GtkWidget* scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 5);

    GtkWidget* text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    



    // Tool buttons
    std::vector<std::string> tools = {
        "dns", "tls", "header", "cookies", "html_body", "redirect", "packets"
    };

    for (const auto& tool : tools) {
        GtkWidget* button = gtk_button_new_with_label(tool.c_str());
        gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 2);

        // Allocate callback data
        CallbackData* data = new CallbackData{ text_view, GTK_ENTRY(url_entry), tool };
        g_signal_connect(button, "clicked", G_CALLBACK(on_tool_clicked), data);
    }

        // Create “Run All” button
    GtkWidget* run_all_button = gtk_button_new_with_label("Run All");
    gtk_box_pack_start(GTK_BOX(button_box), run_all_button, TRUE, TRUE, 2);

    // Allocate RunAllData
    RunAllData* run_all_data = new RunAllData{ text_view, GTK_ENTRY(url_entry), tools };


    // Connect the callback
    g_signal_connect(run_all_button, "clicked", G_CALLBACK(on_run_all_clicked), run_all_data);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}


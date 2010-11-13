/*
 * author: Krzysztof Bochenek
 * email: krzychusan@gmail.com
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <time.h>

#include "config.h"

static void getText(GtkWidget *widget, GtkWidget *entry);
static void getMoreText(GtkWidget *widget, GtkWidget *entry);
static gboolean escapePressed (GtkWidget *window, GdkEventKey *event);
static gboolean tabPressed (GtkWidget *window, GdkEventKey *event);
static void setStatusbar(const char* text);

/* Declare components */
GtkWidget *window;
GtkWidget *vbox;
GtkWidget *inputText;
GtkWidget *outputText;
GtkWidget *additionalText;
GtkWidget *infoLabel;
GtkWidget *statusbar;
GtkWidget *scrolledWindow;
guint statusbarContID;
GtkTextBuffer * bufforText;
GtkTextBuffer * bufforMoreText;

char statbar[100];  //Used to print time in statusbar
char buffor[BUFFOR_SIZE];
struct timeval t1, t2;

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* Create components */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    vbox = gtk_vbox_new(FALSE, 1);
    inputText = gtk_entry_new();
    outputText = gtk_text_view_new();
    additionalText = gtk_text_view_new();
    infoLabel = gtk_label_new("English -> Polish");

    /* Window properties */
    gtk_widget_set_size_request (GTK_WIDGET (window), WIDTH, HEIGHT);
    gtk_window_set_title(GTK_WINDOW (window), "Translator");
    gtk_window_move(GTK_WINDOW (window), POSX, POSY);
    gtk_window_set_icon_from_file(GTK_WINDOW (window), "icon.jpg", NULL);

    /* Scrollbar properties */
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledWindow),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    /* Statusbar */
    statusbar = gtk_statusbar_new();
    statusbarContID = gtk_statusbar_get_context_id(
                            GTK_STATUSBAR (statusbar),
                            "Status bar");
    gtk_statusbar_set_has_resize_grip(
                            GTK_STATUSBAR (statusbar),
                            FALSE);

    /* outputText properties */
    bufforText = gtk_text_buffer_new(NULL);
    gtk_text_view_set_editable(GTK_TEXT_VIEW (outputText), FALSE);

    /* additionalText properties */
    bufforMoreText = gtk_text_buffer_new(NULL);
    gtk_text_view_set_editable(GTK_TEXT_VIEW (additionalText), FALSE);

    /* Adding components to window */
    gtk_container_add(GTK_CONTAINER (window), vbox);
    gtk_box_pack_start(GTK_BOX (vbox), infoLabel, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (vbox), inputText, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (vbox), outputText, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (vbox), scrolledWindow, TRUE, TRUE, 0);
    gtk_scrolled_window_add_with_viewport(
                    GTK_SCROLLED_WINDOW (scrolledWindow),
                    additionalText);
    gtk_box_pack_end(GTK_BOX (vbox), statusbar, FALSE, TRUE, 0);
    gtk_widget_show(vbox);
    gtk_widget_show(infoLabel);
    gtk_widget_show(inputText);
    gtk_widget_show(outputText);
    gtk_widget_show(statusbar);

    /* CALLBACKS */
    /* It's a good idea to do this for all windows. */
    g_signal_connect(window, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);
    /* called when enter is pressed (translation required) */
    g_signal_connect(inputText, "activate",
                    G_CALLBACK (getText), inputText);
    /* escape character for window pressed */
    g_signal_connect(GTK_OBJECT (window), "key_press_event",
                    G_CALLBACK (escapePressed), NULL);
    /* tab character for window released */
    g_signal_connect(GTK_OBJECT (window), "key_release_event",
                    G_CALLBACK (tabPressed), NULL);

    gtk_widget_show(window);
    setStatusbar("Ready!");
    gtk_main ();

    return 0;
}

static void
setStatusbar(const char* text)
{
    gtk_statusbar_push(GTK_STATUSBAR(statusbar),
                        statusbarContID,
                        text);
}

static gboolean
escapePressed (GtkWidget *window, GdkEventKey *event)
{
    if (event->keyval == GDK_Escape) 
    {
        gtk_widget_destroy (window);
    } 
    return FALSE;
}

static gboolean
tabPressed (GtkWidget *window, GdkEventKey *event)
{
    if (event->keyval == GDK_Tab)
    {
        getMoreText(window, inputText);
        gtk_widget_grab_focus(inputText);
    }
    return FALSE;
}

static void
getText(GtkWidget *widget, GtkWidget *entry)
{
    gettimeofday(&t1, NULL);
    setStatusbar("Start");
    const gchar *entry_text;
    entry_text = gtk_entry_get_text (GTK_ENTRY (entry));

    int ans;
    if ((ans=connector("ajax.googleapis.com",
                    buildRequest(BASIC, entry_text, buffor, BUFFOR_SIZE))) != 0)
    {
        printf("Encountered error %d \n", ans);
    }
    else
    {
        entry_text = getBasicText(buffor);
        if (entry_text == NULL) {
            setStatusbar("Translation failed!");
            return;
        }
    gtk_text_buffer_set_text(bufforText, entry_text, strlen(entry_text));
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(outputText), bufforText);
    gettimeofday(&t2, NULL);
    double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    sprintf(statbar, "Translated in %.fms", elapsedTime );
    setStatusbar(statbar);
    }
}

static void
getMoreText(GtkWidget *widget, GtkWidget *entry)
{
    gettimeofday(&t1, NULL);
    setStatusbar("Start");
    gchar *entry_text;
    entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
    memset(buffor, '\0', BUFFOR_SIZE);

    int ans;
    if ((ans=connector("translate.google.com",
                    buildRequest(ADDITIONAL, entry_text, buffor, BUFFOR_SIZE)) != 0))
    {
        printf("Encountered error %d \n", ans);
    }
    else
    {
        entry_text = getAdditionalText(buffor);
        if (entry_text == NULL) {
            setStatusbar("Translation failed!");
            return;
        }

        entry_text = g_convert(entry_text, 
                        strlen(entry_text),
                        "UTF-8",
                        "ISO-8859-2",
                        NULL,
                        NULL,
                        NULL);
    int i, len=strlen(entry_text), h=0;
    for(i=0; i<len; i++)
    {
        if (entry_text[i] == ',')
        {
            entry_text[i] = '\n';
            ++h;
        }
        else if (entry_text[i] == ' ' || entry_text[i] == '[' || 
                    entry_text[i] == ']' || entry_text[i] == '"')
            entry_text[i] = ' ';

    }  

    gtk_text_buffer_set_text(bufforMoreText, entry_text, strlen(entry_text));
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(additionalText), bufforMoreText);
    gtk_box_set_child_packing(GTK_BOX(vbox), outputText, FALSE, FALSE, 0, GTK_PACK_START);
    gtk_window_resize(GTK_WINDOW(window), WIDTH, HEIGHT+HEIGHT_LINE*h);
    gtk_widget_show(additionalText);
    gtk_widget_show(scrolledWindow);

    gettimeofday(&t2, NULL);
    double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    sprintf(statbar, "Translated in %.fms", elapsedTime );
    setStatusbar(statbar);
    }
}

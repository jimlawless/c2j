#include <gtk/gtk.h>

// This callback runs only when the clipboard owner changes
void on_owner_change(GtkClipboard *clipboard, GdkEvent *event, gpointer data) {
    if (gtk_clipboard_wait_is_image_available(clipboard)) {
        GdkPixbuf *original_pixbuf = gtk_clipboard_wait_for_image(clipboard);
        
        if (original_pixbuf) {
            int width = gdk_pixbuf_get_width(original_pixbuf);
            int height = gdk_pixbuf_get_height(original_pixbuf);
            int new_width = 600;
            
            // Only resize if the image is actually wider than 600px
            if (width > new_width) {
                // Calculate proportional height: (original_height / original_width) * new_width
                int new_height = (int)(((double)height / width) * new_width);

                g_print("Resizing from %dx%d to %dx%d...\n", width, height, new_width, new_height);

                // Create the scaled version (GDK_INTERP_BILINEAR is good for photos)
                GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(original_pixbuf, 
                                                                  new_width, 
                                                                  new_height, 
                                                                  GDK_INTERP_BILINEAR);

                // Save as JPEG
                GError *error = NULL;
                gdk_pixbuf_save(scaled_pixbuf, "captured_image.jpg", "jpeg", &error, 
                                "quality", "85", NULL);

                if (error) {
                    g_printerr("Save failed: %s\n", error->message);
                    g_error_free(error);
                } else {
                    g_print("Saved resized image to captured_image.jpg\n");
                }

                g_object_unref(scaled_pixbuf);
            } else {
                // If smaller than 600px, just save the original
                gdk_pixbuf_save(original_pixbuf, "captured_image.jpg", "jpeg", NULL, "quality", "85", NULL);
                g_print("Image small enough; saved original.\n");
            }

            g_object_unref(original_pixbuf);
        }
    }
}

int main(int argc, char *argv[]) {
    gtk_init(NULL,NULL);

    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

    // Connect the "owner-change" signal to our callback function
    g_signal_connect(clipboard, "owner-change", G_CALLBACK(on_owner_change), NULL);

    g_print("Monitoring clipboard... Press Ctrl+C to stop.\n");

    // Enter the GTK main loop (this replaces your while(1) sleep(1) loop)
    gtk_main();

    return 0;
}

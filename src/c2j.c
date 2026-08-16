// c2j - Clipboard (images) to JPEG files
// by Jim Lawless https://github.com/jimlawless
// MIT / X11 license
// GDK interface portions by Gemini

#include <ctype.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int  parse_command_line(int,char **);
void save_clipboard(GtkClipboard *, GdkEvent *, gpointer);
void syntax();

int  _count;
char *_filename_prefix;
int _max_height;
int _max_width;

int _major_version=0;
int _minor_version=80;

int main(int argc,char **argv) {

    gtk_init(NULL,NULL);
    g_print("\nClipboard to JPEG v%d.%d by Jim Lawless (and Gemini)\n\n",_major_version,_minor_version);

    if(!parse_command_line(argc,argv)) {
        syntax();
        return 1;
    }
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    g_signal_connect(clipboard, "owner-change", G_CALLBACK(save_clipboard), NULL);
    
    g_print("Monitoring clipboard... Press Ctrl+C to stop.\n");
    gtk_main();
    return 0;
}   

int parse_command_line(int argc,char **argv) {
    int opt;
    _filename_prefix=NULL;
    _count=1;
    _max_width=600;
    _max_height=4000;

    while((opt=getopt(argc,argv,"p:c:x:y:h"))!=-1) {
       switch(opt) {
           case 'p':
               _filename_prefix=(char *)malloc(strlen(optarg)+1);
               if(_filename_prefix==NULL) {
                   g_printerr("Unable to allocate filename prefix string!");
                   return 0;
               }
               strcpy(_filename_prefix,optarg);
               break;
 
           case 'c':
               _count=atoi(optarg);
               break;

           case 'x':
               _max_width=atoi(optarg);
               break;
           case 'y':
               _max_height=atoi(optarg);
               break;
           case 'h':
               return 1;
           default:
               return 0;
       }
    }
    if(_filename_prefix==NULL) {
       return 0;
    }
    return 1;
}

void save_clipboard(GtkClipboard *clipboard, GdkEvent *event, gpointer data) {
    static char filename[128];
    static int have_filename=0;
    static volatile int in_progress=0;

    if(in_progress)
        return;
    in_progress=1;
    if(!have_filename) {
        for(;;) { 
            sprintf(filename, "%s_%02d.jpg", _filename_prefix, _count);
            if(access(filename,F_OK)==-1) {
                have_filename=1;
                break;
            }
            _count++;
        }
    }
    if (gtk_clipboard_wait_is_image_available(clipboard)) {
        GdkPixbuf *original_pixbuf = gtk_clipboard_wait_for_image(clipboard);
        
        if (original_pixbuf) {
            int width = gdk_pixbuf_get_width(original_pixbuf);
            int height = gdk_pixbuf_get_height(original_pixbuf);
            int new_width = _max_width;
            
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
                gdk_pixbuf_save(scaled_pixbuf, filename, "jpeg", &error, 
                                "quality", "85", NULL);

                if (error) {
                    g_printerr("Save failed: %s\n", error->message);
                    g_error_free(error);
                } else {
                    g_print("Saved resized image to %s\n",filename);
                    have_filename=0;
                }

                g_object_unref(scaled_pixbuf);
            } else {
                // If smaller than 600px, just save the original
                gdk_pixbuf_save(original_pixbuf, filename, "jpeg", NULL, "quality", "85", NULL);
                g_print("Image small enough; saved original to %s.\n",filename);
                have_filename=0;
            }

            g_object_unref(original_pixbuf);
        }
    }
    in_progress=0;
}

void syntax() {
    g_printerr("Syntax:\nc2j [options]\n\n...where options are...\n\n");
    g_printerr("  -p filename_prefix   # output filename prefix for image files (required)\n");
    g_printerr("  -c number            # starting number to use for counter in file suffix\n");
    g_printerr("  -x max_width         # maximum width of output image\n");
    g_printerr("  -y max_height        # maximum height of output image\n");
    g_printerr("  -h                   # this output screen (help)\n");
    g_printerr("\n");

}

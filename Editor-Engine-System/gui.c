#include <gtk/gtk.h>
#include <glade/glade.h>

GladeXML *GUI;


int

main(int argc, char **argv)
{

gtk_init(&argc, &argv);

/* load the interface */

GUI = glade_xml_new("frame.glade",NULL,NULL);

      /*connect the signals in the interface */

      glade_xml_signal_autoconnect(GUI);

      /*Start the event loop */

      gtk_main();

      return 0;
}

void

on_button_clicked(GtkWidget *widget,gpointer data)

        {
               //button event.
}


wrapper     GtkEntry     GtkComboBoxEntry      GtkButton


gtk_entry_text()  //to get text frame input


gtk_entry_set_text()   //to printf windows text frame score.

gtk_combo_box_get_active_text()


glade_xml_get_widget(GUI, "widget name");

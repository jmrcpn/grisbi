/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger    (cedric@grisbi.org)         */
/*          2006-2007 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "export.h"
#include "dialog.h"
#include "export_csv.h"
#include "grisbi_win.h"
#include "gsb_assistant.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_file.h"
#include "qif.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GSList *selected_accounts = NULL;
static GSList *exported_accounts = NULL;

/* bouton pour les options US de l'export QIF */
static 	GtkWidget *box_US;


/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void export_resume_maybe_sensitive_next (GtkWidget *assistant)
{
    GtkWidget *button_select;

    button_select = g_object_get_data (G_OBJECT (assistant), "button_select");
    gtk_widget_show (button_select);

    if (selected_accounts && g_slist_length (selected_accounts))
    {
        gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), TRUE);
    }
    else
    {
        gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), FALSE);
    }
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void export_account_toggled (GtkCellRendererToggle *cell,
									gchar *path_str,
									GtkTreeModel * model)
{
    GtkWidget * assistant;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter;
    gboolean toggle_item;
    gint account_toggled;

    assistant = g_object_get_data (G_OBJECT (model), "assistant");

    /* get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(model),
						&iter,
						0, &toggle_item,
						2, &account_toggled,
						-1);

    if (!toggle_item)		/* We test on _previous_ value */
    {
		selected_accounts = g_slist_append (selected_accounts, GINT_TO_POINTER(account_toggled));
    }
    else
    {
		selected_accounts = g_slist_remove (selected_accounts, GINT_TO_POINTER(account_toggled));
    }

    gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, !toggle_item, -1);

    /* clean up */
    gtk_tree_path_free (path);

    export_resume_maybe_sensitive_next (assistant);
}

/**
 * Callback of the file format menu in the export dialog.
 *
 * It responsible to change the default value of filename in the
 * selector.
 *
 * \param combo		Combo box that triggered event.
 * \param account	A pointer to a structure representing attached
 *			account.
 *
 * \return FALSE
 **/
static gboolean export_account_change_format (GtkWidget *combo,
											  struct ExportedAccount *account)
{
    const gchar *title;
	gchar *tmp_str;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    switch (gtk_combo_box_get_active (GTK_COMBO_BOX (combo)))
    {
        case EXPORT_QIF:
        account->extension = g_strdup ("qif");
        account->format = EXPORT_QIF;
        break;

        case EXPORT_CSV:
        account->extension = g_strdup ("csv");
        account->format = EXPORT_CSV;
        break;
    }

    if (w_etat->accounting_entity && strlen (w_etat->accounting_entity))
    {
        title = w_etat->accounting_entity;
    }
    else
    {
        title = g_get_user_name ();
    }

	tmp_str = g_strconcat (title, "-",
						   gsb_data_account_get_name (account->account_nb),
						   ".",
						   account->extension,
						   NULL);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (account->chooser), tmp_str);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (account->chooser), gsb_file_get_last_path ());
	g_free (tmp_str);

    return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void export_account_all_toggled (GtkToggleButton *button,
										GtkTreeView *tree_view)
{
    GtkWidget *assistant;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint toggle_value;
    gboolean valid;

    model = gtk_tree_view_get_model (tree_view);
    assistant = g_object_get_data (G_OBJECT (model), "assistant");
    toggle_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
    if (toggle_value)
    {
      gtk_button_set_label (GTK_BUTTON (button), _("Unselect all"));
      gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), TRUE);
    }
    else
    {
      gtk_button_set_label (GTK_BUTTON (button), _("Select all"));
      gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (assistant), "button_next"), FALSE);
    }

    /* get first iter */
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);
    while (valid)
    {
        gboolean toggle_item;
        gint account_toggled;

        gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							0, &toggle_item,
							2, &account_toggled,
							-1);

        if (toggle_value && !toggle_item)
        {
            selected_accounts = g_slist_append (selected_accounts, GINT_TO_POINTER (account_toggled));
            gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, !toggle_item, -1);
        }
        else if (!toggle_value && toggle_item)
        {
            selected_accounts = g_slist_remove (selected_accounts, GINT_TO_POINTER (account_toggled));
            gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, !toggle_item, -1);
        }
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
    }

    /* clean up */
    export_resume_maybe_sensitive_next (assistant);
}

/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 *
 * \param checkbutton a pointer to a checkbutton widget.
 * \param event
 * \param pointeur vers la donnée à modifier
 *
 * \return	FALSE
 **/
static gboolean export_account_radiobutton_format_changed (GtkWidget *checkbutton,
														   GdkEventButton *event,
														   gint *pointeur)
{
    if (pointeur)
    {
        gint value = 0;

        value = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (checkbutton), "pointer"));
        *pointeur = value;

		gtk_widget_set_sensitive (box_US, !value);
        gsb_file_set_modified (TRUE);
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *export_create_selection_page (GtkWidget *assistant)
{
    GtkWidget *view, *vbox, *padding_box, *sw;
	GtkWidget *box;
    GtkWidget *button;
    GtkWidget *button_select;
	GtkWidget *separator;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkListStore *model;
    GSList *tmp_list;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);
    padding_box = new_paddingbox_with_title (vbox, TRUE, _("Select accounts to export"));

    /* Create list store */
    model = gtk_list_store_new (3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INT);

    /* Create list view */
    view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	gtk_widget_set_name (view, "tree_view");
    g_object_unref (G_OBJECT(model));

    /* Scroll for tree view. */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start (GTK_BOX (padding_box), sw, TRUE, TRUE, 0);
    gtk_widget_set_size_request (sw, 480, 200);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (sw), view);

    /* Checkbox */
    cell = gtk_cell_renderer_toggle_new ();
    g_object_set (cell, "xalign", 0.5, NULL);
    g_signal_connect (cell, "toggled", G_CALLBACK (export_account_toggled), model);
    g_object_set_data (G_OBJECT (model), "assistant", assistant);
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_end (GTK_TREE_VIEW_COLUMN(column), cell, TRUE);
    gtk_tree_view_column_add_attribute (GTK_TREE_VIEW_COLUMN(column), cell, "active", 0);
    gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN(column), _("Export"));
    gtk_tree_view_column_set_alignment (GTK_TREE_VIEW_COLUMN(column), 0.5);
    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    /* Account name */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN(column), cell, TRUE);
    gtk_tree_view_column_add_attribute (GTK_TREE_VIEW_COLUMN(column), cell, "text", 1);
    gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN(column), _("Account name"));
    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    /* Iterate through accounts. */
    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
		gint i;
		GtkTreeIter iter;

		i = gsb_data_account_get_no_account (tmp_list->data);

		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model),
							&iter,
							0, FALSE,
							1, gsb_data_account_get_name (i),
							2, i,
							-1);
		tmp_list = tmp_list->next;
	}

    button_select = g_object_get_data (G_OBJECT (assistant), "button_select");
    g_object_set_data (G_OBJECT (button_select), "tree_view", view);
    g_signal_connect (G_OBJECT (button_select),
					  "toggled",
					  G_CALLBACK (export_account_all_toggled),
					  view);

    padding_box = gsb_automem_radiobutton3_new_with_title (vbox,
														   _("Select options to export"),
														   _("QIF format"),
														   _("CSV format"),
														   NULL,
														   &etat.export_file_format,
														   G_CALLBACK (export_account_radiobutton_format_changed),
														   &etat.export_file_format,
														   GTK_ORIENTATION_HORIZONTAL);

	/* Adding options for export in US data */
	box = g_object_get_data (G_OBJECT (padding_box), "box");

	separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	gtk_widget_set_margin_start (separator, 50);
	gtk_box_pack_start (GTK_BOX (box), separator, FALSE, FALSE, MARGIN_BOX);

	box_US = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);

    button = gsb_automem_checkbutton_new (_("Force US dates"),
										  &etat.export_force_US_dates,
										  NULL,
										  NULL);
    gtk_box_pack_start (GTK_BOX (box_US), button, FALSE, FALSE, MARGIN_BOX);
    button = gsb_automem_checkbutton_new (_("Force US numbers"),
										  &etat.export_force_US_numbers,
										  NULL,
										  NULL);
    gtk_box_pack_start (GTK_BOX (box_US), button, FALSE, FALSE, MARGIN_BOX);

	gtk_widget_set_sensitive (box_US, !etat.export_file_format);
    gtk_box_pack_start (GTK_BOX (box), box_US, FALSE, FALSE, MARGIN_BOX);

	/* Adding treat all files button */
    button = gsb_automem_checkbutton_new (_("Treat all files as the first"),
										  &etat.export_files_traitement,
										  NULL,
										  NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

    /* return */
    return vbox;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *export_create_resume_page (GtkWidget *assistant)
{
    GtkWidget *sw;
    GtkWidget *view;
    GtkTextBuffer *buffer;

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 12);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 12);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag (buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag (buffer, "indented", "left-margin", 24, NULL);

    g_object_set_data (G_OBJECT (assistant), "text-buffer", buffer);

    /* Scroll for view. */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (sw, 480, 200);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (sw), view);

    return sw;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *export_create_final_page (GtkWidget *assistant)
{
    GtkWidget * view;
    GtkTextBuffer * buffer;
    GtkTextIter iter;

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 12);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (view), 12);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_create_tag (buffer, "x-large",
				 "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Export settings completed successfully"), -1,
					      "x-large", NULL);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
    gtk_text_buffer_insert (buffer, &iter,
			    _("Press the 'Close' button to finish the export."),
			    -1);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

    return view;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *create_export_account_resume_page (struct ExportedAccount *account)
{
    GtkWidget *vbox, *hbox, *label, *combo;
    gchar *tmpstr;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);

    tmpstr = make_pango_attribut ("size=\"x-large\"",
                        g_strdup_printf ("Export of : %s",
                        gsb_data_account_get_name (account->account_nb)));

    label = gtk_label_new (NULL);
    utils_labels_set_alignement (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_label_set_markup (GTK_LABEL (label), tmpstr);
    g_free (tmpstr);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    /* Layout */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("Export format: ")), FALSE, FALSE, 0);

    /* Combo box */
    combo = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), _("QIF format"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), _("CSV format"));
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT(combo),
					  "changed",
					  G_CALLBACK (export_account_change_format),
					  (gpointer) account);

    account->chooser = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_SAVE);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(account->chooser), hbox);
    gtk_box_pack_start (GTK_BOX (vbox), account->chooser, TRUE, TRUE, 0);

    gtk_combo_box_set_active (GTK_COMBO_BOX(combo), etat.export_file_format);

    return vbox;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean export_enter_resume_page (GtkWidget *assistant)
{
    GtkWidget *button_select;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GSList *list;
    gint page = 3;
    gint index = 0;

    buffer = g_object_get_data (G_OBJECT (assistant), "text-buffer");
    gtk_text_buffer_set_text (buffer, "\n", -1);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);

    button_select = g_object_get_data (G_OBJECT (assistant), "button_select");
    gtk_widget_hide (button_select);

    if (selected_accounts && g_slist_length (selected_accounts))
    {
		gtk_text_buffer_insert_with_tags_by_name (buffer,
												  &iter,
												  _("Accounts to export"),
												  -1,
												  "x-large",
												  NULL);
		gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

		gtk_text_buffer_insert (buffer,
								&iter,
								_("The following accounts are to be exported. "
								  "In the next screens, you will choose what to do with "
								  "each of them."),
								-1);
		gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

		while (gtk_notebook_get_n_pages (g_object_get_data (G_OBJECT (assistant), "notebook")) > 3)
		{
			gtk_notebook_remove_page (g_object_get_data (G_OBJECT (assistant), "notebook"), -1);
		}

		list = selected_accounts;
		while (list)
		{
			struct ExportedAccount *account;
			gint i = GPOINTER_TO_INT (list->data);

			gtk_text_buffer_insert_with_tags_by_name (buffer,
													  &iter,
													  g_strconcat ("• ",
																   gsb_data_account_get_name (i),
																   "\n\n",
																   NULL),
													  -1,
													  "indented",
													  NULL);

			account = g_malloc0 (sizeof (struct ExportedAccount));
			account->account_nb = i;
			if (etat.export_file_format)
				account->extension = g_strdup ("csv");
			else
				account->extension = g_strdup ("qif");
			exported_accounts = g_slist_append (exported_accounts, account);

			if (!etat.export_files_traitement || g_slist_length (selected_accounts) == 1)
			{
				gsb_assistant_add_page (assistant,
										create_export_account_resume_page (account),
										page,
										page - 1,
										page + 1,
										G_CALLBACK (NULL));
				page ++;
			}

			index++;
			list = list->next;
		}

		/* And final page */
		gsb_assistant_add_page (assistant,
								export_create_final_page (assistant),
								page,
								page - 1,
								-1,
								G_CALLBACK (NULL));

		/* Replace the "next" button of resume page */
		gsb_assistant_change_button_next (assistant, "gtk-go-forward", GTK_RESPONSE_YES);
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void expert_account_free_account_structure (struct ExportedAccount *account)
{
	if (account->extension)
		g_free (account->extension);
	if (account->filename)
		g_free (account->filename);

    g_free (account);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
void export_accounts (void)
{
    GtkWidget *dialog;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    selected_accounts = NULL;
    exported_accounts = NULL;

    dialog = gsb_assistant_new (_("Exporting Grisbi accounts"),
                        _("This assistant will guide you through the process of "
                        "exporting Grisbi accounts into QIF or CSV files.\n\n"
                        "As QIF and CSV do not support currencies, all "
                        "transactions will be converted into currency of their "
                        "respective account."),
                        "gsb-export-32.png",
                        NULL);

    gsb_assistant_add_page (dialog,
							export_create_selection_page (dialog),
							1,
							0,
							2,
							G_CALLBACK (export_resume_maybe_sensitive_next));
    gsb_assistant_add_page (dialog,
							export_create_resume_page (dialog),
							2,
							1,
							3,
							G_CALLBACK (export_enter_resume_page));


    if (gsb_assistant_run (dialog) == GTK_RESPONSE_APPLY)
    {
		GSList *list;

		list = exported_accounts;
		while (list)
        {
            struct ExportedAccount *account;

            account = (struct ExportedAccount *) list->data;

            if (etat.export_files_traitement && g_slist_length (selected_accounts) > 1)
            {
                const gchar *title;
                gchar *tmp_str;

				if (w_etat->accounting_entity && strlen (w_etat->accounting_entity))
                    title = w_etat->accounting_entity;
                else
                    title = g_get_user_name ();

                tmp_str = g_strconcat (title, "-",
                                gsb_data_account_get_name (account->account_nb),
                                ".",
                                account->extension,
                                NULL);

                account->filename = g_build_filename (gsb_file_get_last_path (), tmp_str, NULL);

                g_free (tmp_str);
            }
            else
            {
                account->filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (account->chooser));
            }

            if (strcmp (account->extension, "qif") == 0)
                qif_export (account->filename, account->account_nb, 0);
            else    /* extension = csv */
                gsb_csv_export_account (account->filename, account->account_nb);

            list = list->next;
        }
    }

    g_slist_free (selected_accounts);
    g_slist_free_full (exported_accounts, (GDestroyNotify) expert_account_free_account_structure);

    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

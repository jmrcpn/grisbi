/* file gsb_file_save.c
 * used to save the gsb files */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_file_save.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "gsb_file_util.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "structures.h"
#include "echeancier_liste.h"
#include "operations_liste.h"
#include "include.h"
#include "echeancier_infos.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gulong gsb_file_save_account_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content );
static gulong gsb_file_save_bank_part ( gulong iterator,
				 gulong *length_calculated,
				 gchar **file_content );
static gulong gsb_file_save_currency_part ( gulong iterator,
				     gulong *length_calculated,
				     gchar **file_content );
static gulong gsb_file_save_financial_year_part ( gulong iterator,
					   gulong *length_calculated,
					   gchar **file_content );
static gulong gsb_file_save_general_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content );
static gulong gsb_file_save_party_part ( gulong iterator,
				  gulong *length_calculated,
				  gchar **file_content );
static gulong gsb_file_save_payment_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content );
static gulong gsb_file_save_reconcile_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content );
static gulong gsb_file_save_scheduled_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content );
static gulong gsb_file_save_transaction_part ( gulong iterator,
					gulong *length_calculated,
					gchar **file_content );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *adr_banque;
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern gchar *chemin_logo;
extern GtkWidget *code_banque;
extern GtkWidget *email_banque;
extern GtkWidget *email_correspondant;
extern struct struct_etat *etat_courant;
extern GtkWidget *fax_correspondant;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GSList *liste_struct_banques;
extern GSList *liste_struct_categories;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern GSList *liste_struct_etats;
extern GSList *liste_struct_exercices;
extern GSList *liste_struct_imputation;
extern GSList *liste_struct_rapprochements;
extern gint nb_colonnes;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkWidget *nom_banque;
extern GtkWidget *nom_correspondant;
extern GtkWidget *nom_exercice;
extern gchar *nom_fichier_backup;
extern gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern GtkWidget *remarque_banque;
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern GtkWidget *tel_banque;
extern GtkWidget *tel_correspondant;
extern gchar *titre_fichier;
extern gint valeur_echelle_recherche_date_import;
extern GtkWidget *web_banque;
/*END_EXTERN*/




/** 
 * save the grisbi file
 * we don't check anything here, all must be done before, here we just write
 * the file and set the permissions
 *
 * \param filename the name of the file
 * \param compress TRUE if we want to compress the file
 *
 * \return TRUE : ok, FALSE : problem
 * */
gboolean gsb_file_save_save_file ( gchar *filename,
				   gboolean compress )
{
    gint do_chmod;
    FILE *grisbi_file;
    gulong iterator;

    gulong length_calculated;
    gchar *file_content;

    gint general_part;
    gint account_part;
    gint transaction_part;
    gint party_part;
    gint category_part;
    gint budgetary_part;
    gint currency_part;
    gint bank_part;
    gint financial_year_part;
    gint reconcile_part;
    gint report_part;

    if ( DEBUG )
	printf ( "gsb_file_save_save_file : %s\n",
		 filename );

    do_chmod = !g_file_test ( filename,
			      G_FILE_TEST_EXISTS );

    etat.en_train_de_sauvegarder = 1;

    /* we begin to try to reserve enough memory to make the entire file
     * if not enough, we will make it growth later
     * the data below are about the memory to take for each part and for 1 of this part
     * with that i think we will allocate enough memory in one time but not too much */

    general_part = 900;
    account_part = 1200;
    transaction_part = 350;
    party_part = 100;
    category_part = 500;
    budgetary_part = 500;
    currency_part = 150;
    bank_part = 300;
    financial_year_part = 100;
    reconcile_part = 50;
    report_part = 2500;
    
    length_calculated = general_part
	+ account_part * gsb_data_account_get_accounts_amount ()
	+ transaction_part * g_slist_length ( gsb_data_transaction_get_transactions_list ())
	+ party_part * g_slist_length ( gsb_data_payee_get_payees_list () )
	+ category_part * g_slist_length ( liste_struct_categories )
	+ budgetary_part * g_slist_length ( liste_struct_imputation )
	+ currency_part * g_slist_length ( liste_struct_devises )
	+ bank_part * g_slist_length ( liste_struct_banques )
	+ financial_year_part * g_slist_length (liste_struct_exercices  )
	+ reconcile_part * g_slist_length ( liste_struct_rapprochements )
	+ report_part * g_slist_length ( liste_struct_etats );

    iterator = 0;
    file_content = malloc ( length_calculated );

    /* begin the file whit xml markup */
    
    iterator = gsb_file_save_append_part ( iterator,
					   &length_calculated,
					   &file_content,
					   g_strdup ("<?xml version=\"1.0\"?>\n<Grisbi>\n"));

    iterator = gsb_file_save_general_part ( iterator,
					    &length_calculated,
					    &file_content );

    iterator = gsb_file_save_account_part ( iterator,
					    &length_calculated,
					    &file_content );

    iterator = gsb_file_save_payment_part ( iterator,
					    &length_calculated,
					    &file_content );

    iterator = gsb_file_save_transaction_part ( iterator,
						&length_calculated,
						&file_content );

    iterator = gsb_file_save_scheduled_part ( iterator,
					      &length_calculated,
					      &file_content );

    iterator = gsb_file_save_party_part ( iterator,
					  &length_calculated,
					  &file_content );

    iterator = gsb_file_save_category_part ( iterator,
					     &length_calculated,
					     &file_content );

    iterator = gsb_file_save_budgetary_part ( iterator,
					      &length_calculated,
					      &file_content );

    iterator = gsb_file_save_currency_part ( iterator,
					     &length_calculated,
					     &file_content );

    iterator = gsb_file_save_bank_part ( iterator,
					 &length_calculated,
					 &file_content );

    iterator = gsb_file_save_financial_year_part ( iterator,
						   &length_calculated,
						   &file_content );

    iterator = gsb_file_save_reconcile_part ( iterator,
					      &length_calculated,
					      &file_content );

    iterator = gsb_file_save_report_part ( iterator,
					   &length_calculated,
					   &file_content,
					   FALSE );
    /* finish the file */

    iterator = gsb_file_save_append_part ( iterator,
					   &length_calculated,
					   &file_content,
					   g_strdup ("</Grisbi>"));

    /* before saving the file, we compress and crypt it if necessary */

    if ( compress )
	iterator = gsb_file_util_compress_file ( &file_content,
						 iterator,
						 TRUE );

    iterator = gsb_file_util_crypt_file ( filename, &file_content, TRUE, iterator );
    
    /* the file is in memory, we can save it */

    grisbi_file = fopen ( filename,
			  "w" );

    if ( !grisbi_file
	 ||
	 !fwrite ( file_content,
		   sizeof (gchar),
		   iterator,
		   grisbi_file ))
    {
	dialogue_error ( g_strdup_printf ( _("Cannot save file '%s': %s"),
					   filename,
					   latin2utf8(strerror(errno)) ));
	free ( file_content);
	return ( FALSE );
    }
    
    fclose (grisbi_file);
    free ( file_content);

    /* if it's a new file, we set the permission */

    if ( do_chmod )
	chmod ( filename,
		S_IRUSR | S_IWUSR );

    etat.en_train_de_sauvegarder = 0;

    return ( TRUE );
}

/**
 * add the string given in arg and
 * check if we don't go throw the upper limit of file_content
 * if yes, we reallocate it
 *
 * \param file_content the file content
 * \param iterator the current iterator
 * \param new_string the string we want to add, it will be freed
 *
 * \return the new iterator
 * */
gulong gsb_file_save_append_part ( gulong iterator,
				   gulong *length_calculated,
				   gchar **file_content,
				   gchar *new_string )
{
    if ( !new_string )
	return iterator;
    
    /* we check first if we don't go throw the upper limit */

    while ( (iterator + strlen (new_string)) >= *length_calculated )
    {
	/* we change the size by adding half of length_calculated */

	*length_calculated = 1.5 * *length_calculated;
	*file_content = realloc ( *file_content,
				  *length_calculated );

	if ( DEBUG )
	    printf ( "The length calculated for saving file should be bigger?\n" );
    }

    memcpy ( *file_content + iterator,
	     new_string,
	     strlen (new_string));
    iterator = iterator + strlen (new_string);
    g_free (new_string);

    return iterator;
}

/**
 * save the general part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_general_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content )
{
    gchar *first_string_to_free;
    gchar *second_string_to_free;
    gchar *third_string_to_free;
    gint i,j;
    gchar *transactions_view;
    gchar *transaction_column_width_ratio;
    gchar *two_lines_showed;
    gchar *tree_lines_showed;
    gchar *scheduler_column_width_ratio;
    gchar *new_string;

    /* prepare stuff to save generals informations */

    /* prepare transactions_view */

    transactions_view = NULL;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j=0 ; j< TRANSACTION_LIST_COL_NB ; j++ )
	    if ( transactions_view )
	    {
		transactions_view = g_strconcat ( first_string_to_free = transactions_view,
						  "-",
						  second_string_to_free = utils_str_itoa ( tab_affichage_ope[i][j] ),
						  NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		transactions_view = utils_str_itoa ( tab_affichage_ope[i][j] );

    /* prepare transaction_column_width_ratio */
    transaction_column_width_ratio = NULL;

    /* prepare two_lines_showed */

    two_lines_showed = g_strconcat ( first_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data )),
				     "-",
				     second_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data )),
				     NULL );
    g_free (first_string_to_free);
    g_free (second_string_to_free);

    /* prepare tree_lines_showed */

    tree_lines_showed = g_strconcat ( first_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data )),
				      "-",
				      second_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data )),
				      "-",
				      third_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data )),
				      NULL );
    g_free (first_string_to_free);
    g_free (second_string_to_free);
    g_free (third_string_to_free);

    /* prepare scheduler_column_width_ratio */

    scheduler_column_width_ratio = NULL;

    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	if ( scheduler_column_width_ratio )
	{
	    scheduler_column_width_ratio = g_strconcat ( first_string_to_free = scheduler_column_width_ratio,
							 "-",
							 second_string_to_free = utils_str_itoa ( scheduler_col_width[i] ),
							 NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	}
	else
	    scheduler_column_width_ratio  = utils_str_itoa ( scheduler_col_width[i] );


    /* save the general informations */

    new_string = g_markup_printf_escaped ( "\t<General\n"
					   "\t\tFile_version=\"%s\"\n"
					   "\t\tGrisbi_version=\"%s\"\n"
					   "\t\tBackup_file=\"%s\"\n"
					   "\t\tCrypt_file=\"%d\"\n"
					   "\t\tFile_title=\"%s\"\n"
					   "\t\tGeneral_address=\"%s\"\n"
					   "\t\tSecond_general_address=\"%s\"\n"
					   "\t\tParty_list_currency_number=\"%d\"\n"
					   "\t\tCategory_list_currency_number=\"%d\"\n"
					   "\t\tBudget_list_currency_number=\"%d\"\n"
					   "\t\tScheduler_view=\"%d\"\n"
					   "\t\tScheduler_custom_number=\"%d\"\n"
					   "\t\tScheduler_custom_menu=\"%d\"\n"
					   "\t\tImport_interval_search=\"%d\"\n"
					   "\t\tUse_logo=\"%d\"\n"
					   "\t\tPath_logo=\"%s\"\n"
					   "\t\tRemind_display_per_account=\"%d\"\n"
					   "\t\tTransactions_view=\"%s\"\n"
					   "\t\tTransaction_column_width_ratio=\"%s\"\n"
					   "\t\tOne_line_showed=\"%d\"\n"
					   "\t\tTwo_lines_showed=\"%s\"\n"
					   "\t\tThree_lines_showed=\"%s\"\n"
					   "\t\tRemind_form_per_account=\"%d\"\n"
					   "\t\tScheduler_column_width_ratio=\"%s\" />\n",
	VERSION_FICHIER,
	VERSION,
	nom_fichier_backup,
	etat.crypt_file,
	titre_fichier,
	adresse_commune,
	adresse_secondaire,
	no_devise_totaux_tiers,
	no_devise_totaux_categ,
	no_devise_totaux_ib,
	affichage_echeances,
	affichage_echeances_perso_nb_libre,
	affichage_echeances_perso_j_m_a,
	valeur_echelle_recherche_date_import,
	etat.utilise_logo,
	chemin_logo,
	etat.retient_affichage_par_compte,
	transactions_view,
	transaction_column_width_ratio,
	ligne_affichage_une_ligne,
	two_lines_showed,
	tree_lines_showed,
	etat.formulaire_distinct_par_compte,
	scheduler_column_width_ratio );

    g_free (transactions_view);
    g_free (transaction_column_width_ratio);
    g_free (two_lines_showed);
    g_free (tree_lines_showed);
    g_free (scheduler_column_width_ratio);

    /* append the new string to the file content
     * and return the new iterator */

    return gsb_file_save_append_part ( iterator,
				       length_calculated,
				       file_content,
				       new_string );
}


/**
 * save the account part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_account_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content )
{
    GSList *list_tmp;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gchar *first_string_to_free;
	gchar *second_string_to_free;
	gchar *third_string_to_free;
	gint i;
	gint j, k;
	gchar *last_reconcile_date;
	gchar *sort_list;
	gchar *sort_kind_column;
	gchar *form_organization;
	gchar *form_columns_width;
	GSList *list_tmp_2;
	gchar *new_string;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	/* set the last reconcile date */

	if ( gsb_data_account_get_current_reconcile_date (i) )
	{
	    last_reconcile_date = g_strconcat ( first_string_to_free = utils_str_itoa ( g_date_day ( gsb_data_account_get_current_reconcile_date (i) ) ),
						"/",
						second_string_to_free = utils_str_itoa ( g_date_month ( gsb_data_account_get_current_reconcile_date (i) ) ),
						"/",
						third_string_to_free = utils_str_itoa ( g_date_year ( gsb_data_account_get_current_reconcile_date (i) ) ),
						NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	    g_free (third_string_to_free);
	}
	else
	    last_reconcile_date = g_strdup ("");

	/* set the sort_list */

	list_tmp_2 = gsb_data_account_get_sort_list (i);
	sort_list = NULL;

	while ( list_tmp_2 )
	{
	    if ( sort_list )
	    {
		sort_list = g_strconcat ( first_string_to_free = sort_list,
					  "/",
					  second_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( list_tmp_2 -> data )),
					  NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		sort_list = utils_str_itoa ( GPOINTER_TO_INT ( list_tmp_2 -> data ));

	    list_tmp_2 = list_tmp_2 -> next;
	}

	/* set the default sort kind for the columns */

	sort_kind_column = NULL;

	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( sort_kind_column )
	    {
		sort_kind_column = g_strconcat ( first_string_to_free = sort_kind_column,
						 "-",
						 second_string_to_free = utils_str_itoa ( gsb_data_account_get_column_sort ( i,
												j )),
						 NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		sort_kind_column = utils_str_itoa ( gsb_data_account_get_column_sort ( i,
								     j ));
	}

	/* set the form organization */

	form_organization = NULL;

	for ( k=0 ; k<4 ; k++ )
	    for ( j=0 ; j< 6 ; j++ )
		if ( form_organization )
		{ 
		    form_organization = g_strconcat ( first_string_to_free = form_organization,
						      "-",
						      second_string_to_free = utils_str_itoa ( gsb_data_account_get_form_organization (i) -> tab_remplissage_formulaire [k][j] ),
						      NULL );
		    g_free (first_string_to_free);
		    g_free (second_string_to_free);
		}
		else
		    form_organization = utils_str_itoa ( gsb_data_account_get_form_organization (i) -> tab_remplissage_formulaire [k][j] );

	/* set the form columns width */

	form_columns_width = NULL;

	for ( k=0 ; k<6 ; k++ )
	    if ( form_columns_width )
	    {
		form_columns_width = g_strconcat ( first_string_to_free = form_columns_width,
						   "-",
						   second_string_to_free = utils_str_itoa ( gsb_data_account_get_form_organization (i) -> taille_colonne_pourcent [k] ),
						   NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		form_columns_width = utils_str_itoa ( gsb_data_account_get_form_organization (i) -> taille_colonne_pourcent [k] );

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Account\n"
					       "\t\tName=\"%s\"\n"
					       "\t\tId=\"%s\"\n"
					       "\t\tNumber=\"%d\"\n"
					       "\t\tOwner=\"%s\"\n"
					       "\t\tKind=\"%d\"\n"
					       "\t\tCurrency=\"%d\"\n"
					       "\t\tBank=\"%d\"\n"
					       "\t\tBank_branch_code=\"%s\"\n"
					       "\t\tBank_account_number=\"%s\"\n"
					       "\t\tKey=\"%s\"\n"
					       "\t\tInitial_balance=\"%4.7f\"\n"
					       "\t\tMinimum_wanted_balance=\"%4.7f\"\n"
					       "\t\tMinimum_authorised_balance=\"%4.7f\"\n"
					       "\t\tLast_reconcile_date=\"%s\"\n"
					       "\t\tLast_reconcile_balance=\"%4.7f\"\n"
					       "\t\tLast_reconcile_number=\"%d\"\n"
					       "\t\tClosed_account=\"%d\"\n"
					       "\t\tShow_marked=\"%d\"\n"
					       "\t\tLines_per_transaction=\"%d\"\n"
					       "\t\tComment=\"%s\"\n"
					       "\t\tOwner_address=\"%s\"\n"
					       "\t\tDefault_debit_method=\"%d\"\n"
					       "\t\tDefault_credit_method=\"%d\"\n"
					       "\t\tSort_by_method=\"%d\"\n"
					       "\t\tNeutrals_inside_method=\"%d\"\n"
					       "\t\tSort_order=\"%s\"\n"
					       "\t\tAscending_sort=\"%d\"\n"
					       "\t\tColumn_sort=\"%d\"\n"
					       "\t\tSorting_kind_column=\"%s\"\n"
					       "\t\tForm_columns_number=\"%d\"\n"
					       "\t\tForm_lines_number=\"%d\"\n"
					       "\t\tForm_organization=\"%s\"\n"
					       "\t\tForm_columns_width=\"%s\" />\n",
	    gsb_data_account_get_name (i),
	    gsb_data_account_get_id (i),
	    i,
	    gsb_data_account_get_holder_name (i),
	    gsb_data_account_get_kind (i),
	    gsb_data_account_get_currency (i),
	    gsb_data_account_get_bank (i),
	    gsb_data_account_get_bank_branch_code (i),
	    gsb_data_account_get_bank_account_number (i),
	    gsb_data_account_get_bank_account_key (i),
	    gsb_data_account_get_init_balance (i),
	    gsb_data_account_get_mini_balance_wanted (i),
	    gsb_data_account_get_mini_balance_authorized (i),
	    last_reconcile_date,
	    gsb_data_account_get_reconcile_balance (i),
	    gsb_data_account_get_reconcile_last_number (i),
	    gsb_data_account_get_closed_account (i),
	    gsb_data_account_get_r (i),
	    gsb_data_account_get_nb_rows (i),
	    gsb_data_account_get_comment (i),
	    gsb_data_account_get_holder_address (i),
	    gsb_data_account_get_default_debit (i),
	    gsb_data_account_get_default_credit (i),
	    gsb_data_account_get_reconcile_sort_type (i),
	    gsb_data_account_get_split_neutral_payment (i),
	    sort_list,
	    gsb_data_account_get_sort_type (i),
	    gsb_data_account_get_sort_column (i),
	    sort_kind_column,
	    gsb_data_account_get_form_organization (i) -> nb_colonnes,
	    gsb_data_account_get_form_organization (i) -> nb_lignes,
	    form_organization,
	    form_columns_width );

	g_free (last_reconcile_date);
	g_free (sort_list);
	g_free (sort_kind_column);
	g_free (form_organization);
	g_free (form_columns_width);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );

	list_tmp = list_tmp -> next;
    }
    return iterator;
}

/**
 * save the methods of payment 
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_payment_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content )
{
    GSList *list_tmp;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	GSList *list_tmp_2;
	gint i;
	gchar *new_string;

	i = gsb_data_account_get_no_account ( list_tmp -> data );
	list_tmp_2 = gsb_data_account_get_method_payment_list (i);

	while ( list_tmp_2 )
	{
	    struct struct_type_ope *method;

	    method = list_tmp_2 -> data;

	    /* now we can fill the file content */

	    new_string = g_markup_printf_escaped ( "\t<Payment Number=\"%d\" Name=\"%s\" Sign=\"%d\" Show_entry=\"%d\" Automatic_number=\"%d\" Current_number=\"%d\" Account=\"%d\" />\n",
						   method -> no_type,
						   method -> nom_type,
						   method -> signe_type,
						   method -> affiche_entree,
						   method -> numerotation_auto,
						   method -> no_en_cours,
						   i );

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );
	    list_tmp_2 = list_tmp_2 -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}

/**
 * save the transactions 
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_transaction_part ( gulong iterator,
					gulong *length_calculated,
					gchar **file_content )
{
    GSList *list_tmp;

    list_tmp = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp )
    {
	gint transaction_number;
	gchar *new_string;

	transaction_number = gsb_data_transaction_get_transaction_number ( list_tmp -> data );
	
	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Transaction Ac=\"%d\" Nb=\"%d\" Id=\"%s\" Dt=\"%s\" Dv=\"%s\" Am=\"%4.7f\" Cu=\"%d\" Exb=\"%d\" Exr=\"%4.7f\" Exf=\"%4.7f\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Br=\"%d\" No=\"%s\" Pn=\"%d\" Pc=\"%s\" Ma=\"%d\" Au=\"%d\" Re=\"%d\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" Vo=\"%s\" Ba=\"%s\" Trt=\"%d\" Tra=\"%d\" Mo=\"%d\" />\n",
					       gsb_data_transaction_get_account_number ( transaction_number ),
					       transaction_number,
					       gsb_data_transaction_get_transaction_id ( transaction_number),
					       gsb_format_gdate ( gsb_data_transaction_get_date ( transaction_number )),
					       gsb_format_gdate ( gsb_data_transaction_get_value_date ( transaction_number )),
					       gsb_data_transaction_get_amount ( transaction_number ),
					       gsb_data_transaction_get_currency_number (transaction_number ),
					       gsb_data_transaction_get_change_between (transaction_number ),
					       gsb_data_transaction_get_exchange_rate (transaction_number ),
					       gsb_data_transaction_get_exchange_fees ( transaction_number),
					       gsb_data_transaction_get_party_number ( transaction_number),
					       gsb_data_transaction_get_category_number ( transaction_number),
					       gsb_data_transaction_get_sub_category_number (transaction_number),
					       gsb_data_transaction_get_breakdown_of_transaction (transaction_number),
					       gsb_data_transaction_get_notes (transaction_number),
					       gsb_data_transaction_get_method_of_payment_number (transaction_number),
					       gsb_data_transaction_get_method_of_payment_content (transaction_number),
					       gsb_data_transaction_get_marked_transaction (transaction_number),
					       gsb_data_transaction_get_automatic_transaction (transaction_number),
					       gsb_data_transaction_get_reconcile_number (transaction_number),
					       gsb_data_transaction_get_financial_year_number (transaction_number),
					       gsb_data_transaction_get_budgetary_number (transaction_number),
					       gsb_data_transaction_get_sub_budgetary_number (transaction_number),
					       gsb_data_transaction_get_voucher (transaction_number),
					       gsb_data_transaction_get_bank_references (transaction_number),
					       gsb_data_transaction_get_transaction_number_transfer (transaction_number),
					       gsb_data_transaction_get_account_number_transfer (transaction_number),
					       gsb_data_transaction_get_mother_transaction_number (transaction_number));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the scheduled transactions
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_scheduled_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_echeances;

    while ( list_tmp )
    {
	struct operation_echeance *echeance;
	gchar *new_string;

	echeance = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Scheduled Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" Am=\"%4.7f\" Cu=\"%d\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Tra=\"%d\" Pn=\"%d\" CPn=\"%d\" Pc=\"%s\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" No=\"%s\" Au=\"%d\" Pe=\"%d\" Pei=\"%d\" Pep=\"%d\" Dtl=\"%s\" Br=\"%d\" Mo=\"%d\" />\n",
					       echeance -> no_operation,
					       gsb_format_gdate ( echeance -> date),
					       echeance -> compte,
					       echeance -> montant,
					       echeance -> devise,
					       echeance -> tiers,
					       echeance -> categorie,
					       echeance -> sous_categorie,
					       echeance -> compte_virement,
					       echeance -> type_ope,
					       echeance -> type_contre_ope,
					       echeance -> contenu_type,
					       echeance -> no_exercice,
					       echeance -> imputation,
					       echeance -> sous_imputation,
					       echeance -> notes,
					       echeance -> auto_man,
					       echeance -> periodicite,
					       echeance -> intervalle_periodicite_personnalisee,
					       echeance -> periodicite_personnalisee,
					       gsb_format_gdate ( echeance -> date_limite),
					       echeance -> operation_ventilee,
					       echeance -> no_operation_ventilee_associee );

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the parties
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_party_part ( gulong iterator,
				  gulong *length_calculated,
				  gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_payee_get_payees_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint payee_number;

	payee_number = gsb_data_payee_get_no_payee (list_tmp -> data);
	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Party Nb=\"%d\" Na=\"%s\" Txt=\"%s\" />\n",
					       payee_number,
					       gsb_data_payee_get_name (payee_number,
								   TRUE ),
					       gsb_data_payee_get_description (payee_number));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the categories
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_category_part ( gulong iterator,
				     gulong *length_calculated,
				     gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_categories;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_categ *category;
	GSList *sub_list_tmp;

	category = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Category Nb=\"%d\" Na=\"%s\" Kd=\"%d\" No_derniere_sous_cagegorie=\"%d\"/>\n",
					       category -> no_categ,
					       category -> nom_categ,
					       category -> type_categ,
					       category -> no_derniere_sous_categ );

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	/* save the sub-categories */

	sub_list_tmp = category -> liste_sous_categ;

	while ( sub_list_tmp )
	{
	    struct struct_sous_categ *sub_category;

	    sub_category = sub_list_tmp -> data;

	    /* now we can fill the file content */

	    new_string = g_markup_printf_escaped ( "\t<Sub_category Nb=\"%d\" Na=\"%s\" Nbc=\"%d\" />\n",
						   sub_category -> no_sous_categ,
						   sub_category -> nom_sous_categ,
						   category -> no_categ);

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the budgetaries
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_budgetary_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_imputation;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_imputation *budgetary;
	GSList *sub_list_tmp;

	budgetary = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Budgetary Nb=\"%d\" Na=\"%s\" Kd=\"%d\" />\n",
					       budgetary -> no_imputation,
					       budgetary -> nom_imputation,
					       budgetary -> type_imputation);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	/* save the sub-budgetaries */

	sub_list_tmp = budgetary -> liste_sous_imputation;

	while ( sub_list_tmp )
	{
	    struct struct_sous_imputation *sub_budgetary;

	    sub_budgetary = sub_list_tmp -> data;

	    /* now we can fill the file content */

	    new_string = g_markup_printf_escaped ( "\t<Sub_budgetary Nb=\"%d\" Na=\"%s\" Nbc=\"%d\" />\n",
						   sub_budgetary -> no_sous_imputation,
						   sub_budgetary -> nom_sous_imputation,
						   budgetary -> no_imputation);
	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );
	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the currencies
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_currency_part ( gulong iterator,
				     gulong *length_calculated,
				     gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_devises;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_devise *currency;

	currency = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Currency Nb=\"%d\" Na=\"%s\" Co=\"%s\" Ico=\"%s\" Mte=\"%d\" Dte=\"%s\" Rbc=\"%d\" Rcu=\"%d\" Ch=\"%4.7f\" />\n",
					       currency -> no_devise,
					       currency -> nom_devise,
					       currency -> code_devise,
					       currency -> code_iso4217_devise,
					       currency -> passage_euro,
					       gsb_format_gdate (currency -> date_dernier_change),
					       currency -> une_devise_1_egale_x_devise_2,
					       currency -> no_devise_en_rapport,
					       currency -> change);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the banks
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_bank_part ( gulong iterator,
				 gulong *length_calculated,
				 gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_banques;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_banque *bank;

	bank = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Bank Nb=\"%d\" Na=\"%s\" Co=\"%s\" Adr=\"%s\" Tel=\"%s\" Mail=\"%s\" Web=\"%s\" Nac=\"%s\" Faxc=\"%s\" Telc=\"%s\" Mailc=\"%s\" Rem=\"%s\" />\n",
					       bank -> no_banque,
					       bank -> nom_banque,
					       bank -> code_banque,
					       bank -> adr_banque,
					       bank -> tel_banque,
					       bank -> email_banque,
					       bank -> web_banque,
					       bank -> nom_correspondant,
					       bank -> fax_correspondant,
					       bank -> tel_correspondant,
					       bank -> email_correspondant,
					       bank -> remarque_banque);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the financials years
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_financial_year_part ( gulong iterator,
					   gulong *length_calculated,
					   gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_exercices;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_exercice *financial_year;

	financial_year = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped( "\t<Financial_year Nb=\"%d\" Na=\"%s\" Bdte=\"%s\" Edte=\"%s\" Sho=\"%d\" />\n",
					      financial_year -> no_exercice,
					      financial_year -> nom_exercice,
					      gsb_format_gdate (financial_year -> date_debut),
					      gsb_format_gdate (financial_year -> date_fin),
					      financial_year -> affiche_dans_formulaire);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the reconcile structures
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_reconcile_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_rapprochements;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_no_rapprochement *reconcile_struct;

	reconcile_struct = list_tmp -> data;

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Reconcile Nb=\"%d\" Na=\"%s\" />\n",
					       reconcile_struct -> no_rapprochement,
					       reconcile_struct -> nom_rapprochement );

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the reports
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 * \param current_report if TRUE, only save the current report (to exporting a report)
 *
 * \return the new iterator
 * */
gulong gsb_file_save_report_part ( gulong iterator,
				   gulong *length_calculated,
				   gchar **file_content,
				   gboolean current_report )
{
    GSList *list_tmp;
	
    list_tmp = liste_struct_etats;

    while ( list_tmp )
    {
	gchar *new_string;
	struct struct_etat *report;
	GSList *pointer_list;
	gchar *general_sort_type;
	gchar *financial_year_select;
	gchar *account_selected;
	gchar *transfer_selected_accounts;
	gchar *categ_selected;
	gchar *budget_selected;
	gchar *payee_selected;
	gchar *payment_method_list;
	GSList *list_tmp_2;

	report = list_tmp -> data;

	/* if we need only the current report, we check here */

	if ( current_report
	     &&
	     etat_courant -> no_etat != report -> no_etat )
	{
	    list_tmp = list_tmp -> next;
	    continue;
	}

	/* set the general sort type */

	pointer_list = report -> type_classement;
	general_sort_type = NULL;

	while ( pointer_list )
	{
	    if ( general_sort_type )
		general_sort_type = g_strconcat ( general_sort_type,
						  "/-/",
						  utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
						  NULL );
	    else
		general_sort_type = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* set the financial_year_select */

	pointer_list = report -> no_exercices;
	financial_year_select = NULL;

	while ( pointer_list )
	{
	    if ( financial_year_select )
		financial_year_select = g_strconcat ( financial_year_select,
						      "/-/",
						      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
						      NULL );
	    else
		financial_year_select = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* set the account_selected */

	pointer_list = report -> no_comptes;
	account_selected = NULL;

	while ( pointer_list )
	{
	    if ( account_selected )
		account_selected = g_strconcat ( account_selected,
						 "/-/",
						 utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
						 NULL );
	    else
		account_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the transfer_selected_accounts */
	
	pointer_list = report -> no_comptes_virements;
	transfer_selected_accounts = NULL;

	while ( pointer_list )
	{
	    if ( transfer_selected_accounts )
		transfer_selected_accounts = g_strconcat ( transfer_selected_accounts,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	    else
		transfer_selected_accounts = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the categ_selected */
	
	pointer_list = report -> no_categ;
	categ_selected = NULL;

	while ( pointer_list )
	{
	    if ( categ_selected )
		categ_selected = g_strconcat ( categ_selected,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	    else
		categ_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the budget_selected */

	pointer_list = report -> no_ib;
	budget_selected = NULL;

	while ( pointer_list )
	{
	    if ( budget_selected )
		budget_selected = g_strconcat ( budget_selected,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	    else
		budget_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the payee_selected */

	pointer_list = report -> no_tiers;
	payee_selected = NULL;

	while ( pointer_list )
	{
	    if ( payee_selected )
		payee_selected = g_strconcat ( payee_selected,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	    else
		payee_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the payment_method_list */

	pointer_list = report -> noms_modes_paiement;
	payment_method_list = NULL;

	while ( pointer_list )
	{
	    if ( payment_method_list )
		payment_method_list = g_strconcat ( payment_method_list,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	    else
		payment_method_list = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}


	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Report\n"
					       "\t\tNb=\"%d\"\n"
					       "\t\tName=\"%s\"\n"
					       "\t\tGeneral_sort_type=\"%s\"\n"
					       "\t\tShow_r=\"%d\"\n"
					       "\t\tShow_transaction=\"%d\"\n"
					       "\t\tShow_transaction_amount=\"%d\"\n"
					       "\t\tShow_transaction_nb=\"%d\"\n"
					       "\t\tShow_transaction_date=\"%d\"\n"
					       "\t\tShow_transaction_payee=\"%d\"\n"
					       "\t\tShow_transaction_categ=\"%d\"\n"
					       "\t\tShow_transaction_sub_categ=\"%d\"\n"
					       "\t\tShow_transaction_payment=\"%d\"\n"
					       "\t\tShow_transaction_budget=\"%d\"\n"
					       "\t\tShow_transaction_sub_budget=\"%d\"\n"
					       "\t\tShow_transaction_chq=\"%d\"\n"
					       "\t\tShow_transaction_note=\"%d\"\n"
					       "\t\tShow_transaction_voucher=\"%d\"\n"
					       "\t\tShow_transaction_reconcile=\"%d\"\n"
					       "\t\tShow_transaction_bank=\"%d\"\n"
					       "\t\tShow_transaction_fin_year=\"%d\"\n"
					       "\t\tShow_transaction_sort_type=\"%d\"\n"
					       "\t\tShow_columns_titles=\"%d\"\n"
					       "\t\tShow_title_column_kind=\"%d\"\n"
					       "\t\tShow_exclude_breakdown_child=\"%d\"\n"
					       "\t\tShow_split_amounts=\"%d\"\n"
					       "\t\tCurrency_general=\"%d\"\n"
					       "\t\tReport_in_payees=\"%d\"\n"
					       "\t\tReport_can_click=\"%d\"\n"
					       "\t\tFinancial_year_used=\"%d\"\n"
					       "\t\tFinancial_year_kind=\"%d\"\n"
					       "\t\tFinancial_year_select=\"%s\"\n"
					       "\t\tDate_kind=\"%d\"\n"
					       "\t\tDate_begining=\"%s\"\n"
					       "\t\tDate_end=\"%s\"\n"
					       "\t\tSplit_by_date=\"%d\"\n"
					       "\t\tSplit_date_period=\"%d\"\n"
					       "\t\tSplit_by_fin_year=\"%d\"\n"
					       "\t\tSplit_day_begining=\"%d\"\n"
					       "\t\tAccount_use_selection=\"%d\"\n"
					       "\t\tAccount_selected=\"%s\"\n"
					       "\t\tAccount_group_transactions=\"%d\"\n"
					       "\t\tAccount_show_amount=\"%d\"\n"
					       "\t\tAccount_show_name=\"%d\"\n"
					       "\t\tTransfer_kind=\"%d\"\n"
					       "\t\tTransfer_selected_accounts=\"%s\"\n"
					       "\t\tTransfer_exclude_transactions=\"%d\"\n"
					       "\t\tCateg_use=\"%d\"\n"
					       "\t\tCateg_use_selection=\"%d\"\n"
					       "\t\tCateg_selected=\"%s\"\n"
					       "\t\tCateg_exclude_transactions=\"%d\"\n"
					       "\t\tCateg_show_amount=\"%d\"\n"
					       "\t\tCateg_show_sub_categ=\"%d\"\n"
					       "\t\tCateg_show_without_sub_categ=\"%d\"\n"
					       "\t\tCateg_show_sub_categ_amount=\"%d\"\n"
					       "\t\tCateg_currency=\"%d\"\n"
					       "\t\tCateg_show_name=\"%d\"\n"
					       "\t\tBudget_use=\"%d\"\n"
					       "\t\tBudget_use_selection=\"%d\"\n"
					       "\t\tBudget_selected=\"%s\"\n"
					       "\t\tBudget_exclude_transactions=\"%d\"\n"
					       "\t\tBudget_show_amount=\"%d\"\n"
					       "\t\tBudget_show_sub_budget=\"%d\"\n"
					       "\t\tBudget_show_without_sub_budget=\"%d\"\n"
					       "\t\tBudget_show_sub_budget_amount=\"%d\"\n"
					       "\t\tBudget_currency=\"%d\"\n"
					       "\t\tBudget_show_name=\"%d\"\n"
					       "\t\tPayee_use=\"%d\"\n"
					       "\t\tPayee_use_selection=\"%d\"\n"
					       "\t\tPayee_selected=\"%s\"\n"
					       "\t\tPayee_show_amount=\"%d\"\n"
					       "\t\tPayee_currency=\"%d\"\n"
					       "\t\tPayee_show_name=\"%d\"\n"
					       "\t\tAmount_currency=\"%d\"\n"
					       "\t\tAmount_exclude_null=\"%d\"\n"
					       "\t\tPayment_method_list=\"%s\"\n"
					       "\t\tUse_text=\"%d\"\n"
					       "\t\tUse_amount=\"%d\" />\n",
	    report -> no_etat,
	    report -> nom_etat,
	    general_sort_type,
	    report -> afficher_r,
	    report -> afficher_opes,
	    report -> afficher_nb_opes,
	    report -> afficher_no_ope,
	    report -> afficher_date_ope,
	    report -> afficher_tiers_ope,
	    report -> afficher_categ_ope,
	    report -> afficher_sous_categ_ope,
	    report -> afficher_type_ope,
	    report -> afficher_ib_ope,
	    report -> afficher_sous_ib_ope,
	    report -> afficher_cheque_ope,
	    report -> afficher_notes_ope,
	    report -> afficher_pc_ope,
	    report -> afficher_rappr_ope,
	    report -> afficher_infobd_ope,
	    report -> afficher_exo_ope,
	    report -> type_classement_ope,
	    report -> afficher_titre_colonnes,
	    report -> type_affichage_titres,
	    report -> pas_detailler_ventilation,
	    report -> separer_revenus_depenses,
	    report -> devise_de_calcul_general,
	    report -> inclure_dans_tiers,
	    report -> ope_clickables,
	    report -> exo_date,
	    report -> utilise_detail_exo,
	    financial_year_select,
	    report -> no_plage_date,
	    gsb_format_gdate (report -> date_perso_debut),
	    gsb_format_gdate (report -> date_perso_fin),
	    report -> separation_par_plage,
	    report -> type_separation_plage,
	    report -> separation_par_exo,
	    report -> jour_debut_semaine,
	    report -> utilise_detail_comptes,
	    account_selected,
	    report -> regroupe_ope_par_compte,
	    report -> affiche_sous_total_compte,
	    report -> afficher_nom_compte,
	    report -> type_virement,
	    transfer_selected_accounts,
	    report -> exclure_ope_non_virement,
	    report -> utilise_categ,
	    report -> utilise_detail_categ,
	    categ_selected,
	    report -> exclure_ope_sans_categ,
	    report -> affiche_sous_total_categ,
	    report -> afficher_sous_categ,
	    report -> afficher_pas_de_sous_categ,
	    report -> affiche_sous_total_sous_categ,
	    report -> devise_de_calcul_categ,
	    report -> afficher_nom_categ,
	    report -> utilise_ib,
	    report -> utilise_detail_ib,
	    budget_selected,
	    report -> exclure_ope_sans_ib,
	    report -> affiche_sous_total_ib,
	    report -> afficher_sous_ib,
	    report -> afficher_pas_de_sous_ib,
	    report -> affiche_sous_total_sous_ib,
	    report -> devise_de_calcul_ib,
	    report -> afficher_nom_ib,
	    report -> utilise_tiers,
	    report -> utilise_detail_tiers,
	    payee_selected,
	    report -> affiche_sous_total_tiers,
	    report -> devise_de_calcul_tiers,
	    report -> afficher_nom_tiers,
	    report -> choix_devise_montant,
	    report -> exclure_montants_nuls,
	    payment_method_list,
	    report -> utilise_texte,
	    report -> utilise_montant);

	g_free (general_sort_type);
	g_free (financial_year_select);
	g_free (account_selected);
	g_free (transfer_selected_accounts);
	g_free (categ_selected);
	g_free (budget_selected);
	g_free (payee_selected);
	g_free (payment_method_list);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );

	/* save the text comparison */

	list_tmp_2 = report -> liste_struct_comparaison_textes;

	while ( list_tmp_2 )
	{
	    struct struct_comparaison_textes_etat *text_comparison;

	    text_comparison = list_tmp_2 -> data;

	    /* now we can fill the file content */

	    new_string = g_markup_printf_escaped ( "\t<Text_comparison\n"
						   "\t\tReport_nb=\"%d\"\n"
						   "\t\tLast_comparison=\"%d\"\n"
						   "\t\tObject=\"%d\"\n"
						   "\t\tOperator=\"%d\"\n"
						   "\t\tText=\"%s\"\n"
						   "\t\tUse_text=\"%d\"\n"
						   "\t\tComparison_1=\"%d\"\n"
						   "\t\tLink_1_2=\"%d\"\n"
						   "\t\tComparison_2=\"%d\"\n"
						   "\t\tAmount_1=\"%d\"\n"
						   "\t\tAmount_2=\"%d\" />\n",
						   report -> no_etat,
						   text_comparison -> lien_struct_precedente,
						   text_comparison -> champ,
						   text_comparison -> operateur,
						   text_comparison -> texte,
						   text_comparison -> utilise_txt,
						   text_comparison -> comparateur_1,
						   text_comparison -> lien_1_2,
						   text_comparison -> comparateur_2,
						   text_comparison -> montant_1,
						   text_comparison -> montant_2);

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );

	    list_tmp_2 = list_tmp_2 -> next;
	}

	/* save the amount comparison */

	list_tmp_2 = report -> liste_struct_comparaison_montants;

	while ( list_tmp_2 )
	{
	    struct struct_comparaison_montants_etat *amount_comparison;

	    amount_comparison = list_tmp_2 -> data;

	    /* now we can fill the file content */

	    new_string = g_markup_printf_escaped ( "\t<Amount_comparison\n"
						   "\t\tReport_nb=\"%d\"\n"
						   "\t\tLast_comparison=\"%d\"\n"
						   "\t\tComparison_1=\"%d\"\n"
						   "\t\tLink_1_2=\"%d\"\n"
						   "\t\tComparison_2=\"%d\"\n"
						   "\t\tAmount_1=\"%4.7f\"\n"
						   "\t\tAmount_2=\"%4.7f\" />\n",
						   report -> no_etat,
						   amount_comparison -> lien_struct_precedente,
						   amount_comparison -> comparateur_1,
						   amount_comparison -> lien_1_2,
						   amount_comparison -> comparateur_2,
						   amount_comparison -> montant_1,
						   amount_comparison -> montant_2);

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );
	    list_tmp_2 = list_tmp_2 -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}





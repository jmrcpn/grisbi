void personnalisation_etat (void);
void annule_modif_config ( void );
void selectionne_liste_exo_etat_courant ( void );
void selectionne_liste_comptes_etat_courant ( void );
void selectionne_liste_virements_etat_courant ( void );
void selectionne_liste_categ_etat_courant ( void );
void selectionne_devise_categ_etat_courant ( void );
void selectionne_liste_ib_etat_courant ( void );
void selectionne_devise_ib_etat_courant ( void );
void selectionne_liste_tiers_etat_courant ( void );
void selectionne_devise_tiers_etat_courant ( void );
void recuperation_info_perso_etat ( void );
void sens_desensitive_pointeur ( GtkWidget *boutton,
				 GtkWidget *widget );
void stylise_tab_label_etat ( gint *no_page );
GtkWidget *onglet_etat_dates ( void );
void click_liste_etat ( GtkCList *liste,
			GdkEventButton *evenement,
			gint origine );
void clique_sur_entree_date_etat ( GtkWidget *entree,
				   GdkEventButton *ev );
void date_selectionnee_etat ( GtkCalendar *calendrier,
			      GtkWidget *popup );
void change_separation_result_periode ( void );
void modif_type_separation_dates ( gint *origine );
void remplissage_liste_exo_etats ( void );
GtkWidget *onglet_etat_comptes ( void );
void remplissage_liste_comptes_etats ( void );
void selectionne_partie_liste_compte_etat ( gint *type_compte );
GtkWidget *onglet_etat_virements ( void );
void remplissage_liste_comptes_virements ( void );
void selectionne_partie_liste_compte_vir_etat ( gint *type_compte );
GtkWidget *onglet_etat_categories ( void );
void click_type_categ_etat ( gint type );
void remplissage_liste_categ_etats ( void );
GtkWidget *onglet_etat_ib ( void );
void click_type_ib_etat ( gint type );
void remplissage_liste_ib_etats ( void );
GtkWidget *onglet_etat_tiers ( void );
void remplissage_liste_tiers_etats ( void );
GtkWidget *onglet_etat_texte ( void );
void remplit_liste_comparaisons_textes_etat ( void );
void ajoute_ligne_liste_comparaisons_textes_etat ( struct struct_comparaison_textes_etat *ancien_comp_textes );
GtkWidget *cree_ligne_comparaison_texte ( struct struct_comparaison_textes_etat *comp_textes );
void retire_ligne_liste_comparaisons_textes_etat ( struct struct_comparaison_textes_etat *ancien_comp_textes );
GtkWidget *cree_bouton_champ ( struct struct_comparaison_textes_etat *comp_textes );
void sensitive_hbox_fonction_bouton_txt ( struct struct_comparaison_textes_etat *comp_textes );
GtkWidget *cree_bouton_operateur_txt ( struct struct_comparaison_textes_etat *comp_textes );
GtkWidget *onglet_etat_montant ( void );
void remplit_liste_comparaisons_montants_etat ( void );
void ajoute_ligne_liste_comparaisons_montants_etat ( struct struct_comparaison_montants_etat *ancien_comp_montants );
GtkWidget *cree_ligne_comparaison_montant ( struct struct_comparaison_montants_etat *comp_montants );
GtkWidget *cree_bouton_lien_lignes_comparaison ( void );
GtkWidget *cree_bouton_comparateur_montant ( struct struct_comparaison_montants_etat *comp_montants );
void change_comparaison_montant ( GtkWidget *menu_item,
				  struct struct_comparaison_montants_etat *comp_montants );
GtkWidget *cree_bouton_comparateur_texte ( struct struct_comparaison_textes_etat *comp_textes );
void change_comparaison_texte ( GtkWidget *menu_item,
				struct struct_comparaison_textes_etat *comp_textes );
GtkWidget *cree_bouton_lien ( GtkWidget *hbox );
void sensitive_widget ( GtkWidget *widget );
void desensitive_widget ( GtkWidget *widget );
void retire_ligne_liste_comparaisons_montants_etat ( struct struct_comparaison_montants_etat *ancien_comp_montants );
GtkWidget *onglet_etat_divers ( void );
GtkWidget *page_organisation_donnees ( void );
void click_haut_classement_etat ( void );
void click_bas_classement_etat ( void );
GtkWidget *page_affichage_donnees ( void );
GtkWidget *onglet_affichage_etat_generalites ( void );
GtkWidget *onglet_affichage_etat_operations ( void );
GtkWidget *onglet_affichage_etat_devises ( void );
GtkWidget *onglet_affichage_etat_divers ( void );
GtkWidget *onglet_etat_mode_paiement ( void );
void remplissage_liste_modes_paiement_etats ( void );
gint recherche_nom_dans_liste ( gchar *nom_liste,
				gchar *nom_test );
void selectionne_liste_modes_paiement_etat_courant ( void );

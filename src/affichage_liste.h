GtkWidget *onglet_affichage_liste ( void );
GtkWidget *cree_menu_quatres_lignes ( void );
void allocation_clist_affichage_liste ( GtkWidget *clist,
					GtkAllocation *allocation );
gboolean pression_bouton_classement_liste ( GtkWidget *clist,
					    GdkEventButton *ev );
gboolean lache_bouton_classement_liste ( GtkWidget *clist,
					 GdkEventButton *ev );
void remplissage_tab_affichage_ope ( GtkWidget *clist );
void toggled_bouton_affichage_liste ( GtkWidget *bouton,
				      gint *no_bouton );
void changement_taille_liste_affichage ( GtkWidget *clist,
					 GtkAllocation *allocation );
void recuperation_noms_colonnes_et_tips ( void );
void raz_affichage_ope ( void );

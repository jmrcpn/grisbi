GtkWidget *creation_details_compte ( void );
GtkWidget *creation_menu_type_compte ( void );
void changement_bouton_adresse_commune_perso ( void );
void modif_detail_compte ( GtkWidget *hbox );
void remplissage_details_compte ( void );
gint recherche_banque_par_no ( struct struct_banque *banque,
			       gint *no_banque );
void modification_details_compte ( void );
void sort_du_detail_compte ( void );
void passage_a_l_euro ( GtkWidget *bouton,
			gpointer null );
void changement_de_banque ( GtkWidget * menu_shell );

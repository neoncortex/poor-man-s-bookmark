#include "edit.h"

static void
move_directory(char* tag)
{
	GtkTreeIter 	iter, iter_copy, child, parent;

	bookmark_list* 	bl 	= bookmark_list_new(); 
	bookmark* 	b 	= NULL;
	bookmark*	sb	= get_data(NULL);
	unsigned int 	parents = 0;

	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, selected_path))
	{
		iter_copy = iter;

		while(gtk_tree_model_iter_parent(GTK_TREE_MODEL(model)
			,&parent, &iter_copy))
		{
			parents++;
			iter_copy = parent;
		}

		if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &child, &iter, 0))
			collect_bookmark(child, bl);
	}

	while((b = bookmark_list_return_next_bookmark(bl)))
	{
		unsigned int	id		= strtol(bookmark_id(b), NULL, 10);
		char*		bm_tag		= bookmark_tag(b);
		char*		bm_tag_bkp 	= strdup(bm_tag);
		char*		res		= NULL;

		if(parents > 0)
			strsep(&bm_tag, "//");

		/*
		printf("--- %s ---\nbookmark_id(sb) = \t %s\n", bookmark_id(sb), bookmark_id(sb));
		printf("bookmark_tag(b) = \t %s\n", bookmark_tag(b));
		printf("bm_tag = \t\t%s\n\n", bm_tag);
		printf("%d\n", parents);
		*/

		if(!bm_tag)
			bookmark_db_edit(db, id, 3, tag);

		else if(strlen(tag) < 2)
		{
			char* n_tag = NULL;

			while(bm_tag)
			{
				n_tag = strsep(&bm_tag, "//");

				if(!(strcmp(n_tag, bookmark_id(sb))))
					break;
			}

			if(bm_tag)
			{
				char* new_tag = calloc(1, (strlen(n_tag) + strlen(bm_tag) + 2) * sizeof(char));

				snprintf(new_tag, strlen(new_tag) - 1, "%s/%s"
					,n_tag, bm_tag);

				bookmark_db_edit(db, id, 3, new_tag);
				free(new_tag);
			}
			else
				bookmark_db_edit(db, id, 3, n_tag);
		}
		else
		{
			if(parents > 1)
			{
				while(strcmp(bm_tag, bookmark_id(sb)))
				{
					res = strsep(&bm_tag, "//");

					if(!(strcmp(res, bookmark_id(sb))))
						break;
				}
			}

			char* new_tag = calloc(1, (strlen(tag) + strlen(bm_tag) + 2) * sizeof(char));

			snprintf(new_tag, strlen(new_tag) - 1, "%s/%s"
					,tag, bm_tag);

			bookmark_db_edit(db, id, 3, new_tag);
			free(new_tag);
		}

		free(bm_tag_bkp);
		bookmark_destroy(b);
	}
	
	if(sb)
		bookmark_destroy(sb);

	if(b)
		bookmark_destroy(b);

	if(bl)
		bookmark_list_destroy(bl);
}

static void
rename_directory(GtkWidget* button, gpointer** args)
{
	bookmark* 	b 		= get_data(NULL);
	bookmark*	bt		= NULL;
	char* 		path 		= get_full_path(b);
	bookmark_list*	bl 		= bookmark_db_search(db, TAG, path);
	char* 		new_tag 	= (char*)gtk_combo_box_text_get_active_text
						(GTK_COMBO_BOX_TEXT(args[0]));
	
	while((bt = bookmark_list_return_next_bookmark(bl)))
	{
		if(new_tag)
		{
			char* new_tag_bkp 	= strdup(new_tag);
			char* b_tag 		= bookmark_tag(bt);
			char* nt_sep 		= strsep(&new_tag_bkp, "//");
			char* tag_sep 		= strsep(&b_tag, "//");
			char* tag_root		= NULL;

			if(nt_sep)
			{
				tag_root = realloc(tag_root, (strlen(nt_sep) + 1) * sizeof(char));
				strcpy(tag_root, nt_sep);
			}

			while(b_tag && new_tag_bkp && !(strcmp(nt_sep, tag_sep)))
			{
				nt_sep 			= strsep(&new_tag_bkp, "//");
				tag_sep 		= strsep(&b_tag, "//");
				char* 		tr_bkp 	= strdup(tag_root);
				unsigned int 	size 	= ((strlen(tr_bkp) * sizeof(char)) 
								+ ((strlen(nt_sep) + 2) * sizeof(char))) 
								* sizeof(char);

				tag_root		= realloc(tag_root, size); 

				snprintf(tag_root, size - 1, tr_bkp, nt_sep);
			}

			if(b_tag)
			{
				unsigned int 	size 	= ((strlen(new_tag) * sizeof(char)) 
								+ (strlen(b_tag) * sizeof(char)) + 2) 
								* sizeof(char);
				char* 		tag 	= calloc(1, size);

				snprintf(tag, size, "%s/%s", new_tag, b_tag);

				if(tag)
				{
					bookmark_db_edit(db, strtol(bookmark_id(bt), NULL, 10), 3, tag);
					free(tag);
				}
			}
			else
				bookmark_db_edit(db, strtol(bookmark_id(bt), NULL, 10), 3, new_tag);
		}

		bookmark_destroy(bt);
	}

	bookmark_destroy(b);
	bookmark_list_destroy(bl);
	close_window(NULL, args[1]);
	g_free(args);
	read_database(NULL, NULL);
}

static void
edit_bookmark(GtkWidget* button, gpointer** args) 
{
	char* name 	= (char*)gtk_entry_get_text(GTK_ENTRY(args[0]));
	char* url 	= (char*)gtk_entry_get_text(GTK_ENTRY(args[1]));
	char* comment 	= (char*)gtk_entry_get_text(GTK_ENTRY(args[2]));
	char* tag 	= (char*)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(args[3]));
	char* message	= NULL;

	if(url) 
	{
		bookmark_list* bl = bookmark_db_search(db, URL, url);
		
		if(bl) 
		{
			bookmark* b = bookmark_list_return_next_bookmark(bl);

			if(b) 
			{
				unsigned int id = strtol(bookmark_id(b), NULL, 10);

				if(name)
					bookmark_db_edit(db, id, 0, name);

				if(url)
					bookmark_db_edit(db, id, 1, url);

				if(comment)
					bookmark_db_edit(db, id, 2, comment);

				if(tag)
					bookmark_db_edit(db, id, 3, tag);

				unsigned int size = (strlen(bookmark_id(b))
						+ strlen(bookmark_url(b)) 
						+ strlen("edit: id = , url = ") + 3)
						* sizeof(char);

				message = calloc(1, size);

				snprintf(message, size - 1, "edit: id = %s, url = %s"
					,bookmark_id(b), bookmark_url(b));

				bookmark_destroy(b);
			}

			bookmark_list_destroy(bl);
		}
	}

	close_window(NULL, args[4]);
	g_free(args);
	read_database(NULL, NULL);
	gtk_label_set_text(GTK_LABEL(info_label), message);
	free(message);
}

static void
move_directory_wrapper(GtkWidget* button, gpointer** args) 
{
	move_directory(((char*)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(args[0]))));
	close_window(NULL, args[1]);
	g_free(args);
	read_database(NULL, NULL);

	gboolean r;
	g_signal_emit_by_name(treeview, "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, -1, &r);
}

static void
move_multiple(GtkWidget* button, gpointer** args)
{
	char* 	tag	= (char*)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(args[0]));
	GList* 	rows 	= gtk_tree_selection_get_selected_rows
				(GTK_TREE_SELECTION(selection)
				,&model);

	do
	{
		if(rows)
		{
			bookmark* b = get_data(rows->data);

			if(strlen(bookmark_url(b)) > 1)
				bookmark_db_edit(db, (strtol(bookmark_id(b), NULL, 10)), 3, tag);
			else
			{
				selected_path = rows->data;	
				move_directory(tag);
			}

			bookmark_destroy(b);
		}
		else
			break;
	}
	while((rows = rows->next));

	g_list_free_full(rows, (GDestroyNotify) gtk_tree_path_free);
	close_window(NULL, args[1]);
	g_free(args);
	read_database(NULL, NULL);

	gboolean r;
	g_signal_emit_by_name(treeview, "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, -1, &r);
}

static void
edit_bookmark_window(bookmark* b, gpointer main_window) 
{
	GtkWidget* 	window 	= dialogs("Edit bookmark", main_window);
	GtkWidget** 	e 	= entries(TRUE);
	GtkWidget* 	tag_box = tag_box_new();

	if(b) 
	{
		if(bookmark_name(b))
			gtk_entry_set_text(GTK_ENTRY(e[1]), bookmark_name(b));

		if(bookmark_url(b))
			gtk_entry_set_text(GTK_ENTRY(e[3]), bookmark_url(b));

		if(bookmark_comment(b))
			gtk_entry_set_text(GTK_ENTRY(e[5])
				,bookmark_comment(b));

		GtkWidget* tag_entry = gtk_bin_get_child(GTK_BIN(tag_box));
		gtk_entry_set_text(GTK_ENTRY(tag_entry), get_full_path(b));
		bookmark_destroy(b);
	}

	GtkWidget** edit_bookmark_args = g_new(GtkWidget*, 5);
	edit_bookmark_args[0] = e[1];
	edit_bookmark_args[1] = e[3];
	edit_bookmark_args[2] = e[5];
	edit_bookmark_args[3] = tag_box;
	edit_bookmark_args[4] = window;

	/* button */
	GtkWidget* edit_button = gtk_button_new_with_mnemonic("_Edit");
	g_signal_connect(edit_button, "clicked", G_CALLBACK(edit_bookmark)
		,edit_bookmark_args);

	GtkWidget* cancel_button = gtk_button_new_with_mnemonic("_Cancel");
	g_signal_connect(cancel_button, "clicked", G_CALLBACK(close_window)
		,window);

	/* grid */
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), 1);

	gtk_grid_attach(GTK_GRID(grid), e[0] 		,0,  0, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), e[1] 		,20, 0, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), e[2] 		,0,  1, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), e[3] 		,20, 1, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), e[4] 		,0,  2, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), e[5] 		,20, 2, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), e[6] 		,0,  3, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), tag_box 	,20, 3, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), edit_button 	,20, 5, 20, 10);
	gtk_grid_attach(GTK_GRID(grid), cancel_button 	,40, 5, 20, 10);

	g_free(e);
	gtk_container_add(GTK_CONTAINER(window), grid);
	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_spinner_start(GTK_SPINNER(spinner));
}

static void
move_directory_window(bookmark* b, gpointer main_window)
{
	GtkWidget* window 	= dialogs("Move directory", main_window);
	GtkWidget* tag_box 	= tag_box_new();

	GtkWidget* name_entry_label = gtk_label_new("To");
	gtk_widget_set_halign(GTK_WIDGET(name_entry_label), GTK_ALIGN_START);

	if(b)
	{
		GtkWidget* tag_entry = gtk_bin_get_child(GTK_BIN(tag_box));
		gtk_entry_set_text(GTK_ENTRY(tag_entry), get_full_path(b));
		bookmark_destroy(b);
	}

	GtkWidget** move_directory_args = g_new(GtkWidget*, 2);
	move_directory_args[0] = tag_box;
	move_directory_args[1] = window;

	/* button */
	GtkWidget* edit_button = gtk_button_new_with_mnemonic("_Move");
	g_signal_connect(edit_button, "clicked", G_CALLBACK(move_directory_wrapper)
		,move_directory_args);

	GtkWidget* cancel_button = gtk_button_new_with_mnemonic("_Cancel");
	g_signal_connect(cancel_button, "clicked", G_CALLBACK(close_window)
		,window);

	/* grid */
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), 1);

	gtk_grid_attach(GTK_GRID(grid), name_entry_label 	,0,  0, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), tag_box 		,20, 0, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), edit_button 		,20, 1, 20, 10);
	gtk_grid_attach(GTK_GRID(grid), cancel_button 		,40, 1, 20, 10);

	gtk_container_add(GTK_CONTAINER(window), grid);
	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_spinner_start(GTK_SPINNER(spinner));
}

static void
move_multiple_window(gpointer main_window)
{
	GtkWidget* window 	= dialogs("Move multiple", main_window);
	GtkWidget* tag_box 	= tag_box_new();

	GtkWidget* name_entry_label = gtk_label_new("To");
	gtk_widget_set_halign(GTK_WIDGET(name_entry_label), GTK_ALIGN_START);

	GtkWidget** move_multiple_args = g_new(GtkWidget*, 2);
	move_multiple_args[0] = tag_box;
	move_multiple_args[1] = window;

	/* button */
	GtkWidget* edit_button = gtk_button_new_with_mnemonic("_Edit");
	g_signal_connect(edit_button, "clicked", G_CALLBACK(move_multiple)
		,move_multiple_args);

	GtkWidget* cancel_button = gtk_button_new_with_mnemonic("_Cancel");
	g_signal_connect(cancel_button, "clicked", G_CALLBACK(close_window)
		,window);

	/* grid */
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), 1);

	gtk_grid_attach(GTK_GRID(grid), name_entry_label 	,0,  0, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), tag_box 		,20, 0, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), edit_button 		,20, 1, 20, 10);
	gtk_grid_attach(GTK_GRID(grid), cancel_button 		,40, 1, 20, 10);

	gtk_container_add(GTK_CONTAINER(window), grid);
	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_spinner_start(GTK_SPINNER(spinner));
}

static void
rename_directory_window(gpointer main_window)
{
	GtkWidget* window 	= dialogs("Rename directory", main_window);
	GtkWidget* tag_box 	= tag_box_new();

	GtkWidget* name_entry_label = gtk_label_new("Name");
	gtk_widget_set_halign(GTK_WIDGET(name_entry_label), GTK_ALIGN_START);

	GtkWidget** rename_multiple_args = g_new(GtkWidget*, 2);
	rename_multiple_args[0] = tag_box;
	rename_multiple_args[1] = window;

	/* button */
	GtkWidget* edit_button = gtk_button_new_with_mnemonic("_Rename");
	g_signal_connect(edit_button, "clicked", G_CALLBACK(rename_directory)
		,rename_multiple_args);

	GtkWidget* cancel_button = gtk_button_new_with_mnemonic("_Cancel");
	g_signal_connect(cancel_button, "clicked", G_CALLBACK(close_window)
		,window);

	/* grid */
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), 1);

	gtk_grid_attach(GTK_GRID(grid), name_entry_label 	,0,  0, 30, 1);
	gtk_grid_attach(GTK_GRID(grid), tag_box 		,20, 0, 50, 1);
	gtk_grid_attach(GTK_GRID(grid), edit_button 		,20, 1, 20, 10);
	gtk_grid_attach(GTK_GRID(grid), cancel_button 		,40, 1, 20, 10);

	gtk_container_add(GTK_CONTAINER(window), grid);
	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_spinner_start(GTK_SPINNER(spinner));
}

void
rename_directory_wrapper(gpointer main_window)
{
	bookmark* b = get_data(NULL);

	if((strlen(bookmark_url(b)) < 2))
		rename_directory_window(main_window);
	
	bookmark_destroy(b);
}

void
edit(GtkWidget* button, gpointer main_window)
{
	if(gtk_tree_selection_count_selected_rows
		(GTK_TREE_SELECTION(selection)) > 1)
	{
		move_multiple_window(main_window);
	}
	else
	{
		bookmark* b = get_data(NULL);

		if(b)
		{
			if(strlen(bookmark_url(b)) > 1)
				edit_bookmark_window(b, main_window);
			else
				move_directory_window(b, main_window);
		}
	}
}


#include "dialog.h"

char*
get_full_path(bookmark* b)
{
  if(strlen(bookmark_tag(b)) > 1)
    {
      return bookmark_tag(b);
    }
  else if(bookmark_id(b))
    {
      GtkTreeIter iter, parent;

      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(g_model), &iter
				 ,g_selected_path))
	{
	  GtkTreePath* path;
	  unsigned int size = 1;
	  char** parents = calloc(size, sizeof(char*));
	  check_oom(parents, "dialog > get_full_path - parents");
	  char*	dir = bookmark_id(b);

	  while(gtk_tree_model_iter_parent(GTK_TREE_MODEL(g_model)
					   ,&parent, &iter))
	    {
	      path = gtk_tree_model_get_path
		(GTK_TREE_MODEL(g_model), &parent);
	      
	      bookmark* bp = get_bookmark_from_row(path);
	      iter = parent;

	      parents[size - 1]	= bookmark_id(bp);
	      parents = realloc(parents, (size + 1) * sizeof(char*));
	      check_oom(parents, "dialog > get_full_path - parents");	      
	      ++size;

	      bookmark_destroy(bp);
	      gtk_tree_path_free(path);
	    }

	  char*	complete_path = calloc(1, sizeof(char));
	  check_oom(complete_path, "dialog > get_full_path - complete_path");
	  unsigned int size_bkp = size - 1;

	  while(size_bkp)
	    {
	      size = strlen(complete_path)
		+ 3;
	      
	      char* t_path = calloc(size, sizeof(char));
	      check_oom(t_path, "dialog > get_full_path - t_path");
	      
	      strcpy(t_path, complete_path);
	      free(complete_path);

	      if(strlen(t_path) > 1)
		{	  
		  size = strlen(t_path)
		    + strlen(parents[size_bkp - 1])
		    + 2;	      

		  complete_path = calloc(size, sizeof(char));
		  
 		  check_oom(complete_path
			    ,"dialog > get_full_path - complete_path");

		  snprintf(complete_path, size, "%s/%s"
			   ,t_path, parents[size_bkp - 1]);
		}
	      else
		{
		  size = strlen(parents[size_bkp - 1])
		    + 1;
		  
		  complete_path = calloc(size, sizeof(char));

 		  check_oom(complete_path
			    ,"dialog > get_full_path - complete_path");
		  
		  snprintf(complete_path, size, "%s"
			   ,parents[size_bkp - 1]);
		}

	      free(parents[size_bkp - 1]);
	      free(t_path);
	      size_bkp--;
	    }

	  if(strlen(complete_path) > 1)
	    {
	      complete_path[strlen(complete_path)] = '/';
	    }

	  size = strlen(complete_path)
	    + strlen(dir)
	    + 2;
	  
	  char* full_path = malloc(size * sizeof(char));
	  check_oom(full_path, "dialog > get_full_path - full_path");
	  
	  snprintf(full_path, size, "%s%s"
		   ,complete_path, dir);

	  free(parents);
	  free(complete_path);
	  return full_path;
	}
      else
	{
	  return bookmark_id(b);
	}
    }

  return NULL;
}

void
collect_bookmark(GtkTreeIter iter, bookmark_list* bl)
{
  GtkTreeIter 	child;
  GtkTreePath*	path;

  do
    {
      path = gtk_tree_model_get_path(GTK_TREE_MODEL(g_model), &iter);
      bookmark* b = get_bookmark_from_row(path);

      if(b)
	{
	  if(strlen(bookmark_url(b)) > 1)
	    bookmark_list_enqueue_bookmark(bl, b);

	  bookmark_destroy(b);
	}

      if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(g_model), &child, &iter
				       ,0))
	{
	  collect_bookmark(child, bl);
	}
    }
  while(gtk_tree_model_iter_next(GTK_TREE_MODEL(g_model), &iter));

  gtk_tree_path_free(path);
}

GtkWidget**
entries(gboolean editable)
{
  /* labels */
  GtkWidget* name_label = gtk_label_new("Name");
  gtk_widget_set_halign(GTK_WIDGET(name_label), GTK_ALIGN_START);

  GtkWidget* url_label = gtk_label_new("Url");
  gtk_widget_set_halign(GTK_WIDGET(url_label), GTK_ALIGN_START);

  GtkWidget* comment_label = gtk_label_new("Comment");
  gtk_widget_set_halign(GTK_WIDGET(comment_label), GTK_ALIGN_START);

  GtkWidget* tag_label = gtk_label_new("Tag");
  gtk_widget_set_halign(GTK_WIDGET(tag_label), GTK_ALIGN_START);

  /* entries */
  GtkWidget* name_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "name");
  gtk_entry_set_icon_from_icon_name
    (GTK_ENTRY(name_entry), GTK_ENTRY_ICON_PRIMARY, "stock_add-bookmark");

  GtkWidget* url_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(url_entry), "url");

  GtkWidget* comment_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(comment_entry), "comment");
  gtk_entry_set_icon_from_icon_name
    (GTK_ENTRY(comment_entry), GTK_ENTRY_ICON_PRIMARY, "user-available");

  GtkWidget* tag_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(tag_entry), "tag");

  if(!editable)
    {
      gtk_editable_set_editable(GTK_EDITABLE(name_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(url_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(comment_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(tag_entry), FALSE);
    }

  GtkWidget** r = g_new(GtkWidget*, 8);
  r[0] = name_label;
  r[1] = name_entry;
  r[2] = url_label;
  r[3] = url_entry;
  r[4] = comment_label;
  r[5] = comment_entry;
  r[6] = tag_label;
  r[7] = tag_entry;

  return r;
}

GdkPixbuf*
favicon_decode(bookmark* b)
{
  if(strcmp(bookmark_favicon(b), "none"))
    {
      gsize favicon_size = strlen(bookmark_favicon(b));
      guchar* favicon = g_base64_decode(bookmark_favicon(b), &favicon_size);

      GInputStream* memst = g_memory_input_stream_new_from_data
	(favicon, favicon_size, NULL);

      GdkPixbuf* icon = gdk_pixbuf_new_from_stream(memst, NULL, NULL);

      g_input_stream_close(memst, NULL, NULL);
      g_free(favicon);
      g_object_unref(memst);
      
      if(icon)
	{
	  return icon;
	}
      else
	{
	  return NULL;
	}
    }

  return NULL;
}

void
set_url_favicon(GtkWidget* entry)
{
  GdkPixbuf* icon;
  GtkTreeIter iter;

  gtk_tree_model_get_iter(GTK_TREE_MODEL(g_model), &iter, g_selected_path);
  gtk_tree_model_get(GTK_TREE_MODEL(g_model), &iter, 5, &icon, -1);

  gtk_entry_set_icon_from_pixbuf
    (GTK_ENTRY(entry), GTK_ENTRY_ICON_PRIMARY , icon);
}

/* get all favicons */
static gboolean
favicon_foreach(GtkTreeModel* m, GtkTreePath* p, GtkTreeIter* i
		,gpointer data)
{
  bookmark* b = get_bookmark_from_row(p);

  if(strlen(bookmark_url(b)) > 2)
    {
      char* favicon = download_favicon(bookmark_url(b));

      if(favicon)
	{
	  bookmark_db_edit(g_db, strtol(bookmark_id(b), NULL, 10), 4
			   ,favicon);
	}
    }

  bookmark_destroy(b);

  return FALSE;
}

void
get_all_favicons(GtkWidget* button)
{
  gtk_tree_model_foreach(GTK_TREE_MODEL(g_model), favicon_foreach, NULL);
}

GtkWidget*
grid_new()
{
  GtkWidget* grid = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), 1);

  return grid;
}

void  
close_window(GtkWidget* button, gpointer window) 
{
  gtk_widget_destroy(window);
  gtk_spinner_stop(GTK_SPINNER(g_spinner));
}

void
dialog_key_press(GtkWidget* window, GdkEventKey* e)
{
  char* key = gdk_keyval_name(e->keyval);

  if(!strcmp(key, "Escape"))
    {
      close_window(NULL, window);
    }
}

GtkWidget*
dialog_new(char* title, gpointer main_window)
{
  if(title && main_window)
    {
      GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title(GTK_WINDOW(window), title);
      gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
      gtk_window_set_transient_for
	(GTK_WINDOW(window), GTK_WINDOW(main_window));

      g_signal_connect
	(window, "key-release-event", G_CALLBACK(dialog_key_press), NULL);

      g_signal_connect
	(window, "destroy", G_CALLBACK(close_window), window);

      return window;
    }
	
  return NULL;
}

/*
 * list.c
 *
 * Copyright 2013-2015 AmatCoder
 *
 * This file is part of Mednaffe.
 *
 * Mednaffe is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mednaffe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mednaffe; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"
#include <string.h>

#ifdef G_OS_WIN32
  #include <windows.h>
#endif

void change_list (guidata *gui)
{
  GtkAdjustment *adjustament;
  gchar *buff, *total;

  buff=g_strdup_printf("%i",
    gtk_tree_model_iter_n_children(GTK_TREE_MODEL(gui->store), NULL));

  total = g_strconcat(" Games in list: ", buff, NULL);
  gtk_statusbar_pop(GTK_STATUSBAR(gui->sbnumber), 1);
  gtk_statusbar_pop(GTK_STATUSBAR(gui->sbname), 1);
  gtk_statusbar_push(GTK_STATUSBAR(gui->sbnumber), 1, total);
  gtk_statusbar_push(GTK_STATUSBAR(gui->sbname), 1, " Game selected: None");
  g_free(gui->rom);
  g_free(gui->fullpath);
  gui->rom = NULL;
  gui->fullpath = NULL;
  g_free(total);
  g_free(buff);

  adjustament =
    gtk_scrolled_window_get_vadjustment(
                                GTK_SCROLLED_WINDOW(gui->scrollwindow));

  gtk_adjustment_set_value (adjustament, 0);
  gtk_widget_grab_focus(gui->gamelist);

}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void remove_folder(GtkWidget *sender, guidata *gui)
{
  GtkTreeIter iter, iter2;
  GtkListStore *combostore;

  combostore =
    GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(gui->cbpath)));

  if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(gui->cbpath), &iter))
  {
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(combostore), &iter2))
      gtk_list_store_remove(combostore, &iter);

    gtk_list_store_clear(gui->store);
    gtk_statusbar_pop(GTK_STATUSBAR(gui->sbnumber), 1);
    gtk_statusbar_push(GTK_STATUSBAR(gui->sbnumber), 1,
                                                  " Games in list: 0 ");

    gtk_statusbar_pop(GTK_STATUSBAR(gui->sbname), 1);
    gtk_statusbar_push(GTK_STATUSBAR(gui->sbname), 1,
                                                " Game selected: None");

    g_free(gui->rompath);
    g_free(gui->rom);
    gui->rompath=NULL;
    gui->rom=NULL;
  }
}

void add_combo(GtkComboBox *combopath, const char *string)
{
  GtkTreeIter iter;
  GtkListStore *combostore;

  combostore = GTK_LIST_STORE(gtk_combo_box_get_model(combopath));

  gtk_list_store_prepend(combostore, &iter);
  gtk_list_store_set(combostore, &iter, 0, string, -1);

  gtk_combo_box_set_active(combopath, 0);
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void open_folder(GtkWidget *sender, guidata *gui)
{
  GtkWidget *folder;

#ifdef GTK2_ENABLED
  folder = gtk_file_chooser_dialog_new(
    "Choose a ROM folder...", GTK_WINDOW(gui->topwindow),
    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL,
    GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL );
#else
  folder = gtk_file_chooser_dialog_new(
    "Choose a ROM folder...", GTK_WINDOW(gui->topwindow),
    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, ("_Cancel"),
    GTK_RESPONSE_CANCEL, ("_Open"), GTK_RESPONSE_ACCEPT, NULL);
#endif

  if (gtk_dialog_run(GTK_DIALOG(folder)) == GTK_RESPONSE_ACCEPT)
  {
    gchar *path;

    path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (folder));
    add_combo(GTK_COMBO_BOX(gui->cbpath), path);
    g_free(path);
  }
  gtk_widget_destroy(folder);
}

int descend_sort(const void * a, const void * b)
{
  return strcmp(a, b);
}

int ascend_sort(const void * a, const void * b)
{
  return strcmp(b, a);
}

#ifdef G_OS_WIN32 /* g_file_test is too slow on Windows */

void scan_files(gchar *romdir, guidata *gui)
{
  WIN32_FIND_DATA FindFileData;

  gchar *romdir2 = g_strconcat(romdir, G_DIR_SEPARATOR_S, "*", NULL);
  HANDLE hFind = FindFirstFile(romdir2, &FindFileData);

  while (hFind != INVALID_HANDLE_VALUE)
  {
    gchar *testdir = g_strconcat(romdir, G_DIR_SEPARATOR_S, FindFileData.cFileName, NULL);
    if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      if (gui->listmode == 1 && (0 != strcmp(FindFileData.cFileName, ".")
        && 0 != strcmp (FindFileData.cFileName, "..")))
      {
        scan_files(testdir, gui);
      }
    }
    else
    {
    if (gui->filter == 0)
    {
      gui->itemlist = g_slist_prepend(gui->itemlist,
        g_strconcat(FindFileData.cFileName, G_DIR_SEPARATOR_S, testdir, NULL));
    }
      else if ((gui->filter == 1) && (g_str_has_suffix(FindFileData.cFileName, ".zip") ||
                               g_str_has_suffix(FindFileData.cFileName, ".ZIP")))
      {
        gui->itemlist = g_slist_prepend(gui->itemlist,
          g_strconcat(FindFileData.cFileName, G_DIR_SEPARATOR_S, testdir, NULL));
      }
      else if ((gui->filter == 2) && (g_str_has_suffix(FindFileData.cFileName, ".cue") ||
                     g_str_has_suffix(FindFileData.cFileName, ".toc") ||
                     g_str_has_suffix(FindFileData.cFileName, ".ccd") ||
                     g_str_has_suffix(FindFileData.cFileName, ".m3u") ||
                     g_str_has_suffix(FindFileData.cFileName, ".CUE") ||
                     g_str_has_suffix(FindFileData.cFileName, ".TOC") ||
                     g_str_has_suffix(FindFileData.cFileName, ".CCD") ||
                     g_str_has_suffix(FindFileData.cFileName, ".M3U")))
      {
        gui->itemlist = g_slist_prepend(gui->itemlist,
          g_strconcat(FindFileData.cFileName, G_DIR_SEPARATOR_S, testdir, NULL));
      }
    }
    g_free(testdir);

    if (!FindNextFile(hFind, &FindFileData))
    {
      FindClose(hFind);
      hFind = INVALID_HANDLE_VALUE;
    }
  }
  g_free(romdir2);
}

#else

void scan_files(gchar *romdir, guidata *gui)
{
  GDir *dir = NULL;

  dir = g_dir_open(romdir, 0, NULL);

  if (dir != NULL)
  {
    const gchar *file = NULL;

    while ((file=g_dir_read_name(dir)) != NULL)
    {
      gchar *testdir = g_strconcat(romdir, G_DIR_SEPARATOR_S, file, NULL);

      if (!g_file_test (testdir, G_FILE_TEST_IS_DIR))
      {
        if (gui->filter == 0)
        {
          gui->itemlist = g_slist_prepend(gui->itemlist,
            g_strconcat(file, G_DIR_SEPARATOR_S, testdir, NULL));
        }
        else if ((gui->filter == 1) && (g_str_has_suffix(file, ".zip") ||
                               g_str_has_suffix(file, ".ZIP")))
        {
          gui->itemlist = g_slist_prepend(gui->itemlist,
            g_strconcat(file, G_DIR_SEPARATOR_S, testdir, NULL));
        }
        else if ((gui->filter == 2) && (g_str_has_suffix(file, ".cue") ||
                     g_str_has_suffix(file, ".toc") ||
                     g_str_has_suffix(file, ".ccd") ||
                     g_str_has_suffix(file, ".m3u") ||
                     g_str_has_suffix(file, ".CUE") ||
                     g_str_has_suffix(file, ".TOC") ||
                     g_str_has_suffix(file, ".CCD") ||
                     g_str_has_suffix(file, ".M3U")))
        {
          gui->itemlist = g_slist_prepend(gui->itemlist,
            g_strconcat(file, G_DIR_SEPARATOR_S, testdir, NULL));
        }
      }
      else
      {
        if (gui->listmode == 1)
          scan_files(testdir, gui);
      }
      g_free(testdir);
    }
    g_dir_close(dir);
  }
}

#endif

void sort_items(guidata *gui)
{
  if (gtk_tree_view_column_get_sort_order(gui->column) == GTK_SORT_ASCENDING)
    gui->itemlist = g_slist_sort(gui->itemlist, (GCompareFunc)descend_sort);
  else
    gui->itemlist = g_slist_sort(gui->itemlist, (GCompareFunc)ascend_sort);
}

void populate_list(guidata *gui)
{
  GtkTreeIter iter;
  GSList *iterator = NULL;
  gint i = 0;

  for (iterator = gui->itemlist; iterator; iterator = iterator->next)
  {
    gchar **str;

  str = g_strsplit (iterator->data, G_DIR_SEPARATOR_S, 2);

    gtk_list_store_insert_with_values(gui->store, &iter, -1,
                           0, str[0], 1, str[1], -1);
    i++;
    g_strfreev(str);
  }
}

void scan_dir(gchar *romdir, guidata *gui)
{
  g_slist_free_full(gui->itemlist, g_free);
  gui->itemlist = NULL;
  scan_files(romdir, gui);
  sort_items(gui);
  populate_list(gui);
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void fill_list(GtkComboBox *combobox, guidata *gui)
{
  GtkTreeModel *model;
  GtkTreeIter iter;


  model=gtk_combo_box_get_model(GTK_COMBO_BOX(gui->cbpath));

  if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(gui->cbpath), &iter))
  {
    g_free(gui->rompath);
    gtk_tree_model_get(model, &iter, 0 ,&gui->rompath, -1);
    gtk_tree_view_set_model(GTK_TREE_VIEW(gui->gamelist), NULL);
    gtk_list_store_clear(gui->store);
    if (gui->rompath!=NULL)
      scan_dir(gui->rompath, gui);

    gtk_tree_view_set_model(GTK_TREE_VIEW(gui->gamelist),
                            GTK_TREE_MODEL(gui->store));
    change_list(gui);

    if (gtk_tree_model_get_iter_first(gtk_tree_view_get_model(
                                  GTK_TREE_VIEW(gui->gamelist)), &iter))
    {
      gtk_tree_selection_select_iter(gtk_tree_view_get_selection(
                                  GTK_TREE_VIEW(gui->gamelist)), &iter);
    }
  }
  gtk_tree_view_column_set_sort_indicator(gui->column, TRUE);


}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void header_clicked(GtkTreeViewColumn *treeviewcolumn, guidata *gui)
{
  if (gtk_tree_view_column_get_sort_order(gui->column) == GTK_SORT_ASCENDING)
    gtk_tree_view_column_set_sort_order(gui->column, GTK_SORT_DESCENDING);
  else
    gtk_tree_view_column_set_sort_order(gui->column, GTK_SORT_ASCENDING);

   gtk_tree_view_set_model(GTK_TREE_VIEW(gui->gamelist), NULL);
  gtk_list_store_clear(gui->store);
  gui->itemlist = g_slist_reverse(gui->itemlist);
  populate_list(gui);
  gtk_tree_view_set_model(GTK_TREE_VIEW(gui->gamelist),
                          GTK_TREE_MODEL(gui->store));

  gtk_tree_view_column_set_sort_indicator(gui->column, TRUE);
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void on_radiomenuall_activate(GtkMenuItem *menuitem, guidata *gui)
{
  if (gui->filter != 0)
  {
    gui->filter=0;
    fill_list(NULL, gui);
    gtk_tree_view_column_set_title(gui->column, " Games");
  }
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void on_radiomenuzip_activate(GtkMenuItem *menuitem, guidata *gui)
{
  if (gui->filter != 1)
  {
    gui->filter=1;
    fill_list(NULL, gui);
    gtk_tree_view_column_set_title(gui->column, " Games (zip)");
  }
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void on_radiomenucue_activate(GtkMenuItem *menuitem, guidata *gui)
{
  if (gui->filter != 2)
  {
    gui->filter=2;
    fill_list(NULL, gui);
    gtk_tree_view_column_set_title(gui->column, " Games (cue/toc/ccd/m3u)");
  }
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void on_normalmenu_activate(GtkMenuItem *menuitem, guidata *gui)
{
  if (gui->listmode != 0)
  {
    gui->listmode = 0;
    fill_list(NULL,gui);
  }
}

#ifdef G_OS_WIN32
G_MODULE_EXPORT
#endif
void on_recursivemenu_activate(GtkMenuItem *menuitem, guidata *gui)
{
  if (gui->listmode != 1)
  {
    gui->listmode = 1;
    fill_list(NULL,gui);
  }
}

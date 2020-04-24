/****************************************
Пример использования SQLite и GtkTreeView
*****************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>

GtkWidget   *window1;
GtkBuilder  *builder1;

sqlite3 *db;    // хэндл БД

// Структуры данных для виджета
GtkListStore        *buffer;    // Буфер таблицы
GtkTreeView         *table;     // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначения полей
enum
{
    C_NUM, C_NAME, C_OKLAD
};

// Структуры данных для базы данных
sqlite3_stmt* stmt; // строка запроса к БД
char *err = 0;
int rc = 0;


int ShowGladeError()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                 "Не найден файл\n main.glade");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return 0;
}


char str[1000];
int lenstr=0;

// Открываем БД
void StartDB()
{
    if( sqlite3_open("b1.db", &db))
    {
        sprintf(str, "Ошибка открытия БД: %s\n", sqlite3_errmsg(db));
    }
}

void EndDB()
{
    sqlite3_close(db);
}

// Запись в БД
int SaveDB()
{
    char* SQL1 = "DROP TABLE IF EXISTS Сотрудники; \
            CREATE TABLE Сотрудники(N,Фамилия,Оклад); \
            INSERT INTO Сотрудники VALUES(1,'Иванов',20000);  \
            INSERT INTO Сотрудники VALUES(2,'Петров',25000);  \
            INSERT INTO Сотрудники VALUES(3,'Сидоров',30000);  \
            ";


     if (sqlite3_exec(db, SQL1, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 2;
    }
    return 0;
}


// Загрузка таблицы из БД в буфер виджета
int LoadDB()
{
    char* SQL2 = "SELECT N, Фамилия, Оклад FROM Сотрудники;";
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        gtk_list_store_append (buffer, &iter);
        gtk_list_store_set (buffer, &iter,
                       C_NUM, sqlite3_column_int(stmt, C_NUM),
                       C_NAME, sqlite3_column_text(stmt, C_NAME),
                       C_OKLAD, (guint) sqlite3_column_int(stmt, C_OKLAD),
                      -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // 5. Освобождаем строку запроса
    sqlite3_finalize(stmt);

    return 0;
}

void ShowError ()
{
    GtkWidget *dialog = NULL;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window1), GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, str);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

int ShowMainWindow()
{
	builder1 = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder1, "main.glade", NULL))
	{
        window1 = GTK_WIDGET(gtk_builder_get_object(builder1, "window1"));
        buffer =  GTK_LIST_STORE(gtk_builder_get_object(builder1, "liststore1"));
        table =  GTK_TREE_VIEW(gtk_builder_get_object(builder1, "treeview1"));



        gtk_window_set_default_size (GTK_WINDOW (window1), 500, 300);
        gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

	    gtk_builder_connect_signals (builder1, NULL);
        g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);

        SaveDB();
        if(LoadDB())
        {
            ShowError();
        }


        gtk_widget_show (window1);

        return TRUE;
	}
    else
	{
	    return FALSE;
	}
}


int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    StartDB();
    if (ShowMainWindow ())
    {
       gtk_main ();
       EndDB();
       return 0;
    }
    else
    {
       ShowGladeError();
       EndDB();
       return 1;
    }
}

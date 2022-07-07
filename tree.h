#include "list.h"

#ifndef htree
#define htree

struct cmd_inf 
  {
    List argv;             /* список из имени команды и аргументов */
    char *infile;          /* переназначенный файл стандартного ввода */
    char *outfile;         /* переназначенный файл стандартного вывода */
    int append;            /* =1, если файл надо открыть на дозапись*/
    int backgrnd;          /* =1, если команда подлежит выполнению в фоновом режиме */
    struct cmd_inf* pipe;  /* следующая команда после “|” */
    struct cmd_inf* next;  /* следующая команда после “;” */
  };

List plex;    /*указатель текущей лексемы*/
              
typedef struct cmd_inf * Tree;

void print_tree(Tree , int);  /*Функция печатати дерева*/

Tree build_tree(List );       /*Функция построения дерева*/

void clear_tree(Tree );       /*Функция освобождает память, занимаемую структурой (деревом), и 
                                делает ее пустой.*/

#endif

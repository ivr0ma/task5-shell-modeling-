#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*======================================================================= 
 * Функция освобождает память, занимаемую списком (если он не пуст), и
 * делает список пустым.
 *=======================================================================
*/
void ClearArgv(List * lst)
  {
    int i;
    if (*lst==NULL) 
	    return;
    for (i=0; *(*lst + i) != NULL; i++)
      free( *(*lst + i) );
    free(*lst);
    *lst=NULL;
  }

/*======================================================================= 
 * Функция освобождает память, занимаемую структурой (деревом), и
 * делает ее пустой.
 *=======================================================================
*/
void clear_tree(Tree t)
  {
    if (t == NULL)
      return;
    clear_tree(t->pipe);
    clear_tree(t->next);
    ClearArgv(&(t->argv));
    free(t);
  }

/*======================================================================= 
 * Функция делает отступ
 *=======================================================================
*/
void make_shift(int n)
  {
    while(n--)
      putc(' ', stderr);
  }

/*======================================================================= 
 * Функция печатает элементы списка argv
 *=======================================================================
*/
void print_argv(char **p, int shift)
  {
    char **q=p;
    if(p!=NULL)
      {
        while(*p!=NULL)
          {
            make_shift(shift);
            fprintf(stderr, "argv[%d]=%s\n",(int) (p-q), *p);
            p++;
          }
      }
  }
  
/*======================================================================= 
 * Функция печатати дерева
 *=======================================================================
*/
void print_tree(Tree t, int shift)
  {
    char **p;
    
    if(t==NULL)
      return;
      
    p=t->argv;
    if(p!=NULL)
      print_argv(p, shift);
    else
      {
        make_shift(shift);
        fprintf(stderr, "psubshell\n");
      }
      
    make_shift(shift);
    if(t->infile==NULL)
      fprintf(stderr, "infile=NULL\n");
    else
      fprintf(stderr, "infile=%s\n", t->infile);
      
    make_shift(shift);
    if(t->outfile==NULL)
      fprintf(stderr, "outfile=NULL\n");
    else
      fprintf(stderr, "outfile=%s\n", t->outfile);
    
    make_shift(shift);
    fprintf(stderr, "append=%d\n", t->append);
    make_shift(shift);
    fprintf(stderr, "background=%d\n", t->backgrnd);
    
    make_shift(shift);
    if(t->pipe==NULL)
      fprintf(stderr, "pipe=NULL \n");
    else
      {
        fprintf(stderr, "pipe---> \n");
        print_tree(t->pipe, shift+5);
      }
    
    make_shift(shift);
    if(t->next==NULL)
      fprintf(stderr, "next=NULL \n");
    else
      {
        fprintf(stderr, "next---> \n");
        print_tree(t->next, shift+5);
      }
  
  }

/*======================================================================= 
 * Функция создает дерево из одного элемента, обнуляет все поля 
 *=======================================================================
*/
Tree make_cmd()
  {
    Tree t;
    
    t=malloc(sizeof(*t));
    t->argv = NULL;
    t->infile = NULL;
    t->outfile = NULL;
    t->append = 0;
    t->backgrnd = 0;
    t->pipe = NULL;
    t->next = NULL;
    
    return t;
  }

/*======================================================================= 
 * Функция добавляет очередной элемент в массив argv текущей команды
 *=======================================================================
*/
void add_arg(Tree t, char * plex)
  {  
    List p;
    int cnt=0;
    
    if (t->argv == NULL)
      {
        if ( ( t->argv=realloc(t->argv,2*sizeof(char *)) ) == NULL)
          {
            fprintf(stderr, "realloc: не выделяется память");
            clear_tree(t);
            return;
          }
        if ( ( *(t->argv) = malloc( (strlen(plex)+1)*sizeof(char) ) ) == NULL )
          {
            fprintf(stderr, "malloc: не выделяется память");
            clear_tree(t);
            return;
          }
        strcpy(*(t->argv), plex);
        *(t->argv+1) = NULL;
      }
    else
      {
        p = t->argv;
        while (*p != NULL)
          {
            cnt++;
            p++;
            /*fprintf(stderr, "cnt=%d, p=%s\n", cnt, *p);*/
          }
        
        if ( ( t->argv=realloc(t->argv,(cnt+2)*sizeof(char *)) ) == NULL )
          {
            fprintf(stderr, "realloc: не выделяется память");
            clear_tree(t);
            return;
          }
        if ( ( *(t->argv+cnt) = malloc( (strlen(plex)+1)*sizeof(char) ) ) == NULL)
          {
            fprintf(stderr, "malloc: не выделяется память");
            clear_tree(t);
            return;
          }
        strcpy(*(t->argv+cnt), plex);
        *(t->argv+cnt+1) = NULL;
      }
    
    
  }

/*======================================================================= 
 * Функция устанавливает поле backgrnd=1 во всех командах конвейера t
 *=======================================================================
*/
void make_bgrnd(Tree t)
  {
    t->backgrnd = 1;
    if(t->pipe != NULL)
      make_bgrnd(t->pipe);
  }

/*======================================================================= 
 * Функция определяет нежелательные символы
 *=======================================================================
*/
int IsBadSymFile(char * str)
  {
    if (str == NULL ||
        strcmp(str, "|") == 0 || 
        strcmp(str, ";") == 0 || 
        strcmp(str, "&") == 0 ||
        strcmp(str, ">") == 0 ||
        strcmp(str, "<") == 0 ||
        strcmp(str, ">>") == 0)
      return 1;
    else
      return 0;
  }

/*=================================================================================
 * Функция построения дерева с помощью L-графа с метками:
 * Begin - начало работы, 
 * Conv - обработка команды с ее аргументами, 
 * End - завершение работы, 
 * Backgrnd - обработка спесимвола &, 
 * In, Out, Out1 - работа с файлами (вход, выход, выход с флагом append), 
 * Conv1 - обработка конвейера,
 * ConvP - обработка спесимвола ;, 
 *=================================================================================
*/
Tree build_tree(List plex)
  {
    typedef enum {Begin, Conv, End, Backgrnd, In, Out, Out1, Conv1, ConvP} vertex;
    vertex V=Begin;
    Tree beg_cmd;
    Tree cur_cmd;
    Tree prev_cmd;
    Tree conv_cmd;
    
    while(1) 
      switch(V)
	      {
          case Begin: 
            if (plex == NULL)
              {
                beg_cmd = NULL;
                V = End;
              }
            else
              {  
                if (strcmp(*plex, "|") == 0 || strcmp(*plex, "&") == 0 || strcmp(*plex, ";") == 0)
                  {
                    fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
                    beg_cmd = NULL;
                    clear_tree(beg_cmd);
		                return NULL;
                  }
                beg_cmd = make_cmd();
                cur_cmd = beg_cmd;
                prev_cmd = cur_cmd;
                conv_cmd = beg_cmd;
                V = Conv;
              }
            break;
          
          case Conv:
            if (*plex == NULL)
              V = End;
            else if (strcmp(*plex, "|") == 0)
              {
                plex++;
                V = Conv1;
              }
            else if (strcmp(*plex, ";") == 0)
              {
                plex++;
                V = ConvP;
              }
            else if (strcmp(*plex, "&") == 0)
              {
                plex++;
                V = Backgrnd;
              }
            else if (strcmp(*plex, "<") == 0)
              {
                plex++;
                V = In;
              }
            else if (strcmp(*plex, ">") == 0)
              {
                plex++;
                V = Out;
              }
            else if (strcmp(*plex, ">>") == 0)
              {
                plex++;
                V = Out1;
              }
            else
              {
                add_arg(cur_cmd, *plex);
                plex++;
              }
            break;
		      
		      case Conv1:
		        if (strcmp(*plex, "|") == 0 || strcmp(*plex, "&") == 0 || strcmp(*plex, ";") == 0 || *plex==NULL)
              {
                fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
                clear_tree(beg_cmd);
		            return NULL;
              }
		        cur_cmd = make_cmd();
		        prev_cmd->pipe = cur_cmd;
		        prev_cmd = cur_cmd;
            V = Conv;
		        break;
		      
		      case ConvP:
		        if (*plex == NULL)
		          {
		            V = End;
		            break;
		          }
		        if (strcmp(*plex, "|") == 0 || strcmp(*plex, "&") == 0 || strcmp(*plex, ";") == 0 || *plex==NULL)
              {
                fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
                clear_tree(beg_cmd);
		            return NULL;
              }
		        cur_cmd = make_cmd();
		        conv_cmd->next = cur_cmd;
		        prev_cmd = cur_cmd;
		        conv_cmd = cur_cmd;
            V = Conv;
		        break;
		      
		      case Backgrnd:
		        if (*plex == NULL)
		          {
		            make_bgrnd(conv_cmd);
		            V = End;
		          }
		        else
		          {
		            if (strcmp(*plex, "|") == 0 || strcmp(*plex, "&") == 0 || strcmp(*plex, ";") == 0)
                  {
                    fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
                    clear_tree(beg_cmd);
		                return NULL;
                  }
		            
		            make_bgrnd(conv_cmd);
		            cur_cmd = make_cmd();
		            conv_cmd->next = cur_cmd;
		            prev_cmd = cur_cmd;
		            conv_cmd = cur_cmd;
                V = Conv;
		          }
		        break;
		      
		      case In:
		        if (IsBadSymFile(*plex))
		          {
		            fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
		            clear_tree(beg_cmd);
		            return NULL;
		          }
		        cur_cmd->infile = *plex;
		        plex++;
		        V = Conv;
		        break;
		      
		      case Out:
		        if (IsBadSymFile(*plex))
		          {
		            fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
		            clear_tree(beg_cmd);
		            return NULL;
		          }
		        cur_cmd->outfile = *plex;
		        cur_cmd->append = 0;
		        plex++;
		        V = Conv;
		        break;
		      
		      case Out1:
		        if (IsBadSymFile(*plex))
		          {
		            fprintf(stderr, "синтаксическая ошибка рядом с неожиданным маркером «%s»\n", *plex);
		            clear_tree(beg_cmd);
		            return NULL;
		          }
		        cur_cmd->outfile = *plex;
		        cur_cmd->append = 1;
		        plex++;
		        V = Conv;
		        break;
		      
		      case End:
		        return beg_cmd;
		        break;
		    }
  }


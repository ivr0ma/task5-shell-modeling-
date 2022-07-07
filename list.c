#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "list.h" 
#define SIZE 16

static int c;         /*текущий символ */
static List lst;      /* список слов (в виде массива)*/
static char * buf;    /* буфер для накопления текущего слова*/
static int sizebuf;   /* размер буфера текущего слова*/
static int sizelist;  /* размер списка слов*/
static int curbuf;    /* индекс текущего символа в буфере*/
static int curlist;   /* индекс текущего слова в списке*/

/*=================================================================================
 * Функция выполняет ввод большими блоками размера BUFSIZ и выдает символ 
 * по одному за раз
 *=================================================================================
*/
int getbuf(void)
  {
    
    static char bufchar[BUFSIZ];
    static char * bbuf = bufchar;
    static int n = 0;
    
    if (n == 0)
      {
        n = read(0, bufchar, BUFSIZ);
        bbuf = bufchar;
      }
    
    if (--n >= 0)
      return (unsigned char) *bbuf++;
    else
      return EOF;
  }

/*=================================================================================
 * Функция освобождает память, занимаемую списком (если он не пуст), и
 * делает список пустым. Переменную sizelist (размер списка) обнуляет, переменную
 * curlist, указывающую очередную свободную позицию в списке, тоже обнуляет.
 *=================================================================================
*/
void ClearList(List * lst)
  {
    int i;
    if (*lst==NULL) 
	    return;
    for (i=0; *(*lst + i) != NULL; i++)
      free( *(*lst + i) );
    free(*lst);
    *lst=NULL;
  }
  
/*=================================================================================
 * Функция присваивает переменной lst, представляющую список, значение
 * (пустой список). Переменную sizelist (размер списка) обнуляет, переменную
 * curlist, указывающую очередную свободную позицию в списке, тоже обнуляет.
 *================================================================================= 
*/
void null_list()
  {
    sizelist=0;
    curlist=0;
    lst=NULL;
  }

/*=================================================================================
 * Функция завершает список, добавляя в позицию curlist и
 * обрезает память, занимаемую списком, до точного размера.
 *=================================================================================
*/
void termlist()
  {
    if (lst==NULL) 
	  return;
    if (curlist>sizelist-1)
      if ( (lst=realloc(lst,(sizelist+1)*sizeof(*lst))) == NULL )
        {
          fprintf(stderr, "не выделяется память");
          exit(-1); 
        }
  
    lst[curlist]=NULL;
    
    if ( (lst=realloc(lst,(sizelist=curlist+1)*sizeof(*lst))) == NULL)  /*выравниваем используемую под 
                                                                          список память точно по размеру списка*/
      {
        fprintf(stderr, "не выделяется память");
        exit(-1); 
      };  
  }
  
/*=================================================================================
 * Функция присваивает переменной buf значение NULL, переменной sizebuf
 * (размер буфера) присваивает значение 0, переменной curbuf, указывающей очередную
 * свободную позицию в буфере, присваивает значение 0
 *=================================================================================
*/
void nullbuf()
  {
    buf=NULL;
    sizebuf=0;
    curbuf=0;
  }

/*=================================================================================
 * Функция добавляет очередной символ в буфер в позицию curbuf , после чего
 * переменная curbuf увеличивается на 1 Если буфер был пуст, то он создается. Если
 * размер буфера превышен, то он увеличивается на константу SIZE, заданную директивой
 * define.
 *=================================================================================
*/
void addsym()
  {
    if (curbuf>sizebuf-1)               /* увеличиваем буфер при необходимости */
      if ( (buf=realloc(buf, sizebuf+=SIZE)) == NULL )
        {
          fprintf(stderr, "не выделяется память");
          exit(-1);
        } 
	
	  buf[curbuf++]=c;
  }

/*=================================================================================
 * Функция завершает текущее слово в буфере, добавляя ’\0’ в позицию curbuf
 * (увеличив, если нужно, буфер), и обрезает память, занимаемую словом, до точного
 * размера; затем добавляет слово в список в позицию curlist, после чего значение
 * curlist увеличивается на 1 Если список был пуст, то он создается. Если размер списка
 * превышен, то он увеличивается на константу SIZE.
 *=================================================================================
*/
void addword()
  {
    if (curbuf>sizebuf-1)             /* для записи ’\0’ увеличиваем буфер при необходимости */
      if ( (buf=realloc(buf, sizebuf+=1)) == NULL )
        {
          fprintf(stderr, "не выделяется память");
          exit(-1);
        }   
    buf[curbuf++]='\0';
	
    if ( (buf=realloc(buf,sizebuf=curbuf)) == NULL )  /*выравниваем используемую память точно по размеру слова*/
      {
        fprintf(stderr, "не выделяется память");
        exit(-1);
      }
      
	  if (curlist>sizelist-1)             /* увеличиваем массив под список при необходимости */
      if ( (lst=realloc(lst, (sizelist+=SIZE)*sizeof(*lst))) == NULL)
        {
          fprintf(stderr, "не выделяется память");
          exit(-1);
        } 
    
    lst[curlist++]=buf;
  }

/*=================================================================================
 * Функция печатает длину списка (число слов, без NULL), в каждой последующей –
 * очередной элемент списка. Если список пустой, ничего не делать.
 *=================================================================================
*/  
void PrintList(List lst)
  {
    int i;
    if (lst==NULL) 
	    return;
    
    printf("%d\n", sizelist-1);
    for (i=0; lst[i] != NULL; i++)
      printf("%s\n",lst[i]);
  }

/*=================================================================================
 * Функция заменяет в списке строки $HOME, $SHELL, $USER, $EUID на их значения
 *=================================================================================
*/  
void ChangeList(List lst)
  {
    #define PATH_MAX 255
    #define NAME_MAX 255
    #define ID_MAX 10
    int i;
    char str_id[ID_MAX];
    
    if (lst==NULL) 
	    return;
	    
    for (i=0; lst[i] != NULL; i++)
      {
        if (strcmp(lst[i], "$HOME") == 0)
          {
            free(lst[i]);
            lst[i] = malloc(PATH_MAX*sizeof(char));
            strcpy(lst[i], getenv("HOME"));
          }
        if (strcmp(lst[i], "$SHELL") == 0)
          {
            free(lst[i]);
            lst[i] = malloc(PATH_MAX*sizeof(char));
            strcpy(lst[i], getenv("PWD"));
          }
        if (strcmp(lst[i], "$USER") == 0)
          {
            free(lst[i]);
            lst[i] = malloc(NAME_MAX*sizeof(char));
            strcpy(lst[i], getlogin());
          }
        if (strcmp(lst[i], "$EUID") == 0)
          {
            sprintf(str_id, "%d", geteuid());
            /*printf("%s\n", str_id);*/
            free(lst[i]);
            lst[i] = malloc(ID_MAX*sizeof(char));
            strcpy(lst[i], str_id);
          }
      }

  }

/*=================================================================================
 * Функция выдает тип символа. Выдает 2, если это символы |, &, > (т.к. они могут 
 * быть частью более приоритных команд ||, &&, >>), выдает 1, если это символы
 * ; , <, (, ), иначе выдает 0
 *=================================================================================
*/
int specsym(int c)
  {
    if (c == ';' || c == '<' || c == '(' || c == ')')
      return 1;
    else if (c == '|' || c == '&' || c == '>')
      return 2;
    else
      return 0;
  }

/*=================================================================================
 * Функция выдает тип символа. Выдает 1, если это любой символ, кроме пробела, 
 * табуляции, перевода строки и специальных символов (|, ||, &, &&, ; , >, >>, 
 * <, (, ).), не конец файла (EOF), иначе выдает 0
 *=================================================================================
*/
int symset(int c)
  {
    return specsym(c) == 0 &&
           c != '\n' &&
           c != ' ' &&
           c != '\t' &&
           c != EOF ;
  }

/*=================================================================================
 * Функция построения списка с помощью L-графа с метками:
 * Start - начало работы, 
 * Slash - обработка символа \, 
 * GridS, GridW - обработка символа #, 
 * Word - обработка слов, 
 * WordK - обработка кавычек, 
 * Greater - обработка обычных спецсимволов (т.е. состоящих из 1 символа)
 * GreaterOr - обработка спецсимволов | и ||, 
 * GreaterAnd - обработка спецсимволов & и &&, 
 * GreaterSd - обработка спецсимволов > и >>, 
 * Greater2 - обработка двойных спецсимволов, 
 * Stop - завершение работы
 *=================================================================================
*/
List BuildList() 
  {
    typedef enum {Start, Slash, GridS, GridW, Word, WordK, Greater, GreaterOr, GreaterAnd, GreaterSd, Greater2, Stop} vertex;
    vertex V=Start;
    c=getbuf();
    null_list();
    while(1==1) 
      switch(V)
	      {
          case Start:
            if(c==' '|| c=='\t') 
			        c=getbuf();
            else if (c==EOF || c=='\n') 
		          {
                termlist();
                V = Stop;
              }
            else if (c == '\\')
              {
                nullbuf();
                V = Slash;
              }
            else if (c == '"')
              {
                nullbuf();
                V = WordK;
                c=getbuf();
              }
            else if (c == '#')
              V = GridS;
            else 
		    		  {
                nullbuf();
                
                addsym();
                
                if (specsym(c) == 1)
                  V = Greater;
                else if (c == '|')
                  V = GreaterOr;
                else if (c == '&')
                  V = GreaterAnd;
                else if (c == '>')
                  V = GreaterSd;
                else
                  V = Word;
                
                c=getbuf();
              }
            break;
		      
		      case GridS:
		        V = Stop;
		        break;
		      
		      case GridW:
		        c=getbuf();
		        if (c == '\n' || c == EOF)
		          {
		            V = Start;
		            addword();
		          }
		        break;
		      
		      case Slash:
		        c=getbuf();
		        if (c != '\n' && c != EOF)
		          {
		            addsym();
		            V = Word;
		            c=getbuf();
		          }
		        else
		          {
		            V = Start;
		            addword();
		          }
		        break;
		      
          case Word:
            if (c == '\\')
              {
                V = Slash;
                break;
              }
            if (c == '"')
              {
                V = WordK;
                c=getbuf();
                break;
              }
            if (c == '#')
              {
                V = GridW;
                break;
              }
            if(symset(c)) 
		    	  	{
                addsym();
                c=getbuf();
              }
            else 
		    	  	{
                V=Start;
                addword();
              }
            break;
          
          case WordK:
            if (c == '"')
              {
                V = Word;
                c=getbuf();
                break;
              }
            if (c != '\n' && c != EOF)
              {
                addsym();
                c=getbuf();
              }
            else 
		    	  	{
                V=Start;
                addword();
              }
            break;
		  
				  case Greater:
            V=Start;
            addword();
            break;
          
          case GreaterOr:
            if(c == '|') 
		    	  	{
                addsym();
                c=getbuf();
                V=Greater2;
              }
            else 
		    	  	{
                V=Start;
                addword();
              }
            break;
          
          case GreaterAnd:
            if(c == '&') 
		    	  	{
                addsym();
                c=getbuf();
                V=Greater2;
              }
            else 
		    	  	{
                V=Start;
                addword();
              }
            break;
          
          case GreaterSd:
            if(c == '>') 
		    	  	{
                addsym();
                c=getbuf();
                V=Greater2;
              }
            else 
		    	  	{
                V=Start;
                addword();
              }
            break;
		  
          case Greater2:
            V=Start;
            addword();
            break;
		  
          case Stop:
            return lst;
            break;
        }
  }

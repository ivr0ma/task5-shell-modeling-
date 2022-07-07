#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "exec.h"

/*============================================================================
 * Функция удаления процесса с определенным pid 
 *============================================================================
*/
void del_pid(PList *p, int pid)
  {
    PList q;
    
    while (*p)
      if ((*p)->pid == pid)
        {
          q=(*p);
          *p=q->next;
          free(q);
          return ;
        }
      else
        p=&(*p)->next;
   
    return ;
  }

/*============================================================================
 * Функция удаления завершенных прцессов
 *============================================================================
*/
void clear_zombie(PList *p)
  {
    int pid;
    
    if (*p == NULL) 
      {
        //fprintf(stderr, "список пуст\n");
	      return;
      }
      
    while (*p != NULL)
      {
        if ( (pid = waitpid((*p)->pid, 0, WNOHANG)) == 0)
          ;
          //fprintf(stderr, "%d еще не завершился\n", (*p)->pid);
        else
          {
            fprintf(stderr, "процесс %d завершился\n", pid);
            del_pid(p, pid);
            return ;
          }
        p = &(*p)->next;
      }
  }

/*============================================================================
 * Функция добавления нового узла в список pid
 *============================================================================
*/
void add_pid(PList * p, int elem)
  {
    if (*p == NULL)
      {
        if ( ( *p = malloc(sizeof(struct backgrndList)) ) == NULL)
          {
            fprintf(stderr, "malloc: не выделяется память");
            return;
          }
        (*p)->pid = elem;
        (*p)->next = NULL;
        
        return ;
      }
    else
      add_pid(&(*p)->next, elem);
  }

/*============================================================================
 * Функция печати списка pid
 *============================================================================
*/
void print_pid(PList p)
  {
    if (p == NULL) 
	    return;
    
    while (p != NULL)
      {
        printf("%d\n",p->pid);
        p = p->next;
      }

  }

/*============================================================================
 * Функция удаления списка pid
 *============================================================================
*/
void del_all_pid(PList p)
{
  if (p == NULL)
    return;
  del_all_pid(p->next);
  free(p);
}

/*============================================================================
 * Функция перенаправляет стандартный поток ввода на вывод из файла
 *============================================================================
*/
void change_in(Tree tcmd)
  {
    int f_inp;
    
    if ( (f_inp = open(tcmd->infile, O_RDONLY )) == -1)
      {
        fprintf(stderr, "не удалось открыть файл %s\n", tcmd->infile);
      }
    else 
      {
        dup2(f_inp, 0);
        //fprintf(stderr, "перенаправил из файла %s\n", tcmd->infile);
        close(f_inp);
      }
  }

/*============================================================================
 * Функция перенаправляет стандартный поток вывода на вывод в файл
 *============================================================================
*/
void change_out(Tree tcmd)
  {
    int f_out;
    
    if (tcmd->append)
      f_out = open(tcmd->outfile, O_WRONLY|O_APPEND, 0666);
    else
      f_out = open(tcmd->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
                  
    if (f_out == -1)
      fprintf(stderr, "не удалось открыть файл %s\n", tcmd->outfile);
    else 
      {
        dup2(f_out, 1);
        //fprintf(stderr, "перенаправил в файл %s\n", tcmd->outfile);
        close(f_out);
      }
  }

/*============================================================================
 * Функция обработки команды cd
 *============================================================================
*/
void exec_cd(char ** argv)
  {
    if (argv[1] != NULL && argv[2] != NULL)
      fprintf(stderr, "не совпадает число параметров в команде cd\n");
    else
      {
        if (argv[1] != NULL)
          if (chdir(argv[1]) == -1)
            fprintf(stderr, "не удалось выполнить команду cd\n");
          else ;
        else
          if (chdir("/") == -1)
            fprintf(stderr, "не удалось выполнить команду cd\n");
          else ;
      }
  }

/*============================================================================
 * Функция, реализующая работу конвейера. В параметрах указывается кол-во 
 * команд конвейера и структура типа Tree.
 *============================================================================
*/
void exec_conv(int cmd_count, Tree tcmd, int IsBack, PList * back_list)
  { 
    int fd[2], in, out, next_in, i, pid, fback;
    Tree term;
    PList wait_list = NULL;
    PList p;
    
    term = tcmd;
    if (cmd_count == 0)
      return ;
    if (cmd_count == 1) 
      {
        if (strcmp(term->argv[0], "cd") == 0)
          {
            exec_cd(term->argv);
            return ;
          } 
        if ( (pid = fork()) == 0)
          {
            if (IsBack)
              signal(SIGINT, SIG_IGN);
              
            if (term->infile != NULL)
              change_in(term);
            if (term->outfile != NULL)
              change_out(term);
            if (IsBack)
              {
                fback = open("/dev/null", O_RDONLY);
                dup2(fback, 0);
                close(fback);
              }
            
            //fprintf(stderr, "первый пошел и все конец\n");
            execvp(term->argv[0],term->argv); 
            fprintf(stderr, "не удалось выполнить команду %s\n", term->argv[0]);
            exit(-1);
            
          }
        
        if (IsBack)
          {
            fprintf(stderr, "процесс %d работает в фоновом режиме\n", pid);
            add_pid(back_list, pid);
          }
        else 
          { 
            //fprintf(stderr, "ждем процесс %d\n", pid);
            waitpid(pid, 0, 0);
            //fprintf(stderr, "дождались (%d)\n", pid);
          }
        return ;
      }
    
    pipe(fd); 
    out=fd[1]; 
    next_in=fd[0];
    
    if (strcmp(term->argv[0], "cd") == 0)
      exec_cd(term->argv);
    else 
      {
        if ( (pid = fork()) == 0)
          {
            if (IsBack)
              signal(SIGINT, SIG_IGN);
           
            close(next_in);
            dup2(out,1);
            close(out);
        
            if (term->infile != NULL)
              change_in(term);
            if (term->outfile != NULL)
              change_out(term);
            if (IsBack)
              {
                fback = open("/dev/null", O_RDONLY);
                dup2(fback, 0);
                close(fback);
              }
        
            //fprintf(stderr, "первый пошел\n");
            execvp(term->argv[0],term->argv); 
            fprintf(stderr, "не удалось выполнить команду %s\n", tcmd->argv[0]);
            exit(-1);
          }
    
        if (IsBack)
          {
            fprintf(stderr, "процесс %d работает в фоновом режиме\n", pid);
            add_pid(back_list, pid);
          }
        else
          {
            add_pid(&wait_list, pid);
          }
      }
    
    in=next_in;
    for (i=1;i<cmd_count-1; i++)
      {
        term = term->pipe;
        close(out);
        pipe(fd);
        out=fd[1];
        next_in=fd[0];
        
        if (strcmp(term->argv[0], "cd") == 0)
          {
            //exec_cd(term->argv);
            continue ;
          }
        
        if( (pid = fork()) == 0)
          {
            if (IsBack)
              signal(SIGINT, SIG_IGN);
          
            close(next_in);
            dup2(in,0);
            close(in);
            dup2(out,1);
            close(out);
            
            if (term->infile != NULL)
              change_in(term);
            if (term->outfile != NULL)
              change_out(term);
            
            //fprintf(stderr, "%d пошел\n", i+1);
            execvp(term->argv[0], term->argv); 
            fprintf(stderr, "не удалось выполнить команду %s\n", term->argv[i]);
            exit(-1);
          }
        
        if (IsBack)
          {
            fprintf(stderr, "процесс %d работает в фоновом режиме\n", pid);
            add_pid(back_list, pid);
          }
        else
          {
            add_pid(&wait_list, pid);
          }
        
        close(in);
        in=next_in;
      }
      
    close(out);
    term = term->pipe;
    
    if (strcmp(term->argv[0], "cd") == 0)
      {
        exec_cd(term->argv);
        if (!IsBack)
          {
            //fprintf(stderr, "++++++\n");
            print_pid(wait_list);
            //fprintf(stderr, "++++++\n");
        
            p = wait_list;
            while (p != NULL)
              {
                //printf("ждем %d\n",p->pid);
                waitpid(p->pid, 0, 0);
                //printf("дождались %d\n",p->pid);
                p = p->next;
              }
        
            del_all_pid(wait_list);
          }
          
        return ;
      }
      
    if ( (pid = fork()) == 0)
      {
        if (IsBack)
          signal(SIGINT, SIG_IGN);
      
        dup2(in,0);
        close(in);
        
        if (term->infile != NULL)
          change_in(term);
        if (term->outfile != NULL)
          change_out(term);
        
        //fprintf(stderr, "последний пошел\n");
        execvp(term->argv[0], term->argv); 
        fprintf(stderr, "не удалось выполнить команду %s\n", term->argv[cmd_count-1]);
        exit(-1);
      }
    
    close(in);
    
    if (IsBack)
      {
        fprintf(stderr, "процесс %d работает в фоновом режиме\n", pid);
        add_pid(back_list, pid);
      }
    else
      {
        add_pid(&wait_list, pid);
        
        //fprintf(stderr, "++++++\n");
        print_pid(wait_list);
        //fprintf(stderr, "++++++\n");
        
        p = wait_list;
        while (p != NULL)
          {
            //printf("ждем %d\n",p->pid);
            waitpid(p->pid, 0, 0);
            //printf("дождались %d\n",p->pid);
            p = p->next;
          }
        
        del_all_pid(wait_list);
      }  
    
    return ;
  }

/*============================================================================
 * Функция подсчитывает число команд конвейера
 *============================================================================
*/
int len_conv(Tree p)
  {
    int i;
    if (p == NULL) 
	    return 0;
    for (i=1; p->pipe != NULL; i++)
      p=p->pipe;
    return i;
  }

/*============================================================================
 * Функция подсчитывает число команд последовательности, разделенных ;
 *============================================================================
*/
int len_next(Tree p)
  {
    int i;
    if (p == NULL) 
	    return 0;
    for (i=1; p->next != NULL; i++)
      p=p->next;
    return i;
  }

/*============================================================================
 * Функция обходит структуру по полям next и вызывает функцию exec_conv
 *============================================================================
*/
void exec_com(Tree tcmd, PList * plst)
  {
    Tree term;
    int cnt_next, i;
    
    term = tcmd;
    cnt_next = len_next(term);
    
    //fprintf(stderr, "len_next=%d\n", cnt_next);
    
    for (i=0; i<cnt_next; i++)
      {
        //fprintf(stderr, "len_conv=%d\n", len_conv(term));
        if (term->backgrnd)
          {
            //fprintf(stderr, "backgrd=%d\n", 1);
            exec_conv(len_conv(term), term, 1, plst);
          }
        else
          {
            //fprintf(stderr, "backgrd=%d\n", 0);
            exec_conv(len_conv(term), term, 0, plst);
          }
        term = term->next;
      }
    return ;
  }


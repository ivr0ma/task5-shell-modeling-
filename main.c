#include "list.h"
#include "tree.h"
#include "exec.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main()
  {
    List lst=NULL;
    Tree tcmd=NULL;
    PList plst=NULL;
    
    while (1)
      {
        clear_tree(tcmd);  /*очистка дерева*/
        ClearList(&lst);   /*очистка списока лексем*/
        
        
        //fprintf(stderr, "||||||||||||||||||||\n");
        //print_pid(plst);
        //fprintf(stderr, "||||||||||||||||||||\n");
        clear_zombie(&plst);
        
        /*очистить_зомби из списка (таблицы) зарегистрированных фоновых процессов;*/
        /* первоначально там никого нет, никаких удалений не потребуется*/
        
        fprintf(stderr, "shell: ");
        lst = BuildList();  /*запрашиваем у пользователя очередную строку 
                              с командой, разбиваем ее на лексемы (строим lst)*/
        
        if (lst == NULL) 
          {
            del_all_pid(plst);
            return 0;
          }
        
        ChangeList(lst);   /*заменяем в списке строки вида $ИМЯ*/
        //PrintList(lst);
        /*ClearList(&lst);*/
    	
        tcmd = build_tree(lst);   /*по списку лексем строим дерево*/
        //print_tree(tcmd, 5);
        
        //fprintf(stderr, "-------------\n");
        exec_com(tcmd, &plst);       /*выполнить команду, обходя дерево и создавая 
                                соответствующие процессы, файлы, каналы;*/
        //fprintf(stderr, "-------------\n");
    
        /*fprintf(stderr, "FINISH!\n");*/
      }
  }
  

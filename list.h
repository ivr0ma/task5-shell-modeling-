#ifndef hlist
#define hlist

typedef char ** List;

void PrintList(List );   /*Функция печатает длину списка (число слов, без NULL), в каждой последующей – 
                           очередной элемент списка.*/
                           
void ClearList(List * ); /*Функция освобождает память, занимаемую списком (если он не пуст), и
                           делает список пустым.*/
                           
List BuildList();        /*Функция построения списка с помощью L-графа с метками*/

void ChangeList(List );  /*Функция заменяет в списке строки $HOME, $SHELL, $USER, $EUID на их значения*/

#endif

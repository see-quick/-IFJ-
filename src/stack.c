/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   stack.c
 *
 * Popis: zdrojove subory zasobnika
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/
#include "stack.h"
#include <stdbool.h>
#include <stdlib.h>
#include "prece.h"

/* Zakladne funckie so Stackom */

/**
 * Funkcia, ktora kontroluje pracu so stackom, ak bude nejaka chyba pri operaciach, automaticky nas nato upozorni
 * @return -1 -> like error
 */
int stack_error(){
    return -1;
}

/**
 * Inicializuje stack na velkost ktoru si zvolime
 * @param memory velkost zasobnika
 * @return vracia zasobnik
 */
tStack* stack_init(unsigned memory){
    tStack* stack = (tStack*) malloc(sizeof(tStack));
    if(stack == NULL){
        stack_error(stack);
    }
    stack->top = -1;
    stack->finderOfParenthesis = -1;
    stack->memory = memory;
    stack->arrayOfNumbers = (int *) malloc(stack->memory * sizeof(int));
    stack->arrayOfItems = (tDataIDF *) malloc(stack->memory * sizeof(tDataIDF));
    return stack;
}

/**
 * Funkcia kontroluje zda je zasobnik prazdny
 * @param stack konkretny zasobnik
 * @return -1 v pripade ze je prazdny
 */
int stack_empty(tStack *stack){
    return stack->top == -1;
}

/**
 * Funkcia, ktora bude kontrolovat ked bude zasobnik plny
 * @param stack
 * @return v pripade ze je plny tak vracia 1
 */
int stack_full(tStack *stack){
    return stack->top == stack->memory - 1;
}

/**
 * Funckia, ktora vracia Token, ktory je na vrchole zasobnika
 * @param stack konkretny zasobnik
 * @return token
 */
int stack_top_token_number(tStack *stack){
    int result;
    if(stack_empty(stack)){
        result = stack_error(stack);
    } else{
        return stack->arrayOfNumbers[stack->top];
    }
    return result;
}

tDataIDF* stack_top_token_data(tStack *stack){
    tDataIDF* result = NULL;
    if(stack_empty(stack)){
       return result;
    }
    return &stack->arrayOfItems[stack->top];
}

/**
 * Funckia, ktora pushuje zvoleny token do zasobnika
 * @param stack konkretny token
 * @param token pushovany token
 */
void stack_push ( tStack *stack, int tokenNumber, tDataIDF tokenData){
    if(stack_full(stack)){
        if(stack_error(stack) == -1){
            return;
        }
    }
    stack->top++;
    stack->finderOfParenthesis++;
    stack->arrayOfNumbers[stack->top] = tokenNumber;
    stack->arrayOfItems[stack->top] = tokenData;
}

/**
 * Funckia, ktora bude popovat dany token von zo zasobniku
 * @param stack konkrenty token
 * @return vrati token na vrchole zasobnika
 */
void stack_pop(tStack *stack){
    // ak zasobnik nie je prazdny
    if(!(stack_empty(stack))){
        stack->arrayOfNumbers[stack->top] = 30; // pre printf funckiu -> mapujeme 30 ako prazdny znak
        stack->top--;
        stack->finderOfParenthesis--;
    }
}
/**
 * Funckia, ktora sa vyuziva pri popovani poloziek ktore su nam pri redukcii uz nepotrebne napriklad  $<i   polozka < nepotrebne tak stack_pop_free, i je potrebne takze dame stack_pop
 * @param stack
 */
void  stack_pop_free(tStack *stack){
    // ak zasobnik nie je prazdny
    if(!(stack_empty(stack))){
        free(&(stack->arrayOfItems[stack->top]));
        stack->arrayOfNumbers[stack->top] = 30;
        stack->top--;
        stack->finderOfParenthesis--;
        }
    return;
}

/**
 * Funkcia, ktora nam vrati hodnotu celeho stacku
 * @param stack konkretny stack
 * @return hodnota stacku
 */
int stack_get_size(tStack *stack){
    return stack->memory;
}

void stack_print_prece(tStack *stack){
    printf("[------------> STACK PRECE BEGIN <-------------]\n");
    for(int i = 0; i < stack_get_size(stack); i++){
        printf("Current stack item is -> %s\n", convert_to_char(stack->arrayOfNumbers[i]));
    }
    printf("[-------------> STACK PRECE END <--------------]\n");
}

/**
 * Pomocna debugovacia funkcia
 * @param stack konkretny stack
 */
void stack_print(tStack *stack){
   printf("[------------> STACK BEGIN <-------------]\n");
   for(int i = 0; i < stack_get_size(stack); i++){
       printf("Current stack item type %d, value %d and uniqueNonTerminal -> %s\n", stack->arrayOfItems[i].type, stack->arrayOfItems[i].value.i, stack->arrayOfItems->nameOfTheNonTerminal);
   }
   printf("[-------------> STACK END <--------------]\n");
}

/**
 * Vzdy pouzi tuto funckiu pred funkciou stack_free(), sluzi iniciliazivanie vsetkych poli na NULL
 * @param stack konkretny stack
 */
void stack_refresh(tStack *stack){
    for(int i = 0; i < stack_get_size(stack); i++){
        stack->arrayOfNumbers[i] = 30;
        setEmptyDataIDF(stack->arrayOfItems[i]);
    }
    stack->top = -1;
    stack->finderOfParenthesis = -1;
}

/**
 * Funkcia, ktora uvolni cely stack
 * @param stack konkretny stack
 */
void stack_free(tStack *stack)
{
    stack->memory = 0;
    stack->finderOfParenthesis = -1;
    stack->top = -1;

     if (item != NULL){
         free(item);
     }


    free(stack->arrayOfNumbers);
    free(stack->arrayOfItems);
    free(stack);
}

/**
 * Funckia, pomocou ktorej sa bude hladat znamienko <, na redukovanie pravidiel E -> E + E a podobne
 * @param stack konkretny stack
 */
void stack_search_for_theorem(tStack *stack) {
    /* this will be testing code */
    for(int i = 0; stack->arrayOfNumbers[stack->finderOfParenthesis] != eSOLVING_RULE; i++) { //solving rule
        stack->finderOfParenthesis--;
    }
}

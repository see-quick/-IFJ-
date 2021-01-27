/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   stack.c
 *
 * Popis: hlavickovy subor zasobnika
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#ifndef STACK_STACK_H
#define STACK_STACK_H

#define stack_pops(COUNT, STACK) for(int i=0; i < COUNT; i++){stack_pop(STACK);}

#include <stdio.h>
#include "string.h"
#include "symtable.h"

typedef struct{
    int top;
    int finderOfParenthesis;
    int memory;
    int *arrayOfNumbers;
    tDataIDF *arrayOfItems;
}tStack;

typedef struct{
    int token_number;
    tDataIDF token_data;
}tItem;


tItem* item; /* GLOBALNY ITEM pre stack pri pravidlach E -> E + E, E -> E - E a podobne. */

tStack* stack_init(unsigned memory);
int stack_error();
int stack_empty(tStack *stack);
int stack_full(tStack *stack);
int stack_get_size(tStack *stack);
int stack_top_token_number(tStack *stack);
tDataIDF* stack_top_token_data(tStack *stack);
void stack_pop(tStack *stack); // sem bude vracat Token
void stack_push ( tStack *stack, int tokenNumber, tDataIDF tokenData);
void stack_free(tStack *stack);
void stack_print(tStack *stack);
void stack_print_prece(tStack *stack);
void stack_refresh(tStack *stack);
void stack_search_for_theorem(tStack *stack);

void  stack_pop_free(tStack *stack);

#endif //STACK_STACK_H

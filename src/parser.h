/**
 * Predmet:  IFJ 
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   parser.h
 *
 * Popis: Syntakticka a semanticka analyza
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#ifndef IFJ_SYNTACTICANALYZER_H
#define IFJ_SYNTACTICANALYZER_H

#include "symtable.h"
#include "list.h"
#include "prece.h"
#include <stdbool.h>

GlobalMap* gMap;		     // globalni tabulka symbolu


int error_lex(void); // kontrola pokud nastala lexikalni chyba
int error_int(void); // kontrola pokud nastala interni chyba

/**
** Pomocna funkce na kontrolu typu tokenu
** @ int type - typ tokenu
**/
int checkTokenType(int);


// funkce pro kontrolu parametru vestavenych funkci
int check_substr_ord_build_in(int param);
int check_chr_build_in();
int check_length_substr_ord_build_in();


/**
** Pocatecni stav pruchodu
** Dale nasleduje zacatek programu <prog> - <main_p> EOF
**/
int prog(void);
int main_p(void);
int st_list();
int stat();
// funkce pro generovani instrukce MOVE
int move_value(expr_return res);
// funkce pro pravidlo prirazeni id = <sth>
int sth();
// funkce pro kontrolu spravnosti formalnich parametru
int pm_list();
int pm_list2();
// funkce pro kontrolu spravnosti argumentu
int term_list(bool);
int term_list2(bool);
// pravidlo pro kontrolu syntakticke spravnosti funkci
int func();
// funkce pro vkladani vestavenych funkci do globalni tabulky symbolu
void insert_build_in_functions();
// funkce pro generovani intrukce EXIT
void instruction_exit(int);
// generuje unikatni index na konci retezce
char * generate_param(char *, unsigned short );
// zacatek parseru
int parse(GlobalMap* , tList *);

#endif



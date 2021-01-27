/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   scanner_list.h
 *
 * Popis: hlavickovy subor scanner_list.
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/
#include <stdio.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1

extern int errflg;
extern int solved;
 
typedef struct tDLElem {                 
		char * data;
        struct tDLElem *lptr;          /* ukazatel na předchozí prvek seznamu */
        struct tDLElem *rptr;        /* ukazatel na následující prvek seznamu */
} *tDLElemPtr;

typedef struct {                                  /* dvousměrně vázaný seznam */
    tDLElemPtr First;                      /* ukazatel na první prvek seznamu */
    tDLElemPtr Act;                     /* ukazatel na aktuální prvek seznamu */
    tDLElemPtr Last;                    /* ukazatel na posledni prvek seznamu */
} tDLList;

                                             /* prototypy jednotlivých funkcí */
void DLInitList (tDLList *);
void DLDisposeList (tDLList *);
void DLInsertFirst (tDLList *, char *);
char * DLCopyFirst (tDLList *);
void print_elements_of_list(tDLList);
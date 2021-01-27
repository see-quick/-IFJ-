/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   prece.h
 *
 * Popis: Hlavickovy subor pre precedencnu analyzu
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#ifndef IFJ_PRECE_H
#define IFJ_PRECE_H

#include "string.h"
#include "error.h"
#include <stdbool.h>
#include "scanner.h"
#include "symtable.h"
#include "list.h"
#include "stack.h"

#define SIZEOFTABLE 16

/* VSTUPNY TOKEN (identifikator) */
/* e = expresion */
#define ePLUS   	0 // +
#define eMINUS      1 // -
#define eMUL        2 // *
#define eDIV        3 // /
#define eLESS		4  // <
#define eGREAT 		5  // >
#define eLEQUAL		6  // <=
#define	eGEQUAL 	7  // >=
#define	eEQUAL		8  // ==
#define	eNEQUAL 	9  // !=
#define eIDENT		10 // i
#define	eLBAR		11 // (
#define	eRBAR		12 // )
#define eDOLAR      13 // dolar $
#define eCOMMA     14 // ,
#define eFCE        15 // fce()


#define eSOLVING_RULE   22 // preventujeme kvoli situacii kde ak najde symbol < a pride dalsi < tak nech sa z toho nezblazni
#define E               42  // E - non terminal
#define EMPTY_CHAR     30

#define eSYNTERROR 800 // syntax error
#define eSEMERROR  801

tDataIDF dataIDF;


// Enum ktorý, bude selectovať o akú operáciu pojde pri vykonavaní precedenčnej analýzy(využívanie stacku a getToken()
typedef enum{
    EQ = 800,   // = Znamienko, znamenajúce jednoduchý shift v precedenčnej tabuľke
    L = 801,    // < Znamienko, znamenajúce obohatený shifting
    G = 802,    // > Znamienko, znamenajúce obohatený redukcia
    Err = 803   // Error
}prece_states;

typedef struct{
    tString *uniqueID;      // non terminal
    bool bool_result;       // pri vyrazoch < > <= >= bude vzdy vysledok true alebo false;
    Types data_type;        // sem ti budem vracat datovy typ ?
    int result;             // pouzivaju sa navratove hodnoty z "error.h"
}expr_return;

// hlavni funkce precedencni analyzy
expr_return parse_expr(LocalMap*, tList*, bool);
void setEmptyDataIDF();

// pomocne debugovanie funckie
char* convert_to_char(int token);


#endif //IFJ_PRECE_H

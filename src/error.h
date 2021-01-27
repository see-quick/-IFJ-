/**
 * Predmet:  IFJ 
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   error.h
 *
 * Popis: Hlavičkový súbor chybove stavy programu
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#ifndef IFJ_ERR_H
#define IFJ_ERR_H

#define  SUCCESS                   0    // spravne ukončenie programu bez chyby
#define  ERROR_LEX                 1    // chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému)
#define  SYN_ERR                   2    // chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu)
#define  SEM_ERR                   3    // sémantická chyba v programu (...)
#define  ERR_INCOMPATIBLE_TYPE     4    // sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech
#define  ERR_PARAMS_COUNT          5    // sémantická chyba nesprávny počet argumentov
#define  ERR_SEMANTIC              6    // ostatní sémantické chyby
#define  ERR_DIVISION              9    // chyby pri delení nulou
#define  INT_ERR                   99   // interní chyba překladače

#endif
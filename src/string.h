/**
 * Predmet:  IFJ 
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   string.h
 *
 * Popis: hlavickovy soubor
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/


#ifndef IFJ_STR_H
#define IFJ_STR_H

/*************************/
/*******Struktury*********/
/*************************/
typedef struct{
  char *str; // data stringu
  unsigned int length; // delka 
  unsigned int memory; // alokovana pamet'
} tString;

typedef struct{
  unsigned int row; // cislo radku pro debugovani
  tString data; 
} tToken;


/* Implementace struktury tString pro data(atribut) tokenu*/
int strInit(tString *);
/*Pridavani symbolu do data tokenu*/
int strAdd(tString *, char);
/*Uvolneni pameti pro data tokenu*/
int strFree(tString *);
// Vytvoreni pole pro data tokenu
tString strCreate(char *);
// Nastaveni nulove delky pole pro data, a zaplneni tohoto pole nulami
int strClear(tString *);
// Nakopirovani data tokenu do pole char *
int strCopy(tString *, char *);
// Nakopirovani pole do pole
int strCompare (tString *, tString *);



#endif
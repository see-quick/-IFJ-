/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   instr_parse.h
 *
 * Popis:  hlavickovy subor parseru instrukcii
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/
#include "list.h"

#ifndef _INSTR_PARSE_H
#define _INSTR_PARSE_H

char* instruct_type(tDatType instruction);
void print_symb(tInstructionData instr_operand);
void parse_instructions(tList *instr_list);
void append_list(tList* root, tList* list);

#endif //_INSTR_PARSE_H

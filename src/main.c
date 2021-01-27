/**
 * Predmet:  IFJ 
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   main.c
 *
 * Popis: Hlavni vykonavaci funkce programu
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

/*Pouzite knihovny */
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "error.h"
#include "parser.h"
#include "list.h"
#include "instr_parse.h"
#include "list.h"

// pro spravne uvolneni pameti
extern char * function_name; // nazev funkce, ve ktere se aktualni nachazime
extern char * variable_name; // nazev promenne, do ktere se prirazuje
extern char * call_name; // nazev funkce, kterou volame

extern tList * while_list;
extern tList * variables_list; // seznam promennych definovanych uvnitr if/while
extern tList * function_statements_list; // seznam funkci, umistime na zacatek
tList * pom_list; // pomocny ukazatel pro swapovani listu

extern tInstructionTypes instr_type; // typ instrukce
extern tInstructionData instr1; // address1
extern tInstructionData instr2; // address2
extern tInstructionData instr3; // address3

/**
 * Hlavna vykonavacia funckcia main()
 * Inicializacia listu, globalnej mapy, volanie parseru, generovanie instrukcii
 * @return SUCCESS v pripade spravneho ukončenia, inak chybove navratove kody definovane v zadani ifj18
 */
int main() {
   
  int result = SUCCESS;

  tList *list = list_init(); // inicializace seznamu instrukci
  pom_list = list_init(); // inicialnizace pomocneho listu

  GlobalMap* globalMap; // inicializace globalni mapy
  globalMap = global_map_init(MAX_SIZE_OF_HASH_TABLE);


  // hlavicka .IFJcode18
  instr_type = INSTRUCT_HEAD;
  insert_item(pom_list, &instr_type, &instr1, &instr2, &instr3);

  // vestavene funkce v ramci generovani kodu
  /********************************************************/
  instr_type = INSTRUCT_LENGTH;
  insert_item(pom_list, &instr_type, &instr1, &instr2, &instr3);
  instr_type = INSTRUCT_CHR;
  insert_item(pom_list, &instr_type, &instr1, &instr2, &instr3);
  instr_type = INSTRUCT_ORD;
  insert_item(pom_list, &instr_type, &instr1, &instr2, &instr3);
  instr_type = INSTRUCT_SUBSTR;
  insert_item(pom_list, &instr_type, &instr1, &instr2, &instr3);

  reverse(&(pom_list->first));
  set_active(pom_list);
  parse_instructions(pom_list);      
  /********************************************************/


  // zacatek parsovani
  result = parse(globalMap, list);
  if (result == SUCCESS){
      instruction_exit(result);
  }

  reverse(&(function_statements_list->first));
  set_active(function_statements_list);
  parse_instructions(function_statements_list);      

  reverse(&(list->first));
  set_active(list);
  parse_instructions(list); // vypis listu s instrukcemi


  // Spravne uvolnovani pameti
  /********************************************************/
  global_map_free(globalMap);

  dispose_list(list);
  dispose_list(while_list);
  dispose_list(variables_list);
  dispose_list(function_statements_list);

  free(list);
  free(while_list);
  free(variables_list);
  free(function_statements_list);


  if (function_name != NULL){ free(function_name); }
  if (call_name != NULL ){ free(call_name); }
  if (variable_name != NULL){ free(variable_name); }

  /********************************************************/
  return result;  
}
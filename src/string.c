/**
 * Predmet:  IFJ 
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   string.c
 *
 * Popis: program pro praci s retezcemi
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

/*Pouzite knihovny */
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "scanner.h"
#include "error.h"


/**********************************/
/***********Strings****************/
/**********************************/

int strInit (tString *s)
{
  s->length = 0;
  if ((s->str = (char *) calloc(SIZE, sizeof(char))) == NULL){
    return INT_ERR;
  }
  s->memory = SIZE;

  return SUCCESS;
}

int strAdd (tString *s, char c)
{
  if (s->memory == s->length) {
    s->memory += SIZE;
    if ((s->str = (char *) realloc(s->str, s->memory * sizeof(char))  ) == NULL)
      return INT_ERR;
  }

  s->str[s->length++] = c;
  s->str[s->length] = '\0';

  return SUCCESS;
}



int strFree (tString *s)
{
  s->str = NULL;
  s->memory = 0;
  s->length = 0;

  return SUCCESS;
}


tString strCreate (char *array)
{
  tString s = {NULL, 0, 0};
  if (strInit(&s) == INT_ERR)
    return s;

  int i = 0;
  while (array[i] != '\0')
    if (strAdd(&s, array[i++]) == INT_ERR) {
      strFree(&s);
      return s;
    }

  return s;
}

int strClear (tString *s)
{
  if (s->str){
    free(s->str);
  }
   s->memory = 0;
   s->length = 0;

  return SUCCESS;
}

int strCopy (tString *s, char *array)
{
  array = (char *) realloc(array, s->length + 1);
  if (array == NULL)
    return INT_ERR;

  int i = s->length;
  do
    array[i] = s->str[i];
  while (i--);

  return SUCCESS;
}

int strCompare(tString *s1, tString *s2){
  return (strcmp(s1->str, s2->str));
}

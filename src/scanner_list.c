/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   scanner_list.c
 *
 * Popis: zdrojovy subor scanner_list.
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/
#include "scanner_list.h"

int solved;
int errflg;

void DLError() {
    errflg = TRUE;            
    return;
}

void DLInitList (tDLList *L) {
/* Provede inicializaci seznamu L před jeho prvním použitím  */
	L->First = NULL;
	L->Act = NULL;
	L->Last = NULL;
}

void DLDisposeList (tDLList *L) {
/*
** Zruší všechny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Rušené prvky seznamu budou korektně
** uvolněny voláním operace free. 
**/
	while(L->First != NULL) 
	{
		if(L->First == L->Act)  
		{
			L->Act = NULL;
		}
		if(L->First == L->Last) 
		{
			L->Last = NULL;
		}
		tDLElemPtr pomocny = L->First; 
		L->First = L->First->rptr;
		free(pomocny);
	}
}

void DLInsertFirst (tDLList *L, char * val) {
/*
** Vloží nový prvek na začátek seznamu L.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
	tDLElemPtr pomocny = malloc(sizeof(struct tDLElem));
	if(pomocny == NULL) 
	{
		DLError();
	}
	else 
	{
		pomocny->data = val; //nahrame data
		pomocny->lptr = NULL; 
		pomocny->rptr = L->First; 
		if(L->Last == NULL) 
		{
			L->Last = pomocny; 
		}
		else 
		{
			L->First->lptr = pomocny; 
		}
		L->First = pomocny; 
	}	
}
char * DLCopyFirst (tDLList *L) {
/*
** Prostřednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
	if(L->First == NULL) 
	{
		return NULL;
	}
	else 
	{
		return L->First->data;
	}	
}

void print_elements_of_list(tDLList TL)	{
	
	tDLList TempList=TL;

	int CurrListLength = 0;

	printf("-------TOKEN LIST-------");

	while ((TempList.First!=NULL) && (CurrListLength<100))	{
		printf("\n \t%s",TempList.First->data);
		if ((TempList.First==TL.Act) && (TL.Act!=NULL))
			printf("\t <= toto je aktivní prvek ");
		TempList.First=TempList.First->rptr;
		CurrListLength++;
	}
    if (CurrListLength>=100){
        printf("\nList exceeded maximum length!");
	}
	printf("\n----------------------\n");     
}

/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   list.c
 *
 * Popis:  zdrojovy subor pre list s instrukciami
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac               xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#include "list.h"
#include "error.h"
#include "parser.h"

extern tList * function_statements_list;
extern bool is_LF;

/**
 * Tato funkcia bude volana pri vsetkych chybach nad listom
 * @return 99 on error
 */
int list_error() {
  instruction_exit(INT_ERR);
  return INT_ERR;
}

/**
 * @brief Inicializuje list
 *
 * @param filename nazov suboru kam ukladame instrukcie
 * @return prazdny list
 */
tList* list_init ()  {

  tList* instr_list = (tList*)malloc(sizeof(tList));
  if(instr_list == NULL)  {
    list_error();
  }
  instr_list->first = NULL;
  instr_list->last = NULL;
  instr_list->act = NULL;
  return instr_list;
}

/**
 * @brief Vlozenie instrukcie do listu
 *
 * @param instr_list list instrukcii
 * @param instr_name nazov instrukcie
 * @param addr1 adresa prveho operandu(neterminalu) instrukcie
 * @param addr2 adresa druheho operandu(neterminalu) instrukcie
 * @param addr3 adresa tretieho operandu(neterminalu) instrukcie
 */
void insert_item(tList *instr_list, tInstructionTypes *instr_name , tInstructionData *addr1, tInstructionData *addr2, tInstructionData *addr3)  {

  tNode *new_instr = NULL;
  if((new_instr = (tNode*)malloc(sizeof(tNode))) == NULL ) {
    list_error();
  }
  else {
    new_instr->data.type = *instr_name;
    new_instr->data.address1.type = addr1->type;
    new_instr->data.address1.value = addr1->value;
    new_instr->data.address2.type = addr2->type;
    new_instr->data.address2.value = addr2->value;
    new_instr->data.address3.type = addr3->type;
    new_instr->data.address3.value = addr3->value;

    if (is_LF){
        new_instr->next = function_statements_list->first;  //do ukazatela na dalsi dame aktualne prvy v prvok v zozname
        function_statements_list->first = new_instr;  //do prveho dame novy
        if(function_statements_list->last == NULL) function_statements_list->last = new_instr;  //poslednym prvkom bude stale prvok ktory sme vlozili ako uplne prvy
    }
    else {
        new_instr->next = instr_list->first;  //do ukazatela na dalsi dame aktualne prvy v prvok v zozname
        instr_list->first = new_instr;  //do prveho dame novy
        if(instr_list->last == NULL) instr_list->last = new_instr;  //poslednym prvkom bude stale prvok ktory sme vlozili ako uplne prvy
    }
  }
}


/**
 * @brief Uvolnuje vsetky prvky listu
 *
 * @param instr_list list ktory chceme zrusit
 */
void dispose_list(tList *instr_list) {

  while(instr_list->first != NULL) //postupne rusenie vsetkych prvkov porade
  {
    if(instr_list->first == instr_list->act) //ak narazim na aktualny zrusim aj ten
    {
      instr_list->act = NULL;
    }
    tNode* temp = instr_list->first; //dame prvy prvok do pomocnej aby sme ho mohli uvolnit
    instr_list->first = instr_list->first->next; //druhy prvok sa stane prvym
    free(temp);
  }
}

/**
 * @brief Vrati hodnotu aktivneho prvku
 *
 * @param instr_list list instrukcii
 * @return instrukcia
*/
tNode* return_instruct(tList *instr_list) {
  if(instr_list->act != NULL) { //list je aktivny
    return instr_list->act;
  }
  else  {
    return NULL;
  }
}

/**
 * @brief Posunieme aktivitu na nasledujuci prvok po aktivnom prvku
 *
 * @param instr_list instrukcii
*/
void move_activity(tList *instr_list) {
  if(instr_list->act != NULL) { //zoznam je aktivny
    if(instr_list->act->next != NULL) { //dalsi prvok po aktivnom nie je posledny prvkom
      instr_list->act = instr_list->act->next;
    }
    else  instr_list->act = NULL;
  }
}

/**
 * @brief Nastavenie aktivity listu na prvy prvok
 *
 * @param instr_list list ktoreho aktivitu nastavujeme
 */

void set_active(tList *instr_list)  {
  if(instr_list->first != NULL) {
    instr_list->act = instr_list->first;
  }
}

/**
 * @brief Reverzacia listu na opacne poradie
 *
 * @param head_ref prvy prvok zoznamu
 */
void reverse(struct Node** head_ref)  {
    struct Node* prev = NULL;
    struct Node* current = *head_ref;
    struct Node* next = NULL;
    while (current != NULL) {

        next = current->next;  // ulozim dalsi
        current->next = prev; // Obratenie aktualneho ukazavatela current

        // Posuniem ukazatele o jednu poziciu dopredu
        prev = current;
        current = next;
    }
    *head_ref = prev;
}
/*
 * @brief Prejde cely list a hlada instrukciu na temporary frame
 *
 * @param instr_list list instrukcii
 * @return  instrukcia na TF existuje -> 1 inak vracia 0
 */
int temporary_exists(tList *instr_list)  {
  tNode *list_head = instr_list->first;
  int exists = -1;

  while(list_head)  {
    if(list_head->data.address1.type == TF || list_head->data.address2.type == TF || list_head->data.address3.type == TF) {
      exists = 1;
    }
    else exists = 0;

    list_head = list_head->next;
  }

  return exists;
}

/*
 *  @brief  Prejde cely list a hlada ci uz zadana premenna bola deklarovana
 *
 *  @param  instr_list list instrukcii
 *  @param  variable_name meno premennej
 *  @return vracia 1(true) ak dana premenna existuje inak 0
 */
int variable_exists(tList *instr_list, char * variable_name)
{
  tNode *list_head = instr_list->first;
  int exists = 0; // 0 == false

  while(list_head)  {
    if(list_head->data.type == INSTRUCT_DEFVAR)
    {
      if(strcmp(list_head->data.address1.value.s, variable_name) == 0)
      {
        exists = 1;
        break;
      }
    }
    else exists = 0;

    list_head = list_head->next;
  }

  return exists;
}
/**
 * @brief Zistuje co sme naplnili do unionu instrukcie a printuje to
 *
 * @param order first/second/third instruction operand
 * @param instr_operand hodnota z unionu(integer,double..)
 * @param instr_type typ instrukcie(GF,LF..)
 */
void operand_type(char* order, tValue instr_operand, tDatType instr_type)  {
  if(instr_type == I)  printf("%s operand value: %d type: %u\n\t", order, instr_operand.i, instr_type);
  else if(instr_type == F)  printf("%s operand value: %f type: %u\n\t", order, instr_operand.f, instr_type);
  else  printf("%s operand value: %s type: %u\n\t", order, instr_operand.s, instr_type);
}

/**
 * @brief Vytlacenie vsetkych instrukcii listu
 *
 * @param instr_list list ktory chceme vytlacit
 */
void print_list_elements(tList *instr_list) {

  tList *tmp_list = (tList*)malloc(sizeof(tList));
  *tmp_list = *instr_list;

  char *first, *second, *third;
  first = "first";
  second = "second";
  third = "third";

  printf("Header of the list\n-----------------\n");

  while (tmp_list->first != NULL) {
    printf("\n\t INSTRUCT enum number = %d\n\t", tmp_list->first->data.type);
    if((tmp_list->first->data.address1.type) != 0 ) {
      operand_type(first,tmp_list->first->data.address1.value, tmp_list->first->data.address1.type);
    }
    if((tmp_list->first->data.address2.type) != 0) {
      operand_type(second, tmp_list->first->data.address2.value, tmp_list->first->data.address2.type);
    }
    if((tmp_list->first->data.address3.type) != 0) {
      operand_type(third, tmp_list->first->data.address3.value, tmp_list->first->data.address3.type);
    }
    printf("\n---------------------------------\n");

    tmp_list->first = tmp_list->first->next;
  }

  free(tmp_list);
  printf("\n\nEnd of the list\n-----------------\n");
}
/** Koniec list.c **/

/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   list.h
 *
 * Popis:  hlavickovy subor pre list s instrukciami
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef _LIST_H
#define _LIST_H

//INSTRUCTRUCNA SADA
typedef enum
{
    //praca s ramcami, volanie funkcií
            INSTRUCT_MOVE = 600,
            INSTRUCT_CREATEFREAME = 601,
            INSTRUCT_PUSHFRAME = 602,
            INSTRUCT_DEFVAR = 603,
            INSTRUCT_CALL = 604,
            INSTRUCT_RETURN = 605,
    //praca so zasobnikom
            INSTRUCT_PUSHS = 606,
            INSTRUCT_POPS = 607,
            INSTRUCT_CLEARS = 608,
    //aritmeticke, relacne, boolovske a konverzni INSTRUCT.
            INSTRUCT_ADD = 609,
            INSTRUCT_SUB = 610,
            INSTRUCT_MUL = 611,
            INSTRUCT_DIV = 612,
            INSTRUCT_ADDS = 613,
            INSTRUCT_SUBS = 614,
            INSTRUCT_MULS = 615,
            INSTRUCT_DIVS = 616,
            INSTRUCT_LT = 617,
            INSTRUCT_GT = 618,
            INSTRUCT_EQ = 619,
            INSTRUCT_LTS = 620,
            INSTRUCT_GTS = 621,
            INSTRUCT_EQS = 622,
            INSTRUCT_AND = 623,
            INSTRUCT_OR = 624,
            INSTRUCT_NOT = 625,
            INSTRUCT_ANDS = 626,
            INSTRUCT_ORS = 627,
            INSTRUCT_NOTS = 628,
            INSTRUCT_INT2FLOAT = 629,
            INSTRUCT_FLOAT2INT = 630,
            INSTRUCT_INT2CHAR = 633,
            INSTRUCT_STRI2INT = 634,
            INSTRUCT_INT2FLOATS = 635,
            INSTRUCT_FLOAT2INTS = 636,
            INSTRUCT_INT2CHARS = 639,
            INSTRUCT_STRI2INTS = 640,
    //vstupno-vstupne INSTRUCT.
            INSTRUCT_READ = 641,
            INSTRUCT_WRITE = 642,
    //praca s retezcami
            INSTRUCT_CONCAT = 643,
            INSTRUCT_STRLEN = 644,
            INSTRUCT_GETCHAR = 645,
            INSTRUCT_SETCHAR = 646,
    //praca s typy
            INSTRUCT_TYPE = 647,
    //INSTRUCT. pre riadenie toku programu  toku programu
            INSTRUCT_LABEL = 648,
            INSTRUCT_JUMP = 649,
            INSTRUCT_JUMPIFEQ = 650,
            INSTRUCT_JUMPIFNEQ = 651,
            INSTRUCT_JUMPIFEQS = 652,
            INSTRUCT_JUMPIFNEQS = 653,
            INSTRUCT_EXIT = 654,
    //ladiacie INSTRUCT.
            INSTRUCT_BREAK = 655,
            INSTRUCT_DPRINT  = 656,

    //hlavicka .IFJcode18 + nove instrukcie
            INSTRUCT_HEAD = 657,
            INSTRUCT_POPFRAME = 658,
            INSTRUCT_IDIV = 659,
            INSTRUCT_IDIVS = 660,
            INSTRUCT_LENGTH = 661,
            INSTRUCT_SUBSTR = 662,
            INSTRUCT_PRINT = 663,
            INSTRUCT_CHR = 664,
            INSTRUCT_ORD = 665,
            INSTRUCT_INPUT_S = 666,
            INSTRUCT_INPUT_I = 667,
            INSTRUCT_INPUT_F = 668,
            INSTRUCT_WHILE_START = 669,
            INSTRUCT_WHILE_STATS = 670,
            INSTRUCT_WHILE_END =  671,
            INSTRUCT_IF_ELSE = 672,
            INSTRUCT_IF_THEN = 673,
            INSTRUCT_JUMP_ENDIF = 674,
            INSTRUCT_ENDIF = 675,
            INSTRUCT_LABEL_MAIN = 676,

} tInstructionTypes;

typedef enum
{
    GF = 700,    // Identificator Global frame
    LF = 701,    // Identificator Local frame
    TF = 702,    // Identificator Temporary frame
    I = 703,     // Number int@13
    F = 704,     // Float  %a format
    S = 705,     // String
    FCE = 706,   // pre funkcie
    EMPTY = 707, // prazdna adresa
    N = 708      // napr:nil@nil   <--
} tDatType;

typedef struct value
{
    int i;
    double f;
    char *s;
} tValue;

typedef struct InstD
{
    tDatType type;
    tValue value;
} tInstructionData;

typedef struct Data
{
    tInstructionTypes type;
    tInstructionData address1;
    tInstructionData address2;
    tInstructionData address3;
} tData;

typedef struct Node
{
    tData data;
    struct Node *next;
} tNode;

typedef struct List
{
    tNode *first;
    tNode *last;
    tNode *act;
} tList;

// Prototypy funkcii
int list_error();
tList* list_init();
void insert_item (tList *instr_list, tInstructionTypes *instr_name , tInstructionData *addr1, tInstructionData *addr2, tInstructionData *addr3);
void dispose_list(tList *instr_list);
tNode* return_instruct(tList *instr_list);
void move_activity(tList *instr_list);
void set_active(tList *instr_list);
void reverse(struct Node** head_ref);
int temporary_exists(tList *instr_list);
void operand_type(char* order, tValue instr_operand, tDatType instr_type);
void print_list_elements(tList *instr_list);

#endif //_LIST_H

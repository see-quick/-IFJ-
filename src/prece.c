/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   prece.h
 *
 * Popis: SA pre výrazy pomocou precedenčnej tabuľky
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/
#include "prece.h"
#include "stack.h"
#include "string.h"
#include "parser.h"

#define STACK_POP4 stack_pops(4, stack)

extern char * function_name;

bool isFirstVariable = false;
bool isThirdVariable = false;
bool is_result = false;
tInstructionTypes instr_type;
tInstructionData instr1;
tInstructionData instr2;
tInstructionData instr3;

extern tDataFunction gData;

extern bool is_LF;

unsigned short labelCounter = 0;
unsigned short divCounter = 0;
unsigned short int2floatCounter1 = 0;
unsigned short int2floatCounter2 = 0;
unsigned short int2floatCounter3 = 0;
unsigned short int2floatCounter4 = 0;
unsigned short ifstringCounter = 0;
unsigned short label2Counter2 = 0;
unsigned short label3Counter3 = 0;
unsigned short nilEndCounter = 0;

// PRECENDENCE TABLE
prece_states prece_table [SIZEOFTABLE][SIZEOFTABLE] = {
/*        +    -    *    /    <    >   <=   >=   ==   !=   i    (    )    $   ,   f          <------- ACT TOKEN */
/* + */ { G ,  G,   L,   L,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* - */ { G ,  G,   L,   L,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* * */ { G ,  G,   G,   G,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},     // TODO: ocekovat na zakladne zadania
/* / */ { G ,  G,   G,   G,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* < */ { L ,  L,   L,   L,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* > */ { L ,  L,   L,   L,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* <=*/ { L ,  L,   L,   L,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* >=*/ { L ,  L,   L,   L,   G,   G,  G,   G,   G,   G,   L,   L,   G,   G,  G,  L},
/* ==*/ { L ,  L,   L,   L,   L,   L,  L,   L,   G,   G,   L,   L,   G,   G,  G,  L},
/* !=*/ { L ,  L,   L,   L,   L,   L,  L,   L,   G,   G,   L,   L,   G,   G,  G,  L},
/* i */ { G ,  G,   G,   G,   G,   G,  G,   G,   G,   G, Err, Err,   G,   G,  G,Err},
/* ( */ { L ,  L,   L,   L,   L,   L,  L,   L,   L,   L,   L,   L,  EQ, Err, EQ,  L},
/* ) */ { G ,  G,   G,   G,   G,   G,  G,   G,   G,   G, Err, Err,   G,   G,  G,Err},
/* $ */ { L ,  L,   L,   L,   L,   L,  L,   L,   L,   L,   L,   L, Err, Err, Err, L},
/* , */ { L  , L  , L ,  L ,  L ,  L  ,L  , L  , L  , L  , L ,  L  , EQ , L ,EQ,  L},
/* f */ {Err, Err, Err, Err, Err, Err,Err,Err, Err , Err , Err, EQ, Err, Err,Err,Err},
};

/**
 * Funckia ktora inicializuje, mapovu śtruktúru kde sa ukladajú dočasne informácie o tokene
 */
void setEmptyDataIDF() {
    dataIDF.type = 500;
    dataIDF.defined = false;
    dataIDF.value.nil = true;
}

/**
 * KVOLI INDEXOVANIU NA PRECEDENCNI TABULKU
 * @param indexer konkretne cislo tokenu ziskane z parseru
 * @param lMap localna mapa
 * @return vracia prekonvertovany token na index pre precedencnu tabulku
 */
int indexerOfPreceTable (int indexer, LocalMap* lMap)
{
    int type = indexer;          // vyberieme si co je aktualny token a budeme ho indexovat
    switch (type)
    {
        /* OPERACIE */
        case LEX_ADDITION: type = ePLUS; break;         // +
        case LEX_SUBSTRACTION: type = eMINUS; break;    // -
        case LEX_MULTIPLICATION: type = eMUL; break;    // *
        case LEX_DIVISION: type = eDIV; break;          // /
        case LEX_LESSER: type = eLESS; break;           // <
        case LEX_GREATER: type = eGREAT; break;         // >
        case LEX_LESSER_EQUAL: type = eLEQUAL; break;   // <=
        case LEX_GREATER_EQUAL: type = eGEQUAL; break;  // >=
        case LEX_DOUBLE_EQUAL: type = eEQUAL; break;    // ==
        case LEX_UNEQUAL: type = eNEQUAL; break;        // !=
        case LEX_L_BRACKET: type = eLBAR; break;        // (
        case LEX_R_BRACKET: type = eRBAR; break;        // )
        /* IDENTIFIKATOR STAVY */
        case LEX_ID:
        case LEX_ID_F:
        case KW_LENGTH:
        case KW_SUBSTR:
        case KW_ORD:
        case KW_CHR:
            if(global_map_contain(gMap, gToken.data.str)){
                type = eFCE;            // nachadza sa to v gMap je to funckia
            }
            else if(local_map_contain(lMap, gToken.data.str)){
                type = eIDENT;         // nachadza sa to v lMap je to premenna
            }
            else if (is_LF){
                gData = global_map_get_value(gMap, function_name);
                if (local_map_contain(gData.lMap, gToken.data.str)){
                    type = eIDENT;
                }
            }
            else{
                return eSEMERROR;
            }
            break;

        case LEX_STRING: type = eIDENT; break;          // ked pride string  premenime ho na index 10 cize identifikator
        case LEX_NUMBER: type = eIDENT; break;          // ked pride cislo -> i
        case LEX_REAL_NUMBER: type= eIDENT; break;      // ked pride cislo -> i
        /* KONIEC PRECE */
        case LEX_EOL: type = eDOLAR; break;             // v pripade ze to bude EOL napriklad bude then tak sa to bude spravat ako $
        case LEX_EOF: type = eDOLAR; break;             // v pripade ze to bude EOF na konci suboru
        case KW_THEN: type = eDOLAR; break;             // v pripade ze to bude then taktiez ukoncuj
        case KW_DO: type = eDOLAR; break;               // v pripade ze pripde DO
        /* COMMA a FCE */
        case LEX_COMMA: type = eCOMMA; break;       // comma ,
        case KW_DEF: type = eFCE; break;            // funkcie
        default:
            /* SYNTAKTICKA CHYBA */
            return eSYNTERROR;
    }
    return type;
}

/**
 * Funckia, ktorá vytvára MOVE inštrukciu , v prípade ak sa jedná o bool
 * @param list list tokenov
 * @param is_bool pomocny flag
 * @return  MOVE inštrukcia alebo nić na základe návratovej hodnoty
 */
bool move(tList *list, bool is_bool){
    if (is_bool){
        instr_type = INSTRUCT_MOVE;
            if (token == LEX_NUMBER){
                instr2.type = I;
                instr2.value.i = atoi(gToken.data.str);
            }
            else if (token == LEX_REAL_NUMBER){
                instr2.type = F;
                instr2.value.f = atof(gToken.data.str);
            }
            else if (token == LEX_STRING) {
                instr2.type = S;
                instr2.value.s = gToken.data.str;
            }
            else if (token == LEX_ID){
                instr2.type = LF;
                instr2.value.s = gToken.data.str;
            }

            instr1.type = GF;
            instr1.value.s = "$result\0";

            insert_item(list, &instr_type, &instr1, &instr2, &instr3);
    }

    return false;
}

/**
 * Funkcia, ktora je primarne urcena na debugovanie, premiena cisla tokenov na znaky aby vypis bol rychlejsie pochopitelny
 * @param token aktualny token
 * @return konvertovany znak
 */
char* convert_to_char(int token){
    switch(token){
        case 0: return "+\0";
        case 1: return "-\0";
        case 2: return "*\0";
        case 3: return "/\0";
        case 4: return "<\0";
        case 5: return ">\0";
        case 6: return "<=\0";
        case 7: return ">=\0";
        case 8: return "==\0";
        case 9: return "!=\0";
        case 10: return "i\0";
        case 11: return "(\0";
        case 12: return ")\0";
        case 13: return "$\0";
        case 14: return ",\0";
        case 15: return "f\0";
        case 22: return "[<]\0";
        case 30: return "\0";
        case 42: return "E\0";
    }
    return "\0";
}

/**
 * Nastavanie 1. a 2. premennej na $result premennu
 * @param instruction_type konkretna inštrukcia (napr: ADD, SUB..)
 */
void setFirstAndSecondVariableToGenerate(int instruction_type){
    instr_type = instruction_type;
    instr1.type = GF;
    instr1.value.s = "$result\0";        // generovanie UNIQUE // generate non Term -> Unikatny nazov
    instr2.type = GF;
    instr2.value.s = "$result\0";
}

/**
 * Funckia, ktora vygeneruje instruckie pre boolean
 * @param list list inštrukcií
 * @param stack zásobník dát
 * @param instruction_type konkrétna inštrukcia
 * @param positionForConcreteInstruction pozicia v zasobniku +1 or +3 E -> E([1]) + E([3])
 */
void setFirstAndSecondForBoolean(tList*list, tStack* stack, tInstructionTypes instruction_type, int positionForConcreteInstruction){
    instr_type = instruction_type;
    instr1.type = GF;
    instr1.value.s = "$$EXPR1";
    instr2.type = GF;
    instr2.value.s = "$result";
    instr3.type = LF;
    instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].nameOfTheVariable;
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);


    instr_type = INSTRUCT_EQ;
    instr1.value.s = "$$EXPR2\0";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_OR;
    instr1.value.s = "$result\0";
    instr2.value.s = "$$EXPR1\0";
    instr3.type = GF;
    instr3.value.s = "$$EXPR2\0";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

void generateType(tList* list, tStack * stack){
    instr_type = INSTRUCT_TYPE;
    instr1.type = GF;
    instr1.value.s = "$type";
    if (isFirstVariable){
        instr2.type = GF;
        instr2.value.s = "$result";
    }
    else if (isThirdVariable){
        instr2.type = LF;
         instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
    }
   
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

/**
 * Fukcia, ktora generuje navestia JUMP
 * @param list list instrukci
 * @param type
 */
void generateLabelJumps(tList* list, int type){
    switch(type){
        case I:
            instr3.value.s = "int"; // tady muze byt string , float, integer, none

            instr_type = INSTRUCT_JUMPIFEQ;
            instr1.value.s = generate_param("MATH_OPERATION", labelCounter);
            instr2.type = GF;
            instr2.value.s = "$type";
            instr3.type = S;
            instr3.value.s = "int";
            insert_item(list, &instr_type, &instr1, &instr2, &instr3);

            break;
        case F:
            instr3.value.s = "float"; // tady muze byt string , float, integer, none

            instr_type = INSTRUCT_JUMPIFEQ;
            instr1.value.s = generate_param("MATH_OPERATION", labelCounter++);
            instr2.type = GF;
            instr2.value.s = "$type";
            instr3.type = S;
            instr3.value.s = "int";
            insert_item(list, &instr_type, &instr1, &instr2, &instr3);

            instr_type = INSTRUCT_JUMPIFEQ;
            instr1.value.s = generate_param("MATH_OPERATION", labelCounter);
            instr2.type = GF;
            instr2.value.s = "$type";
            instr3.type = S;
            instr3.value.s = "float";
            insert_item(list, &instr_type, &instr1, &instr2, &instr3);

            labelCounter--;
            break;
        case S:
            instr3.value.s = "string"; // tady muze byt string , float, integer, none

            instr_type = INSTRUCT_JUMPIFEQ;
            instr1.value.s = generate_param("MATH_OPERATION_FOR_STRING", labelCounter);
            instr2.type = GF;
            instr2.value.s = "$type";
            instr3.type = S;
            instr3.value.s = "string";
            insert_item(list, &instr_type, &instr1, &instr2, &instr3);
            break;
        default:
            break;
    }
}

/**
 * Generovanie float label
 * @param list konkrenty list
 */
void generatingFloatLabel(tList* list){
    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("MATH_OPERATION", labelCounter++); // SUB, MUL
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

/**
 * Generovanie int label
 * @param list konkrenty list
 */
void generatingIntLabel(tList* list){
    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("MATH_OPERATION", labelCounter++); // SUB, MUL
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

/**
 * Generovanie string label
 * @param list konkrenty list
 */
void generatingStringLabel(tList* list){
    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("MATH_OPERATION_FOR_STRING", labelCounter++); // SUB, MUL
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

/**
 * Generovanie konverzie int to float
 * @param list konkrenty list
 * @param stack konkrenty stack
 * @param position pozicia v zasobniku +1 or +3 E -> E([1]) + E([3])
 */
void generatingIntToFloat(tList* list, tStack *stack, int positionForIntToFloatInstruction ){
    instr1.type = GF;
    instr1.value.s = "$result\0";

    // v pripade ze sa je o prve spracovanie tak nechavame
    if (isFirstVariable){
        instr2.type = LF;
        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
    }
    else if(isThirdVariable){
        instr1.type = LF;
        instr1.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
        instr2.type = LF;
        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
    }
    else { instr2.type = I;
        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + positionForIntToFloatInstruction].value.i;
    }

    instr_type = INSTRUCT_INT2FLOAT;
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

/*
 * Funckia, ktora rozhodne medzi typov instrukcie a potom nastavi vsetky argumenty (ADD <var <symb> <symb> a vlozi ich do listu)
 */
void generatingConcreteInstruction(tList* list, tStack *stack, int type, char * instruction_type, int positionForConcreteInstruction){

    if (strcmp(instruction_type, "CONCAT") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_CONCAT);
    }
    else if (strcmp(instruction_type, "GT") == 0 ){
        setFirstAndSecondVariableToGenerate(INSTRUCT_GT);
    }
    else if (strcmp(instruction_type, "LT") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_LT);
    } else if (strcmp(instruction_type, "SUB") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_SUB);
    }else if (strcmp(instruction_type, "MUL") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_MUL);
    }else if (strcmp(instruction_type, "DIV") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_DIV);
    }else if (strcmp(instruction_type, "IDIV") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_IDIV);
    }else if (strcmp(instruction_type, "EQ") == 0){
        setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
    }
    else setFirstAndSecondVariableToGenerate(INSTRUCT_ADD);

    switch(type){
        case I:
            if (isThirdVariable == true ){
                instr3.type = LF;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].nameOfTheVariable;
            }
            else if(isFirstVariable == true){
                instr3.type = LF;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].nameOfTheVariable;
            }
            else { instr3.type = I;
                instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].value.i; // tady bude [+1], value.d, value.s ...
            }
            break;
        case S:
            // v pripade ze sa je o prve spracovanie tak nechavame
            if (isFirstVariable){
                instr3.type = LF;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
            }
            else{
                instr3.type = S;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].value.string.str;   // tady bude [+1], value.d, value.s ...
            }
            if (isThirdVariable){
                instr3.type = LF;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
            }
            else { instr3.type = S;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].value.string.str;   /// tady bude [+1], value.d, value.s ...
            }
            break;
        case F:
            // v pripade ze sa je o prve spracovanie tak nechavame
            if (isFirstVariable){
                instr3.type = LF;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
            }
            else{
                instr3.type = F;
                instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].value.f;   // tady bude [+1], value.d, value.s ...
            }
            if (isThirdVariable){
                instr3.type = LF;
                instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
            }
            else { instr3.type = F;
                instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].value.f;   // tady bude [+1], value.d, value.s ...
            }
            break;
        case N:
            //  todo
            break;
    }

    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
}

/**
 * Funckia, ktora robi celu pracu generovania
 * @param list konkretny list
 * @param stack konkretny zasobnik
 * @param type type -> I, S, F alebo NONE ?
 * @param instruction_type ADD, SUB etc.
 * @param position pozicia v zasobniku +1 or +3 E -> E([1]) + E([3])
 */
void generateInstructionForType(tList* list, tStack *stack, int type, char * instruction_type, int positionForConcreteInstruction, int positionForIntToFloatInstruction){
    // toto generuje konkretny typ
    generateType(list, stack);
    // toto generuje konkretny navestie
    generateLabelJumps(list,type);
    // error pre INCOMPATIBLE_TYPE
    instruction_exit(ERR_INCOMPATIBLE_TYPE);

    // pre float generovanie
    if(type == F){
        // generovanie ADD_float label
        generatingIntLabel(list);
        // TOTO JE KONVERZIA... INT TO FLOAT...
        generatingIntToFloat(list, stack, positionForIntToFloatInstruction);

        // generovanie ADD_int label
        generatingFloatLabel(list);
    } else if (type == I) {
        // generovanie ADD_float label
        generatingIntLabel(list);
    }
    else{
        generatingStringLabel(list);
    }

    // ZISKANIE KONKRETNEHO TYPU INSTRUKCIE
    // generovanie konkretny instrukcie f.e. ADD, SUB etc...
    generatingConcreteInstruction(list, stack, type, instruction_type, positionForConcreteInstruction);
}

/**
 * Funckia, ktora sluzi na generovania erroru pri deleni nulou
 */
void generateIntDivisionError(tList * list, tStack *stack, int positionForConcreteInstruction){
    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$label_err_9", divCounter++);
    instr2.type = LF;
    instr2.value.s =  stack->arrayOfItems[stack->finderOfParenthesis + positionForConcreteInstruction].nameOfTheVariable;
    instr3.type = I;
    instr3.value.i = 0;
    insert_item(list,&instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMP;
    instr1.value.s = generate_param("MATH_OPERATION", labelCounter);
    insert_item(list,&instr_type, &instr1, &instr2, &instr3);

    // LABEL ERR 9
    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$label_err_9", divCounter);
    insert_item(list,&instr_type, &instr1, &instr2, &instr3);
    // INSTRUCTION_EXIT
    instruction_exit(ERR_DIVISION);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("MATH_OPERATION", labelCounter++);
    insert_item(list,&instr_type, &instr1, &instr2, &instr3);
}

/**
 * Genrovanie instrukcii, pre stav NIL a NIL
 * @param list konkrentny list
 * @param stack konkretny zasobnik
 * @param instruction_type konkrenta instrukcia
 */
void generateInstructionForNones(tList* list, tStack *stack, tInstructionTypes instruction_type){
    (void)instruction_type;


    instr_type = INSTRUCT_TYPE;
    instr1.type = GF;
    instr1.value.s = "$type1";
    instr2.type = LF;
    instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
    insert_item(list,&instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_TYPE;
    instr1.type = GF;
    instr1.value.s = "$type2";
    instr2.type = LF;
    instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);


    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$if_string", ++ifstringCounter);
    instr2.type = GF;
    instr2.value.s = "$type1";
    instr3.type = GF;
    instr3.value.s = "$type2";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$int_2_float2", ++int2floatCounter2);
    instr2.type = GF;
    instr2.value.s = "$type1";
    instr3.type = S;
    instr3.value.s = "int";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$int_2_float4", ++int2floatCounter4);
    instr2.type = GF;
    instr2.value.s = "$type1";
    instr3.type = S;
    instr3.value.s = "float";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instruction_exit(ERR_INCOMPATIBLE_TYPE);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$int_2_float4", int2floatCounter4);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$int_2_float3", ++int2floatCounter3);
    instr2.type = GF;
    instr2.value.s = "$type2";
    instr3.type = S;
    instr3.value.s = "int";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);


    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$label_3", ++label3Counter3);
    instr2.type = GF;
    instr2.value.s = "$type2";
    instr3.type = S;
    instr3.value.s = "float";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instruction_exit(ERR_INCOMPATIBLE_TYPE);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$int_2_float1", ++int2floatCounter1);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_INT2FLOAT;
    instr1.type = LF;
    instr1.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
    instr2.type = LF;
    instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMP;
    instr1.value.s = generate_param("$label_3", label3Counter3);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$if_string", ifstringCounter);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$label_2", ++label2Counter2);
    instr2.type = GF;
    instr2.value.s = "$type1";
    instr3.type = S;
    instr3.value.s = "string";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$label_3", label3Counter3);
    instr2.type = GF;
    instr2.value.s = "$type1";
    instr3.type = S;
    instr3.value.s = "int";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$label_3", label3Counter3);
    instr2.type = GF;
    instr2.value.s = "$type1";
    instr3.type = S;
    instr3.value.s = "float";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$label_2", label2Counter2);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_CONCAT;
    instr1.type = GF;
    instr1.value.s = "$result";
    instr2.type = LF;
    instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
    instr3.type = LF;
    instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMP;
    instr1.value.s = generate_param("$nil_end", ++nilEndCounter);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);


    instr_type = INSTRUCT_LABEL;
    instr1.value.s =  generate_param("$label_3", label3Counter3);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = instruction_type;
    instr1.type = GF;
    instr1.value.s = "$result";
    instr2.type = LF;
    instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
    instr3.type = LF;
    instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;

    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMP;
    instr1.value.s = generate_param("$nil_end", nilEndCounter);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$int_2_float2", int2floatCounter2);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = generate_param("$int_2_float1", int2floatCounter1);
    instr2.type = GF;
    instr2.value.s = "$type2";
    instr3.type = S;
    instr3.value.s = "float";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMPIFEQ;
    instr1.value.s = "$label3";
    instr2.type = GF;
    instr2.value.s = "$type2";
    instr3.type = S;
    instr3.value.s = "int";
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instruction_exit(ERR_INCOMPATIBLE_TYPE);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$int_2_float3", int2floatCounter3);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_INT2FLOAT;
    instr1.type = LF;
    instr1.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
    instr2.type = LF;
    instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_JUMP;
    instr1.value.s =  generate_param("$label_3", label3Counter3);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

    instr_type = INSTRUCT_LABEL;
    instr1.value.s = generate_param("$nil_end", nilEndCounter);
    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

}

// @varName pravidlo id = <sth>
// @lMap je lokalni Mapa
/**
 * JADRO celej precedenčnej analýzy
 *  1) Na zaciatku celej precedencnej analyzy inicializuje vsetky potrebne struktury ako zasobnik, expr_return
 *  2) Nasledne zacneme cyklus v ktorom si ziskame indexy do precedencnej tabulky (ktore prechadzaju urcitymi konvertovmi)
 *  3) Po indexoch rozlisujeme a filtrujeme konkrenty token
 *  4) AK ho priradime tak na zaklade zasobnikej situacie zvoli pravidlo < > = Err
 *  5) Po zvoleni pravidla sa prevedie konkrenta logiku celej casti
 *  6) Ukončujeme v pripade zlej postupnosti, nespravnosti typova alebo aj ked prideme do bodu ked budeme mat na
 *  zasobniku iba jeden non-terminal $E$
 * @param lMap locálna mapa
 * @param list konkrétny list
 * @param is_bool flag zda sa jedna o bool
 * @return vracia strukturu expr_return
 */
expr_return parse_expr(LocalMap* lMap, tList* list, bool is_bool){

    /* INICIALIZACIA STRUKTUR */
    expr_return resultOfPrece = {.result=SUCCESS, .bool_result=false};
    tStack* stack = stack_init(200);
    int actTokenIndexToPreceTable = 0;
    int stackTopTokenIndexToPreceTable = 0;

    stack_refresh(stack);                       // vycistenie stacku
    /* ZACIATOK PRECEDENCNEJ ANALYZY */
    stack_push(stack, eDOLAR, dataIDF);         // pushnutie na stack $

    setEmptyDataIDF(dataIDF);                   // nastavenie default hodnot

    do{
        stack->finderOfParenthesis = stack->top; // vyrovnanie top so finderom
        if(stack->arrayOfNumbers[stack->top] == E){
            actTokenIndexToPreceTable = indexerOfPreceTable(token, lMap);  // pretypovanie na dany index
            stackTopTokenIndexToPreceTable = stack->arrayOfNumbers[stack->finderOfParenthesis - 1];     // pozerame sa o jedno miesto nizsie
        }
        else{
            actTokenIndexToPreceTable = indexerOfPreceTable(token, lMap);  // pretypovanie na dany index
            stackTopTokenIndexToPreceTable  = stack_top_token_number(stack); // pretypovanie na dany index
        }

        if(actTokenIndexToPreceTable == eSYNTERROR) {
            resultOfPrece.result = SYN_ERR;
            return resultOfPrece;
        }

        if (token == LEX_NUMBER){
            dataIDF.type = INTEGER;
            dataIDF.value.i = atoi(gToken.data.str);
            resultOfPrece.uniqueID = &gToken.data;
            dataIDF.isVariable = false;
            is_bool = move(list, is_bool);
        }
        else if (token == LEX_REAL_NUMBER){
            dataIDF.type = FLOAT;
            dataIDF.value.f = atof(gToken.data.str);
            resultOfPrece.uniqueID = &gToken.data;
            dataIDF.isVariable = false;
            instr2.type = F;
            instr2.value.f = dataIDF.value.f;
            is_bool = move(list, is_bool);
        }
        else if (token == LEX_STRING){
            dataIDF.type = STRING;
            dataIDF.value.string.str = gToken.data.str;
            resultOfPrece.uniqueID = &gToken.data;
            dataIDF.isVariable = false;
            instr2.type = S;
            instr2.value.s = gToken.data.str;
            is_bool = move(list, is_bool);
        }
        else if(token == LEX_ID || token == LEX_ID_F || token == KW_LENGTH || token == KW_CHR || token == KW_ORD || token == KW_SUBSTR ){
            if(global_map_contain(gMap, gToken.data.str)){
                dataIDF.type = FUNCTION;
                dataIDF.value.string.str = gToken.data.str;
                resultOfPrece.data_type = FUNCTION;
                resultOfPrece.uniqueID = &gToken.data;
                return resultOfPrece;
            }
            // ak sa premenna nachadza v lokalnej mape tak
            else if(local_map_contain(lMap, gToken.data.str)){
                dataIDF = local_map_get_value(lMap, gToken.data.str);
                dataIDF.nameOfTheVariable = gToken.data.str;
                dataIDF.isVariable = true;
                resultOfPrece.uniqueID = &gToken.data;
                is_bool = move(list, is_bool);
            }
            else if (is_LF){
                gData = global_map_get_value(gMap, function_name);
                if (!local_map_contain(gData.lMap, gToken.data.str)){
                    resultOfPrece.result = SEM_ERR;
                    return resultOfPrece;
                }
                else {
                    dataIDF = local_map_get_value(gData.lMap, gToken.data.str);
                    dataIDF.nameOfTheVariable = gToken.data.str;
                    dataIDF.isVariable = true;
                    resultOfPrece.uniqueID = &gToken.data;
                    is_bool = move(list, is_bool);
                }
            }
            else{
                // premenna nebola najdena v localnej mape a tym padom sa jedna o semanticku chybu
                resultOfPrece.result = SEM_ERR;
                return resultOfPrece;
            }
            dataIDF.defined = true;
        }

        switch(prece_table[stackTopTokenIndexToPreceTable][actTokenIndexToPreceTable]) {
            case EQ:
                stack->finderOfParenthesis = stack->top;
                stack_search_for_theorem(stack);
                stack_push(stack, actTokenIndexToPreceTable, dataIDF);
                token = getToken();
                if(!error_lex()){
                    resultOfPrece.result =  ERROR_LEX;
                    return resultOfPrece;
                } else if (!error_int()){
                    resultOfPrece.result = INT_ERR;
                    return resultOfPrece;
                }
                break;
            case L:
                if (stack_top_token_number(stack) == E) {
                    dataIDF = stack->arrayOfItems[stack->finderOfParenthesis];
                    stack_pop(stack);
                    stack_push(stack, eSOLVING_RULE, dataIDF);
                    stack_push(stack, E, dataIDF);
                    stack_push(stack, actTokenIndexToPreceTable, dataIDF);
                } else {
                    stack_push(stack, eSOLVING_RULE, dataIDF);
                    stack_push(stack, actTokenIndexToPreceTable, dataIDF);
                }
                token = getToken();
                if(!error_lex()){
                    resultOfPrece.result =  ERROR_LEX;
                    return resultOfPrece;
                } else if (!error_int()){
                    resultOfPrece.result = INT_ERR;
                    return resultOfPrece;
                }
                break;
            case G:
                stack_search_for_theorem(stack);
                /** PRAVIDLO E -> i **/
                if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 1]) == eIDENT) {
                    dataIDF = stack->arrayOfItems[stack->finderOfParenthesis];
                    dataIDF.nameOfTheNonTerminal =  "$result\0";      // generovanie UNIQUE
                    // toto je v pripade ze pride a = (10+20) napriklad
                    if(stack->arrayOfNumbers[stack->finderOfParenthesis - 1] == 11){
                        instr_type = INSTRUCT_MOVE;
                        instr1.type = GF;
                        instr1.value.s = "$result\0";
                        if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true; }

                        if (isFirstVariable == true ){
                            instr2.type = LF;
                            instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                        }
                        else{
                            switch(stack->arrayOfItems[stack->finderOfParenthesis + 1].type){
                                case INTEGER:
                                    instr2.type = I;
                                    instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    break;
                                case FLOAT:
                                    instr2.type = F;
                                    instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f;
                                    break;
                                case STRING:
                                    instr2.type = S;
                                    instr2.value.s =  stack->arrayOfItems[stack->finderOfParenthesis + 1].value.string.str;
                                    break;
                                default:
                                    break;
                            }
                        }
                        insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                    }
                    isFirstVariable = false;
                    stack_pop(stack);
                    stack_pop(stack);
                    stack_push(stack, E, dataIDF);
                    break;
                }
                    /** PRAVIDLO E -> (E) **/
                else if (((stack->arrayOfNumbers[stack->finderOfParenthesis + 1]) == eLBAR) &&
                         ((stack->arrayOfNumbers[stack->finderOfParenthesis + 2]) == E) &&
                         ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) == eRBAR)) {
                    dataIDF = stack->arrayOfItems[stack->finderOfParenthesis + 2];
                    dataIDF.nameOfTheNonTerminal =  "$result\0";
                    STACK_POP4;
                    stack_push(stack, E, dataIDF);
                    break;
                } else {  // TODO: pravidlo -> E -> f(E), E -> f(E, E), E -> f(E, E , E ), E -> f(E, E, ...)
                    int concreteOperator = stack->arrayOfNumbers[stack->finderOfParenthesis + 2];
                    switch (concreteOperator) {
                        case ePLUS:
                            // PRAVIDLO E -> E + E
                             if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {

                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){

                                        isFirstVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "ADD", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT){
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "ADD", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            generateInstructionForType(list, stack, S, "CONCAT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_ADD);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        isThirdVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "ADD", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT){
                                            generateInstructionForType(list, stack, F, "ADD", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING){
                                            generateInstructionForType(list, stack, S, "CONCAT", 3, 1);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }

                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                     (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {
                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                     else{ isThirdVariable = false; }
                                     if (isThirdVariable == true ){
                                         instr3.type = LF;
                                         instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                     }
                                     else{ instr3.type = S;
                                         instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                     }
                                     dataIDF.type = STRING;
                                     setFirstAndSecondVariableToGenerate(INSTRUCT_CONCAT);
                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                     // generovnaie CONCAT %s@%s %s@%s %s@%s
                                     isThirdVariable = false;
                                 } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                            (stack->arrayOfItems[stack->finderOfParenthesis + 3].type != STRING)) {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;
                                 } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type != STRING) &&
                                            (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;
                                 }
                                 else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                            (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {

                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                     else{ isThirdVariable = false; }

                                     if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                     }
                                     else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                     }
                                     dataIDF.type = INTEGER;
                                     setFirstAndSecondVariableToGenerate(INSTRUCT_ADD);
                                     dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i + stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                     // generovanie ADD %s@%s %s@%s %s@%s
                                     isThirdVariable = false;
                                 } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                            (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                     else{ isFirstVariable = false; }
                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                     else{ isThirdVariable = false; }

                                     if (isFirstVariable){
                                         instr2.type = LF;
                                         instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                     }
                                     else { instr2.type = I;
                                         instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                     }

                                     instr_type = INSTRUCT_INT2FLOAT;
                                     instr1.value.s = "$result\0";
                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                     if (is_result) {
                                         instr2.type = GF;
                                         instr2.value.s = "$result\0";
                                     }
                                     else {
                                         if (isThirdVariable == true ){
                                             instr3.type = LF;
                                             instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                         }
                                         else{ instr3.type = F;
                                             instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                         }
                                     }
                                     dataIDF.type = FLOAT;
                                     setFirstAndSecondVariableToGenerate(INSTRUCT_ADD);
                                     dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i + stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                     // generovanie  INT2FLOAT %s@%s %s@%s
                                     // generovanie  ADD %s@%s %s@%s %s@%s
                                     is_result = false;
                                     isFirstVariable = false;
                                     isThirdVariable = false;
                                 } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                            (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                     else{ isFirstVariable = false; }
                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                     else{ isThirdVariable = false; }

                                     if (isThirdVariable){
                                         instr2.type = LF;
                                         instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                     }
                                     else { instr2.type = I;
                                         instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                     }

                                     instr_type = INSTRUCT_INT2FLOAT;
                                     instr1.value.s = "$result\0";
                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                     if (is_result) {
                                         instr2.type = GF;
                                         instr2.value.s = "$result\0";
                                     }
                                     else {
                                         if (isFirstVariable == true ){
                                             instr3.type = LF;
                                             instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                         }
                                         else{ instr3.type = F;
                                             instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f;
                                         }
                                     }

                                     dataIDF.type = FLOAT;
                                     setFirstAndSecondVariableToGenerate(INSTRUCT_ADD);
                                     dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f + stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                     // generovanie INT@FLOAT %s@%s %s@%s
                                     // generovanie ADD %s@%s %s@%s %s@%s
                                     is_result = true;
                                     isFirstVariable = false;
                                     isThirdVariable = false;
                                 } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                            (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                     if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                     else{ isThirdVariable = false; }
                                     if (isThirdVariable == true ){
                                         instr3.type = LF;
                                         instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                     }
                                     else{ instr3.type = F;
                                         instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                     }
                                     dataIDF.type = FLOAT;
                                     setFirstAndSecondVariableToGenerate(INSTRUCT_ADD);
                                     dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f + stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                     insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                     // generovanie ADD %s@%s %s@%s %s@%s
                                     isThirdVariable = false;
                                 }
                                 else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;
                                 }

                             } else {
                                 resultOfPrece.result = SYN_ERR;
                                 resultOfPrece.bool_result = false;
                                 return resultOfPrece;
                             }
                            resultOfPrece.bool_result = false;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);                 // nakoniec pushneme E + datovu strukturu
                        break;

                            // PRAVIDLO E -> E - E
                        case eMINUS:
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                 if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){

                                        isFirstVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "SUB", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT){
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "SUB", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_SUB);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        isThirdVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "SUB", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT){
                                            generateInstructionForType(list, stack, F, "SUB", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING){
                                            resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) ||
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;

                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    dataIDF.type = INTEGER;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_SUB);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie SUB %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = I;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_INT2FLOAT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = F;
                                            instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                        }
                                    }
                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_SUB);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  INT2FLOAT %s@%s %s@%s
                                    // generovanie  SUB %s@%s %s@%s %s@%s
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = I;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    instr_type = INSTRUCT_INT2FLOAT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = F;
                                            instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f;
                                        }
                                    }

                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_SUB);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie INT@FLOAT %s@%s %s@%s
                                    // generovanie SUB %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_SUB);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  SUB %s@%s %s@%s %s@%s
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = false;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);                 // nakoniec pushneme E + datovu strukturu
                        break;
                            // PRAVIDLO E -> E * E
                        case eMUL:
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){

                                        isFirstVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "MUL", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT){
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "MUL", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_MUL);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        isThirdVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "MUL", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT){
                                            generateInstructionForType(list, stack, F, "MUL", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING){
                                            resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }

                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) ||
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;

                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    dataIDF.type = INTEGER;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_MUL);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i * stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    //generovanie MUL %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = I;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_INT2FLOAT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = F;
                                            instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                        }
                                    }
                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_MUL);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i * stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  INT2FLOAT %s@%s %s@%s
                                    // generovanie  MUL %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = I;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    instr_type = INSTRUCT_INT2FLOAT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = F;
                                            instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f;
                                        }
                                    }

                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_MUL);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f * stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie INT@FLOAT %s@%s %s@%s
                                    // generovanie MUL %s@%s %s@%s %s@%s
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;

                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_MUL);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    //generovanie MUL %s@%s %s@%s %s@%s
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;
                                }

                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = false;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);                 // nakoniec pushneme E + datovu strukturu
                        break;
                            // PRAVIDLO E -> E / E
                        case eDIV:
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){

                                        isFirstVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateIntDivisionError(list, stack, 1);
                                            generateInstructionForType(list, stack, I, "IDIV", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT){
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "DIV", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_DIV);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        isThirdVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER){
                                            generateIntDivisionError(list, stack, 3);
                                            generateInstructionForType(list, stack, I, "IDIV", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT){
                                            generateInstructionForType(list, stack, F, "DIV", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING){
                                            resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) ||
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;



                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
        

                                    if (isThirdVariable == true ){
                                        generateIntDivisionError(list, stack, 3);
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        
                                    }
                                    else { 
                                        instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        if (instr3.value.i == 0){
                                            resultOfPrece.result = ERR_DIVISION;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                    }
                                    dataIDF.type = INTEGER;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_IDIV);
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    //generovanie IDIV %s@%s %s@%s %s@%s
                                    isThirdVariable = false;

                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = I;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_INT2FLOAT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = F;
                                            instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                            if (instr3.value.f == 0.0){
                                                resultOfPrece.result = ERR_DIVISION;
                                                resultOfPrece.bool_result = false;
                                                return resultOfPrece;
                                            }
                                        }
                                    }
                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_DIV);

                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i / stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  INT2FLOAT %s@%s %s@%s
                                    // generovanie  DIV %s@%s %s@%s %s@%s
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = I;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        if (instr2.value.i == 0){
                                            resultOfPrece.result = ERR_DIVISION;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                    }

                                    instr_type = INSTRUCT_INT2FLOAT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = F;
                                            instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f;
                                        }
                                    }

                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_DIV);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f / stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie INT@FLOAT %s@%s %s@%s
                                    // generovanie DIV %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                        if (instr3.value.f == 0.0){
                                            resultOfPrece.result = ERR_DIVISION;
                                            resultOfPrece.bool_result = false;
                                            return resultOfPrece;
                                        }
                                    }
                                    dataIDF.type = FLOAT;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_DIV);

                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f * stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  DIV %s@%s %s@%s %s@%s

                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = false;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = false;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);
                        break;
                            // PRAVIDLO E -> E < E
                        case eLESS:
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {

                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){
                                        isFirstVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "LT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT){
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "LT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            generateInstructionForType(list, stack, S, "LT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_LT);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        isThirdVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "LT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT){
                                            generateInstructionForType(list, stack, F, "LT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING){
                                            generateInstructionForType(list, stack, S, "LT", 3, 1);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }

                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = S;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_LT);
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_LT);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i < stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable){
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                        }
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_LT);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  FLOAT2INT %s@%s %s@%s
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        }
                                    }

                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_LT);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f < stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie INT@FLOAT %s@%s %s@%s
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_LT);
                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f <  stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie LT %s@%s %s@%s %s@%s
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = true;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = true;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);
                        break;
                            // PRAVIDLO E -> E > E
                        case eGREAT:
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {

                                 if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){
                                     isFirstVariable = true;

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "GT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "GT", 3, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            generateInstructionForType(list, stack, S, "GT", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_GT);
                                        }
                                     isFirstVariable = false;
                                     isThirdVariable = false;
                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                     isThirdVariable = true;
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER)
                                            generateInstructionForType(list, stack, I, "GT", 3, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT)
                                            generateInstructionForType(list, stack, F, "GT", 3, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING)
                                            generateInstructionForType(list, stack, S, "GT", 3, 1);
                                        
                                     isFirstVariable = false;
                                     isThirdVariable = false;
                                }

                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = S;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_GT);
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_GT);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i > stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    // v pripade ze sa je o prve spracovanie tak nechavame

                                     if (isThirdVariable){
                                         instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                     }
                                     else { instr2.type = F;
                                         instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                     }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";        // generovanie UNIQUE // generate non Term -> Unikatny nazov
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                        }
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_GT);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i > stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                    // generovanie  FLOAT2INT %s@%s %s@%s
                                    // generovanie GT %s@%s %s@%s %s@%s
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    // ziskanie zo zasobnika zda je token premenna alebo nie v pripade ak ano tak nastavime isFirst alebo isThird na true...
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        }
                                    }

                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_GT);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f * stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie INT@FLOAT %s@%s %s@%s
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_GT);
                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f <  stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie GT %s@%s %s@%s %s@%s
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                     resultOfPrece.bool_result = true;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = true;
                            isFirstVariable = false;
                            isThirdVariable = false;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);                 // nakoniec pushneme E + datovu strukturu
                        break;
                            // PRAVIDLO E -> E <= E
                        case eLEQUAL: // <=
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_LT, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)
                                            // pridat pretypovani int2float
                                           setFirstAndSecondForBoolean(list, stack, INSTRUCT_LT, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_LT, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_LT);
                                        }

                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER)
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_LT, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT)
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_LT, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING)
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_LT, 3);
                                }

                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = S;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_LT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i <= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_LT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i <= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable){
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                        }
                                    }
                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_LT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i - stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                    // generovanie  FLOAT2INT %s@%s %s@%s
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    // ziskanie zo zasobnika zda je token premenna alebo nie v pripade ak ano tak nastavime isFirst alebo isThird na true...
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        }
                                    }

                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_GT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f >= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie  FLOAT2INT %s@%s %s@%s
                                    // generovanie LT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                    // generovanie LTS %s@%s %s@%s %s@%s
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_LT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f <=  stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie LTS %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = true;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = true;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);
                        break;
                            // PRAVIDLO E -> E >= E
                        case eGEQUAL: // >=
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){

                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_GT, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)
                                            // pridat pretypovani int2float
                                           setFirstAndSecondForBoolean(list, stack, INSTRUCT_GT, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_GT, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_GT);
                                        }

                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER)
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_GT, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT)
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_GT, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING)
                                            setFirstAndSecondForBoolean(list, stack, INSTRUCT_GT, 3);
                                } 
                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = S;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_GT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i <= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                    // generovanie GTS %s@%s %s@%s %s@%s
                                    instr_type = INSTRUCT_GTS;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_GT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i >= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    // generovanie GTS %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable){
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                        }
                                    }
                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_GT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i >= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie FLOAT2INT %s@%s %s@%s
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        }
                                    }

                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_GT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f >= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie FLOAT2INT %s@%s %s@%s
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    instr_type = INSTRUCT_GT;
                                    instr1.type = GF;
                                    instr1.value.s = "$$EXPR1\0";
                                    instr2.type = GF;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_EQ;
                                    instr1.value.s = "$$EXPR2\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_OR;
                                    instr1.value.s = "$result\0";
                                    instr2.value.s = "$$EXPR1\0";
                                    instr3.type = GF;
                                    instr3.value.s = "$$EXPR2\0";
                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f  >= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie OR %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = true;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = true;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);
                        break;
                            // PRAVIDLO E -> E == E
                        case eEQUAL: // ==
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){
                                    isFirstVariable = true;
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "EQ", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "EQ", 3, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            generateInstructionForType(list, stack, S, "EQ", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_EQ);
                                        }
                                    isFirstVariable = false;
                                    isThirdVariable = false;

                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                    isThirdVariable = true;
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER)
                                            generateInstructionForType(list, stack, I, "EQ", 1, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT)
                                            generateInstructionForType(list, stack, F, "EQ", 1, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING)
                                            generateInstructionForType(list, stack, S, "EQ", 1, 3);
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }

                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = S;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }

                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i == stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable){
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                        }
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i == stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                    // generovanie  FLOAT2INT %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        }
                                    }

                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f == stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f  >= stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = true;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;
                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = true;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);                 // nakoniec pushneme E + datovu strukturu
                        break;
                            // PRAVIDLO E -> E != E
                        case eNEQUAL: // !=
                            if ((stack->arrayOfNumbers[stack->finderOfParenthesis + 3]) != EMPTY_CHAR) {
                                if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == NONE ){
                                    isFirstVariable = true;
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER){
                                            generateInstructionForType(list, stack, I, "EQ", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)
                                            // pridat pretypovani int2float
                                            generateInstructionForType(list, stack, F, "EQ", 3, 1);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING){
                                            generateInstructionForType(list, stack, S, "EQ", 3, 1);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE){
                                            generateInstructionForNones(list, stack, INSTRUCT_EQ);
                                        }

                                        instr_type = INSTRUCT_NOT;
                                        instr1.type = GF;
                                        instr1.value.s = "$result\0";
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                        insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    isFirstVariable = false;
                                    isThirdVariable = false;

                                }
                                else if (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == NONE ){
                                    isThirdVariable = true;
                                        if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER){
                                            //printf("Som tu...\n");
                                            generateInstructionForType(list, stack, I, "EQ", 1, 3);
                                        }
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT)
                                            generateInstructionForType(list, stack, F, "EQ", 1, 3);
                                        else if (stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING)
                                            generateInstructionForType(list, stack, S, "EQ", 1, 3);

                                        instr_type = INSTRUCT_NOT;
                                        instr1.type = GF;
                                        instr1.value.s = "$result\0";
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                        insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                }
                                
                                else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == STRING) &&
                                    (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == STRING)) {

                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = S;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.string.str;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type= INSTRUCT_NOT;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie NOT %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr3.type = I;
                                        instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i != stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_NOT;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    // generovanie NOT %s@%s %s@%s %s@%s
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == INTEGER) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }

                                    if (isThirdVariable){
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isThirdVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                        }
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i == stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;

                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_NOT;
                                    instr2.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);

                                    is_result = false;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                    // generovanie  FLOAT2INT %s@%s %s@%s
                                    // generovanie GT %s@%s %s@%s %s@%s
                                    // generovanie NOT %s@%s %s@%s %s@%s
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == INTEGER)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 1].isVariable){ isFirstVariable = true;}
                                    else{ isFirstVariable = false; }
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isFirstVariable){
                                        instr2.type = LF;
                                        instr2.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 1].nameOfTheVariable;
                                    }
                                    else { instr2.type = F;
                                        instr2.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.i;
                                    }

                                    instr_type = INSTRUCT_FLOAT2INT;
                                    instr1.value.s = "$result\0";
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    if (is_result) {
                                        instr2.type = GF;
                                        instr2.value.s = "$result\0";
                                    }
                                    else {
                                        if (isFirstVariable == true ){
                                            instr3.type = LF;
                                            instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                        }
                                        else{ instr3.type = I;
                                            instr3.value.i = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                        }
                                    }

                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.f = (float) stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f != stack->arrayOfItems[stack->finderOfParenthesis + 3].value.i;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_NOT;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie NOT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s
                                    is_result = true;
                                    isFirstVariable = false;
                                    isThirdVariable = false;
                                } else if ((stack->arrayOfItems[stack->finderOfParenthesis + 1].type == FLOAT) &&
                                           (stack->arrayOfItems[stack->finderOfParenthesis + 3].type == FLOAT)) {
                                    if(stack->arrayOfItems[stack->finderOfParenthesis + 3].isVariable){ isThirdVariable = true; }
                                    else{ isThirdVariable = false; }
                                    if (isThirdVariable == true ){
                                        instr3.type = LF;
                                        instr3.value.s = stack->arrayOfItems[stack->finderOfParenthesis + 3].nameOfTheVariable;
                                    }
                                    else{ instr3.type = F;
                                        instr3.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    }
                                    dataIDF.type = BOOLEAN;
                                    setFirstAndSecondVariableToGenerate(INSTRUCT_EQ);
                                    dataIDF.value.f = stack->arrayOfItems[stack->finderOfParenthesis + 1].value.f != stack->arrayOfItems[stack->finderOfParenthesis + 3].value.f;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    instr_type = INSTRUCT_NOT;
                                    insert_item(list, &instr_type, &instr1, &instr2, &instr3);
                                    // generovanie NOT %s@%s %s@%s %s@%s
                                    // generovanie EQ %s@%s %s@%s %s@%s

                                } else {
                                    resultOfPrece.result = ERR_INCOMPATIBLE_TYPE;
                                    resultOfPrece.bool_result = true;
                                    return resultOfPrece;
                                }
                            } else {
                                resultOfPrece.result = SYN_ERR;
                                resultOfPrece.bool_result = false;

                                return resultOfPrece;
                            }
                            resultOfPrece.bool_result = true;
                            STACK_POP4;
                            stack_push(stack, E, dataIDF);
                            break;
                            // tato podmienka je kvoli prikladu a = ++ ....

                        default:
                            resultOfPrece.result = SYN_ERR;
                            resultOfPrece.bool_result = false;
                            return resultOfPrece;
                    }
                }
            break;
            case Err:
                if(actTokenIndexToPreceTable == eDOLAR && stackTopTokenIndexToPreceTable == eDOLAR){
                    /* v pripade ak prijde vyraz a = then  v preklade $$ medzi nimi nebude nic ziaden nontermian(E) tak to vychodi syntakticku chybu */
                    if(stack->top == 0){
                        resultOfPrece.result = SYN_ERR;
                        return resultOfPrece;
                    }

                    resultOfPrece.result = SUCCESS;                                                         // vratenie navratovej hodnoty
                    resultOfPrece.uniqueID->str = stack_top_token_data(stack)->nameOfTheNonTerminal;        // vratenie UNIQUE nazvu identifikatora
                    resultOfPrece.data_type = stack_top_token_data(stack)->type;                        // vratenie typu identificatora
                    stack_free(stack);
                    return resultOfPrece;
                }
                resultOfPrece.result = SYN_ERR;
                return resultOfPrece;
        }
    }while(1);
}

/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   parser.c
 *
 * Popis: Syntakticka a semanticka analyza
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "scanner.h"
#include "error.h"
#include "prece.h"
#include "instr_parse.h"

int token;        	         // aktualni token

bool is_LF = false;          // promenna, ktera rika, jestli jsme se funkci
bool zavorka = false;        // pomocna promenna pro kontrolu spravnosti zavorek
char * function_name;        // nazev funkce , ve ktere se aktualne nachazime
char * variable_name;        // nazev promenne do ktere se prirazuje
char *call_name;             // nazev funkce, kterou volame
bool in_while = false;       // pomocna promenna, rika, jestli je statement v cyklu
int while_counter = 0;       // pocet while vnoreni
int function_counter = 0;    // pocet funkci
int return_type = -1;       
bool in_stat = false; // false - volani fuknce bez prirazeni, true - s prirazenim
bool in_print = false; // rika, jestli je to funkce print()

/*********************************************************************/
/*LOKALNI TABULKA SYMBOLU*/
LocalMap* localMap;
tDataIDF lData;
/*********************************************************************/

/*********************************************************************/
/* GLOBALNI TABULKA SYMBOLU */
GlobalMap* gMap;		     // globalni tabulka symbolu
//tDataFunction *gDataptr;	 // ukazatel na uzel globalni tabulky symbolu
tDataFunction gData;
int paramCount = 0;          // pocet parametru funkce
int argCount = 0;            // pocet argumentu pri volani funkce
unsigned short labelCountOrd = 0;
unsigned short labelCountChr = 0;
unsigned short labelSubstrCount1 = 0;
unsigned short labelSubstrCount2= 0;

/*********************************************************************/

/*********************************************************************/
/*GLOBALNI PROMENNE PRO UKLADANI INSTRUKCI DO PASKY TRIADRESNEHO KODU*/
tList * ilist;               // instruction list
tList * while_list;          // list pro ukladani statements, ktere jsou v telu whilu
tList * tmp_list;            // pomocny list pro swap listu 
tList * variables_list;      // seznam promennych definovanych v telu while/if
   
tList * function_statements_list;
tInstructionTypes instr_type;
tInstructionData instr1;
tInstructionData instr2;
tInstructionData instr3;
/*********************************************************************/

/**
 * Funkcia, ktora inicializuje vsetky build_in funkcie
 */
void insert_build_in_functions(){
	gData.paramCount = 1;
	gData.returnType = INTEGER;
    global_map_put(gMap, "length", gData);

    gData.paramCount = 3;
    gData.returnType = STRING;
    global_map_put(gMap, "substr", gData);

    gData.paramCount = 2;
    gData.returnType = INTEGER;
    global_map_put(gMap, "ord", gData);

    gData.paramCount = 1;
    gData.returnType = STRING;
    global_map_put(gMap, "chr", gData);

    gData.paramCount = 0;
    gData.returnType = NONE;
    //gData.returnType = STRING;
    global_map_put(gMap, "inputs", gData);

    gData.paramCount = 0;
    gData.returnType = NONE;
    //gData.returnType = INTEGER;
    global_map_put(gMap, "inputi", gData);

    gData.paramCount = 0;
    gData.returnType = NONE;
    //gData.returnType = FLOAT;
    global_map_put(gMap, "inputf", gData);
}

/**
 * Funckia, ktora generuje unikatny nazov navestia, funckia atd.
 * @param string konkretny nazov navestia, funckie atd.
 * @param d cislo (suffix nazvu navestia, fce)
 * @return vracia vytvorene unikatne navestie, funkciu atd.
 */
char * generate_param(char *string, unsigned short d){
	char *c = (char*)malloc(sizeof(char) * 2);
    sprintf(c, "%hu", (unsigned short)d+1);
	// size_t length = strlen(string);
	char *generate = malloc(strlen(string) + strlen(c) + 1);
	strcpy(generate, string);
	strcat(generate, c);
	return generate;
}

/**
 * Function which generating instruction EXIT for terminating of the program
 * @param ret_val 0 - if succes
 */
void instruction_exit(int ret_val){
	instr_type = INSTRUCT_EXIT;
	instr1.type = I;
	instr1.value.i = ret_val;
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
}


// funkce zkontrolujem pokud nenastala lexikalni chyba
int error_lex(void){
	if(token == ERROR_LEX || token == LEX_UNKNOWN){
		return 0;
	}

	return 1;
}

// Funkce zkontroluje jestli nenastava interni chyba
int error_int(void){
	if(token == INT_ERR){
		return 0;
	}
	return 1;
}

// Zkontroluje jestli typ tokenu je ten, ktery ocekavame
int checkTokenType(int tokenType){ //funkce na kotnrolu typu tokenu
	return (token == tokenType) ? 1 : 0;
}


/*
* Pravidlo pro argument pri volani funkce
* Muze byt identifikator, cislo, relane cislo, radek
*/
int term(void){
	int result = SUCCESS;

	switch(token){
		case LEX_ID:
		case LEX_NUMBER:
		case LEX_REAL_NUMBER:
		case LEX_STRING:
			// semanticka akce
			// if LEX_ID -> zkontrolovat zda je promenna definovana 

			if(!in_print){
				// zkontrolujeme argumenty prave pro vestavene funkce
				if ( (result = check_substr_ord_build_in(argCount)) != SUCCESS) return result;
			}

			// pokud to neni vestavena funkce, vygenerujeme unikalni promennu, do ktere ulozime argument
			// treba TF@%param1
			instr_type = INSTRUCT_DEFVAR;
			instr1.type = TF;
			instr1.value.s = generate_param("%", argCount);

			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			// presuneme hodnotu argumentu do teto promenne
			instr_type = INSTRUCT_MOVE;
			instr1.type = TF;
			instr1.value.s = generate_param("%", argCount);

			// pokud je celociselny, vygenerovat urcitou instrukci pro int
			if (token == LEX_NUMBER){
				instr2.type = I; //integer
				instr2.value.i = atoi(gToken.data.str);
				insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			}
			// pokud je cislo relane, vygenerovat urcitou instrukci pro float
			else if (token == LEX_REAL_NUMBER){
				instr2.type = F; //float
				instr2.value.f = atof(gToken.data.str);
				insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
			}
			// pokud je radek, vygenerovat urcitou instrukci pro string
			else if (token == LEX_STRING){
				instr2.type = S; //string
				instr2.value.s = gToken.data.str;
				insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
			}
			// pokud je identifikator
			else if (token == LEX_ID){
				// zkontrolovat jestli se nachazi v globalni tabulce
				if (!local_map_contain(localMap, gToken.data.str)){
				 	if (is_LF){
				 		// pokud jsme ve funkci, podivame se do lokalni mapy teto funkce
				 		// pokud je to lokalni promenna funkce
						gData = global_map_get_value(gMap, function_name);
						if (!local_map_contain(gData.lMap, gToken.data.str)){
							instruction_exit(SEM_ERR);
							return SEM_ERR;
						}
						// pokud promenna neni definovala - > jedna se o semantickou chybu
					} else{
						instruction_exit(SEM_ERR);
						return SEM_ERR;
					}
				}
				else {
					instr2.type = LF; //string
					instr2.value.s = gToken.data.str;
					insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
				}
			}
			// nacteme token pro dalsi pravidlo
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			return SUCCESS;
		break;

		default:
			// zkontrolujeme jestli pocet argumentu pri volani odpovida poctu parametru
			if (global_map_contain(gMap, call_name)){
				gData = global_map_get_value(gMap, call_name);
				if (argCount != gData.paramCount){
					instruction_exit(ERR_PARAMS_COUNT);
					return ERR_PARAMS_COUNT;
				}
				else {
					// jiny token chyba
					instruction_exit(SYN_ERR);
					return SYN_ERR;
				}
			}
			else {
					//jiny token = chyba!
					instruction_exit(SYN_ERR);
					return SYN_ERR;
			}
	}

	return result;
}



/*
*	Funkce zkontroluje typ parametru vestavenych funkci
* Prvni paramentr funkci length, substr, ord musi byt radek
*/
int check_length_substr_ord_build_in(){
	if (  ( (strcmp(call_name, "length")) == 0 )  ||  ( (strcmp(call_name, "substr")) == 0 ) ||  ( (strcmp(call_name, "ord")) == 0 ) ) {
		if (token != LEX_STRING){ // pokud neni radek
			if (token != LEX_ID){ // pokud je to definovana promenna
				instruction_exit(ERR_INCOMPATIBLE_TYPE); // jinak se jedna o nekompatibilni typy
				return ERR_INCOMPATIBLE_TYPE;
			}
			else {
				// jestli lokalni mapa hlavniho telo programu obsahuje tuto promennou
				if (!local_map_contain(localMap, gToken.data.str)){
					if (is_LF){
						// pokud jsme ve funkci, podivame se na jeji lokalni promenne
						gData = global_map_get_value(gMap, function_name);
						if (!local_map_contain(gData.lMap, gToken.data.str)){
							// pokud neni definovana -> jedna se o semantickou chybu
							instruction_exit(SEM_ERR);
							return SEM_ERR;
						}
						else {
							// pokud jsme ve funkci
							lData = local_map_get_value(gData.lMap, gToken.data.str);
							// pokud typ promenne je string
							if (lData.type != STRING){
								if ( lData.type != NONE ){
									instruction_exit(ERR_INCOMPATIBLE_TYPE);
									return ERR_INCOMPATIBLE_TYPE;
								}
								else {
									// zkusime zjistit typ promenne dynamicky a porovnat jestli je to string
									instr_type = INSTRUCT_TYPE;
									instr1.type = GF;
									instr1.value.s = "$tmp2";
									instr2.type = LF;
									instr2.value.s = gToken.data.str;
									insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

									instr_type = INSTRUCT_JUMPIFEQ;
									instr1.value.s = generate_param("$label_true", ++labelCountOrd);
									instr2.type = GF;
									instr2.value.s = "$tmp2";
									instr3.type = S;
									instr3.value.s = "string";
									insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

									instruction_exit(ERR_INCOMPATIBLE_TYPE);

									instr_type = INSTRUCT_LABEL;
									instr1.value.s = generate_param("$label_true", labelCountOrd);

									insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
								}
							}
						}
					}
				}
				else{
					// jinak nekomplatibilni typy
						lData = local_map_get_value(localMap, gToken.data.str);
						if (lData.type != STRING){
							instruction_exit(ERR_INCOMPATIBLE_TYPE);
							return ERR_INCOMPATIBLE_TYPE;
						}
				}
			}
		}
	}
	return SUCCESS;
}

/*
*	Funkce zkontroluje typ parametru vestavene funkce chr
*  Prvni paramentr funkce chr musi byt cislo
*/
int check_chr_build_in(){
	if ((strcmp(call_name, "chr"))==0){
				if (token != LEX_NUMBER){ // pokud neni cislo
					if (token != LEX_ID){ // ani identifikator
						instruction_exit(ERR_INCOMPATIBLE_TYPE); // jedna se o semantickou chybu
						return ERR_INCOMPATIBLE_TYPE;
					}
					else {
						// pokud lokalni mapa neobsahuje promennou
						if (!local_map_contain(localMap, gToken.data.str)){
							if (is_LF){
								// pokud jsme ve funkci, podivame se na jeji lokalni promenne
								gData = global_map_get_value(gMap, function_name);
								if (!local_map_contain(gData.lMap, gToken.data.str)){
									// pokud promenna neni definovana jedna se o semantickou chybu
									instruction_exit(SEM_ERR);
									return SEM_ERR;
								}
								else {
									// pokud jsme ve funkci
									// zjistime typ promenne, musi byt integer
									lData = local_map_get_value(gData.lMap, gToken.data.str);
									if (lData.type != INTEGER){
										if (lData.type == NONE){
											// pokud je nil, zjistime typ dynamicky a porovname ho s int
												instr_type = INSTRUCT_TYPE;
												instr1.type = GF;
												instr1.value.s = "$tmp2";
												instr2.type = LF;
												instr2.value.s = gToken.data.str;
												insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

												instr_type = INSTRUCT_JUMPIFEQ;
												instr1.value.s = generate_param("$label_true_chr", ++labelCountChr);
												instr2.type = GF;
												instr2.value.s = "$tmp2";
												instr3.type = S;
												instr3.value.s = "int";
												insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

												instruction_exit(ERR_INCOMPATIBLE_TYPE);

												instr_type = INSTRUCT_LABEL;
												instr1.value.s = generate_param("$label_true_chr", labelCountChr);

												insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

												instr_type = INSTRUCT_MOVE;
												instr1.type = GF;
												instr1.value.s = "$tmp2";
												instr2.type = LF;
												instr2.value.s = gToken.data.str;
												insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

										}
										else {
											// jinak se jedna o nekompatibilni typ
											instruction_exit(ERR_INCOMPATIBLE_TYPE);
											return ERR_INCOMPATIBLE_TYPE;
										}
									}
									else {
										// zkontrolujeme range tohoto cisla, pokud neni spravny jedna se o chybe cislo 6
										int i = atoi(gToken.data.str);
										if (i >= 299 || i < 0){
											instruction_exit(ERR_SEMANTIC);
											return ERR_SEMANTIC;
										}
									}
								}
							}
						}
						else{
							// pokud nejsme ve funkci, ale v hlavnim telu
								lData = local_map_get_value(localMap, gToken.data.str);
								if (lData.type != INTEGER){
									// pokud promenna ma jiny typ nez integer -> nekomplatibilni typy
									instruction_exit(ERR_INCOMPATIBLE_TYPE);
									return ERR_INCOMPATIBLE_TYPE;
								}
								else {
										int i = atoi(gToken.data.str);
										if (i >= 299){
											instruction_exit(ERR_SEMANTIC);
											return ERR_SEMANTIC;
										}
								}
						}
					}
				} else {
					// zkontrolujeme range
					int i = atoi(gToken.data.str);
					if (i >= 299){
						instruction_exit(ERR_SEMANTIC);
						return ERR_SEMANTIC;
					}
				}
	}
	return SUCCESS;
}


/*
* Funkce zkontroluje druhy a treti argumenty pri volani funkce substr a ord, musi byt integery
*/
int check_substr_ord_build_in(int param){
	switch(param){
		case 1:
		case 2:
			if (param == 2 &&  ( (strcmp(call_name, "ord")) == 0 ) ){
				// pokud ord ma vic nez 2 argumenty -> spatny pocet argumentu
				instruction_exit(ERR_PARAMS_COUNT);
				return ERR_PARAMS_COUNT;
			}
			else{
				if (  ( (strcmp(call_name, "substr")) == 0 ) ||  ( (strcmp(call_name, "ord")) == 0 ) ) {
					if (token != LEX_NUMBER){ // druhy argument musi byt integer
						if (token != LEX_ID){ // pokud to neni ani identifikator
							instruction_exit(ERR_INCOMPATIBLE_TYPE); // jedna se o nekompatibini typy
							return ERR_INCOMPATIBLE_TYPE;
						}
						else {
							// pokud lokalni mapa neobsahuje promennou
							if (!local_map_contain(localMap, gToken.data.str)){
								if (is_LF){
									// pokud jsme ve funkci
									gData = global_map_get_value(gMap, function_name);
									// a v jeji lokalni tabulce neni 
									if (!local_map_contain(gData.lMap, gToken.data.str)){
										instruction_exit(SEM_ERR); // -> jedna se o semantickou chybu, promenna neni definovana
										return SEM_ERR;
									}
									else {

										// pokud jsme ve funkci
										lData = local_map_get_value(gData.lMap, gToken.data.str);
										if (lData.type != INTEGER){ // pokud typ promenne neni integer
											if (lData.type == NONE){ // zkusime zjistit typ dynamocky a vygenerujeme instrukce pro porovnani typu s int
													instr_type = INSTRUCT_TYPE;
													instr1.type = GF;
													instr1.value.s = "$tmp2";
													instr2.type = LF;
													instr2.value.s = gToken.data.str;
													insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

													switch(param){
														case 1:
															instr_type = INSTRUCT_JUMPIFEQ;
															instr1.value.s = generate_param("$label_true_2", ++labelSubstrCount1);
															instr2.type = GF;
															instr2.value.s = "$tmp2";
															instr3.type = S;
															instr3.value.s = "int";
															insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

															instruction_exit(ERR_INCOMPATIBLE_TYPE);

															instr_type = INSTRUCT_LABEL;
															instr1.value.s = generate_param("$label_true_2", labelSubstrCount1);

															insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
														break;

														case 2:
															instr_type = INSTRUCT_JUMPIFEQ;
															instr1.value.s = generate_param("$label_true_3", ++labelSubstrCount2);
															instr2.type = GF;
															instr2.value.s = "$tmp2";
															instr3.type = S;
															instr3.value.s = "int";
															insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

															instruction_exit(ERR_INCOMPATIBLE_TYPE);

															instr_type = INSTRUCT_LABEL;
															instr1.value.s = generate_param("$label_true_3", labelSubstrCount2);

															insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
														break;
													}
													
											}
											else {
												// jinak se jedna o nekompatibilni typy
												instruction_exit(ERR_INCOMPATIBLE_TYPE);
												return ERR_INCOMPATIBLE_TYPE;
											}
										}
									}
								}
							}
							else{
								// nekomplatibilni typy
									lData = local_map_get_value(localMap, gToken.data.str);
									if (lData.type != INTEGER){
										instruction_exit(ERR_INCOMPATIBLE_TYPE);
										return ERR_INCOMPATIBLE_TYPE;
									}
							}
						}
					}
				}
			}
		break;

		default:
			break;
	}
	return SUCCESS;
}


/*
* Funkce rekurzivne vola funkce term() a kontroluje syntakticou spravnost argumentu
* pri volani funkce a take realizuje semanticke kontroly pro kontrolu definice promenne
*/
int term_list2(bool zavorka){
	int result;

	switch(token){

		//<TERM-LIST2> -> , <TERM> <TERM-LIST2>
		case LEX_COMMA:

		// je carka
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			// zavola pravidlo term()
			result = term();

			argCount++;

			if(result != SUCCESS){
				return result;
			}


			// rekurzivne vola sama sebe pro kontrolu dalsich argumentu
			return term_list2(zavorka);

		break;

		// pokud je prava zavorna
		case LEX_R_BRACKET:
			if (zavorka){return SUCCESS;}
			else {
				// pokud leva zavorka nebyla, a prava byla -> syntakticka chyba
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}
			
		break;

		case LEX_EOL:
		case LEX_EOF:
			if (!zavorka){return SUCCESS;}
			else{
				// pokud leva zavorka byla, a prava nebyla -> syntakticka chyba
					instruction_exit(SYN_ERR);
					return SYN_ERR;
			}
		break;

		default:
				//cokoliv jineho = chyba
				instruction_exit(SYN_ERR);
				return SYN_ERR;
	}
}

/*
*	 Funkce zkontroluje syntaktickou spravnou prvniho argumentu pri volani funkce
*    Argument muze byt identifikator, cislo, realne cislo a nebo radek
*    Zavola dalsi funkci term_list2()
*/

int term_list(bool zavorka){
	int result = SUCCESS;

	switch(token){
		case LEX_ID:
		case LEX_NUMBER:
		case LEX_REAL_NUMBER:
		case LEX_STRING:

			// semanticka akce
			// if LEX_ID -> zkontrolovat zda je promenna definovana
			if (!in_print){
				// pokud volana funkce je vestavene, zavolame pravidlo pro kontrolu typu vestavenych funkce
				if ( (result = check_length_substr_ord_build_in()) != SUCCESS) return result;
				if ( (result = check_chr_build_in()) != SUCCESS) return result;
			}

			// definujeme unikatni promennu pro argument
			// naprikla TF@%param1
			instr_type = INSTRUCT_DEFVAR;
			instr1.type = TF;
			instr1.value.s = generate_param("%", argCount);

			// presuneme hodnotu argumentu do teto promenne
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
			instr_type = INSTRUCT_MOVE;
			instr1.type = TF;
			instr1.value.s = generate_param("%", argCount);

			// pokud je cislo, vygenerujeme urcitou instrukci pro int
			if (token == LEX_NUMBER){
				instr2.type = I; //integer
				instr2.value.i = atoi(gToken.data.str);
			}
			// pokud je to realne cislo, vygenerujeme urcitou instrukci pro float
			else if (token == LEX_REAL_NUMBER){
				instr2.type = F; //float
				instr2.value.f = atof(gToken.data.str);
			}
			// pokud je to radek, vygenerujeme instrukci pro string
			else if (token == LEX_STRING){
				instr2.type = S; //string
				instr2.value.s = gToken.data.str;
			}
			// pokud je to identifikator, zkontrolujeme jeslti je definovana
			else if (token == LEX_ID){
				if (!local_map_contain(localMap, gToken.data.str)){
				 	if (is_LF){ // pokud jsme ve funkci a jeji lokalni mapa neobsahuje tuto promennou
						gData = global_map_get_value(gMap, function_name);
						if (!local_map_contain(gData.lMap, gToken.data.str)){
							instruction_exit(SEM_ERR); // jedna je o semantickou chybus
							return SEM_ERR;
						}
					}
					else {
						instruction_exit(SEM_ERR);
						return SEM_ERR;
					}
				}
				else {
					instr2.type = LF; //string
					instr2.value.s = gToken.data.str;
				}
			}

			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			// zvetsit pocet argumentu
			argCount++;

			//nacteni a volani term_list2()
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}
		
			// funkce zavola dalsi providlo
			return term_list2(zavorka);
		break;

		// pokud je prava zavorka, ale leva nebyla -> syntakticka chyba
		case LEX_R_BRACKET:
			if (zavorka){ return SUCCESS;}
			else { 
					instruction_exit(SYN_ERR);
					return SYN_ERR;
			}
			
		break;

		case LEX_EOL:
		case LEX_EOF:
		// pokud byla leva zavorka, ale nebyla prava -> syntakticka chyba
			if (!zavorka){return SUCCESS;}
			else{ 
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}
		break;


		default:
		// neco jineho syntakticka chyba
			instruction_exit(SYN_ERR);
			return SYN_ERR;
	}

	// cteme dalsi token
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}


	return result;

}

/*
* Funkce generuje insturke MOVE po ukonceni precedenci analyzy
* @expr_return je struktura, ktera obsahuje navratovy typ
*/
int move_value(expr_return res){
	instr_type = INSTRUCT_MOVE;

	lData.type = res.data_type; // navratovy typ z precedenci analyzy
	if (is_LF){// ve funkce
		gData = global_map_get_value(gMap, function_name);
		local_map_put(gData.lMap, variable_name, lData);
	}
	else{ // v hlavnim telu
		local_map_put(localMap, variable_name, lData);
	}

	instr1.type = LF;
	instr2.type = GF;
	instr1.value.s = variable_name;
	instr2.value.s = "$result";
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
	return SUCCESS;
}

/*
*	 Funkce kontroluje syntaktickout spravnost funkci inputi, inputf, inputs
*/
int check_input(){
	if ( (strcmp(call_name, "inputi") == 0 ) ){

		if (local_map_contain(localMap, variable_name)) {
			instr1.type = LF;}
		else {
			instr1.type = LF;
		}

		lData.type = INTEGER;
		if (!is_LF){
			local_map_put(localMap, variable_name, lData);
		}else {
			gData = global_map_get_value(gMap, function_name);
			local_map_put(gData.lMap, variable_name, lData);
		}
		
	 
	 	instr1.value.s = variable_name;
		instr_type = INSTRUCT_INPUT_I;
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
		// prirazeni typu promenne
	}
	else if ( (strcmp(call_name, "inputf") == 0 ) ) {
		if (local_map_contain(localMap, variable_name)) { instr1.type = LF;}
		else {instr1.type = LF;}

		lData.type = FLOAT;
		if (!is_LF){
			local_map_put(localMap, variable_name, lData);
		}else {
			gData = global_map_get_value(gMap, function_name);
			local_map_put(gData.lMap, variable_name, lData);
		}

		instr1.value.s = variable_name;
		instr_type = INSTRUCT_INPUT_F;
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
	}
	else if ( (strcmp(call_name, "inputs") == 0 ) ) {
		if (local_map_contain(localMap, variable_name)) { instr1.type = LF;}
		else {instr1.type = LF;}

		lData.type = STRING;
		//lData.type = NONE;
		if (!is_LF){
			local_map_put(localMap, variable_name, lData);
		}else {
			gData = global_map_get_value(gMap, function_name);
			local_map_put(gData.lMap, variable_name, lData);
		}

		instr1.value.s = variable_name;
		instr_type = INSTRUCT_INPUT_S;
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
	}
	else {
		return 0;
	}

	return 1;
}

/*
*	 Funkce pro spravnost volani funkce s levou zavorkou
*/
int call_left_bracket(tDataFunction *tmp){
	int result;
	// cteni tokenu
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	// vytvareni ramce
	instr_type = INSTRUCT_CREATEFREAME;
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	//volane term_list()
	zavorka = true;
	result = term_list(zavorka);
	if(result != SUCCESS){
		return result;
	}

	//dalsi token je nacten, musi = ')'

	if(!checkTokenType(LEX_R_BRACKET)){
		instruction_exit(SYN_ERR);
		return SYN_ERR;
	}

	// pokud pocet formalnich parametru neni stejny jako pocet argumentu
	if ( tmp->paramCount != argCount ){
		instruction_exit(ERR_PARAMS_COUNT);
		return ERR_PARAMS_COUNT;
	}

	// pro dalsi volani funkce
	argCount = 0; // obnovime promennou

	if (!in_stat){
		result = check_input();
	}
	if (!result){
		// instrukce pro volani funkce

		instr_type = INSTRUCT_CALL;
		instr1.type = FCE;
		if (in_stat) { instr1.value.s = variable_name; }
		else instr1.value.s = call_name;
		

		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

		if (!in_stat){
			// dynamick prirazeni promenne typu navratove hodnoty
			gData = global_map_get_value(gMap, call_name);
			lData.type = gData.returnType;
			if (!is_LF){
				local_map_put(localMap, variable_name, lData);
			} else {
				gData = global_map_get_value(gMap, function_name);
				local_map_put(gData.lMap, variable_name, lData);
			}
			

			// instrukce MOVE
			instr_type = INSTRUCT_MOVE;
			instr1.type = LF;
			instr1.value.s = variable_name;
			instr2.type = TF;
			instr2.value.s = "%retval";
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
		}
		// navratovy typ funkce 
		return_type = tmp->returnType;
	}

	// cteni dalsiho tokenu
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	return SUCCESS;

}


/*
* 	Funkce pro kontrolu spravnosti volani funkce bez zavorek
*/
int call_without_bracket(tDataFunction *tmp){
	int result; 

	instr_type = INSTRUCT_CREATEFREAME;
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	//volane term_list()
	zavorka = false;
	result = term_list(zavorka);
	if(result != SUCCESS){
		return result;
	}

	if ( tmp->paramCount != argCount ){
		instruction_exit(ERR_PARAMS_COUNT);
		return ERR_PARAMS_COUNT;
	}

	// pro dalsi volani funkce
	argCount = 0;

	// instrukce pro volani funkce

	instr_type = INSTRUCT_CALL;
	instr1.type = FCE;
	if (in_stat) {instr1.value.s = variable_name;}
	else instr1.value.s = call_name;

	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	if (!in_stat){
		// dynamick prirazeni promenne typu navratove hodnoty
		gData = global_map_get_value(gMap, call_name);
		lData.type = gData.returnType;
		if (!is_LF){
			local_map_put(localMap, variable_name, lData);
		} else {
			gData = global_map_get_value(gMap, function_name);
			local_map_put(gData.lMap, variable_name, lData);
		}

		instr_type = INSTRUCT_MOVE;
		instr1.type = LF;
		instr1.value.s = variable_name;
		instr2.type = TF;
		instr2.value.s = "%retval";
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
	}

	return_type = tmp->returnType;

	return SUCCESS;
}

/*
*	Funkce pro kontrolu spravnosti volani funkce
*/
int call_function(){
	int result;
	// je funkce	
	tDataFunction * tmp;				
	call_name = gToken.data.str; 

	tmp = global_map_get_pointer_to_value(gMap, call_name); // zjistime polozku globalni mapy pro volanou funkci


	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	if ( (token == LEX_EOL || token == LEX_EOF) && tmp->paramCount == 0  ){
		// pro dalsi volani funkce
		argCount = 0;

		result = check_input();
		if (!result) {

			// instrukce pro volani funkce

			instr_type = INSTRUCT_CREATEFREAME;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			instr_type = INSTRUCT_CALL;
			instr1.type = FCE;
			instr1.value.s = call_name;

			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			// promenna pro navratovou hodnotu funkce
			instr_type = INSTRUCT_MOVE;
			instr1.type = LF;
			instr1.value.s = variable_name;
			instr2.type = TF;
			instr2.value.s = "%retval";
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


			// dynamick prirazeni promenne typu navratove hodnoty
			gData = global_map_get_value(gMap, call_name);
			lData.type = gData.returnType;
			if (!is_LF){
				local_map_put(localMap, variable_name, lData);
			} else {
				gData = global_map_get_value(gMap, function_name);
				local_map_put(gData.lMap, variable_name, lData);
			}
			

			return_type = tmp->returnType;
		}


		return SUCCESS;
	}
	else {
		switch(token){
			case LEX_L_BRACKET:
				result = call_left_bracket(tmp);
				if (result != SUCCESS){
					return result;
				}
			break;
			case LEX_ID:
			case LEX_NUMBER:
			case LEX_REAL_NUMBER:
			case LEX_STRING:
				result = call_without_bracket(tmp);
				if (result != SUCCESS){
					return result;
				}

			break;
			default:
				instruction_exit(SYN_ERR);
				return SYN_ERR;
		} 

	}
	return SUCCESS;
}


/*
* FUNKCE REALIZUJICI JEDNO Z NEJDULEZITEJSICH PRAVIDEL 
* ID = <STH>
* <STH> muze byt identifikator, volani uzivatelem definovane funkce
* volani vestavene funkce
* volani funkci inputi, inputs, inputf
*/

int sth(){
	int result = SUCCESS;
	expr_return res;

	tDataFunction *tmp;


	switch(token){
		case LEX_ID_F:
		case LEX_ID:
		case KW_LENGTH:
		case KW_SUBSTR:
		case KW_ORD:
		case KW_CHR:
		case KW_INPUT_S:
		case KW_INPUT_I:
		case KW_INPUT_F:

				//SEMANTICKA AKCE, KONTROLA DEFINICE FUNKCE
				tmp = global_map_get_pointer_to_value(gMap, gToken.data.str);
				if (tmp == NULL){
					// neni funkce
					if (!is_LF){
						if (!local_map_contain(localMap, gToken.data.str)){
								instruction_exit(SEM_ERR);
								return SEM_ERR;
						}
					}

					if (!is_LF){ // pokud jsme se funkci
						lData = local_map_get_value(localMap, variable_name);
					}else {
						gData = global_map_get_value(gMap, function_name);
						lData = local_map_get_value(gData.lMap, variable_name);
					}
					
					if (strcmp(variable_name, gToken.data.str) == 0 && lData.type == NONE){
						
							instr_type = INSTRUCT_MOVE;
							instr1.type = LF;
							instr1.value.s = gToken.data.str;
							instr2.type = N;
							instr2.value.s = "nil";
							insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

							instr_type = INSTRUCT_MOVE;
							instr1.type = GF;
							instr1.value.s = "$result\0";
							instr2.type = LF;
							instr2.value.s = gToken.data.str;
							insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
					
					}
					else {
						instr_type = INSTRUCT_MOVE;
						instr1.type = GF;
						instr1.value.s = "$result\0";
						instr2.type = LF;
						instr2.value.s = gToken.data.str;
						insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
					}

					// je to promenna prirazenu typu a = b
					res = parse_expr(localMap, ilist, false);
					result = res.result;
					if (is_LF) return_type = res.data_type;

					if (res.bool_result){
						instruction_exit(ERR_SEMANTIC);
						return ERR_SEMANTIC;
					}

					if (result == SUCCESS){
						move_value(res);
					}
					// to muze byt funkce
					
					else {
						instruction_exit(result);
						return result;
					}

					// cokoliv jineho syntakticka chyba
					if (token != LEX_EOL){
						instruction_exit(SYN_ERR);
						return SYN_ERR;
					}
						
				}
				else{
					result = call_function();
					if (result != SUCCESS){
						return result;
					}
				}			

		break;

		// semanticka chyba prirazeni
		case LEX_EOL:
		case LEX_EOF:
			instruction_exit(SEM_ERR);
			return SEM_ERR;
		break;

		//pokud neni token LEX_ID_F, prozenem to precedencni SA
		case LEX_NUMBER:
		case LEX_STRING:
		case LEX_REAL_NUMBER:
		case LEX_L_BRACKET:

			if (token == LEX_NUMBER || token == LEX_REAL_NUMBER || token == LEX_STRING || token == LEX_ID){
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

				insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
			}

			// volani precedencni analyzy
			res = parse_expr(localMap, ilist, false);
			result = res.result;

			if (is_LF) return_type = res.data_type;


			if(res.bool_result){
				// pokud je to nevalidni prirazeni
				instruction_exit(ERR_SEMANTIC);
				return ERR_SEMANTIC;
			}


			if (res.data_type == FUNCTION){
				// volani funkce 
				// volani funkce v ramci prirazeni, cast rozsireni FUNEXP
						tmp = global_map_get_pointer_to_value(gMap, gToken.data.str);

        	    		token = getToken();
						if(!error_lex()){
							instruction_exit(ERROR_LEX);
							return ERROR_LEX;
						} else if (!error_int()){
							return INT_ERR;
						}

						if ( (token == LEX_EOL || token == LEX_EOF) && tmp->paramCount == 0 ){}
						else {

							switch(token){
								case LEX_L_BRACKET:
									token = getToken();
									if(!error_lex()){
										instruction_exit(ERROR_LEX);
										return ERROR_LEX;
									} else if (!error_int()){
										return INT_ERR;
									}

									instr_type = INSTRUCT_CREATEFREAME;
									insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

									//volane term_list()
									zavorka = true;
									result = term_list(zavorka);
									if(result != SUCCESS){
										return result;
									}

									//dalsi token je nacten, musi = ')'

									if(!checkTokenType(LEX_R_BRACKET)){
										instruction_exit(SYN_ERR);
										return SYN_ERR;
									}

									return SUCCESS;
								break;
								case LEX_ID:
								case LEX_NUMBER:
								case LEX_REAL_NUMBER:
								case LEX_STRING:

									//volane term_list()
									zavorka = false;
									result = term_list(zavorka);
									if(result != SUCCESS){
										return result;
									}

									return SUCCESS;
								break;
								default:
									instruction_exit(SYN_ERR);
									return SYN_ERR;
							}

						}

						if ( tmp->paramCount != argCount ){
							instruction_exit(ERR_PARAMS_COUNT);
							return ERR_PARAMS_COUNT;
						}

						// pro dalsi volani funkce
						argCount = 0;

						instr_type = INSTRUCT_CALL;
						instr1.type = FCE;
						instr1.value.s = call_name;

						insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


						token = getToken();
						if(!error_lex()){
							instruction_exit(ERROR_LEX);
							return ERROR_LEX;
						} else if (!error_int()){
							return INT_ERR;
						}

						if (token == LEX_ADDITION || token == LEX_SUBSTRACTION || token == LEX_MULTIPLICATION || token == LEX_DIVISION){

							token = getToken();
							if(!error_lex()){
								instruction_exit(ERROR_LEX);
								return ERROR_LEX;
							} else if (!error_int()){
								return INT_ERR;
							}

							return sth();
						}

						is_LF = false;

			}

			else if (result == SUCCESS){
					move_value(res);
			}
			else {
				instruction_exit(result);
				return result;
			}

			// cokoliv jineho syntakticka chyba
			if (token != LEX_EOL){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			return result;
			break;
		default:
				instruction_exit(SYN_ERR);
				return SYN_ERR;
	} // end switch 

	return result;
}

/*
*	FUNKCE REALIZUJICI PRAVIDLO PRO
* 	ID = <STH>
*   IF <EXPR> THEN <ST-LIST> ELSE <ST-LIST> END
* 	WHILE <EXPR> DO <ST-LIST> END
*   PRINT 
*/
int stat(){
	int result = SUCCESS;
	expr_return res;

	tDataFunction * tmp;

	switch(token){
		//<STAT> -> id = <STH>
		case LEX_ID:
			variable_name = gToken.data.str;

			//dalsi musi byt '=' nebo EOL
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			switch(token){
				case LEX_EQUAL:
						if (global_map_contain(gMap, variable_name)){
							instruction_exit(SEM_ERR);
							return SEM_ERR;
						}

						if (is_LF){
							gData = global_map_get_value(gMap, function_name);
							if (!local_map_contain(gData.lMap, variable_name) && !(local_map_contain(localMap, variable_name))){
								lData.defined = 1;
								lData.value.nil = true;
								lData.type = 500;

								local_map_put(gData.lMap, variable_name, lData);

								instr_type = INSTRUCT_DEFVAR;
								instr1.type = LF;

								instr1.value.s = variable_name; // nazev promenne
								if (in_while) insert_item(variables_list, &instr_type, &instr1, &instr2, &instr3);
								else {insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);}
							}
						}
						else {
							if ( !local_map_contain(localMap, variable_name) ){
								// ulozeni promenne do lokalni mapy, hodnota nil, typ nil
								lData.defined = 1;
								lData.value.nil = true;
								lData.type = 500; // typ nil - NONE


								// nazev promenne ziskame z list pro tokeny pomoci funkce DLCOPYFISRT
								local_map_put(localMap, variable_name, lData);
					

								// generovani instrukce pro definice promenne s typem nil a hodnotou nil
								instr_type = INSTRUCT_DEFVAR;
								instr1.type = LF;

								instr1.value.s = variable_name; // nazev promenne
								if (in_while) insert_item(variables_list, &instr_type, &instr1, &instr2, &instr3);
								else {insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);}
							
							}
						}
						// jinak promenna je v lokalni mape, nebudeme ukladat promennou s typem none
						
						// nacteni dalsiho tokenu , musi byt  bud' identifikator nebo vyraz nebo funkce
						token = getToken();
						if(!error_lex()){
							instruction_exit(ERROR_LEX);
							return ERROR_LEX;
						} else if (!error_int()){
							return INT_ERR;
						}

						// volani pravidla sth()
						result = sth(localMap);

						if(result != SUCCESS){
							return result;
						}

				break;

				case LEX_EOL:
					if (!global_map_contain(gMap, variable_name)){
						if (!local_map_contain(localMap, variable_name)){
							if (is_LF){
								gData = global_map_get_value(gMap, function_name);
								if (!local_map_contain(gData.lMap, variable_name)){
									instruction_exit(SEM_ERR);
									return SEM_ERR;
								}
								else {
									lData = local_map_get_value(gData.lMap, variable_name);
									return_type = lData.type;
								}
							}
							else {
								instruction_exit(SEM_ERR);
								return SEM_ERR;
							}
						}
					}else {
						tmp = global_map_get_pointer_to_value(gMap, variable_name);
						
						if (tmp->paramCount == 0){
								argCount = 0;

								//result = check_input();
								//if (!result) {
									// instrukce pro volani funkce

									instr_type = INSTRUCT_CREATEFREAME;
									insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

									instr_type = INSTRUCT_CALL;
									instr1.type = FCE;
									instr1.value.s = variable_name;

									insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

									if (is_LF){
										return_type = tmp->returnType;
									}
								//}
						}
					}
					
				break;
				case LEX_L_BRACKET:
					tmp = global_map_get_pointer_to_value(gMap, variable_name);
					if (!global_map_contain(gMap, variable_name)){
						instruction_exit(SEM_ERR);
						return SEM_ERR;
					}
					else {
						in_stat = true;
						result = call_left_bracket(tmp);
						if (result != SUCCESS) return result;
						else {
							if (is_LF){
								return_type = tmp->returnType;
							}
						}
					}
					in_stat = false;
				break;
				case LEX_NUMBER:
				case LEX_REAL_NUMBER:
				case LEX_STRING:
					tmp = global_map_get_pointer_to_value(gMap, call_name);
					if (!global_map_contain(gMap, call_name)){
						instruction_exit(SEM_ERR);
						return SEM_ERR;
					}
					else {
						in_stat = true;
						result = call_without_bracket(tmp);
						if (result != SUCCESS) return result;
						else {
							if (is_LF){
								return_type = tmp->returnType;
                            }
						}
					}
					in_stat = false;
					break;

				default:
					instruction_exit(SYN_ERR);
					return SYN_ERR;
			}

			return SUCCESS;


		case LEX_NUMBER:
		case LEX_REAL_NUMBER:
		case LEX_STRING:

			if (is_LF){
					switch(token){
						case LEX_NUMBER:
							return_type = INTEGER;
						break;
						case LEX_REAL_NUMBER:
							return_type = FLOAT;
						break;
						case LEX_STRING:
							return_type = STRING;
						break;
				}
			}

			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			if (!checkTokenType(LEX_EOL)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			return SUCCESS;

		break;


		case KW_PRINT:

			in_print = true;

			argCount = 0;

			token = getToken();
			
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			switch(token){
				case LEX_L_BRACKET:

					token = getToken();
					if(!error_lex()){
						instruction_exit(ERROR_LEX);
						return ERROR_LEX;
					} else if (!error_int()){
						return INT_ERR;
					}

					//volane term_list()
					zavorka = true;

					instr_type = INSTRUCT_CREATEFREAME;
					insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

					result = term_list(zavorka);
					if(result != SUCCESS){
						return result;
					}

					token = getToken();
					if(!error_lex()){
						instruction_exit(ERROR_LEX);
						return ERROR_LEX;
					} else if (!error_int()){
						return INT_ERR;
					}

					if (argCount == 0){
						instruction_exit(ERR_PARAMS_COUNT);
						return ERR_PARAMS_COUNT;
					}


					instr1.type = I;
					instr1.value.i = argCount;

					instr_type = INSTRUCT_PRINT;
					insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

				break;

				case LEX_ID:
				case LEX_NUMBER:
				case LEX_STRING:
				case LEX_REAL_NUMBER:

					//volane term_list()
					zavorka = false;
					
					instr_type = INSTRUCT_CREATEFREAME;
					insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


					result = term_list(zavorka);
					if(result != SUCCESS){
						return result;
					}


					if (argCount == 0){
						instruction_exit(ERR_PARAMS_COUNT);
						return ERR_PARAMS_COUNT;
					}

					instr1.type = I;
					instr1.value.i = argCount;

					instr_type = INSTRUCT_PRINT;
					insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

				break;

				default:
					//fprintf(stderr, "Syntakticka chyba, ocekavano '(',terminal na radku %d\n", gToken.row );
					instruction_exit(ERR_PARAMS_COUNT);
					return ERR_PARAMS_COUNT;
			}

			in_print = false;

			argCount = 0;

			return SUCCESS;

		break;


		//<STAT> -> if <EXPR> then eol <ST-LIST> else eol <ST-LIST> end if
		case KW_IF:

			in_while = true;

			++while_counter;

			if (while_counter == 1){
				tmp_list = ilist;
				ilist = while_list;
			}

			//nacteni a predani do vyrazove SA
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			res = parse_expr(localMap, ilist, true);
			result = res.result;

			if (!res.bool_result){
				ilist = tmp_list;

				reverse(&(variables_list->first));
      			set_active(variables_list);

      			append_list(ilist, variables_list);

				reverse(&(while_list->first));
      			set_active(while_list);

				append_list(ilist, while_list);
			}

			if(result != SUCCESS){
				//printf("i m here\n");
				instruction_exit(result);
				return result;
			}

			//token je jiz nacteny, musi = KW_THEN
			if(!checkTokenType(KW_THEN)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}


			instr_type =  INSTRUCT_IF_THEN;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			//dalsi = LEX_EOL
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			if(!checkTokenType(LEX_EOL)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			//dalsi token -> volani st_list()
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			result = st_list();
			if(result != SUCCESS){
				return result;
			}

			instr_type = INSTRUCT_JUMP_ENDIF;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


			//token nacteny z st_list() = KW_ELSE
			if(!checkTokenType(KW_ELSE)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			instr_type = INSTRUCT_IF_ELSE;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);



			//dalsi = LEX_EOL
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			if(!checkTokenType(LEX_EOL)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			//nacteni tokenu a volani st_list()
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			result = st_list();
			if(result != SUCCESS){
				return result;
			}

			instr_type = INSTRUCT_JUMP_ENDIF;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


			//token nacten z fce st_list() = KW_END
			if(!checkTokenType(KW_END)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}
			if (while_counter == 1){


				ilist = tmp_list;

				reverse(&(variables_list->first));
      			set_active(variables_list);

      			append_list(ilist, variables_list);

				reverse(&(while_list->first));
      			set_active(while_list);

				append_list(ilist, while_list);

			}

			while_counter--;

			instr_type = INSTRUCT_ENDIF;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			if (while_counter == 0){
				in_while = false;
			}

			return SUCCESS;
		break;


		//<STAT> -> while <EXPR> do eol <ST-LIST> end
		case KW_WHILE:

			in_while = true;

			++while_counter;

			if (while_counter == 1){
				tmp_list = ilist;
				ilist = while_list;
			}


			instr_type = INSTRUCT_WHILE_START;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


			//vyrazova SA, pro precedencni analyzu
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			res = parse_expr(localMap, ilist, true);
			result = res.result;

			if (!res.bool_result){
				ilist = tmp_list;

				reverse(&(variables_list->first));
      			set_active(variables_list);

      			append_list(ilist, variables_list);

				reverse(&(while_list->first));
      			set_active(while_list);

				append_list(ilist, while_list);
				
			}

			instr_type = INSTRUCT_WHILE_STATS;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


			// token nacteny z precedencni analyzy musi byt DO


			if(result != SUCCESS){
				instruction_exit(result);
				return result;
			}

			if(!checkTokenType(KW_DO)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}


			if(!checkTokenType(LEX_EOL)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			//volani st_list
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			result = st_list();


			if(result != SUCCESS){
				return result;
			}


			//dalsi token je zase nacten z fce, musi byt END
			if(!checkTokenType(KW_END)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			if (while_counter == 1){


				ilist = tmp_list;

				reverse(&(variables_list->first));
      			set_active(variables_list);

      			append_list(ilist, variables_list);

				reverse(&(while_list->first));
      			set_active(while_list);

				append_list(ilist, while_list);

			}

			while_counter--;

			instr_type = INSTRUCT_WHILE_END;
			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

			if (while_counter == 0){
					in_while = false;
			}

			return result;
		break;

		default:
			instruction_exit(SYN_ERR);
			return SYN_ERR;
	}

	return result;


}

int st_list(){
	int result = SUCCESS;
	//pravidlo <ST-LIST> -> <STAT> eol <ST-LIST>
	switch(token){

		case KW_IF:
		case KW_WHILE:
		case LEX_ID:
		case KW_PRINT:

			//podle pravidla zavolame stat()
			result = stat();
			if(result != SUCCESS){
				return result;
			}

			// token nacteny z stat() muze byt dalsi statment nebo LEX_EOL/LEX_EOF

			if(checkTokenType(LEX_EOF)){
				return SUCCESS;
			}
			else{

				token = getToken();
			
				if(!error_lex()){
					instruction_exit(ERROR_LEX);
					return ERROR_LEX;
				} else if (!error_int()){
					return INT_ERR;
				}
				return st_list();
			}


		//<ST-LIST> -> nic
      case KW_ELSE:
      case KW_END:
      		return SUCCESS;
      	break;

      case KW_DEF:

			result = func();
			if(result != SUCCESS){
				return result;
			}
			// dalsi token je nacteny z func()
			if(!checkTokenType(LEX_EOL)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

			// dalsi token pro st_list
			token = getToken();
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}
		
			return st_list();


      break;


      // ignoring EOL....

      case LEX_EOL:
      		token = getToken();
      		if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}

		
			return st_list();
      	break;

	}

	return result;
}

int pm_list2(){
	switch(token){
		case LEX_COMMA:

			token = getToken();

			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}


			if(!checkTokenType(LEX_ID)){
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}


			if (!local_map_contain(localMap, gToken.data.str)){
				// definovat tuto promennou s typem nil a hodnotou nil
				lData.defined = 1;
				lData.value.nil = true;
				lData.type = 500; // typ nil - NONE
				gData = global_map_get_value(gMap, function_name);
				local_map_put(gData.lMap, gToken.data.str, lData);
			}

			instr_type = INSTRUCT_DEFVAR;
			instr1.type = LF;
			instr1.value.s = gToken.data.str;
			insert_item(ilist,&instr_type, &instr1, &instr2, &instr3 );


			instr_type = INSTRUCT_MOVE;
			instr1.type = LF;
			instr1.value.s = gToken.data.str;
			instr2.type = LF;
			instr2.value.s = generate_param("%", paramCount);
			insert_item(ilist,&instr_type, &instr1, &instr2, &instr3 );

			// + jeden parametr
			paramCount++;

			token = getToken();

			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}


			return pm_list2();

		case LEX_R_BRACKET:
			return SUCCESS;

		default:
			instruction_exit(SYN_ERR);
			return SYN_ERR;
	}

	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if(!error_int()){
		return INT_ERR;
	}

	return SUCCESS;
}


int pm_list(){
	int result = SUCCESS;

	if (token == LEX_ID){

		if (!local_map_contain(localMap, gToken.data.str)){
			// definovat tuto promennou s typem nil a hodnotou nil
			lData.defined = 1;
			lData.value.nil = true;
			lData.type = 500; // typ nil - NONE
			gData = global_map_get_value(gMap, function_name);
			local_map_put(gData.lMap, gToken.data.str, lData);
		}

		instr_type = INSTRUCT_DEFVAR;
		instr1.type = LF;
		instr1.value.s = gToken.data.str;
		insert_item(ilist,&instr_type, &instr1, &instr2, &instr3 );


		instr_type = INSTRUCT_MOVE;
		instr1.type = LF;
		instr1.value.s = gToken.data.str;
		instr2.type = LF;
		instr2.value.s = generate_param("%", paramCount);
		insert_item(ilist,&instr_type, &instr1, &instr2, &instr3 );


		paramCount++;

		token = getToken();

		if(!error_lex()){
			instruction_exit(ERROR_LEX);
			return ERROR_LEX;
		} else if (!error_int()){
			return INT_ERR;
		}

		return pm_list2();
	}
	else{
		return SUCCESS;
	}

	return result;
}

int func(){
	int result;

	is_LF = true;

	//pravidlo <func> -> def ID ( <pm_list> ) eol <ST-LIST> end

	//dalsi token musi byt typu LEX_ID
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if(!error_int()){
		return INT_ERR;
	}

	if(!checkTokenType(LEX_ID) && !checkTokenType(LEX_ID_F)){
		instruction_exit(SYN_ERR);
		return SYN_ERR;
	}

	// ukladam do GTS definice funkce
    // pokud tam ID neni tak vepsat


    if (!global_map_contain(gMap, gToken.data.str)){
    	if (local_map_contain(localMap, gToken.data.str)){
    		instruction_exit(SEM_ERR);
    		return SEM_ERR;
    	}
    	else{
    		gData.defined = 1;
    		gData.lMap = local_map_init(MAX_SIZE_OF_HASH_TABLE);
    		global_map_put(gMap, gToken.data.str, gData);
    	}
    }
    else {
        //uz byla definovana
        instruction_exit(SEM_ERR);
        return SEM_ERR;
    }


	instr_type = INSTRUCT_LABEL;
	instr1.type = FCE;
	instr1.value.s = DLCopyFirst(&tlist);

	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	instr_type = INSTRUCT_PUSHFRAME;
	instr1.type = EMPTY;
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	function_name = gToken.data.str;

	// navratova hodnota
	instr_type = INSTRUCT_DEFVAR;
	instr1.type = LF;
	instr1.value.s = "%retval";
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	instr_type = INSTRUCT_MOVE;
	instr1.type = LF;
	instr1.value.s = "%retval";
	instr2.type = N;
	instr2.value.s = "nil";

	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


   	//dalsi token musi byt '('
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	if(!checkTokenType(LEX_L_BRACKET)){
		instruction_exit(SYN_ERR);
		return SYN_ERR;
	}

	//po kontrole dalsiho tokenu volane pm_list()

	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	result = pm_list();
	if(result != SUCCESS){
		return result;
	}

	//dalsi token mame z fce pm_list(), musi byt ')'

	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	if(!checkTokenType(LEX_R_BRACKET)){
		instruction_exit(SYN_ERR);
		return SYN_ERR;
	}

	// ulozeni poctu formalnich parametru funkce
	gData.paramCount = paramCount;
	global_map_put(gMap, function_name, gData);


	// pro dalsi funkce
	paramCount = 0;


	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	if(!checkTokenType(LEX_EOL)){
		instruction_exit(SYN_ERR);
		return SYN_ERR;
	}


	//dalsi token pude do funce ST-LIST
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	result = st_list();
	if(result != SUCCESS){
		return result;
	}

	//dalsi token je nacten, musi = KW_END

	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}

	if(!(checkTokenType(KW_END))){
		instruction_exit(SYN_ERR);
		return SYN_ERR;
	}

	// konec funkce

	if (strcmp(call_name, "") != 0){
		if (return_type != -1){
			instr_type = INSTRUCT_MOVE;
			instr1.type = LF;
			instr1.value.s = "%retval";
			instr2.type = TF;
			instr2.value.s = "%retval";

			insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
		}
	}else {
		instr_type = INSTRUCT_MOVE;
		instr1.type = LF;
		instr1.value.s = "%retval";
		instr2.type = GF;
		instr2.value.s = "$result";

		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
	}


	// ukladani typu navratove hodnoty do globalni mapy
	gData.returnType = return_type;
	tDataFunction* tmp = global_map_get_pointer_to_value(gMap, function_name);
	gData.paramCount = tmp->paramCount;
	global_map_put(gMap, function_name, gData);

	instr_type = INSTRUCT_POPFRAME;
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

	// return retval of function
	instr_type = INSTRUCT_RETURN;
	insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
	// end of function, navrat na predchozi pozici


	//uvolneni lokalni mapy z polozky globalni mapy pro funkce

	gData = global_map_get_value(gMap, function_name);

	is_LF = false;


	//nacteni a kontrola dalsiho tokenu
	token = getToken();
	if(!error_lex()){
		instruction_exit(ERROR_LEX);
		return ERROR_LEX;
	} else if (!error_int()){
		return INT_ERR;
	}


	return SUCCESS;

}



int main_p(void){

	int result = SUCCESS;

	switch(token){
		case KW_IF:
		case LEX_ID:
		case KW_WHILE:
		case KW_PRINT:
		case KW_DEF:

			//volani st_list()
			result = st_list();
			if(result != SUCCESS){
				return result;
			}

			// //dalsi musi byt EOL / EOF
			token = getToken();

			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}


			return SUCCESS;
		break;


		default:
			if(!error_lex()){
				instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}
			instruction_exit(SYN_ERR);
			return SYN_ERR;
	}

	return result;

}


int prog(){
	int result; //stav funkce

	switch(token){
		case KW_DEF:
		case KW_IF:
		case KW_WHILE:
		case LEX_ID:
		case KW_PRINT:
			result = main_p();
			if(result != SUCCESS){
				resetToken();
				return result;
			}

			//token nacten z main_p() = EOF
			if(!checkTokenType(LEX_EOL) && !checkTokenType(LEX_EOF)) {
				instruction_exit(SYN_ERR);
				return SYN_ERR;
			}

    		return SUCCESS;
    	break;

    	default:
    		if(!error_lex()){
    			instruction_exit(ERROR_LEX);
				return ERROR_LEX;
			} else if (!error_int()){
				return INT_ERR;
			}
			else{
				// prazdny program a je to spravne
				return SUCCESS;
			}
    		
	}
}




int parse(GlobalMap* globalMap, tList *list) {

	int result = SUCCESS;
	gMap = globalMap;

	ilist = list;
	localMap = local_map_init(MAX_SIZE_OF_HASH_TABLE);
	insert_build_in_functions();

	function_name = (char *) malloc( sizeof(char));
	variable_name = (char *) malloc( sizeof(char));
	call_name = (char *)malloc(sizeof(char));


	if(initToken() == INT_ERR){
		result = INT_ERR;
	}

	// inicializace listu pro tokeny
	DLInitList(&tlist);

	while_list = list_init();
	tmp_list = list_init();
	variables_list = list_init();
	function_statements_list = list_init();

	do{
		if((token = getToken()) == ERROR_LEX) {
			instruction_exit(ERROR_LEX);
			result = ERROR_LEX;
		} else if (token == INT_ERR) {
			return INT_ERR;
		}
	}while(token == LEX_EOL);


	if(result == SUCCESS){

		instr_type = INSTRUCT_LABEL_MAIN;
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);


		// vytvoreni docasneho ramce pro funkce
		instr_type =  INSTRUCT_CREATEFREAME;
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);

		instr_type =  INSTRUCT_PUSHFRAME;
		insert_item(ilist, &instr_type, &instr1, &instr2, &instr3);
		

		result = prog();
	}


	DLDisposeList(&tlist);
	freeToken();
	local_map_free(localMap);
	return result;

}

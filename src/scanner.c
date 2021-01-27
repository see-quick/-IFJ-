/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   scanner.c
 *
 * Popis: Lexikalni anazyla, zalocena na konecnem automatu s koncovymi stavy
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#include "scanner.h"
#include "error.h"
#include "string.h"

//trackovanie zapornych cisel mimo vyrazy
bool expr = false;
bool sub = false;
int digit_check = 0;


/*********************************************************/
/*************Funkce pro prace se strukturou token********/
/*********************************************************/


/*
* Inicializace globalniho tokenu
* @return 0, pokud pamet byla spravne alokovana, jina 99 - intern chyba
*/
int initToken(){
    gToken.row = 1;

    if ( strInit(&(gToken.data)) == INT_ERR)
      return INT_ERR;
    return SUCCESS;
}

/*
* Funkce pridava symbol int character do data tokenu
*/
void pushToken(int character){
    strAdd(&(gToken.data), (char)character);
}

/*
* Funkce vypisuje data tokenu, debugovaci funkce
*/
void returnToken(){
    char* word = gToken.data.str;
    int length = gToken.data.length;
    int i;
    for ( i = 0; i < length; i++){
        printf("%c",word[i]);
    }
    printf("\n");
}

/*
* Funkce vyprazdni strukturu pro data tokenu
*/
void resetToken(){
    strFree(&(gToken.data));
}


/*
* Funkce uvoln'uje pamet tokenu
*/
void freeToken(){
    if (gToken.data.str != NULL){
        free(gToken.data.str);
    }
   gToken.data.memory = 0;
   gToken.data.length = 0;
}


/*
* Struktura, obsahujici nazvy vsech klicovych a rezervovanych slov,vestavenych funkci
*/
const char* keyWords[] = {
    "def" ,"do", "else",
    "end","if",
    "not",
    "nil",
    "then", "while",
    "inputs", "inputi", "inputf",
    "print", "length", "substr",
    "ord", "chr",
};


/*****************************************************************************/
/*Funkce ktere hledaji klicove nebo rezervovane slovo a vraci jeho typ lexemy*/
/*****************************************************************************/
int isKeyword(tString *word){
    int i;
    for(i = 0; i < KEYWORD_COUNT; i++){
        if(strcmp(keyWords[i], word->str) == 0)
            return i+SHIFT;
    }
    return SUCCESS;
}


/********************************************************************/
/*Hlavni funkce implementujici konecny automat Lexikalniho analytoru*/
/*Navratovy typ - typ lexemu ****************************************/
/********************************************************************/
int getToken(){

    //Reset tokenu do pocatecniho stavu
    resetToken();

    int state = S_START; // pocatecni stav

    bool flag = false;
    int zero_cnt = 0;

    int c, ascii_cnt; // pomocne pormenne pro zpracovani ASCII retezcu
    char ascii_val[2];
    int temp;

    //Hlavni nacitaci smycka
    while(1){

        c = getchar(); // nacte symbol z stdin
        switch(state){
            case S_START:
                if(isspace(c)){
                    if (c == '\n') {
                        gToken.row++;
                        state = S_EOL; // pokud je EOL
                        continue;
                    }
                    else {
                        state = S_START;
                    }
                }

                if(c == ' ' || c == '\r' || c == '\t') break;
                else if(c == '\n'){ pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_EOL; }
                else if(c == EOF){ return LEX_EOF; }
                else if(c == '(') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_L_BRACKET; }      // leva zavorka
                else if(c == ')') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_R_BRACKET; }      // prava zavorka
                else if(c == '[') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_L_SBRACKET; }     // leva hranata zavorka
                else if(c == ']') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_R_SBRACKET; }     // prava hranata zavorka
                else if(c == '+') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_ADDITION; }       // plus
                else if(c == '-') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_SUBSTRACTION;}   // minus || zaporne cislo
                else if(c == '*') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_MULTIPLICATION; } // hvezdicka
                else if(c == '/') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_DIVISION; }       // deleni
                else if(c == '=') { pushToken(c); state = S_EQUAL; }          // rovnitko
                else if(c == ',') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_COMMA; }          // carka
                else if(c == ':') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_COLON; }          // dvojtecka
                else if(c == '.') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_DOT; }            // tecka
                else if(c == ';') { pushToken(c); DLInsertFirst(&tlist, gToken.data.str); return LEX_SEMICOLON;}       // strednik

                else if(c == '!') { pushToken(c); state = S_AS_EXCM; }                       // vykricnik
                else if(c == '>') { pushToken(c); state = S_GREATER; }                       // vetsitko
                else if(c == '<') { pushToken(c); state = S_LESSER; }                        // mensitko
                else if(c == '\"'){ state = S_STRING; }


                else if(c == '#'){ pushToken(c); state = S_COMMENT_ROW; }                    // radkovy komentar


                else if (isdigit(c)) {
                    if ( c == '0'){
                        flag = true;
                        zero_cnt = 1;
                    }
                    pushToken(c);
                    state = S_NUMBER;
                }                     // Cislo
                else if(islower(c) || c == '_'){ pushToken(c); state = S_ID; }               // Identifikator (a-z, '_')
                else{
                    pushToken(c);                                                            // Chybny znak
                    return LEX_UNKNOWN;
                }


                break;

            /************************************************************/
            /************************************************************/
            // Zacatek blokoveho komentare, pravidla pro =begin ...
            // =end
            case S_COMMENT_BLOCK_B:
                if (c == 'e'){
                    state = S_COMMENT_BLOCK_E;
                }
                else {
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
            break;

            case S_COMMENT_BLOCK_E:
                if (c == 'g'){
                    state = S_COMMENT_BLOCK_G;
                }
                else {
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
            break;

            case S_COMMENT_BLOCK_G:
                if (c == 'i'){
                    state = S_COMMENT_BLOCK_I;
                }
                else {
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
            break;

            case S_COMMENT_BLOCK_I:
                if (c == 'n'){
                    state = S_COMMENT_BLOCK_N;
                }
                else{
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
            break;

            case S_COMMENT_BLOCK_N:
                if(c == EOF){
                    return ERROR_LEX;
                }
                else if( c == '\n'){
                  state = S_COMMENT_BLOCK_N_NEWLINE;
                  gToken.row++;
                 }
                else if( c == '=' || c == ' ' || c == '\r' || c == '\t'){
                  state = S_COMMENT_BLOCK_IN;
                }
                else{
                    state = S_COMMENT_BLOCK_N;
                }
            break;

            case S_COMMENT_BLOCK_N_NEWLINE:
              if(c == EOF){
                return ERROR_LEX;
              }
              else if(c == '=') state = S_COMMENT_END;
              else state = S_COMMENT_BLOCK_N_NEWLINE;
            break;

            /************************************************************/

            // Telo blokoveho komentare 

            case S_COMMENT_BLOCK_IN:
              if(c == 'e'){
                state = S_COMMENT_BLOCK_IN_E;
              }
              else {
                state = S_COMMENT_BLOCK_N;
              }
            break;


            case S_COMMENT_BLOCK_IN_E:
              if(c == 'n'){
                state = S_COMMENT_BLOCK_IN_N;
              }
              else{
                state = S_COMMENT_BLOCK_N;
              }
            break;

            case S_COMMENT_BLOCK_IN_N:
              if(c == 'd'){
                return ERROR_LEX;
              }
              else {
                state = S_COMMENT_BLOCK_N;
              }
            break;

            case S_COMMENT_END:
                if (c == 'e'){
                    state = S_COMMENT_END_E;
                }
                else{
                    state = S_COMMENT_END;
                }
            break;

            // Ukonceni blokoveho komentare =end
            case S_COMMENT_END_E:
                if (c == 'n') {state = S_COMMENT_END_N;}
                else {
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
            break;

            case S_COMMENT_END_N:
                if (c == 'd'){
                    state = S_IGNORE_END_COMMENT;
                }
                else{
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
            break;

            case S_IGNORE_END_COMMENT:
                if (c == '\n'){
                    state = S_START;
                }
                else {
                    state = S_IGNORE_END_COMMENT;
                }
            break;
            /************************************************************/
            /************************************************************/

            case S_EQUAL:
                if (c == '='){
                    pushToken(c);
                    DLInsertFirst(&tlist, gToken.data.str);
                    return LEX_DOUBLE_EQUAL;
                }
                else if (c == 'b'){
                    resetToken();
                    state = S_COMMENT_BLOCK_B;
                }
                else{
                    expr = false;
                    ungetc(c, stdin);
                    return LEX_EQUAL;
                }
            break;


            case S_EOL:
                    if(c == '\n'){
                        gToken.row++;
                    }
                    else{
                        expr = false;
                        ungetc(c, stdin);
                        return LEX_EOL;
                    }
            break;

            //Cislo - cela cast
            case S_NUMBER:
                    if(isdigit(c)){
                        // ignorujeme vsechny nuly v cislu
                        if(c == '0' && zero_cnt > 1 && flag) return ERROR_LEX;
                        else if(c == '0') zero_cnt++;
                        pushToken(c);
                        state = S_NUMBER;
                    }
                    else if(expr == false && sub == true && c == '\n') return ERROR_LEX;
                    else if(c == '.'){
                        state = S_NUMBER_POINT;
                        pushToken(c);
                    }
                    else if (c == 'e' || c == 'E'){
                        state = S_NUMBER_EXPONENT;
                        pushToken(c);
                    }
                    else{
                        ungetc(c, stdin);
                        if (flag && zero_cnt > 1){
                            return ERROR_LEX;
                        }
                        else if(isspace(c) || c == ',' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/'
                                  || c == '>' || c == '<'    ){     // is delimiter     && digit_lock == false
                            expr = true;
                            DLInsertFirst(&tlist, gToken.data.str);
                            return LEX_NUMBER;
                        }
                        else return ERROR_LEX;

                    }
                    break;

            //Cislo - desetina cast
            case S_NUMBER_POINT:
                if(isdigit(c)){
                    pushToken(c);
                    state = S_REAL;
                }
                else
                    return ERROR_LEX;
                break;

            //Cislo - exponent
            case S_NUMBER_EXPONENT:
                if(isdigit(c) || c == '+' || c == '-'){
                    pushToken(c);
                    state = S_REAL;
                    if (c == '+' || c == '-'){
                        digit_check = 1;    // pro kontrolu 1e+ erroru
                    }
                    else if (isdigit(c)){
                        digit_check = 0;
                    }
                }
                else
                    return ERROR_LEX;
                break;

            // Realne cislo
            case S_REAL:
                if(isdigit(c)){
                    pushToken(c);
                    state = S_REAL;
                }
                else if(expr == false && sub == true && c == '\n') return ERROR_LEX;
                else if (c == 'e' || c == 'E') {
                    pushToken(c);
                    state = S_NUMBER_EXPONENT;
                }
                else if (c == '.'){
                    ungetc(c, stdin);
                    return ERROR_LEX;
                }
                else{
                    ungetc(c, stdin);
                    if (flag && zero_cnt > 1){
                        return ERROR_LEX;
                    }
                    else if (digit_check == 1){
                        return ERROR_LEX;
                    }
                    else{
                        zero_cnt = 0;
                        expr = true;
                        DLInsertFirst(&tlist, gToken.data.str);
                        return LEX_REAL_NUMBER;
                    }
                }
                digit_check = 0;
                break;



            // Pravidlo pro identifikator, nesmi zacinat z velkeho pismena
            case S_ID:
                if (isalnum(c) || c == '_'){
                  pushToken(c);
                  state = S_ID;
                }
                else if(c == '!' || c == '?'){ // nazev funkce 
                    pushToken(c);
                    state = S_ID_F_END;
                }
                else{
                  ungetc(c, stdin);
                  if ( (temp = isKeyword(&(gToken.data))) != SUCCESS) // klicove slovo
                     { DLInsertFirst(&tlist, gToken.data.str); return temp; }
                  else { expr = true; DLInsertFirst(&tlist, gToken.data.str); return LEX_ID; }
                }
                break;
            // koce indentifikatoriu id!, id?    
            case S_ID_F_END:
                if (isspace(c) || ',' || ')'){ // is delimiter
                    ungetc(c, stdin);
                    DLInsertFirst(&tlist, gToken.data.str);
                    return LEX_ID_F;
                }
                else{
                    return ERROR_LEX;
                }
                break;
            // Radkovy komentar
            case S_COMMENT_ROW:
                if( c == EOF )
                  return LEX_EOF;
                else if( c == '\n'){
                    resetToken(); // ignoruje komentare
                    gToken.row++;
                    state = S_EOL;
                }
                else state = S_COMMENT_ROW;
                break;


            //Mensi nez nebo mensi nebo rovno
            case S_LESSER:
                if(c == '='){
                    pushToken(c);
                    DLInsertFirst(&tlist, gToken.data.str);
                    return LEX_LESSER_EQUAL;
                }
                else{
                    ungetc(c, stdin);
                    return LEX_LESSER;
                }
                break;

            //Vetsi nez nebo vetsi nebo rovno
            case S_GREATER:
                if(c == '='){
                    pushToken(c);
                    DLInsertFirst(&tlist, gToken.data.str);
                    return LEX_GREATER_EQUAL;
                }
                else{
                    ungetc(c, stdin);
                    return LEX_GREATER;
                }
                break;

            //Retezec
            case S_STRING:
                    if ( c == '\n'){
                        return ERROR_LEX;
                    }
                    if( c == EOF)
                        return ERROR_LEX;
                    else if (c == '\\'){
                        state = S_STRING_ESCAPED;
                        //pushToken(c);
                    }
                    else if(c == '\"'){
                        return LEX_STRING;
                    }
                    else
                        pushToken(c);
                break;


            // Escape sekvekce
            case S_STRING_ESCAPED:
                if(c == 'n'){
                    state = S_STRING;
                    pushToken('\n');
                }
                else if(c == 't'){
                    state = S_STRING;
                    pushToken('\t');

                }
                else if(c == '\\'){
                    state = S_STRING;
                    pushToken('\\');

                }
                else if(c == '"'){
                    state = S_STRING;
                    pushToken('\"');

                }
                else if(c == 's'){
                    state = S_STRING;
                    pushToken(' ');
                }
                else if (c == 'x') {
                    state = S_STRING_ASCII;
                    ascii_cnt = 0;
                }
                else
                    return ERROR_LEX;
                break;

            // ASCII retezec
            case S_STRING_ASCII:
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    if (ascii_cnt < 2)
                        ascii_val[ascii_cnt++] = c;
                    else {
                        ungetc(c, stdin);
                        char *endptr = NULL;
                        long ascii_tmp = strtol(ascii_val, &endptr, 16);

                        if (*endptr != '\0' || strcmp(endptr, ascii_val) == 0){
                            return ERROR_LEX;
                        }
                            

                        pushToken((int) ascii_tmp);
                        state = S_STRING;
                    }
                }
                else if (ascii_cnt == 2) {
                    ungetc(c, stdin);
                    char *endptr = NULL;
                    long ascii_tmp = strtol(ascii_val, &endptr, 16);
                    if (*endptr != '\0' || strcmp(endptr, ascii_val) == 0){
                        return ERROR_LEX;
                    }
                        

                    pushToken((int) ascii_tmp);
                    state = S_STRING;
                }
                else{
                    return ERROR_LEX;
                }
                break;

            // Vykricnik 

            case S_AS_EXCM:
                if(c == '='){
                    pushToken(c);
                    DLInsertFirst(&tlist, gToken.data.str);
                    return LEX_UNEQUAL;
                }
                else{
                    ungetc(c, stdin);
                    DLInsertFirst(&tlist, gToken.data.str);
                    return LEX_EXCM;
                }
                break;


            default:
                return ERROR_LEX;
                break;
        }
    }
    return ERROR_LEX;
}

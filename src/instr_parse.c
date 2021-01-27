/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   instr_parse.c
 *
 * Popis:  zdrojovy subor parseru instrukcii
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/
#include "instr_parse.h"
#include "list.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>


int while_count = 0;        // počítadlo cyklov while
int if_count = 0;           // počítadlo if
int arg_count;              // počítadlo argumentov

/**
 * @brief Replace v retazci
 *
 * @param str nas retazec s ktorym pracujeme
 * @param orig retazec ktory hladame
 * @param rep retaec za ktory ho vymenime
 * Je na uzivateli pak uvolneni str.
*/
void replace_str(char **str, char *orig, char *rep) {
  if (*str == NULL || orig == NULL || rep == NULL) return;
  char *p;
  if ((p = strstr(*str, orig)) == NULL) return;
  char *buffer = malloc(strlen(*str) - strlen(orig) + strlen(rep) + 1);
  memset(buffer, '\0', strlen(buffer));

  strncpy(buffer, *str, p-(*str));
  buffer[p-(*str)] = '\0';

  sprintf(buffer+(p-(*str)), "%s%s", rep, p+strlen(orig));
  buffer[strlen(buffer)] = '\0';

  // free((*str));
  *str = buffer;
}

/**
 * @brief Ulahcenie vypisu podla typu instrukcie
 *
 * @param instruction instrukcia
 * @return cislo v enume prevedene na retazec
 */
char* instruct_type(tDatType instruction) {
  char *instr_type;

  switch (instruction) {
    case I:
          return instr_type = "int";
    break;

    case F:
          return instr_type = "float";
    break;

    case S:
          return instr_type = "string";
    break;

    case N:
          return instr_type = "nil";
    break;

    case GF:
          return instr_type = "GF";
    break;

    case LF:
          return instr_type = "LF";
    break;

    case TF:
          return instr_type = "TF";
    break;

    case FCE:
    case EMPTY:
      break;
  }

  return NULL;
}
/*
 * @brief Printuje symbol na zaklade jeho typu(int,id,float..)
 *
 * @param instr_operand data a typ instrukcie
 */
void print_symb(tInstructionData instr_operand)  {
  if (instr_operand.type == I)  {
    printf("%d",instr_operand.value.i);
  }
  else if (instr_operand.type == F) printf("%a",instr_operand.value.f);
  else
  {
     /* escape seq ---- > \000 */
     /* handle '\n' and others */
    if (instr_operand.value.s == NULL){
        return;
    }
    if (strstr(instr_operand.value.s, "\n") != NULL) {
      while((strstr(instr_operand.value.s, "\n") != NULL)) {
        replace_str(&instr_operand.value.s, "\n", "\\010");
      }
    }
    if (strstr(instr_operand.value.s, "\t") != NULL) {
      while ((strstr(instr_operand.value.s, "\t") != NULL)) {
        replace_str(&instr_operand.value.s, "\t", "\\009");
      }
    }
    if (strstr(instr_operand.value.s, "\v") != NULL) {
      while(strstr(instr_operand.value.s, "\v") != NULL) {
        replace_str(&instr_operand.value.s, "\v", "\\011");
      }
    }
    if (strstr(instr_operand.value.s, "\f") != NULL) {
      while(strstr(instr_operand.value.s, "\f") != NULL) {
        replace_str(&instr_operand.value.s, "\f", "\\012");
      }
    }
    if (strstr(instr_operand.value.s, "\r") != NULL) {
      while(strstr(instr_operand.value.s, "\r") != NULL) {
        replace_str(&instr_operand.value.s, "\r", "\\013");
      }
    }
    if (strstr(instr_operand.value.s, " ") != NULL) {
      while(strstr(instr_operand.value.s, " ") != NULL) {
        replace_str(&instr_operand.value.s, " ", "\\032");
      }
    }
    printf("%s",instr_operand.value.s);
  }
}

/*
 * @brief Printuje symbol na zaklade jeho typu(int,id,float..) --> pomocna funkcia
 *
 * @param instr_operand data a typ instrukcie
 */
void print_multiple_symb(tInstructionData instr_operand)  {
  if (instr_operand.type == I)  printf("%d ",instr_operand.value.i);
  else if (instr_operand.type == F) printf("%a ",instr_operand.value.f);
  else printf("%s ",instr_operand.value.s);
}

/*
 * @brief Urci typ instrukcie a vrati jej nazov na zaklade enumu
 *
 * @param act_instr instrukcia ktoru skumame
 */
void print_arit_instr(tNode *act_instr) {
  char *name;
  switch (act_instr->data.type) {
    case INSTRUCT_ADD: name = "ADD"; break;
    case INSTRUCT_SUB: name = "SUB"; break;
    case INSTRUCT_MUL: name = "MUL"; break;
    case INSTRUCT_DIV: name = "DIV"; break;
    case INSTRUCT_IDIV: name = "IDIV"; break;
    case INSTRUCT_ADDS: name = "ADDS"; break;
    case INSTRUCT_SUBS: name = "SUBS"; break;
    case INSTRUCT_MULS: name = "MULS"; break;
    case INSTRUCT_DIVS: name = "DIVS"; break;
    case INSTRUCT_IDIVS: name = "IDIVS"; break;
    case INSTRUCT_LT: name = "LT"; break;
    case INSTRUCT_GT: name = "GT"; break;
    case INSTRUCT_EQ: name = "EQ"; break;
    case INSTRUCT_LTS: name = "LTS"; break;
    case INSTRUCT_GTS: name = "GTS"; break;
    case INSTRUCT_EQS: name = "EQS"; break;
    case INSTRUCT_AND: name = "AND"; break;
    case INSTRUCT_OR: name = "OR"; break;
    case INSTRUCT_ANDS: name = "ANDS"; break;
    case INSTRUCT_ORS: name = "ORS"; break;
    case INSTRUCT_STRI2INT: name = "STRI2INT"; break;
    case INSTRUCT_STRI2INTS: name = "STRI2INTS"; break;
    case INSTRUCT_CONCAT: name = "CONCAT"; break;
    case INSTRUCT_GETCHAR: name = "GETCHAR"; break;
    case INSTRUCT_SETCHAR: name = "SETCHAR"; break;
    default:
      break;
  }

  printf("%s %s@", name, instruct_type(act_instr->data.address1.type) );
  print_multiple_symb(act_instr->data.address1);
  printf("%s@",  instruct_type(act_instr->data.address2.type) );
  print_multiple_symb(act_instr->data.address2);
  printf("%s@", instruct_type(act_instr->data.address3.type));
  print_symb(act_instr->data.address3);
  printf("\n");
}



/**
 * @brief Append `list` to root list
 *
 * @param root Main list
 * @param list Appended list
 * @return True on success, otherwise False
 */
void append_list(tList* root, tList* list) {
    if (list->first == NULL) {
        return;
    }

    for (tNode* tmp = list->first; tmp != NULL; tmp = tmp->next) {
        tData data = tmp->data;
        insert_item(root, &data.type, &data.address1, &data.address2, &data.address3);
    }
}


/*
 * @brief Parsuje list s instrukciami a printuje na stdout instrukcie v IFJcode18
 *
 * @param instr_list list s instrukciami
 * @warning predtym ako sa zavola parse_instructions musis/musite zavolat reverse() a set_active() na list
 */
void parse_instructions(tList *instr_list)  {
  tNode *act_instr;

  while(1) {

    if((act_instr = return_instruct(instr_list)) == NULL )  break;
    move_activity(instr_list);

    switch (act_instr->data.type) {
      case INSTRUCT_HEAD:
          printf(".IFJcode18\n");
          printf("JUMP $$main\n");
      break;

      case INSTRUCT_LABEL_MAIN:
          printf("LABEL $$main\n");
          printf("DEFVAR GF@$$var_integer\n");
          printf("MOVE GF@$$var_integer int@0\n");
          printf("DEFVAR GF@$$var_double\n");
          printf("MOVE GF@$$var_double float@0x0p+0\n");
          printf("DEFVAR GF@$$var_string\n");
          printf("MOVE GF@$$var_string string@\n");
          printf("DEFVAR GF@$$EXPR1\n");
          printf("MOVE GF@$$EXPR1 int@0\n");
          printf("DEFVAR GF@$$EXPR2\n");
          printf("MOVE GF@$$EXPR2 int@0\n");
          printf("DEFVAR GF@$result\n");
          printf("DEFVAR GF@$type\n");
          printf("DEFVAR GF@$tmp\n");
          printf("DEFVAR GF@$tmp2\n");
          printf("DEFVAR GF@$type1\n");
          printf("DEFVAR GF@$type2\n");
      break;

      case INSTRUCT_CREATEFREAME:
          printf("CREATEFRAME\n");
      break;

      case INSTRUCT_PUSHFRAME:
          printf("PUSHFRAME\n");
      break;

      case INSTRUCT_POPFRAME:
          printf("POPFRAME\n");
      break;

      case INSTRUCT_RETURN:
          printf("RETURN\n");
      break;

      case INSTRUCT_CLEARS:
          printf("CLEARS\n");
      break;

      case INSTRUCT_BREAK:
          printf("BREAK\n");
      break;

      case INSTRUCT_MOVE:
          printf("MOVE %s@",instruct_type(act_instr->data.address1.type));
          print_symb(act_instr->data.address1);
          printf(" %s@",instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_DEFVAR:
          printf("DEFVAR %s@%s\n", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s);
      break;

      case INSTRUCT_CALL:
          printf("CALL %s\n", act_instr->data.address1.value.s);
      break;

      case INSTRUCT_PUSHS:
          printf("PUSHS %s@", instruct_type(act_instr->data.address1.type));
          print_symb(act_instr->data.address1);
          printf("\n");
      break;

      case INSTRUCT_POPS:
          printf("POP %s@%s\n", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s);
      break;

      case INSTRUCT_ADD:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_SUB:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_MUL:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_DIV:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_IDIV:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_ADDS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_SUBS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_MULS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_DIVS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_IDIVS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_LT:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_GT:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_EQ:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_LTS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_GTS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_EQS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_AND:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_OR:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_NOT:
          printf("NOT %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_ANDS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_ORS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_NOTS:
          printf("NOTS %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_INT2FLOAT:
          printf("INT2FLOAT %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_FLOAT2INT:
          printf("FLOAT2INT %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_INT2CHAR:
          printf("INT2CHAR %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_STRI2INT:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_INT2FLOATS:
          printf("INT2FLOATS %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_FLOAT2INTS:
          printf("INT2FLOAT %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_INT2CHARS:
          printf("INT2CHARS %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_STRI2INTS:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_READ:
          printf("READ %s@%s %s\n", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
      break;

      case INSTRUCT_WRITE:
          printf("WRITE %s@", instruct_type(act_instr->data.address1.type));
          print_symb(act_instr->data.address1);
          printf("\n");
      break;

      case INSTRUCT_CONCAT:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_STRLEN:
          printf("STRLEN %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_GETCHAR:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_SETCHAR:
          print_arit_instr(act_instr);
      break;

      case INSTRUCT_TYPE:
          printf("TYPE %s@%s %s@", instruct_type(act_instr->data.address1.type), act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_symb(act_instr->data.address2);
          printf("\n");
      break;

      case INSTRUCT_LABEL:
          printf("LABEL %s\n", act_instr->data.address1.value.s);
      break;

      case INSTRUCT_JUMP:
          printf("JUMP %s\n", act_instr->data.address1.value.s);
      break;

      case INSTRUCT_JUMPIFEQ:
          printf("JUMPIFEQ %s %s@", act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_multiple_symb(act_instr->data.address2);
          printf("%s@", instruct_type(act_instr->data.address3.type));
          print_symb(act_instr->data.address3);
          printf("\n");
      break;

      case INSTRUCT_JUMPIFNEQ:
          printf("JUMPIFNEQ %s %s@", act_instr->data.address1.value.s, instruct_type(act_instr->data.address2.type));
          print_multiple_symb(act_instr->data.address2);
          printf("%s@", instruct_type(act_instr->data.address3.type));
          print_symb(act_instr->data.address3);
          printf("\n");

      break;

      case INSTRUCT_JUMPIFEQS:
          printf("JUMPIFEQS %s\n", act_instr->data.address1.value.s);
      break;

      case INSTRUCT_JUMPIFNEQS:
          printf("JUMPIFNEQS %s\n", act_instr->data.address1.value.s);
      break;

      case INSTRUCT_EXIT:
          printf("EXIT %s@", instruct_type(act_instr->data.address1.type));
          print_symb(act_instr->data.address1);
          printf("\n");
      break;

      case INSTRUCT_DPRINT:
          printf("DPRINT %s@", instruct_type(act_instr->data.address1.type));
          print_symb(act_instr->data.address1);
          printf("\n");
      break;

      case INSTRUCT_LENGTH:
          printf("LABEL length\n");
          printf("PUSHFRAME\n");
          printf("DEFVAR LF@%%retval\n");
          printf("MOVE LF@%%retval nil@nil\n");
          printf("STRLEN LF@%%retval LF@%%1\n");
          printf("POPFRAME\n");
          printf("RETURN\n");
      break;


      case INSTRUCT_INPUT_S:
           printf("READ LF@%s string\n", act_instr->data.address1.value.s);
           printf("WRITE %s@", instruct_type(act_instr->data.address1.type));
           printf("%s\n", act_instr->data.address1.value.s);
      break;
      case INSTRUCT_INPUT_I:
           printf("READ LF@%s int\n", act_instr->data.address1.value.s);
           printf("WRITE %s@", instruct_type(act_instr->data.address1.type));
           printf("%s\n", act_instr->data.address1.value.s);
      break;
      case INSTRUCT_INPUT_F:
           printf("READ LF@%s float\n", act_instr->data.address1.value.s);
           printf("WRITE %s@", instruct_type(act_instr->data.address1.type));
           printf("%s\n", act_instr->data.address1.value.s);
      break;

      case INSTRUCT_CHR:
          printf("LABEL chr\n");
          printf("PUSHFRAME\n");
          printf("DEFVAR LF@%%retval\n");
          printf("MOVE LF@%%retval nil@nil\n");
          printf("GT GF@$result LF@%%1 int@255\n");
          printf("JUMPIFEQ $chr_error4 GF@$result bool@true\n");
          printf("INT2CHAR  LF@%%retval LF@%%1\n");
          printf("JUMP $chr_end\n");
          printf("LABEL $chr_error4\n");
          printf("EXIT int@4\n");
          printf("LABEL $chr_end\n");
          printf("POPFRAME\n");
          printf("RETURN\n");
      break;

      case INSTRUCT_ORD:
          printf("LABEL ord\n");
          printf("PUSHFRAME\n");
          printf("DEFVAR LF@%%retval\n");
          printf("MOVE LF@%%retval nil@nil\n");
          printf("JUMPIFEQ $ord_end LF@%%2 int@0\n");

          printf("STRLEN GF@$$var_integer LF@%%1\n");
          printf("LT GF@$$var_double GF@$$var_integer LF@%%2\n");
          printf("JUMPIFEQ $ord_end bool@true GF@$$var_double\n");

          printf("SUB LF@%%2 LF@%%2 int@1\n");
          printf("GETCHAR GF@$$var_string LF@%%1 LF@%%2\n");
          printf("STRI2INT GF@$$var_integer GF@$$var_string int@0\n");
          printf("MOVE LF@%%retval GF@$$var_integer\n");
          printf("JUMP $ord_end\n");


          printf("LABEL $ord_end\n");
          printf("POPFRAME\n");
          printf("RETURN\n");

      break;


      case INSTRUCT_PRINT:
          // while pocet parametru ... zatim vypisu jen jeden parametr
          arg_count = act_instr->data.address1.value.i;
          for (int i=1; i <= arg_count; i++){
              printf("WRITE TF@%%");
              printf("%d\n", i);
          }
      break;

      case INSTRUCT_SUBSTR:
          printf("LABEL substr\n");
          printf("PUSHFRAME\n");
          printf("DEFVAR LF@%%retval\n");
          printf("MOVE LF@%%retval nil@nil\n");
          printf("MOVE GF@$result string@\n");
          printf("STRLEN GF@$$var_integer LF@%%1\n");

          printf("LT GF@$$var_double GF@$$var_integer LF@%%2\n"); // i > length(str)
          printf("JUMPIFEQ $substr_end bool@true GF@$$var_double\n");
          printf("LT GF@$$var_double  GF@$$var_integer LF@%%3\n");  // n > length(str)
          printf("JUMPIFEQ $substr_n_err bool@true GF@$$var_double\n");
          printf("EQ GF@$$var_double LF@%%2 int@0\n"); // if i == 0
          printf("JUMPIFEQ $substr_end bool@true GF@$$var_double\n");
          printf("LT GF@$$var_double LF@%%3 int@0\n"); // if n < 0
          printf("JUMPIFEQ $substr_end bool@true GF@$$var_double\n");
          printf("JUMP $label_substr_read\n");


          printf("LABEL $substr_n_err\n");
          //printf("STRLEN GF@$$var_integer LF@%%1\n");
          printf("EQ GF@$$var_double LF@%%2 GF@$$var_integer\n");
          printf("JUMPIFEQ $label_substr GF@$$var_double bool@true\n");

          printf("GETCHAR GF@$$var_string LF@%%1 LF@%%2\n");
          printf("CONCAT GF@$result GF@$result GF@$$var_string\n");
          printf("ADD LF@%%2 LF@%%2 int@1\n");  //n--
          printf("JUMP $substr_n_err\n");


          printf("LABEL $label_substr_read\n");
          printf("EQ GF@$$var_double LF@%%2 LF@%%3\n");
          printf("JUMPIFEQ $label_substr GF@$$var_double bool@true\n");
          printf("GETCHAR GF@$$var_string LF@%%1 LF@%%2\n");
          printf("CONCAT GF@$result GF@$result GF@$$var_string\n");
          printf("ADD LF@%%2 LF@%%2 int@1\n");  //n--
          printf("JUMP $label_substr_read\n");




          printf("LABEL $label_substr\n");
          printf("MOVE LF@%%retval GF@$result\n");
          printf("LABEL $substr_end\n");
          printf("POPFRAME\n");
          printf("RETURN\n");
      break;

      case INSTRUCT_WHILE_START:
          printf("LABEL while_label%d\n",++while_count);
      break;
      case INSTRUCT_WHILE_STATS:
          printf("JUMPIFEQ while_label%d_end GF@$result bool@false\n", while_count);
      break;

      case INSTRUCT_WHILE_END:
          printf("JUMP while_label%d\n", while_count);
          if (while_count == 1){
            printf("LABEL while_label1_end\n");
          }else {
            printf("LABEL while_label%d_end\n",while_count--);
          }
      break;

      case INSTRUCT_IF_THEN:
          printf("JUMPIFEQ if_label_then%d GF@$result bool@true\n", ++if_count);
          printf("JUMP if_label_else%d\n", if_count);
          printf("LABEL if_label_then%d\n", if_count);
      break;

      case INSTRUCT_IF_ELSE:
          printf("LABEL if_label_else%d\n", if_count);
      break;

      case INSTRUCT_JUMP_ENDIF:
          printf("JUMP if_label%d_end\n", if_count);
      break;

      case INSTRUCT_ENDIF:
          if (if_count == 1){
            printf("LABEL if_label1_end\n");
          }else {
            printf("LABEL if_label%d_end\n",if_count--);
          }
      break;


    }
  }
}

//
// Created by majkl on 5.10.2018.
//


#include "../src/stack.h"
#include "../src/parser.h"


void stackTestFce(){

    tStack* stack = stack_init(20);
    tDataIDF dataOfToken;
    dataOfToken.type = 501;
    dataOfToken.value.i = 1000;
    stack_push(stack, 10, dataOfToken);
    stack_pop(stack);

    stack_print_prece(stack);


    stack_free(stack);

}

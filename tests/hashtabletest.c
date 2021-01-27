/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   hashtabletest.c
 *
 * Popis: Testovanie hashovacej tabulky
 *
 * Datum: 22.9.2018 23:56
 *
 * Autori:   Maros Orsak       vedouci
 *           Polishchuk Kateryna     <xpolis03@fit.vutbr.cz>
 *           Igor Ignac
 *           Marek Rohel

*/

#include "../src/symtable.h"
#include <stdio.h>

/**
 * Procedura, ktora sluzi na testovanie funckionality interface s mapou
 */
void hashtableTestGlobal(){
    GlobalMap* globalMap = NULL;
    global_map_init(globalMap);
    globalMap = (GlobalMap*) malloc ( sizeof(GlobalMap) );
    for ( int i=0; i<MAX_SIZE_OF_HASH_TABLE; (*globalMap)[i++] = undefined_pointer_global );

    printf ("---------------------------\n");
    printf ("\nLet's set HTSIZE to 19 ( must be prvocislo )\n");
    global_map_print(globalMap);
    tDataFunction globalneData;

    /* pre globalnu budu data reprezentovat tIDF strukturu*/
    globalneData.defined = 0;
    globalneData.functionParametersNames.data = "int";
    globalneData.type = 501;
    globalneData.returnType = 504;

    global_map_put(globalMap, "IntegerValue10", globalneData);
    global_map_put(globalMap, "IntegerValue20", globalneData);
    global_map_put(globalMap, "IntegerValue30", globalneData);
    global_map_put(globalMap, "IntegerValue40", globalneData);

    global_map_print(globalMap);

    GlobalMapItem *mapItem = global_map_get_item(globalMap, "IntegerValue20");
    printf("Pointer on Map value -> %p \n", global_map_get_pointer_to_value(globalMap, "IntegerValue20")); // pointer na item
    printf("Pointer on Map Key -> %p \n", global_map_get_pointer_to_key(globalMap, "IntegerValue20")); // pointer na item
    printf("Map key -> %s \n", global_map_get_key(globalMap, "IntegerValue30")); // takto sa ziska danu stringovu hodnotu z kluca
    printf("Map value -> %d \n", global_map_get_value(globalMap, "IntegerValue30")); // takto sa ziska honodta z kluca
    printf("Key -> %s , Value -> %d \n", mapItem->key, mapItem->globalData);
    printf("Vysledna hodnota je -> %d", global_map_contain(globalMap, "IntegerVal20")); // FALSE -> 0
    printf("Vysledna hodnota je -> %d", global_map_contain(globalMap, "IntegerValue20")); // TRUE -> 1
    global_map_remove(globalMap, "IntegerValue20");
    global_map_print(globalMap);
    global_map_free(globalMap);
    global_map_print(globalMap);

}

void hashtableTestLocal(){
    LocalMap* localMap = NULL;
    local_map_init(localMap);
    localMap = (LocalMap*) malloc ( sizeof(LocalMap) );
    for ( int i=0; i<MAX_SIZE_OF_HASH_TABLE; (*localMap)[i++] = undefined_pointer_local );

    printf ("---------------------------\n");
    printf ("\nLet's set HTSIZE to 19 ( must be prvocislo )\n");
    local_map_print(localMap);

    tDataIDF localneData;

    /* pre globalnu budu data reprezentovat tIDF strukturu*/
    localneData.defined = 0;
    localneData.value.d = 20.4;
    localneData.type = 501;

    local_map_put(localMap, "IntegerValue10", localneData);
    local_map_put(localMap, "IntegerValue20", localneData);
    local_map_put(localMap, "IntegerValue30", localneData);
    local_map_put(localMap, "IntegerValue40", localneData);

    local_map_print(localMap);

    LocalMapItem *mapItem = local_map_get_item(localMap, "IntegerValue20");
    printf("Pointer on Map value -> %p \n", local_map_get_pointer_to_value(localMap, "IntegerValue20")); // pointer na item
    printf("Pointer on Map Key -> %p \n", local_map_get_pointer_to_key(localMap, "IntegerValue20")); // pointer na item
    printf("Map key -> %s \n", local_map_get_key(localMap, "IntegerValue30")); // takto sa ziska danu stringovu hodnotu z kluca
    printf("Map value -> %d \n", local_map_get_value(localMap, "IntegerValue30")); // takto sa ziska honodta z kluca
    printf("Key -> %s , Value -> %d \n", mapItem->key, mapItem->localData);
    printf("Vysledna hodnota je -> %d", local_map_contain(localMap, "IntegerVal20")); // FALSE -> 0
    printf("Vysledna hodnota je -> %d", local_map_contain(localMap, "IntegerValue20")); // TRUE -> 1
    local_map_remove(localMap, "IntegerValue20");
    local_map_print(localMap);
    local_map_free(localMap);
    local_map_print(localMap);

    // TODO: iba pre local a pre global
}


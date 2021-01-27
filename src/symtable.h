/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   hashtable.h
 *
 * Popis: Hlavickovy subor pre HashTable
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#ifndef  _HASHMAP_H_
#define  _HASHMAP_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "string.h"

#define MAX_SIZE_OF_HASH_TABLE 101 // cislo musi byt prvocislo

typedef char* String; // (toto bude kluc)

/** Enum pre vsetky typy  **/
typedef enum Types
{
    NONE     = 500, // neincializaova hodnota
    INTEGER  = 501,
    FLOAT    = 502,
    STRING   = 503,
    BOOLEAN  = 504,  // toto je pre rozsirenie
    FUNCTION = 505
} Types;

/** Union pre, konkretnu hodnotu datatypu napr -> int i = 10 (10 bude ulozene v i) **/
typedef union DataTypes
{
    int i;
    double f;
    tString string;
    bool nil;
} dType;

/** struktura pre data indentifikatorov (lokalna mapa) **/
typedef struct DataIdentificator { // (toto bude obsah pre localnu mapu)
    Types type;                     // typ identifikatora
    dType value;                    // hodnota identifikatora
    bool defined;                   // zda bola premmena definovana (ale taktiez to zistime tak ak bude nebude value
    char* nameOfTheNonTerminal;     // nazov nonterminalu $1, $240 atd.
    char* nameOfTheVariable;
    bool isVariable;        // pomocna premenna ktora nam presne povie zda je dany token premenna alebo nie
} tDataIDF;

/*Datová položka TRP s explicitně řetězenými synonymy pre Localnu mapu*/
typedef struct LocalMapItem{
    String key;                 // kluc
    tDataIDF localData;         // obsah pre localnu mapu
    struct LocalMapItem* ptrnext;   // ukazatel na dalsi synonym
} LocalMapItem;

typedef struct LocalMap{
    unsigned int size;
    struct LocalMapItem **list;
}LocalMap;


/** struktura pre data funkcie (globalna mapa) **/
typedef struct DataFunction {
    Types type;                     // typ parametrov
    Types returnType;               // navratova hodnota
    bool defined;                   // zda bola funckcia definovana
    int paramCount;                 // pocet parametrov vo funckii
    LocalMap* lMap;
} tDataFunction;

/*Datová položka TRP s explicitně řetězenými synonymy pre Globalnu mapu*/
typedef struct GlobalMapItem{
    String key;                 // kluc
    tDataFunction globalData;   // obsah pre globalnu mapu
    struct GlobalMapItem* ptrnext;  // ukazatel na dalsi synonym
} GlobalMapItem;

typedef struct GlobalMap{
    int size;
    struct GlobalMapItem **list;
}GlobalMap;

// popis k proceduram / funckiam -> programova dokumentacia ak pouzivate Clion (staci ku funckii a pouzit CTLR + q)
/** VSEOBECNE FUNCKIE **/
int hashcode ( String key );

/** FUNKCIE S LOCALNOU MAPOU **/

LocalMap* local_map_init (unsigned int size);
LocalMapItem* local_map_get_item ( LocalMap* ptrMap, String key );
tDataIDF* local_map_get_pointer_to_value ( LocalMap* ptrMap, String key );
tDataIDF local_map_get_value(LocalMap* ptrMap, String key);
String* local_map_get_pointer_to_key ( LocalMap* ptrMap, String key );
String local_map_get_key(LocalMap* ptrMap, String key);
bool local_map_contain(LocalMap* ptrMap, String key);
void local_map_remove ( LocalMap* ptrMap, String key );
void local_map_free ( LocalMap* ptrMap );
void local_map_print( LocalMap* ptrMap );
void local_map_put ( LocalMap* ptrMap, String key, tDataIDF data );

/** FUNKCIE S GLOBALNOU MAPOU **/
GlobalMap* global_map_init (unsigned int size);
GlobalMapItem* global_map_get_item ( GlobalMap* ptrMap, String key );
tDataFunction* global_map_get_pointer_to_value ( GlobalMap* ptrMap, String key );
tDataFunction global_map_get_value(GlobalMap* ptrMap, String key);
String* global_map_get_pointer_to_key ( GlobalMap* ptrMap, String key );
String global_map_get_key(GlobalMap* ptrMap, String key);
bool global_map_contain(GlobalMap* ptrMap, String key);
void global_map_remove ( GlobalMap* ptrMap, String key );
void global_map_free ( GlobalMap* ptrMap );
void global_map_print( GlobalMap* ptrMap );
void global_map_put ( GlobalMap* ptrMap, String key, tDataFunction data );

// Testovacie funckie
void hashtableTestGlobal();
void hashtableTestLocal();
#define  _HASHMAP_H_

#endif //IFJPROJ_HASHTABLE_H

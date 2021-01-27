/**
 * Predmet:  IFJ
 * Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 * Soubor:   hashtable.c
 *
 * Popis:  Hashovacia tabulka implementovana s explicitne retazenymi synonymami
 *
 * Autori:   Maros Orsak            	xorsak02@stud.fit.vutbr.cz
 *           Polishchuk Kateryna     	xpolis03@stud.fit.vutbr.cz
 *           Igor Ignac                 xignac00@stud.fit.vutbr.cz
 *           Marek Rohel            	xrohel01@stud.fit.vutbr.cz
*/

#include <stdio.h>
#include "symtable.h"

/**
 * Hashovacia funckia, moznost vylepsenia...
 * @param key dany kluc
 * @return hashovaciu hodnotu
 */

int hashcode ( String key ) {
    int retval = 1;
    int keylen = strlen(key);
    for ( int i=0; i < keylen; i++ ){
        retval += key[i];
    }
    return ( retval % MAX_SIZE_OF_HASH_TABLE );
}

/************ FUNCKIE PRE PRACU S LOCALNOU MAPOU *****************/

/**
 * Inicializacia tabulky
 * @param ptrMap pointer na mapu
 */

LocalMap* local_map_init (unsigned int size) {
    LocalMap *ptrMap = (LocalMap*)malloc(sizeof(LocalMap));
    ptrMap->size = size;
    ptrMap->list = malloc(sizeof(LocalMapItem*)*size);
    unsigned int i;
    for(i=0;i<size;i++)
        ptrMap->list[i] = NULL;
    return ptrMap;
}

/**
 * Funkcia, ktora nam vrati pointer na danu polozku z mapy
 * @param ptrMap pointer na mapu
 * @param key hladany kluc
 * @return item z mapy
 */

LocalMapItem* local_map_get_item ( LocalMap* ptrMap, String key ) {
    if (ptrMap != NULL){        // ak bude existovat ukazatel na tabulku
        LocalMapItem *item = (ptrMap->list)[hashcode(key)]; // pomocna premmenna (polozka v tabulke)
        while (item != NULL){
            if (strcmp(item->key , key) != 0){  // porovnanie s hladanym klucom
                item = item->ptrnext;
            } else{
                return  item;                   //  vracia sa najdeny item
            }
        }
        // prvok nebol najdeny
        return NULL;
    } else{
        // ukazatel na tabulku neexistuje
        return NULL;
    }
}

/**
 * Procedura, ktora vklada do mapy kluc a hodnotu. Pokial sa do mapy zapise pod ten isty key ina hondota tak sa aktualizuje na novu.
 * @param ptrMap pointer na Mapu
 * @param key dany kluc, ktory bude vkladany
 * @param data dane data, ktore budu vkladane
 */

void local_map_put ( LocalMap* ptrMap, String key, tDataIDF data ) {
    int position = hashcode(key);
    LocalMapItem *list = ptrMap->list[position];
    LocalMapItem *temp = list;
    while(temp){
        if(strcmp(temp->key, key) == 0){
            temp->localData = data;
            return;
        }
        temp = temp->ptrnext;
    }
    LocalMapItem* localMapItem = (LocalMapItem*)malloc(sizeof(LocalMapItem));
    localMapItem->key = key;
    localMapItem->localData = data;
    localMapItem->ptrnext = list;
    ptrMap->list[position] = localMapItem;
}

/**
 * Funkcia, ktora nam vrati ointer na hodnotu, ktoru zistkame podla kluca, ak polozka nebude najdena tak sa vracia NULL
 * @param ptrMap Pointer na mapu
 * @param key kluc v mape
 * @return pointer na hodnotu
 */
tDataIDF* local_map_get_pointer_to_value ( LocalMap* ptrMap, String key ) {
    if (ptrMap != NULL){                            // ak existuje
        LocalMapItem *item =  local_map_get_item(ptrMap, key);  //hladame polozku
        if (item != NULL){
            return &item->localData;            // polozka bola najdena (vracia ukazatel na polozku)
        }
        else {
            return NULL;                        // polozka nebola najdena
        }
    }
    return NULL;                            // neexistuje
}

/**
 * Funckia, ktora nam vrati hodnotu, podla kluca v danej mape
 * @param ptrMap Pointer na Mapu
 * @param key Kluc, podla ktoreho sa bude hladat
 * @return hodnotu kluca
 */

tDataIDF local_map_get_value(LocalMap* ptrMap, String key){
    return *(local_map_get_pointer_to_value(ptrMap,key));
}

/**
 * Funckia, ktora nam vrati pointer stringoveho kluca
 * @param ptrMap Pointer na mapu
 * @param key Dany kluc
 * @return ukazatel na polozku
 */

String* local_map_get_pointer_to_key ( LocalMap* ptrMap, String key ) {
    if (ptrMap != NULL){                            // ak existuje
        LocalMapItem *item = local_map_get_item(ptrMap, key);   //hladame polozku
        if (item != NULL){
            return &(item->key);            // polozka bola najdena (vracia ukazatel na polozku)
        }
        else {
            return NULL;                        // polozka nebola najdena
        }
    }
    return NULL;                            // neexistuje
}

/**
 * Funkcia, ktora nam vrati hodnotu stringoveho kluca
 * @param ptrMap Pointer na Mapu
 * @param key Kluc, podla ktoreho sa bude hladat
 * @return  hodnotu stringoveho  kluca
 */
String local_map_get_key(LocalMap* ptrMap, String key){
    return *(local_map_get_pointer_to_key(ptrMap, key));
}

/**
 * Funckia, ktora zisti zda sa dany item nachadza v mape a vracia prislusnu hodnotu
 * @param ptrMap pointer na mapu
 * @param key hodnota hladaneho kluca
 * @return v pripade najdenia -> true inak false
 */
bool local_map_contain(LocalMap* ptrMap, String key){
    if (ptrMap != NULL){        // ak bude existovat ukazatel na tabulku
        LocalMapItem *item = (ptrMap->list)[hashcode(key)]; // pomocna premmenna (polozka v tabulke)
        while (item != NULL){
            if (strcmp(item->key , key) != 0){  // porovnanie s hladanym klucom
                item = item->ptrnext;
            } else{
                return true;                    //  vracia sa najdeny item
            }
        }
        // prvok nebol najdeny
        return false;
    } else{
        // ukazatel na tabulku neexistuje
        return false;
    }
}

/**
 * Procedura zoberie polozku s danym klucom z tabulky, na to uvolni danu polozku. Pokial polozka dana neexistuje tak sa nespravi nic
 * @param ptrMap Pointer na Mapu
 * @param key Dany kluc
 */

void local_map_remove ( LocalMap* ptrMap, String key ) {
    if (ptrMap != NULL){        // ak bude existovat ukazatel na tabulku
        LocalMapItem *item = (ptrMap->list)[hashcode(key)];         // polozka v tabulke
        LocalMapItem *tmp = NULL;
        while (item != NULL){
            if (strcmp(item->key , key) != 0){          // porovanie kluca s hladanym
                tmp = item;
                item = item->ptrnext;                   // premiestnenie sa na dalsi item pretoze aktualny nie je ten ktory chceme
            } else{
                if (tmp == NULL){                       // kluc sa nasiel
                    (ptrMap->list)[hashcode(key)] = item->ptrnext;
                } else{
                    tmp->ptrnext = item->ptrnext;
                }
                free(item);             //uvolnenie polozky
                item = NULL;
            }
        }
        return;         // prvok s danym klucom nebol najdeny
    } else{             // ukazatel na tabulku neexistuje

        return;
    }
}

/**
 * Procedura zrusi vsetky polozky a uvolni pamat, hned na to uvedie tabulku do poc. stavu
 * @param ptrMap pointer na Mapu
 */

void local_map_free ( LocalMap* ptrMap ) {
    if (ptrMap != NULL){// ak bude existovat ukazatel na tabulku
        int i = 0;                              // indexovac nastavneny
        while (i < MAX_SIZE_OF_HASH_TABLE){                     // cyklus pojde po max pola
            LocalMapItem *item = (ptrMap->list)[i];         // polozky tabulky
            LocalMapItem *tmp;                      // pomocna aby sa nestratil ukazatel po uvolneni
            while (item != NULL){
                tmp = item;
                item = item->ptrnext;           // skok na dalsi prvok
                free(tmp);
            }
            (ptrMap->list)[i] = NULL;
            i++;
        }
        free(ptrMap->list);
        free(ptrMap);
    }
}

/**
 * Ukaze cely obsah obsah tabulky (Pridane z IAL iba pre kontrolu)
 * @param ptrMap pointer na Mapu
 */
void local_map_print( LocalMap* ptrMap ) {
    int maxlen = 0;
    int sumcnt = 0;

    printf ("------------HASH TABLE--------------\n");
    for (unsigned int i=0; i<ptrMap->size; i++ ) {
        printf ("%i:",i);
        int cnt = 0;
        LocalMapItem* ptr = (ptrMap->list[i]);
        while ( ptr != NULL ) {
            printf (" (%s, %d)",ptr->key,ptr->localData.type);
            cnt++;
            ptr = ptr->ptrnext;
        }
        printf ("\n");

        if (cnt > maxlen)
            maxlen = cnt;
        sumcnt+=cnt;
    }

    printf ("------------------------------------\n");
    printf ("Items count %i   The longest list %i\n",sumcnt,maxlen);
    printf ("------------------------------------\n");
}

/************************ KONIEC FUNKCII PRE LOCALNU MAPU ****************************/



/************ FUNCKIE PRE PRACU S GLOBALNOU MAPOU *****************/

/**
 * Inicializacia tabulky
 * @param ptrMap pointer na mapu
 */

GlobalMap* global_map_init (unsigned int size) {
    GlobalMap *ptrMap = (GlobalMap*)malloc(sizeof(GlobalMap));
    ptrMap->size = size;
    ptrMap->list = malloc(sizeof(GlobalMapItem*)*size);
    unsigned int i;
    for(i=0;i<size;i++)
        ptrMap->list[i] = NULL;

    return ptrMap;
}

/**
 * Funkcia, ktora nam vrati pointer na danu polozku z mapy
 * @param ptrMap pointer na mapu
 * @param key hladany kluc
 * @return item z mapy
 */

GlobalMapItem* global_map_get_item ( GlobalMap* ptrMap, String key ) {
    if (ptrMap != NULL){        // ak bude existovat ukazatel na tabulku
        GlobalMapItem *item = (ptrMap->list)[hashcode(key)];    // pomocna premmenna (polozka v tabulke)
        while (item != NULL){
            if (strcmp(item->key , key) != 0){  // porovnanie s hladanym klucom
                item = item->ptrnext;
            } else{
                return item;                    //  vracia sa najdeny item
            }
        }
        // prvok nebol najdeny
        return NULL;
    } else{
        // ukazatel na tabulku neexistuje
        return NULL;
    }
}

/**
 * Procedura, ktora vklada do mapy kluc a hodnotu. Pokial sa do mapy zapise pod ten isty key ina hondota tak sa aktualizuje na novu.
 * @param ptrMap pointer na Mapu
 * @param key dany kluc, ktory bude vkladany
 * @param data dane data, ktore budu vkladane
 */

void global_map_put ( GlobalMap* ptrMap, String key, tDataFunction data ) {
    int position = hashcode(key);
    GlobalMapItem *list = ptrMap->list[position];
    GlobalMapItem *temp = list;
    while(temp){
        if(strcmp(temp->key, key) == 0){
            temp->globalData = data;
            return;
        }
        temp = temp->ptrnext;
    }

    GlobalMapItem* globalMapItem = (GlobalMapItem*)malloc(sizeof(GlobalMapItem));
    globalMapItem->key = key;
    globalMapItem->globalData = data;
    globalMapItem->ptrnext = list;
    ptrMap->list[position] = globalMapItem;
}

/**
 * Funkcia, ktora nam vrati ointer na hodnotu, ktoru zistkame podla kluca, ak polozka nebude najdena tak sa vracia NULL
 * @param ptrMap Pointer na mapu
 * @param key kluc v mape
 * @return pointer na hodnotu
 */
tDataFunction* global_map_get_pointer_to_value ( GlobalMap* ptrMap, String key ) {
    if (ptrMap != NULL){                            // ak existuje
        GlobalMapItem *item = global_map_get_item(ptrMap, key); //hladame polozku
        if (item != NULL){
            return &item->globalData;           // polozka bola najdena (vracia ukazatel na polozku)
        }
        else {
            return NULL;                        // polozka nebola najdena
        }
    }
    return NULL;                            // neexistuje
}

/**
 * Funckia, ktora nam vrati hodnotu, podla kluca v danej mape
 * @param ptrMap Pointer na Mapu
 * @param key Kluc, podla ktoreho sa bude hladat
 * @return hodnotu kluca
 */

tDataFunction global_map_get_value(GlobalMap* ptrMap, String key){
    return *(global_map_get_pointer_to_value(ptrMap,key));
}

/**
 * Funckia, ktora nam vrati pointer stringoveho kluca
 * @param ptrMap Pointer na mapu
 * @param key Dany kluc
 * @return ukazatel na polozku
 */

String* global_map_get_pointer_to_key ( GlobalMap* ptrMap, String key ) {
    if (ptrMap != NULL){                            // ak existuje
        GlobalMapItem *item = global_map_get_item(ptrMap, key); //hladame polozku
        if (item != NULL){
            return &(item->key);            // polozka bola najdena (vracia ukazatel na polozku)
        }
        else {
            return NULL;                        // polozka nebola najdena
        }
    }
    return NULL;                            // neexistuje
}

/**
 * Funkcia, ktora nam vrati hodnotu stringoveho kluca
 * @param ptrMap Pointer na Mapu
 * @param key Kluc, podla ktoreho sa bude hladat
 * @return  hodnotu stringoveho  kluca
 */
String global_map_get_key(GlobalMap* ptrMap, String key){
    return *(global_map_get_pointer_to_key(ptrMap, key));
}

/**
 * Funckia, ktora zisti zda sa dany item nachadza v mape a vracia prislusnu hodnotu
 * @param ptrMap pointer na mapu
 * @param key hodnota hladaneho kluca
 * @return v pripade najdenia -> true inak false
 */
bool global_map_contain(GlobalMap* ptrMap, String key){
    if (ptrMap != NULL){        // ak bude existovat ukazatel na tabulku
        GlobalMapItem *item = (ptrMap->list)[hashcode(key)];    // pomocna premmenna (polozka v tabulke)
        while (item != NULL){
            if (strcmp(item->key , key) != 0){  // porovnanie s hladanym klucom
                item = item->ptrnext;
            } else{
                return true;                    //  vracia sa najdeny item
            }
        }
        // prvok nebol najdeny
        return false;
    } else{
        // ukazatel na tabulku neexistuje
        return false;
    }
}

/**
 * Procedura zoberie polozku s danym klucom z tabulky, na to uvolni danu polozku. Pokial polozka dana neexistuje tak sa nespravi nic
 * @param ptrMap Pointer na Mapu
 * @param key Dany kluc
 */

void global_map_remove ( GlobalMap* ptrMap, String key ) {
    if (ptrMap != NULL){        // ak bude existovat ukazatel na tabulku
        GlobalMapItem *item = (ptrMap->list)[hashcode(key)];        // polozka v tabulke
        GlobalMapItem *tmp = NULL;
        while (item != NULL){
            if (strcmp(item->key , key) != 0){          // porovanie kluca s hladanym
                tmp = item;
                item = item->ptrnext;                   // premiestnenie sa na dalsi item pretoze aktualny nie je ten ktory chceme
            } else{
                if (tmp == NULL){                       // kluc sa nasiel
                    (ptrMap->list)[hashcode(key)] = item->ptrnext;
                } else{
                    tmp->ptrnext = item->ptrnext;
                }
                free(item);             //uvolnenie polozky
                item = NULL;
            }
        }
        return;         // prvok s danym klucom nebol najdeny
    } else{             // ukazatel na tabulku neexistuje

        return;
    }
}

/**
 * Procedura zrusi vsetky polozky a uvolni pamat, taktiez uvolni pamat listu a celej mapy
 * @param ptrMap pointer na Mapu
 */

void global_map_free ( GlobalMap* ptrMap ) {
    if (ptrMap != NULL){// ak bude existovat ukazatel na tabulku
        int i = 0;                              // indexovac nastavneny
        while (i < MAX_SIZE_OF_HASH_TABLE){                     // cyklus pojde po max pola
            GlobalMapItem *item = (ptrMap->list)[i];        // polozky tabulky
            GlobalMapItem *tmp;                     // pomocna aby sa nestratil ukazatel po uvolneni
            while (item != NULL){
                tmp = item;
                item = item->ptrnext;           // skok na dalsi prvok
                free(tmp);
            }
            (ptrMap->list)[i] = NULL;
            i++;
        }
        free(ptrMap->list);
        free(ptrMap);
    }
}

/**
 * Ukaze cely obsah obsah tabulky (Pridane z IAL iba pre kontrolu)
 * @param ptrMap pointer na Mapu
 */
void global_map_print( GlobalMap* ptrMap ) {
    int maxlen = 0;
    int sumcnt = 0;

    printf("------------HASH TABLE--------------\n");
    for (unsigned int i = 0; i <= sizeof(ptrMap); i++) {
        printf("%i:", i);
        int cnt = 0;
        GlobalMapItem *ptr = (ptrMap->list[i]);

        while (ptr != NULL) {
            printf(" (%s, %d)", ptr->key, ptr->globalData.paramCount);
            cnt++;
            ptr = ptr->ptrnext;
        }
        printf("\n");

        if (cnt > maxlen)
            maxlen = cnt;
        sumcnt += cnt;
    }

    printf("------------------------------------\n");
    printf("Items count %i   The longest list %i\n", sumcnt, maxlen);
    printf("------------------------------------\n");
    /************************ KONIEC FUNKCII PRE GLOBALNU MAPU ****************************/
}
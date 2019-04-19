
/**
 * @brief       For now this is an hackish adaptation on top of liblc,
 *              will be replaced by a proper implementation later.
 */

#include <stdio.h>
#include <stdlib.h>

#include "swif_full_symbol.h" 
#include "swif_symbol.h"

#include <assert.h>
/*---------------------------------------------------------------------------*/

#define COEF_POS_NONE 0xffffu


struct s_swif_full_symbol_t {
    //coded_packet_t* coded_packet;
    //uint32_t symbol_size;
    uint8_t* coef;
    symbol_id_t first_id; // coef[0] is the coefficient of first symbol_id
    symbol_id_t last_id; // included

    symbol_id_t first_nonzero_id;
    symbol_id_t last_nonzero_id;    
    
    uint8_t* data;
    uint32_t data_size;
};

/*---------------------------------------------------------------------------*/

struct s_swif_full_symbol_set_t {
    
};

/*---------------------------------------------------------------------------*/


/**
 * @brief Create a full_symbol set, that will be used to do gaussian elimination
 */
swif_full_symbol_set_t *full_symbol_set_alloc()
{
    return NULL;
}

/**
 * @brief Free a full_symbol set
 */
void full_symbol_set_free(swif_full_symbol_set_t *set)
{
    //memset(&set->packet_set, 0, sizeof(packet_set_t));
    //free(set);
}


/**
 * @brief Add a full_symbol to a packet set.
 * 
 * Gaussian elimination can occur.
 * Return the pivot associated to the new full_symbol 
 * or SYMBOL_ID_NONE if dependent (e.g. redundant) packet
 * 
 * The full_symbol is not freed and also reference is not captured.
 */
uint32_t swif_full_symbol_set_add
(swif_full_symbol_set_t* set, swif_full_symbol_t* full_symbol)
{
    return SYMBOL_ID_NONE;
}

/*---------------------------------------------------------------------------*/

/**
 * @brief Create a full_symbol from a raw packet (a set of bytes)
 *        and initialize it with content '0'
 */
swif_full_symbol_t *full_symbol_alloc
(symbol_id_t first_symbol_id, symbol_id_t last_symbol_id, uint32_t symbol_size) // data_size == symbol_size
{ 
    symbol_id_t symbol_id_size;
    if (first_symbol_id == SYMBOL_ID_NONE) {
        symbol_id_size=0;
    } else {
        assert(first_symbol_id <= last_symbol_id);
        symbol_id_size = last_symbol_id-first_symbol_id+1;

    }


    swif_full_symbol_t *result
        = (swif_full_symbol_t *)calloc(1, sizeof(swif_full_symbol_t));
    if (result == NULL) {
        return NULL;
    }
  
    // alloc de coef et data
    swif_full_symbol_t *coef
        = (swif_full_symbol_t *)calloc(symbol_id_size , sizeof(uint8_t));
    if (coef == NULL) {
        // deallouer en cas de pb
        free(result);
        return NULL; 
    }


     swif_full_symbol_t *data
        = (swif_full_symbol_t *)calloc(symbol_size, sizeof(uint8_t));
    if (data == NULL) {
        // deallouer en cas de pb
        free(result->coef);
        free(result);
        return NULL;

    }

    result->first_id = first_symbol_id;
    result->last_id = last_symbol_id;
    full_symbol_adjust_min_max_coef(result);

    return result;

}


/**
 * @brief get the coefficient corresponding to the specified symbol identifier
 */
uint8_t full_symbol_get_coef
(swif_full_symbol_t *full_symbol, uint32_t symbol_id)
{

    if (full_symbol_includes_id(full_symbol , symbol_id)){
        return full_symbol->coef[symbol_id-full_symbol->first_id];
    }
    return 0; 
}


symbol_id_t full_symbol_get_coef_index
(swif_full_symbol_t *full_symbol, uint32_t symbol_id)
{
    assert(full_symbol_includes_id(full_symbol , symbol_id));
    return symbol_id-full_symbol->first_id;
    
}

/**
 * @brief Create a full_symbol from a raw packet (a set of bytes)
 *        and initialize it with content '0'
 */
swif_full_symbol_t *full_symbol_create_from_source
(uint32_t symbol_id, uint8_t *symbol_data, uint32_t symbol_size)
{
    swif_full_symbol_t *full_symbol = full_symbol_alloc(symbol_id_t symbol_id, symbol_id_t symbol_id, symbol_size);
    symbol_id_t coef_index = full_symbol_get_coef_index( full_symbol, symbol_id);
    full_symbol->coef[coef_index]=1;
    return full_symbol;
}

swif_full_symbol_t *full_symbol_create_internal
(uint8_t* symbol_coef_table, uint32_t min_symbol_id, uint32_t nb_symbol_id,
 uint8_t* symbol_data, uint32_t symbol_size)
{
    swif_full_symbol_t *full_symbol = full_symbol_alloc( min_symbol_id,  min_symbol_id+nb_symbol_id-1 , symbol_size);
    full_symbol->coef = symbol_coef_table;
    full_symbol->first_id = min_symbol_id;
    full_symbol->last_id = min_symbol_id+nb_symbol_id-1;
    full_symbol_adjust_min_max_coef(full_symbol);
    full_symbol->data = symbol_data;
    full_symbol->data_size = symbol_size;

    return full_symbol;
}

/**
 * @brief Release a full_symbol
 */
void full_symbol_free(swif_full_symbol_t* full_symbol)
{
    assert(full_symbol != NULL);
    assert(full_symbol->coef != NULL);
    free(full_symbol->coef);
    full_symbol->coef = NULL;
    assert(full_symbol->data != NULL);
    free(full_symbol->data);
    full_symbol->data = NULL;
    free(full_symbol);
}

/**
 * @brief Create a new (unlinked) copy of a full_symbol
 */
swif_full_symbol_t *full_symbol_clone(swif_full_symbol_t* full_symbol)
{
    //allouer et copier
    swif_full_symbol_t *result = full_symbol_alloc( full_symbol->first_nonzero_id,  full_symbol->last_nonzero_id , full_symbol->data_size);
    memcpy(result->coef, full_symbol->coef, (full_symbol_count_coef(full_symbol)) * sizeof(uint8_t));
    result->first_id = full_symbol->first_id;
    result->last_id = full_symbol->last_id;
    result->first_nonzero_id = full_symbol->first_nonzero_id;
    result->last_nonzero_id = full_symbol->last_nonzero_id;
    result->data_size = full_symbol->data_size;
    memcpy(result->data, full_symbol->data, full_symbol->data_size * sizeof(uint8_t));
    return result;
}

/**
 * @brief get the size of the data
 */
uint32_t full_symbol_get_size(swif_full_symbol_t *full_symbol)
{
    return full_symbol->data_size; 
}




static inline bool full_symbol_has_sufficient_size(swif_full_symbol_t* symbol,
                                           symbol_id_t id1, symbol_id_t id2)
{ 
    assert(id1 <= id2);
    symbol_id_t symbol_id_size = full_symbol_count_coef(symbol);
    symbol_id_t requested_size = id2 - id1 + 1;
    if (symbol_id_size >= requested_size){
        /////////////////////////////////// XXX 
        id1=symbol->first_id;
        id2=id2-id1;
        return true; 
    }
    else{
        return false; 
    }
    
}

static inline bool full_symbol_includes_id(swif_full_symbol_t* symbol,
                                           symbol_id_t symbol_id)
{
    return (symbol_id >= symbol->first_id && symbol_id <= symbol->last_id );
}

// adjust full symbol min coef


static bool full_symbol_adjust_min_coef(swif_full_symbol_t* symbol)
{
    REQUIRE( symbol->first_id != COEF_POS_NONE
	    && symbol->last_id != COEF_POS_NONE );

  ///////////////////////
    bool result = true;
    for (symbol_id_t  i=symbol->first_id ;i<=symbol->last_id; i++) {
        if (symbol->coef[i] != 0){
            symbol->first_nonzero_id = symbol->coef[i];
            break;
        }
        else {
            result = false ;
        }
    }
  return result;
}



// adjust full symbol max coef

static bool full_symbol_adjust_max_coef(swif_full_symbol_t* symbol)
{
    REQUIRE( symbol->first_id != COEF_POS_NONE
	    && symbol->last_id != COEF_POS_NONE );

  
    bool result = true;
    for (symbol_id_t  i=symbol->last_id ;i<=symbol->first_id; i--) {
        if (symbol->coef[i] != 0){
            symbol->last_nonzero_id = symbol->coef[i];
            break;
        }
        else {
            result = false ;
        }
    }
    return result;
}




// adjust full symbol min and max coef
bool full_symbol_adjust_min_max_coef(swif_full_symbol_t* symbol)
{
  bool result = true;
  if (symbol->first_id == COEF_POS_NONE) {
    ASSERT( symbol->last_id == COEF_POS_NONE );
    result = false;
  }
  
  if (result)
    result = full_symbol__adjust_min_coef(symbol);
  if (result)
    result = full_symbol__adjust_max_coef(symbol);
  return result;
}



/**
 * @brief get the minimum source index that appears in the symbol
 *        SYMBOL_ID_NONE if there is none (e.g. symbol is 0)
 */
uint32_t full_symbol_get_min_symbol_id(swif_full_symbol_t *full_symbol)
{
    full_symbol_adjust_min_max_coef(full_symbol);
    return full_symbol->first_nonzero_id;
        
}

/**
 * @brief get the maximum source index that appears in the symbol
 *        returns SYMBOL_ID_NONE if there is none (e.g. symbol is 0)
 */
uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol)
{
    full_symbol_adjust_min_max_coef(full_symbol);
    return full_symbol->last_nonzero_id;
    
}


/**
 * @brief get the symbol 'data'. result_data should be a pointer to a
 *        a block of memory of full_symbol_get_size(full_symbol)
 */
void full_symbol_get_data
(swif_full_symbol_t *full_symbol, uint8_t *result_data)
{
    assert( result_data != NULL);
    //////////
    memcpy();
    uint32_t i ; 
    for (i=0; i<full_symbol->data_size; i++)
        result_data[i] = full_symbol->data[i];
}

void full_symbol_dump(swif_full_symbol_t *full_symbol, FILE *out)
{
    fprintf(out, "{'size':%u, 'internal':", full_symbol->data_size);
    fprintf(out, "{ 'type':'full_symbol'");
    fprintf(out, ", 'nb_coef':%u", full_symbol_count_coef(full_symbol));
    fprintf(out, ", 'data_size': %u", full_symbol->data_size);
    fprintf(out, ", 'first_nonzero_id':%u, 'last_nonzero_id':%u", 
	        full_symbol->first_nonzero_id, full_symbol->last_nonzero_id);
    fprintf(out, ", 'coef_value':[");
    uint32_t i;
    if ( full_symbol->first_id != COEF_POS_NONE) {
        for (i=full_symbol->first_id; i<=full_symbol->last_id; i++) {
            if (i > full_symbol->first_id)
                fprintf(out, ", ");
        fprintf(out, "%u", full_symbol_get_coef(full_symbol,i));
        }
    }
    fprintf(out, "]");
    fprintf(out, ", 'data':");
    fprintf(out, "'");
    for (i=0; i<full_symbol->data_size; i++)
        fprintf(out, "\\x%02x", full_symbol->data[i]);
    fprintf(out, "'");
    fprintf(out, " }");
    fprintf(out,"\n");
    fprintf(out, "}");  
}

/*---------------------------------------------------------------------------*/

/**
 * @brief Take a symbol and add another symbol multiplied by a 
 *        coefficient, e.g. performs the equivalent of: p1 += coef * p2
 * @param[in,out] p1     First symbol (to which coef*p2 will be added)
 * @param[in]     coef2  Coefficient by which the second packet is multiplied
 * @param[in]     p2     Second symbol
 */
void full_symbol_add_scaled
(void *symbol1, uint8_t coereef, void *symbol2, uint32_t symbol_size);


/**
 * @brief Take a symbol and add another symbol to it, e.g. performs the equivalent of: p3 = p1 + p2
 * @param[in] p1     First symbol (to which p2 will be added)
 * @param[in] p2     Second symbol
 * @param[in] p3     result XXX
 */
void full_symbol_add_base(swif_full_symbol_t *symbol1, swif_full_symbol_t *symbol2, swif_full_symbol_t *symbol_result)
{
    // assert (symbol1->data != NULL && symbol2->data != NULL .... )
    assert (symbol1->data != NULL && symbol2->data != NULL && symbol_result->data != NULL);
    // assert (symbol_result->data_size >= symbol2->data_size);
    assert (symbol_result->data_size >= symbol1->data_size && symbol_result->data_size >= symbol2->data_size);

    // assert (symbol2->first_nonzero_id -> inclus dans l'entete symbol_result);
    // assert (symbol2->last_nonzero_id -> inclus dans l'entete symbol_result);
    assert (full_symbol_includes_id(symbol_result, symbol1->first_nonzero_id));
    assert (full_symbol_includes_id(symbol_result, symbol1->last_nonzero_id));
    assert (full_symbol_includes_id(symbol_result, symbol2->first_nonzero_id));
    assert (full_symbol_includes_id(symbol_result, symbol2->last_nonzero_id));

///////////////////////////////// 
    for (uint32_t i = 0; i <= full_symbol_count_coef(symbol_result); i++){
        if ( full_symbol_get_coef(symbol1, symbol1->coef[i]) && !full_symbol_get_coef(symbol2, symbol2->coef[i]) )
            symbol_result->coef[i] = full_symbol_get_coef(symbol1, symbol1->coef[i]);

        else if ( !full_symbol_get_coef(symbol1, symbol1->coef[i]) && full_symbol_get_coef(symbol2, symbol2->coef[i]) )
            symbol_result->coef[i] = full_symbol_get_coef(symbol2, symbol2->coef[i]);

        else 
            symbol_result->coef[i] = full_symbol_get_coef(symbol1, symbol1->coef[i]) ^ full_symbol_get_coef(symbol2, symbol2->coef[i]);
    }

    for (uint32_t i = 0; i <= symbol_result->data_size; i++)
        symbol_add((void *)symbol1->data[i], (void *)symbol2->data[i], full_symbol_get_size(symbol_result), (uint8_t * )symbol_result->data+i); //get data 
}



swif_full_symbol_t* full_symbol_add
(swif_full_symbol_t *symbol1, swif_full_symbol_t *symbol2)
{
    return NULL;

    //create result ( debut , fin)
    //full_symbol_add_base
}



/* Substration */
//void full_symbol_sub(void *symbol1, void *symbol2, uint32_t symbol_size, uint8_t* result);


/**
 * @brief Take a symbol and multiply it by another symbol, e.g. performs the equivalent of: result = p1 * p2
 * @param[in] p1     First symbol (to which coeff will be multiplied)
 * @param[in]     coeff      Coefficient by which the second packet is multiplied
 */
//void full_symbol_mul(void *symbol1, uint8_t coeff, uint32_t symbol_size, uint8_t* result);


/**
 * @brief Take a symbol and divide it by another symbol, e.g. performs the equivalent of: result = p1 / p2
 * @param[in] p1     First symbol (to which coeff will be divided)
 * @param[in]     coeff      Coefficient by which the second packet is divided
 */
//void full_symbol_div(void *symbol1, uint32_t symbol_size, uint8_t coeff, uint8_t* result);

/*---------------------------------------------------------------------------*/













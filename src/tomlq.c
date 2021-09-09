/**
 * @file tomlq.c
 * @brief Define TOML parser for q.
 */ 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                     Load Libraries                    //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#include <string.h>
#include <stdlib.h>
#include <toml.h>
#include <k.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                    Global Variables                   //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//%% Additional Type %%//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/

/**
 * @brief Error type indicator.
 */
const int K_ERROR = -128;

/**
 * @brief Null pointer of K used to return null from a function directly called by q.
 */
const K K_NULL = (K) 0;

//%% Utility %%//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/

/**
 * @brief One day in nanoseconds.
 */
const J ONEDAY_NANOS = 86400000000000LL;

//%% Error %%//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/

/**
 * @brief Buffer to capture error at parsing TOML file.
 */
char ERROR_BUFFER[64];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                   Private Functions                   //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//&& Pre-declaration %%//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/

K get_array(toml_array_t *array);
K get_table(toml_table_t *table);

//%% Drop Object %%//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/

/**
 * @brief Free resource of TOML document.
 * @param document: Result object of parsing a TOML file.
 */
K free_toml_document(K document){
  toml_free((toml_table_t*) kK(document)[1]);
  kK(document)[0]=0;
  kK(document)[1]=0;
  return (K) 0;
}

//%% Accessor %%//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/

/** 
 * @brief Get TOML boolean value.
 * @param element: Pointer to TOML element.
 * @return
 * - bool
 */
K get_bool(toml_datum_t *element){
  return kb(element->u.b);
}

/**
 * @brief Get TOML int value.
 * @param element: Pointer to TOML element.
 * @return
 * - long
 */
K get_int(toml_datum_t *element){
  return kj(element->u.i);
}

/**
 * @brief Get TOML double value.
 * @param element: Pointer to TOML element.
 * @return
 * - float
 */
K get_double(toml_datum_t *element){
  return kf(element->u.d);
}

/**
 * @brief Get TOML string value.
 * @param element: Pointer to TOML element.
 * @return
 * - string: If the length of the string is more than 30.
 * - symbol: Otherwise.
 */
K get_string(toml_datum_t *element){
  int length = strlen(element->u.s);
  if(length > 30){
    // String
    K string = ktn(KC, length);
    memcpy(kC(string), element->u.s, length);
    // String must be freed
    free(element->u.s);
    return string;
  }
  else{
    // Symbol
    char symbol[32];
    strcpy(symbol, element->u.s);
    // String must be freed
    free(element->u.s);
    return ks(symbol);
  }
}

/**
 * @brief Extract the nanoseconds since 2000.01.01D00:00:00 from TOML timestamp while
 *  freeing the allocated timestamp.
 * @param element: Pointer to TOML element.
 * @return
 * - int: The number of nanoseconds since 2000.01.01D00:00:00.
 */
J extract_timestamp_from_timestamp(toml_datum_t *element){
  // yyyy-mm-dd
  J nanoseconds = ONEDAY_NANOS * ymd(*element->u.ts->year, *element->u.ts->month, *element->u.ts->day);
  // HH:MM:SS
  nanoseconds += ((*element->u.ts->hour) * 60 * 60 + (*element->u.ts->minute) * 60 + *element->u.ts->second) * 1000000000LL;
  if(element->u.ts->millisec){
    // Millisecond is optional
    nanoseconds += (*element->u.ts->millisec) * 1000000LL;
  }
  if(element->u.ts->z){
    // Offset is optional
    if((element->u.ts->z)[0] == '-'){
      // UTC - offset
      // Add offset
      nanoseconds += (60 * 60 * 1000000000LL) * (10 * ((element->u.ts->z)[1] - '0') + (element->u.ts->z)[2] - '0');
    }
    else if((element->u.ts->z)[0] == '+'){
      // UTC + offset
      // Subtract offset
      nanoseconds -= (60 * 60 * 1000000000LL) * (10 * ((element->u.ts->z)[1] - '0') + (element->u.ts->z)[2] - '0'); 
    }
    else{
      // 'Z' and 'z' are parsed as "Z0"
      // https://github.com/cktan/tomlc99/blob/master/toml.c#L1959
      // UTC
      // Nothing to do
    }
  }
  // Timestamp must be freed
  free(element->u.ts);
  return nanoseconds;
}

/**
 * @brief Extract the number of days since 2000.01.01 from TOML timestamp while freeing
 *  the allocated timestamp.
 * @param element: Pointer to TOML element.
 * @return
 * - int: The number of days since 2000.01.01.
 */
int extract_date_from_timestamp(toml_datum_t *element){
  int days = ymd(*element->u.ts->year, *element->u.ts->month, *element->u.ts->day);
  // Timestamp must be freed
  free(element->u.ts);
  return days;
}

/**
 * @brief Extract time from TOML timestamp while freeing the allocated timestamp.
 * @param element: Pointer to TOML element.
 * @return
 * - int: Milliseconds from midnight.
 */
int extract_time_from_timestamp(toml_datum_t *element){
  int time = ((*element->u.ts->hour) * 60 * 60 + (*element->u.ts->minute) * 60 + *element->u.ts->second) * 1000;
  if(element->u.ts->millisec){
    // Millisecond exists
    time +=*element->u.ts->millisec;
  }
  // Timestamp must be freed
  free(element->u.ts);
  return time;
}

/**
 * @brief Get TOML timestamp value.
 * @param element: Pointer to TOML element.
 * @return
 * - timestamp: If year and hour exists.
 * - date: If year exists.
 * - time: If year does not exist and millisec exists.
 * - second: Otherwise.
 */
K get_timestamp(toml_datum_t *element){
  if(element->u.ts->year && element->u.ts->hour){
    // Timestamp
    return ktj(-KP, extract_timestamp_from_timestamp(element));
  }
  else if(element->u.ts->year){
    // Date
    return kd(extract_date_from_timestamp(element));
  }
  else if(element->u.ts->second){
    // Time
    return kt(extract_time_from_timestamp(element));
  }
  else{
    // Hour and minute should not be in TOML
    return krr("unknown time type");
  }
}

/**
 * @brief Get an TOML element from an array with an index.
 * @param array: TOML array.
 * @param index: Index of an element to retrieve.
 * @return
 * - bool
 * - long
 * - float
 * - symbol
 * - timestamp
 * - date
 * - time
 * - list of types above
 * - dictionary: Table.
 */
K get_array_element_with_index(toml_array_t *array, int index){

  toml_datum_t element = toml_string_at(array, index);
  if(element.ok){
    return get_string(&element);
  }

  element = toml_int_at(array, index);
  if(element.ok){
    return get_int(&element);
  }

  element = toml_bool_at(array, index);
  if(element.ok){
    return get_bool(&element);
  }

  element = toml_double_at(array, index);
  if(element.ok){
    return get_double(&element);
  }

  toml_array_t* inner_array = toml_array_at(array, index);
  if(inner_array){
    return get_array(inner_array);
  }

  element = toml_timestamp_at(array, index);
  if(element.ok){
    return get_timestamp(&element);
  }

  toml_table_t* table = toml_table_at(array, index);
  if(table){
    return get_table(table);
  }

}

/**
 * @brief Get TOML array value.
 * @param array: TOML array element.
 * @return
 * - empty list: If size of the array is 0.
 * - list of bool
 * - list of long
 * - list of float
 * - list of symbol
 * - list of timestamp
 * - list of date
 * - list of time
 * - compound list
 */
K get_array(toml_array_t *array){
  // List to return
  K list = (K) 0;

  int size = toml_array_nelem(array);
  if(!size){
    // Empty list
    return ktn(0, 0);
  }

  char item_type = toml_array_kind(array);
  switch(item_type){
    case 'a':
      // List of list
      list = ktn(0, size);
      for(int i = 0; i!= size; ++i){
        kK(list)[i] = get_array(toml_array_at(array, i));
      }
      return list;
    case 't':
      // List of table
      list = ktn(0, size);
      for(int i = 0; i!= size; ++i){
        kK(list)[i] = get_table(toml_table_at(array, i));
      }
      return list;
    case 'm':
      // Compound list
      list = ktn(0, size);
      for(int i = 0; i!= size; ++i){
        kK(list)[i] = get_array_element_with_index(array, i);
      }
      return list;
    default:
      // Simple list
      // Nothing to do here
      break;
  }

  // Get value type
  item_type = toml_array_type(array);
  switch(item_type){
    case 'b':
      // Bool list
      list = ktn(KB, size);
      for(int i = 0; i!= size; ++i){
        kG(list)[i] = (G) toml_bool_at(array, i).u.b;
      }
      break;
    case 'i':
      // Long list
      list = ktn(KJ, size);
      for(int i = 0; i!= size; ++i){
        kJ(list)[i] = toml_int_at(array, i).u.i;
      }
      break;
    case 'd':
      // Float list
      list = ktn(KF, size);
      for(int i = 0; i!= size; ++i){
        kF(list)[i] = toml_double_at(array, i).u.d;
      }
      break;
    case 's':
      {
        // Symbol list
        list = ktn(KS, size);
        // Assume the length of symbol does not exceed 64.
        char symbol[64];
        kS(list)[0] = ss(symbol);
        for(int i = 0; i!= size; ++i){
          toml_datum_t item = toml_string_at(array, i);
          strcpy(symbol, item.u.s);
          free(item.u.s);
          kS(list)[i] = ss(symbol);
        }
      }
      break;
    case 'T':
      {
        // Timestamp list
        list = ktn(KP, size);
        for(int i = 0; i!= size; ++i){
          toml_datum_t element = toml_timestamp_at(array, i);
          kJ(list)[i] = extract_timestamp_from_timestamp(&element);
        }
      }
      break;
    case 'D':
      {
        // Date list
        list = ktn(KD, size);
        for(int i = 0; i!= size; ++i){
          toml_datum_t element = toml_timestamp_at(array, i);
          kI(list)[i] = extract_date_from_timestamp(&element);
        }
      }
      break;
    case 't':
      {
        // Time list
        list = ktn(KT, size);
        for(int i = 0; i!= size; ++i){
          toml_datum_t element = toml_timestamp_at(array, i);
          kI(list)[i] = extract_time_from_timestamp(&element);
        }
      }
      break;
    default:
      // Unreachable
      return krr("unreachable");
  }

  return list;
}

/**
 * @brief Get an TOML element from a table with a key.
 * @param table: TOML table.
 * @param key: Key of an element to retrieve.
 * @return
 * - bool
 * - long
 * - float
 * - symbol
 * - timestamp
 * - date
 * - time
 * - list of types above
 * - dictionary: Table.
 */
K get_table_element_with_key(toml_table_t *table, const char *key){

  // Look for table
  toml_table_t* inner_table = toml_table_in(table, key);
  if (inner_table) {
    return get_table(inner_table);
  }

  // Look for bool
  toml_datum_t element = toml_bool_in(table, key);
  if(element.ok){
    return get_bool(&element);
  }

  // Look for int
  element = toml_int_in(table, key);
  if(element.ok){
    // int is int64
    return get_int(&element);
  }

  // Look for double
  element = toml_double_in(table, key);
  if(element.ok){
    return get_double(&element);
  }

  // Look for string
  element = toml_string_in(table, key);
  if(element.ok){
    return get_string(&element);
  }

  // Look for timestamp
  element = toml_timestamp_in(table, key);
  if(element.ok){
    return get_timestamp(&element);
  }

  // Look for array
  toml_array_t* array = toml_array_in(table, key);
  if (array) {
    return get_array(array);
  }

  // Specfied key does not exist
  return krr("no such element");
}

/**
 * @brief Get TOML table value.
 * @param table: TOML table element.
 * @return
 * - dictionary
 */
K get_table(toml_table_t *table){

  // Total number of keys
  int key_length = toml_table_nkval(table) + toml_table_narr(table) + toml_table_ntab(table);
  // List of keys
  K keys = ktn(KS, key_length);
  K values = ktn(0, key_length);
  for(int i = 0; i!=key_length; ++i){
    const char* key = toml_key_in(table, i);
    kS(keys)[i] = ss((S) key);
    K result = ee(get_table_element_with_key(table, (S) key));
    if(result->t == K_ERROR){
      r0(keys);
      r0(values);
      // Propagate the error
      return result;
    }
    else{
      kK(values)[i] = result;
    }
  }

  return xD(keys, values);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                       Interface                       //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/**
 * @brief Parse a TOML file.
 * @param file_path_: File handle to a TOML file.
 * @return 
 * - dictionary: Parsed document.
 */
K load_toml(K file_path_){

  // Trim preceding ":"
  K file_path = k(0, "{[path] `$1 _ string path}", r1(file_path_), (K) 0);
  
  // Open file
  FILE *file = fopen(file_path->s, "r");

  // Free file path which is no longer necessary
  r0(file_path);

  if(!file){
    // Error in opening file
    return krr("failed to open file");
  }

  // Parse file
  toml_table_t* document = toml_parse_file(file, ERROR_BUFFER, sizeof(ERROR_BUFFER));

  // Close file
  fclose(file);

  if (!document) {
    // Parse error
    return krr(ERROR_BUFFER);
    //return krr("failed to parse");
  }

  return get_table(document);
}


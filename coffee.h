#ifndef CM_MACHINE_H
#define CM_MACHINE_H

// #include "external/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CM_VERSION "0.1.0"
#define COFEE_MACHINE_VERSION CM_VERSION


#ifndef CM_API
  #if defined(_WIN32)
    #if defined(BUILD_SHARE)
      #define CM_API __declspec(dllexport)
    #elif defined(USE_SHARED)
      #define CM_API __declspec(dllimport)
    #else
      #define CM_API
    #endif
  #else
    #define CM_API
  #endif
#endif

#ifndef CM_MALLOC
  #define CM_MALLOC malloc
#endif
#ifndef CM_FREE
  #define CM_FREE free
#endif
#ifndef CM_CALLOC
  #define CM_CALLOC calloc
#endif
#ifndef CM_REALLOC
  #define CM_REALLOC realloc
#endif

#ifndef VEC2_TYPE
  #define VEC2_TYPE cm_vec2_t
#endif
#ifndef VEC3_TYPE
  #define VEC3_TYPE cm_vec3_t
#endif
#ifndef VEC4_TYPE
  #define VEC4_TYPE cm_vec4_t
#endif

#define CM_STR(x) #x
#define CM_ASSERT(s) if (!(s)) {CM_TRACELOG("Assertion '%s' failed", CM_STR(s)); exit(-1);}

#define CM_TRACEERR(message...) cm_tracelog(1, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define CM_LOGERR(message...) cm_log(1, message)

#define CM_TRACELOG(message...) cm_tracelog(0, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define CM_LOG(message...) cm_log(0, message)

#define cm_vec2(x, y) ((VEC2_TYPE){(x), (y)})
#define cm_vec3(x, y, z) ((VEC3_TYPE){(x), (y), (z)})
#define cm_vec4(x, y, z, w) ((VEC4_TYPE){(x), (y), (w), (w)})


#define CM_META       "__meta__"
#define CM_META_INDEX "__index__"
#define CM_META_NAME  "__name__"
#define CM_META_TYPE  "__type__"

#define _GET_ERROR -590362


#define cm_vec_t(n) \
  struct cm_vec##n##_t { \
    float data[n]; \
  }

typedef cm_vec_t(2) cm_vec2_t;
typedef cm_vec_t(3) cm_vec3_t;
typedef cm_vec_t(4) cm_vec4_t;

typedef int cm_bool_t;
#define CM_FALSE 0
#define CM_TRUE 1

#define cm_object_foreach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

#define coffee_table_foreach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
#define coffee_foreach(element, array) for(element = (array != NULL) ? (array)->table->child : NULL; element != NULL; element = element->next)
#define cm_object_parent(element) (element ? element->parent : NULL);

// #define coffee_foreach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

#define CM_FUNCTION_DEF(func_name, ...) \
func_name (cm_object_t *type, __VA_ARGS__)

// CM_API cm_object_t* cm_##name_p##_from_json(cJSON *json); \
// CM_API cJSON* cm_##name_p##_to_json(cm_object_t *object, int export_meta);
#define CM_TDEF(name_p, type_t) \
CM_API cm_object_t* cm_create_##name_p(type_t value); \
CM_API type_t cm_object_get_##name_p(cm_object_t *object, const char *name); \
CM_API type_t cm_object_get_opt_##name_p(cm_object_t *object, const char *name, type_t opt); \
CM_API cm_object_t* cm_object_add_##name_p(cm_object_t *object, const char *name, type_t value); \
CM_API type_t cm_object_to_##name_p(cm_object_t *object); \
CM_API type_t cm_object_to_opt_##name_p(cm_object_t *object, type_t value); \
CM_API void cm_object_set_##name_p(cm_object_t *object, type_t value); \


typedef struct coffee_machine_t coffee_machine_t;
typedef struct cm_object_t cm_object_t;
// typedef struct cm_coffee_t cm_coffee_t;
typedef int(*cm_function)(coffee_machine_t*);

typedef enum {
  CM_TNULL = 0,
  CM_TNUMBER,
  CM_TSTRING,
  CM_TBOOLEAN,
  CM_TVEC2,
  CM_TVEC3,
  CM_TVEC4,
  CM_TARRAY,
  CM_TTABLE,
  CM_TUSERDATA,
  CM_TFUNCTION,
  CM_TREFERENCE,
  CM_TMAX
} CM_T_;

#define MAX_STACK 256

typedef struct cm_hashmap_t {

} cm_hashmap_t;

#define CHECK_TYPE(coffee, c_type) \
  CM_ASSERT(coffee != NULL); \
  CM_ASSERT(coffee->type == c_type);

typedef enum COFFEE_T_ {
  COFFEE_TNULL = 0,
  COFFEE_TNUMBER,
  COFFEE_TSTRING,
  COFFEE_TBOOL,
  COFFEE_TVEC2,
  COFFEE_TVEC3,
  COFFEE_TVEC4,
  COFFEE_TARRAY,
  COFFEE_TTABLE,
  COFFEE_TUSERDATA,
  COFFEE_TFUNCTION,
  COFFEE_TREF,
  COFFEE_TMAX
} COFFEE_T_;

typedef void* coffee_ptr;
typedef int(*coffee_fn)(coffee_ptr*);
/* coffee types */
typedef struct coffee_s          coffee_t;
typedef struct coffee_float_s    coffee_float_t;
typedef struct coffee_int_s      coffee_int_t;
typedef struct coffee_number_s   coffee_number_t;
typedef struct coffee_boolean_s  coffee_boolean_t;
typedef struct coffee_string_s   coffee_string_t;
typedef struct coffee_vec2_s     coffee_vec2_t;
typedef struct coffee_vec3_s     coffee_vec3_t;
typedef struct coffee_vec4_s     coffee_vec4_t;
typedef struct coffee_table_s    coffee_table_t;
typedef struct coffee_userdata_s coffee_udata_t;
typedef struct coffee_function_s coffee_function_t;

struct coffee_machine_t {
  coffee_t *root;
  coffee_t *gc;

  coffee_t* stack[MAX_STACK];
  int stack_index;
  int fp;

  const char*(*read_file)(const char *, const char*);
  void(*write_file)(const char *, const char *, size_t, const char*);
};

struct coffee_float_s {
  float value;
};

struct coffee_int_s {
  int value;
};

typedef enum {
  COFFEE_NUMBER_FLOAT = 0,
  COFFEE_NUMBER_INT
} COFFEE_NUMBER_T_;

struct coffee_number_s {
  union {
    coffee_float_t value_float;
    coffee_int_t value_int;
    double value;
  };
  COFFEE_NUMBER_T_ type;
};

struct coffee_string_s {
  int len;
  char *value;
};

struct coffee_boolean_s {
  unsigned char value;
};

struct coffee_userdata_s {
  coffee_table_t *meta;
  void *data;
};

struct coffee_vec2_s {
  union {
    double data[2];
    struct {
      double x;
      double y;
    };
  };
};

struct coffee_vec3_s {
  // VEC3_TYPE vec;
  union {
    double data[3];
    struct {
      double x;
      double y;
      double z;
    };
  };
};

struct coffee_vec4_s {
  // VEC4_TYPE vec;
  union {
    double data[4];
    struct {
      double x;
      double y;
      double z;
      double w;
    };
  };
};

struct coffee_table_s {
  coffee_table_t *meta;
  int len;
  coffee_t *child;
};

typedef enum COFFEE_FUNC_TYPE_ {
  COFFEE_VM_FUNC,
  COFFEE_OBJ_FUNC
} COFFEE_FUNC_TYPE_;

struct coffee_function_s {
  COFFEE_FUNC_TYPE_ type;
  union {
    int(*vm)(coffee_machine_t*);
    int(*obj)(coffee_t*);
  };  
};

struct coffee_s {
  COFFEE_T_ type;
  unsigned char marked;
  char *name;

  union {
    coffee_number_t number;
    coffee_string_t string;
    coffee_vec2_t vec2;
    coffee_vec3_t vec3;
    coffee_vec4_t vec4;
    coffee_table_t *array;
    coffee_table_t *table;
    coffee_udata_t userdata;
    coffee_fn *func;
    coffee_t *ref;
  };

  coffee_t *prev;
  coffee_t *next;
};

#if defined(__cplusplus)
extern "C" {
#endif

/*======================================*
 *                 VM                   *
 *======================================*/

CM_API int cm_init();
CM_API void cm_deinit();
CM_API coffee_machine_t* cm_get_vm();
CM_API void cm_gc();

CM_API const char* cm_version();

CM_API coffee_t* cm_get_type(const char *type_name);
CM_API coffee_t* cm_set_type(const char *type_name, coffee_t *type);

CM_API coffee_t* cm_get_object(const char *name);
CM_API coffee_t* cm_set_object(const char *name, coffee_t *object);

CM_API int cm_call(coffee_machine_t *vm, int args, int ret);
CM_API void cm_settop(coffee_machine_t *vm, int top);
CM_API int cm_gettop(coffee_machine_t *vm);

CM_API void cm_push(coffee_machine_t *vm, coffee_t *obj);
CM_API coffee_t *cm_pop(coffee_machine_t *vm);
CM_API coffee_t *cm_get(coffee_machine_t *vm, int index);

CM_API void cm_newtable(coffee_machine_t *vm);
CM_API void cm_setmetatable(coffee_machine_t *vm);
CM_API void cm_getmetatable(coffee_machine_t *vm);
CM_API void cm_newarray(coffee_machine_t *vm);

CM_API void cm_pushnumber(coffee_machine_t *vm, float number);
CM_API void cm_pushstring(coffee_machine_t *vm, const char *string);
CM_API void cm_pushboolean(coffee_machine_t *vm, int boolean);
CM_API void cm_pushvec2(coffee_machine_t *vm, double data[2]);
CM_API void cm_pushvec3(coffee_machine_t *vm, double data[3]);
CM_API void cm_pushvec4(coffee_machine_t *vm, double data[4]);
CM_API void cm_pushcfunction(coffee_machine_t *vm, cm_function *fn);
// CM_API void cm_pushtable(coffee_machine_t *vm);
CM_API void cm_pushvalue(coffee_machine_t *vm, int index);

CM_API float cm_tonumber(coffee_machine_t *vm, int index);
CM_API const char* cm_tostring(coffee_machine_t *vm, int index);
CM_API int cm_toboolean(coffee_machine_t *vm, int index);
CM_API double* cm_tovec2(coffee_machine_t *vm, int index);
CM_API double* cm_tovec3(coffee_machine_t *vm, int index);
CM_API double* cm_tovec4(coffee_machine_t *vm, int index);
CM_API coffee_t* cm_toobject(coffee_machine_t *vm, int index);
CM_API void *cm_touserdata(coffee_machine_t *vm, int index);
CM_API coffee_t* cm_toref(coffee_machine_t *vm, int index);
CM_API cm_function* cm_tofunction(coffee_machine_t *vm, int index);

CM_API float cm_checknumber(coffee_machine_t *vm, int index);
CM_API const char* cm_checkstring(coffee_machine_t *vm, int index);
CM_API int cm_checkboolean(coffee_machine_t *vm, int index);
CM_API double* cm_checkvec2(coffee_machine_t *vm, int index);
CM_API double* cm_checkvec3(coffee_machine_t *vm, int index);
CM_API double* cm_checkvec4(coffee_machine_t *vm, int index);
CM_API coffee_t* cm_checkobject(coffee_machine_t *vm, int index);
CM_API void *cm_checkudata(coffee_machine_t *vm, int index);
CM_API coffee_t* cm_checkref(coffee_machine_t *vm, int index);
CM_API cm_function* cm_checkfunction(coffee_machine_t *vm, int index);

CM_API float cm_optnumber(coffee_machine_t *vm, int index, float opt);
CM_API const char* cm_optstring(coffee_machine_t *vm, int index, const char *opt);
CM_API int cm_optboolean(coffee_machine_t *vm, int index, int opt);
CM_API double* cm_optvec2(coffee_machine_t *vm, int index, double data[2]);
CM_API double* cm_optvec3(coffee_machine_t *vm, int index, double data[3]);
CM_API double* cm_optvec4(coffee_machine_t *vm, int index, double data[4]);
CM_API coffee_t* cm_optobject(coffee_machine_t *vm, int index, coffee_t *opt);
CM_API void *cm_optudata(coffee_machine_t *vm, int index, void *opt);
CM_API coffee_t* cm_optref(coffee_machine_t *vm, int index, coffee_t *opt);
CM_API cm_function* cm_optfunction(coffee_machine_t *vm, int index, cm_function *opt);

/*====================================*
 *               Coffee               *
 *====================================*/

CM_API void coffee_print(coffee_t *coffee);

CM_API coffee_t* coffee_create(COFFEE_T_ type);
CM_API coffee_t* coffee_load(const char *filename);
CM_API coffee_t* coffee_load_json(const char *filename);
CM_API coffee_t* coffee_clone(coffee_t *coffee);
CM_API void coffee_delete(coffee_t *coffee);
CM_API void coffee_clear(coffee_t *coffee);

CM_API coffee_t* coffee_parse(const char *coffee_string);
CM_API coffee_t* coffee_parse_json(const char *json_string);

CM_API coffee_t* coffee_from_json(coffee_t *coffee_json);
CM_API const char* coffee_print_json(coffee_t *coffee_json, int *char_count_out);
CM_API const char* coffee_print_coffee(coffee_t *coffee);

// CM_API void coffee_set_metatable(coffee_t *coffee, )
CM_API void coffee_setmetatable(coffee_table_t *table, coffee_table_t *metatable);
CM_API coffee_table_t* coffee_getmetatable(coffee_table_t *table);
CM_API void coffee_setname(coffee_t *coffee, const char *name);
CM_API const char* coffee_getname(coffee_t *coffee);

CM_API coffee_t* coffee_get(coffee_t *coffee, const char *name);
CM_API coffee_t* coffee_index(coffee_t *coffee, int index);
CM_API int coffee_length(coffee_t *coffee);

CM_API coffee_table_t* coffee_newtable();

/*===========*
 * Null      *
 *===========*/

CM_API int coffee_isnull(coffee_t *coffee);
CM_API coffee_t* coffee_create_null();

/*===========*
 * Number    *
 *===========*/

CM_API coffee_t* coffee_create_number(double value, int type);
CM_API int coffee_isnumber(coffee_t *coffee);
CM_API double coffee_tonumber(coffee_t *coffee);
CM_API double coffee_checknumber(coffee_t *coffee);
CM_API void coffee_setnumber(coffee_t *coffee, double number);
CM_API coffee_t* coffee_clone_number(coffee_t *coffee);

CM_API coffee_t* coffee_create_float(float value);
CM_API int coffee_isfloat(coffee_t *coffee);
CM_API float coffee_tofloat(coffee_t *coffee);
CM_API float coffee_checkfloat(coffee_t *coffee);
CM_API void coffee_setfloat(coffee_t *coffee, float number);

CM_API coffee_t* coffee_create_int(int value);
CM_API int coffee_isint(coffee_t *coffee);
CM_API int coffee_toint(coffee_t *coffee);
CM_API int coffee_checkint(coffee_t *coffee);
CM_API void coffee_setint(coffee_t *coffee, int number);

/*===========*
 * String    *
 *===========*/

CM_API coffee_t* coffee_create_string(const char *string);
CM_API int coffee_isstring(coffee_t *coffee);
CM_API const char* coffee_tostring(coffee_t *coffee);
CM_API const char* coffee_checkstring(coffee_t *coffee);
CM_API void coffee_setstring(coffee_t *coffee, const char *string);
CM_API coffee_t* coffee_clone_string(coffee_t *coffee);

/*===========*
 * Bool      *
 *===========*/

CM_API coffee_t* coffee_create_boolean(int boolean);
CM_API int coffee_isboolean(coffee_t *coffee);
CM_API int coffee_toboolean(coffee_t *coffee);
CM_API int coffee_checkboolean(coffee_t *coffee);
CM_API void coffee_setboolean(coffee_t *coffee, int boolean);
CM_API coffee_t* coffee_clone_boolean(coffee_t *coffee);

/*===========*
 * Vec2      *
 *===========*/

CM_API coffee_t* coffee_create_vec2(double x, double y);
CM_API int coffee_isvec2(coffee_t *coffee);
CM_API double* coffee_tovec2(coffee_t *coffee);
CM_API double* coffee_checkvec2(coffee_t *coffee);
CM_API void coffee_setvec2(coffee_t *coffee, double x, double y);
CM_API coffee_t* coffee_clone_vec2(coffee_t* coffee);

/*===========*
 * Vec3      *
 *===========*/

CM_API coffee_t* coffee_create_vec3(double x, double y, double z);
CM_API int coffee_isvec3(coffee_t *coffee);
CM_API double* coffee_tovec3(coffee_t *coffee);
CM_API double* coffee_checkvec3(coffee_t *coffee);
CM_API void coffee_setvec3(coffee_t *coffee, double x, double y, double z);
CM_API coffee_t* coffee_clone_vec3(coffee_t* coffee);

/*===========*
 * Vec4      *
 *===========*/

CM_API coffee_t* coffee_create_vec4(double x, double y, double z, double w);
CM_API int coffee_isvec4(coffee_t *coffee);
CM_API double* coffee_tovec4(coffee_t *coffee);
CM_API double* coffee_checkvec4(coffee_t *coffee);
CM_API void coffee_setvec4(coffee_t *coffee, double x, double y, double z, double w);
CM_API coffee_t* coffee_clone_vec4(coffee_t* coffee);

/*===========*
 * Array     *
 *===========*/

CM_API coffee_t* coffee_create_array();
CM_API int coffee_isarray(coffee_t *coffee);
CM_API coffee_table_t* coffee_toarray(coffee_t *coffee);
CM_API coffee_table_t* coffee_checkarray(coffee_t *coffee);
CM_API coffee_table_t* coffee_setarray(coffee_t *coffee, coffee_table_t *array);
CM_API int coffee_array_length(coffee_table_t *array);
CM_API coffee_t* coffee_array_get(coffee_table_t *array, int index);
CM_API void coffee_array_set(coffee_table_t *array, int index, coffee_t *value);
CM_API void coffee_array_push(coffee_table_t *array, coffee_t *value);
CM_API coffee_t* coffee_array_pop(coffee_table_t *array);
CM_API coffee_t* coffee_clone_array(coffee_t* coffee);

/*===========*
 * Table     *
 *===========*/

CM_API coffee_t* coffee_create_table();
CM_API int coffee_istable(coffee_t *coffee);
CM_API coffee_table_t* coffee_totable(coffee_t *coffee);
CM_API coffee_table_t* coffee_checktable(coffee_t *coffee);
CM_API coffee_table_t* coffee_settable(coffee_t *coffee, coffee_table_t *table);
CM_API int coffee_table_length(coffee_table_t *table);
CM_API coffee_t* coffee_table_get(coffee_table_t *array, const char *name);
CM_API void coffee_table_set(coffee_table_t *array, const char *name, coffee_t *value);
CM_API coffee_t* coffee_clone_table(coffee_t* coffee);

CM_API void coffee_table_setnumber(coffee_table_t *table, const char *name, double value);
CM_API void coffee_table_setstring(coffee_table_t *table, const char *name, const char* value);
CM_API void coffee_table_setboolean(coffee_table_t *table, const char *name, int value);
CM_API void coffee_table_setvec2(coffee_table_t *table, const char *name, double *value);
CM_API void coffee_table_setvec3(coffee_table_t *table, const char *name, double *value);
CM_API void coffee_table_setvec4(coffee_table_t *table, const char *name, double *value);
CM_API void coffee_table_setarray(coffee_table_t *table, const char *name, coffee_t *value);
CM_API void coffee_table_settable(coffee_table_t *table, const char *name, coffee_t *value);
CM_API void coffee_table_setuserdata(coffee_table_t *table, const char *name, void *value);
CM_API void coffee_table_setfunction(coffee_table_t *table, const char *name, coffee_fn *value);
CM_API void coffee_table_setref(coffee_table_t *table, const char *name, coffee_t* value);

CM_API double coffee_table_getnumber(coffee_table_t *table, const char *name);
CM_API const char* coffee_table_getstring(coffee_table_t *table, const char *name);
CM_API int coffee_table_getboolean(coffee_table_t *table, const char *name);
CM_API double* coffee_table_getvec2(coffee_table_t *table, const char *name);
CM_API double* coffee_table_getvec3(coffee_table_t *table, const char *name);
CM_API double* coffee_table_getvec4(coffee_table_t *table, const char *name);
CM_API coffee_t* coffee_table_getarray(coffee_table_t *table, const char *name);
CM_API coffee_t* coffee_table_gettable(coffee_table_t *table, const char *name);
CM_API void* coffee_table_getuserdata(coffee_table_t *table, const char *name);
CM_API coffee_fn* coffee_table_getfunction(coffee_table_t *table, const char *name);
CM_API coffee_t*  coffee_table_getref(coffee_table_t *table, const char *name);

/*=============*
 * Userdata    *
 *=============*/

CM_API coffee_t* coffee_create_userdata(void *udata);
CM_API int coffee_isuserdata(coffee_t *coffee);
CM_API void* coffee_touserdata(coffee_t *coffee);
CM_API void* coffee_checkudata(coffee_t *coffee);
CM_API void* coffee_setuserdata(coffee_t *coffee, void *userdata);
CM_API coffee_t* coffee_clone_userdata(coffee_t* coffee);

/*=============*
 * Function    *
 *=============*/

CM_API coffee_t* coffee_create_function(coffee_fn *func);
CM_API int coffee_isfunction(coffee_t *coffee);
CM_API coffee_fn* coffee_tofunction(coffee_t *coffee);
CM_API coffee_fn* coffee_checkfunction(coffee_t *coffee);
CM_API void coffee_setfunction(coffee_t *coffee, coffee_fn* fn);
CM_API int coffee_call_function(coffee_t *coffee);
CM_API coffee_t* coffee_clone_function(coffee_t* coffee);

/*==============*
 * Reference    *
 *==============*/

CM_API coffee_t* coffee_create_ref(coffee_t *ref);
CM_API int coffee_isref(coffee_t *coffee);
CM_API coffee_t* coffee_toref(coffee_t *coffee);
CM_API coffee_t* coffee_checkref(coffee_t *coffee);
CM_API coffee_t* coffee_setref(coffee_t *coffee, coffee_t *ref);
CM_API coffee_t* coffee_clone_ref(coffee_t* coffee);

/*=========================================================*
 *                        Debug                            *
 *=========================================================*/

CM_API void cm_tracelog(int mode, const char *func, const char *file, int line, const char *fmt, ...);
CM_API void cm_log(int mode, const char *fmt, ...);


/*==============================*
 *          DON'T USE           *
 *==============================*/

struct cm_object_t {
  CM_T_ type;
  unsigned char marked;
  char *name;
  cm_object_t *meta;

  union {
    float number;
    char *string;
    int boolean;
    VEC2_TYPE vec2;
    VEC3_TYPE vec3;
    VEC4_TYPE vec4;
    void *userdata;
    cm_object_t *ref;
    cm_function* func;
  };

  cm_object_t *prev;
  cm_object_t *next;
  cm_object_t *parent;
  cm_object_t *child;
};

CM_API cm_object_t* cm_parse_json(const char *json_string);
// CM_API cm_object_t* cm_load_coffee_from_json(const char *json_string);
CM_API const char* cm_print_json(cm_object_t *object, int *char_count_out);
CM_API cm_object_t* cm_json_to_coffee(cm_object_t *object);

CM_API cm_object_t* cm_parse_coffee(const char *coffee_string);
CM_API const char* cm_print_coffee(cm_object_t *object);

CM_API void cm_object_get_uid(cm_object_t *obj, char *out);

CM_API void cm_object_print(cm_object_t *obj);

CM_API cm_object_t* cm_object_create(CM_T_ type);
CM_API cm_object_t* cm_object_load(const char *path);
CM_API cm_object_t* cm_object_clone(cm_object_t *obj);
CM_API cm_object_t* cm_object_replace(cm_object_t *obj, const char *name, cm_object_t *new_item);
// CM_API cm_object_t* cm_object_load_from_
// CM_API cm_object_t* cm_object_from_json(cJSON *const json);
CM_API void cm_object_remove(cm_object_t *object, const char *name);
CM_API void cm_object_delete(cm_object_t *object);
CM_API void cm_object_clear(cm_object_t *object);

CM_API void cm_object_update_meta(cm_object_t *object);
CM_API void cm_object_set_meta(cm_object_t *object, cm_object_t *meta);
CM_API cm_object_t* cm_object_get_meta(cm_object_t *object);
CM_API void cm_object_set_type(cm_object_t *object, CM_T_ type);
CM_API CM_T_ cm_object_get_type(cm_object_t *object);

CM_API void cm_object_set_name(cm_object_t *object, const char *name);
CM_API const char* cm_object_get_name(cm_object_t *object);

CM_API cm_object_t* cm_object_get(cm_object_t *object, const char *name);
CM_API cm_object_t* cm_object_set(cm_object_t *object, const char *name, cm_object_t *item);
CM_API void cm_object_push(cm_object_t *object, int index, cm_object_t *item);
CM_API cm_object_t* cm_object_pop(cm_object_t *object);
CM_API cm_object_t* cm_object_add(cm_object_t *object, const char *name, cm_object_t *item);
CM_API int cm_object_length(cm_object_t *object);

CM_API void cm_array_to_vec2(cm_object_t *array);
CM_API void cm_array_to_vec3(cm_object_t *array);
CM_API void cm_array_to_vec4(cm_object_t *array);

CM_API cm_object_t* cm_vec2_from_array(cm_object_t *array);
CM_API cm_object_t* cm_vec3_from_array(cm_object_t *array);
CM_API cm_object_t* cm_vec4_from_array(cm_object_t *array);

CM_API cm_object_t* cm_create_null();

CM_TDEF(number, float);
CM_TDEF(string, const char*);
CM_TDEF(boolean, int);
CM_TDEF(vec2, VEC2_TYPE);
CM_TDEF(vec3, VEC3_TYPE);
CM_TDEF(vec4, VEC4_TYPE);
CM_TDEF(array, cm_object_t*);
CM_TDEF(table, cm_object_t*);
CM_TDEF(userdata, void*);
CM_TDEF(function, cm_function*);
CM_TDEF(reference, cm_object_t*);

#if defined(__cplusplus)
}
#endif

#endif
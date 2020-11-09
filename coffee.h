#ifndef CM_MACHINE_H
#define CM_MACHINE_H

// #include "external/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "external/cJSON.h"

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


#define cm_vec_t(n) \
  struct cm_vec##n##_t { \
    float data[n]; \
  }

typedef cm_vec_t(2) cm_vec2_t;
typedef cm_vec_t(3) cm_vec3_t;
typedef cm_vec_t(4) cm_vec4_t;
typedef int cm_bool_t;

#define cm_object_foreach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
#define cm_object_parent(element) (element ? element->parent : NULL);

#define CM_FUNCTION_DEF(func_name, ...) \
func_name (cm_object_t *type, __VA_ARGS__)

#define CM_TDEF(name_p, type_t) \
CM_API cm_object_t* cm_create_##name_p(type_t value); \
CM_API cm_object_t* cm_##name_p##_from_json(cJSON *json); \
CM_API cJSON* cm_##name_p##_to_json(cm_object_t *object, int export_meta); \
CM_API type_t cm_object_get_##name_p(cm_object_t *object, const char *name); \
CM_API type_t cm_object_get_opt_##name_p(cm_object_t *object, const char *name, type_t opt); \
CM_API cm_object_t* cm_object_add_##name_p(cm_object_t *object, const char *name, type_t value); \
CM_API type_t cm_object_to_##name_p(cm_object_t *object); \
CM_API type_t cm_object_to_opt_##name_p(cm_object_t *object, type_t value); \
CM_API void cm_object_set_##name_p(cm_object_t *object, type_t value); \


typedef struct cm_object_t cm_object_t;
// typedef struct cm_coffee_t cm_coffee_t;

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
  CM_TCUSTOM,
  CM_TUSERDATA,
  CM_TREFERENCE,
  CM_TMAX
} CM_T_;

typedef cm_object_t*(*loadFromJson)(cJSON*);
typedef cm_object_t*(*exportToJson)(cJSON*);

#define MAX_STACK 256

typedef struct coffee_vm_t {
  cm_object_t *root;
  cm_object_t *gc;

  cm_object_t* stack[MAX_STACK];
  int stack_index;

  const char*(*read_file)(const char *, const char*);
  void(*write_file)(const char *, const char *, size_t, const char*);
} cm_vm_t;

struct cm_object_t {
  CM_T_ type;
  char name[128];
  cm_object_t *meta;
  int own_meta;

  union {
    float number;
    char string[256];
    int boolean;
    VEC2_TYPE vec2;
    VEC3_TYPE vec3;
    VEC4_TYPE vec4;
    cm_object_t *ref;
    void *userdata;
  };

  cm_object_t *prev;
  cm_object_t *next;
  cm_object_t *parent;
  cm_object_t *child;
};

#if defined(__cplusplus)
extern "C" {
#endif

/*=====================*
 *         VM          *
 *=====================*/

CM_API int cm_vm_init();
CM_API void cm_vm_deinit();
CM_API cm_vm_t* cm_get_vm();
CM_API void cm_vm_gc();

CM_API const char* cm_vm_version();

CM_API void cm_vm_push(cm_object_t *obj);
CM_API cm_object_t *cm_vm_pop();

CM_API cm_object_t* cm_vm_get_type(const char *type_name);
CM_API cm_object_t* cm_vm_set_type(const char *type_name, cm_object_t *type);

CM_API cm_object_t* cm_vm_get_object(const char *name);
CM_API cm_object_t* cm_vm_set_object(const char *name, cm_object_t *object);

// CM_API cm_object_t* cm_object_get_path(cm_object_t *object, const char *path);
// CM_API void cm_object_set_path(cm_object_t *object, const char *path, cm_object_t *item);
// CM_API void cm_object_clear_path(cm_object_t *object, const char *path);
// CM_API void cm_object_delete_path(cm_object_t *object, const char *path);
// CM_API void cm_object_push_to_path(cm_object_t *object, const char *path, cm_object_t *item);

/*======================*
 *        Coffee        *
 *======================*/

CM_API cm_object_t* cm_parse_json(const char *json_string);
CM_API const char* cm_print_json(cm_object_t *object);
CM_API cm_object_t* cm_parse_coffee(const char *coffee_string);
CM_API const char* cm_print_coffee(cm_object_t *object);

CM_API cm_object_t* cm_object_create(CM_T_ type);
CM_API cm_object_t* cm_object_load(const char *path);
// CM_API cm_object_t* cm_object_load_from_
CM_API cm_object_t* cm_object_from_json(cJSON *const json);
CM_API void cm_object_delete(cm_object_t *object);
CM_API void cm_object_clear(cm_object_t *object);

CM_API void cm_object_update_meta(cm_object_t *object);
CM_API void cm_object_set_meta(cm_object_t *object, cm_object_t *meta);
CM_API cm_object_t* cm_object_get_meta(cm_object_t *object);
CM_API void cm_object_set_type(cm_object_t *object, CM_T_ type);
CM_API CM_T_ cm_object_get_type(cm_object_t *object);

CM_API cm_object_t* cm_object_get(cm_object_t *object, const char *name);
CM_API cm_object_t* cm_object_set(cm_object_t *object, const char *name, cm_object_t *item);
CM_API void cm_object_push(cm_object_t *object, int index, cm_object_t *item);
CM_API cm_object_t* cm_object_pop(cm_object_t *object);
CM_API cm_object_t* cm_object_add(cm_object_t *object, const char *name, cm_object_t *item);
CM_API int cm_object_length(cm_object_t *object);

CM_API cm_object_t* cm_vec2_array_from_json(cJSON *const json);

CM_TDEF(number, float);
CM_TDEF(string, const char*);
CM_TDEF(boolean, int);
CM_TDEF(vec2, VEC2_TYPE);
CM_TDEF(vec3, VEC3_TYPE);
CM_TDEF(vec4, VEC4_TYPE);
CM_TDEF(array, cm_object_t*);
CM_TDEF(table, cm_object_t*);
CM_TDEF(userdata, void*);

/*===============*
 *     JSON      *
 *===============*/

#define JSON_TYPE(name_t, type_t) \
CM_API cJSON* json_create_##name_t (type_t value); \
CM_API int json_is_##name_t (cJSON *const json); \
CM_API void json_set_##name_t (cJSON *const json, type_t value); \
CM_API type_t json_to_##name_t (cJSON *const json); \
CM_API type_t json_opt_##name_t (cJSON *const json, type_t opt); \
CM_API cJSON* json_add_##name_t (cJSON *const json, const char *name, type_t value); \
CM_API type_t json_get_##name_t (cJSON *const json, const char *name, int index); \
CM_API type_t json_get_opt_##name_t (cJSON *const json, const char *name, int index, type_t opt)

CM_API int json_is_valid(const char *filename);
CM_API cJSON* json_open(const char *filename);
CM_API cJSON* json_parse(const char *jsonStr);
CM_API cJSON* json_clone(cJSON *src);
CM_API void json_save(const char *filename, cJSON* const json);
CM_API char* json_print(cJSON* const json);

CM_API cJSON* json_create();
CM_API void json_delete(cJSON* const json);

CM_API int json_add_item(cJSON *const json, const char *name, cJSON *const item);
CM_API cJSON *json_get_item(cJSON *const json, const char *name, int index);


JSON_TYPE(string, const char*);
JSON_TYPE(number, float);
JSON_TYPE(boolean, int);
JSON_TYPE(vec2, VEC2_TYPE);
JSON_TYPE(vec3, VEC3_TYPE);
JSON_TYPE(vec4, VEC4_TYPE);
JSON_TYPE(array, const cJSON*);
JSON_TYPE(int_array, int*);
JSON_TYPE(object, const cJSON*);

CM_API int json_get_array_size(const cJSON* json);

/*===============*
 *     Debug     *
 *===============*/

CM_API void cm_tracelog(int mode, const char *func, const char *file, int line, const char *fmt, ...);
CM_API void cm_log(int mode, const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif
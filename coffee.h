#ifndef COFFEE_MACHINE_H
#define COFFEE_MACHINE_H

// #include "external/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "external/cJSON.h"


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

#ifndef CM_VEC2
  #define CM_VEC2 cm_vec2_t
#endif
#ifndef CM_VEC3
  #define CM_VEC3 cm_vec3_t
#endif
#ifndef CM_VEC4
  #define CM_VEC4 cm_vec4_t
#endif

#define CM_STR(x) #x
#define CM_ASSERT(s) if (!(s)) {CM_TRACELOG("Assertion '%s' failed", CM_STR(s)); exit(-1);}

#define CM_TRACEERR(message...) cm_tracelog(1, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define CM_LOGERR(message...) cm_log(1, message)

#define CM_TRACELOG(message...) cm_tracelog(0, __FILE__, __PRETTY_FUNCTION__, __LINE__, message)
#define CM_LOG(message...) cm_log(0, message)

#define cm_vec2(x, y) ((CM_VEC2){(x), (y)})
#define cm_vec3(x, y, z) ((CM_VEC3){(x), (y), (z)})
#define cm_vec4(x, y, z, w) ((CM_VEC4){(x), (y), (w), (w)})


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

#define CM_TYPE_DEF(name_p, type_t) \
CM_API cm_object_t* cm_create_##name_p(type_t value, cm_object_t *meta); \
CM_API cm_object_t* cm_##name_p##_from_json(cJSON *json, cm_object_t *meta); \
CM_API cJSON* cm_##name_p##_to_json(cm_object_t *object, int export_meta); \
CM_API type_t cm_object_get_##name_p(cm_object_t *object, const char *name); \
CM_API type_t cm_object_get_opt_##name_p(cm_object_t *object, const char *name, type_t opt); \
CM_API cm_object_t* cm_object_add_##name_p(cm_object_t *object, const char *name, type_t value); \
CM_API type_t cm_object_to_##name_p(cm_object_t *object); \
CM_API type_t cm_object_to_opt_##name_p(cm_object_t *object, type_t value); \
CM_API void cm_object_set_##name_p(cm_object_t *object, type_t value); \

typedef struct cm_object_t cm_object_t;

typedef enum {
  CM_TYPE_NULL = 0,
  CM_TYPE_NUMBER,
  CM_TYPE_STRING,
  CM_TYPE_BOOLEAN,
  CM_TYPE_VEC2,
  CM_TYPE_VEC3,
  CM_TYPE_VEC4,
  CM_TYPE_ARRAY,
  CM_TYPE_TABLE,
  CM_TYPE_CUSTOM,
  CM_TYPE_USERDATA,
  CM_TYPE_REFERENCE,
  CM_TYPE_MAX
} CM_TYPE_;

typedef cm_object_t*(*loadFromJson)(cJSON*);
typedef cm_object_t*(*exportToJson)(cJSON*);

typedef struct coffee_vm_t {
  cm_object_t *root;
  cm_object_t *gc;
  const char*(*read_file)(const char *, const char*);
  void(*write_file)(const char *, const char *, size_t, const char*);
} cm_vm_t;

struct cm_object_t {
  CM_TYPE_ type;
  char name[128];
  cm_object_t *meta;
  int own_meta;

  float number;
  char string[256];
  int boolean;
  CM_VEC2 vec2;
  CM_VEC3 vec3;
  CM_VEC4 vec4;
  cm_object_t *ref;
  void *userdata;

  cm_object_t *prev;
  cm_object_t *next;
  cm_object_t *parent;
  cm_object_t *child;
};

#if defined(__cplusplus)
extern "C" {
#endif

CM_API int cm_vm_init();
CM_API void cm_vm_deinit();
CM_API cm_vm_t* cm_get_vm();
CM_API void cm_vm_gc();

CM_API cm_object_t* cm_vm_get_type(const char *type_name);
CM_API cm_object_t* cm_vm_set_type(const char *type_name, cm_object_t *type);

CM_API cm_object_t* cm_vm_get_object(const char *name);
CM_API cm_object_t* cm_vm_set_object(const char *name, cm_object_t *object);

CM_API cm_object_t* cm_object_create(CM_TYPE_ type);
CM_API cm_object_t* cm_object_load(const char *path);
CM_API cm_object_t* cm_object_from_json(cJSON *const json, cm_object_t *meta);
CM_API void cm_object_delete(cm_object_t *object);
CM_API void cm_object_clear(cm_object_t *object);

// CM_API cm_object_t* cm_object_get_path(cm_object_t *object, const char *path);
// CM_API void cm_object_set_path(cm_object_t *object, const char *path, cm_object_t *item);
// CM_API void cm_object_clear_path(cm_object_t *object, const char *path);
// CM_API void cm_object_delete_path(cm_object_t *object, const char *path);
// CM_API void cm_object_push_to_path(cm_object_t *object, const char *path, cm_object_t *item);

CM_API void cm_object_set_meta(cm_object_t *object, cm_object_t *meta);
CM_API cm_object_t* cm_object_get_meta(cm_object_t *object);
CM_API void cm_object_set_type(cm_object_t *object, CM_TYPE_ type);
CM_API CM_TYPE_ cm_object_get_type(cm_object_t *object);

CM_API cm_object_t* cm_object_get(cm_object_t *object, const char *name);
CM_API cm_object_t* cm_object_set(cm_object_t *object, const char *name, cm_object_t *item);
CM_API void cm_object_push(cm_object_t *object, int index, cm_object_t *item);
CM_API cm_object_t* cm_object_pop(cm_object_t *object);
CM_API cm_object_t* cm_object_add(cm_object_t *object, const char *name, cm_object_t *item);
CM_API int cm_object_length(cm_object_t *object);

CM_API cm_object_t* cm_vec2_array_from_json(cJSON *const json);

CM_TYPE_DEF(number, float);
CM_TYPE_DEF(string, const char*);
CM_TYPE_DEF(boolean, int);
CM_TYPE_DEF(vec2, CM_VEC2);
CM_TYPE_DEF(vec3, CM_VEC3);
CM_TYPE_DEF(vec4, CM_VEC4);
CM_TYPE_DEF(array, cm_object_t*);
CM_TYPE_DEF(table, cm_object_t*);
CM_TYPE_DEF(userdata, void*);

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
JSON_TYPE(vec2, CM_VEC2);
JSON_TYPE(vec3, CM_VEC3);
JSON_TYPE(vec4, CM_VEC4);
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
#include "coffee.h"

#define CM_META_TYPE "__type__"

#define CM_HEADER_NAME "__header__"
#define CM_DATA_NAME "__data__"
#define CM_META_NAME "__meta__"

#define CM_META_TYPE "__type__"

#define CM_META_COLOR "__color__"
#define CM_META_FORMAT "__format__"
#define CM_META_V_MIN "__v_min__"
#define CM_META_V_MAX "__v_max__"
#define CM_META_SPEED "__speed__"
#define CM_META_RES_TYPE "__res_type__"
#define CM_META_LOCAL_POS "__local_pos__"

#define CHECK_VM(vm) \
  CM_ASSERT(vm != NULL); \
  CM_ASSERT(vm->root != NULL)

cm_vm_t *vm;

const struct {
  const char *name;
  int type;
} type_names[] = {
  {"null", CM_TNULL},
  {"number", CM_TNUMBER},
  {"string", CM_TSTRING},
  {"boolean", CM_TBOOLEAN},
  {"vec2", CM_TVEC2},
  {"vec3",   CM_TVEC3},
  {"vec4",   CM_TVEC4},
  {"array", CM_TARRAY},
  {"table", CM_TTABLE},
  {"custom", CM_TCUSTOM},
  {"userdata", CM_TUSERDATA},
  {"reference", CM_TREFERENCE}
};

static int get_type_from_string(const char *string) {
  CM_ASSERT(vm != NULL);
  cm_object_t *types = cm_object_get(vm->root, CM_META_TYPE);
  int i;
  for (i = 0; i < CM_TMAX; i++) {
    if (!strcmp(type_names[i].name, string)) return type_names[i].type;
  }

  return CM_TNULL;
}

int get_type_from_cjson_type(int cjson_type, cJSON *const json) {
  int type = CM_TNULL;
  switch (cjson_type) {
    case cJSON_Number:
      type = CM_TNUMBER;
      break;
    case cJSON_True:
      type = CM_TBOOLEAN;
      break;
    case cJSON_False:
      type = CM_TBOOLEAN;
      break;
    case cJSON_String:
      type = CM_TSTRING;
      break;
    case cJSON_Object:
      type = CM_TTABLE;
      break;
    case cJSON_Array:
      // {
      //   int sz = -1;
      //   if (json) sz = cJSON_GetArraySize(json);
      //   if (sz == 2) type = CM_TVEC2;
      //   else if (sz == 3) type = CM_TVEC3;
      //   else if (sz == 4) type = CM_TVEC4;
      //   else type = CM_TARRAY;
      // }
      type = CM_TARRAY;
      break;
    default:
      type = CM_TCUSTOM;
  }

  return type;
}

static void cm_get_uid(cm_object_t *object, char *uid) {
  CHECK_VM(vm);
  sprintf(uid, "%s##", object->name);

  cm_object_t *parent = object->parent;
  while (parent) {
    strcat(uid, "_");
    strcat(uid, parent->name);
    parent = parent->parent;
  }
}

const char *read_file(const char *path, const char* mode) {
  if (!path) return NULL;

  FILE *fp = fopen(path, mode);
  if (!fp) return NULL;

  fseek(fp, 0, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *buffer = (char*)malloc(sz);
  fread(buffer, 1, sz, fp);

  fclose(fp);

  return buffer;
}

void write_file(const char *filename, const char *string, size_t size, const char *mode) {
  FILE *fp = fopen(filename, mode);
  if (!fp) return;

  size_t ssize = size > 0 ? size : strlen(string);
  int bytes = fwrite(string, 1, ssize, fp);
  if (!bytes) CM_TRACEERR("Failed to write file: %s", filename);
  fclose(fp);
}


/*===================*
 *        VM         *
 *===================*/

int cm_vm_init() {
  vm = CM_CALLOC(1, sizeof(*vm));
  if (!vm) return 0;

  vm->root = cm_object_create(CM_TTABLE);
  vm->gc = cm_object_create(CM_TTABLE);
  vm->read_file = read_file;
  vm->write_file = write_file;
  cm_object_add_table(vm->root, CM_META_TYPE, NULL);

  return 1;
}

void cm_vm_deinit() {
  cm_object_delete(vm->root);
  cm_object_delete(vm->gc);
  CM_FREE(vm);
}

cm_vm_t* cm_get_vm() {
  CHECK_VM(vm);
  return vm;
}

void cm_vm_gc() {
  CHECK_VM(vm);
  cm_object_clear(vm->gc);
}

cm_object_t *cm_vm_set_type(const char *typename, cm_object_t* type) {
  CHECK_VM(vm);
  if (!typename) return NULL;
  cm_object_t *types = cm_object_get(vm->root, typename);
  if (types) return NULL;

  return cm_object_set(types, typename, type);
}

cm_object_t *cm_vm_get_type(const char *typename) {
  CHECK_VM(vm);
  if (!typename) return NULL;

  cm_object_t *types = cm_object_get(vm->root, CM_META_TYPE);
  cm_object_t *type = cm_object_get(types, typename);

  return type;
}

cm_object_t* cm_vm_get_object(const char *name) {
  if (!name) return NULL;
  const char *part = strstr(name, "//");
  int sz = strlen(name);
  if (part) sz = part - name;

  char key[sz];
  memcpy(key, name, sz);
  cm_object_t *obj = cm_object_get(vm->root, key);
  if (part) return cm_object_get(obj, part+2);

  return NULL;
}

cm_object_t* cm_vm_set_object(const char *name, cm_object_t *object) {
  return NULL;
}

cm_object_t* cm_object_create(CM_T_ type) {
  cm_object_t *object = malloc(sizeof(*object));
  memset(object, 0, sizeof(*object));

  object->next = NULL;
  object->prev = NULL;
  object->child = NULL;
  object->meta = NULL;
  object->ref = NULL;
  object->userdata = NULL;

  object->type = type;

  return object;
}

cm_object_t *cm_object_load(const char *filename) {
  if (!filename) return NULL;
  // cJSON *json = json_open(filename);
  const char *content = vm->read_file(filename, "rb");

  cm_object_t *obj = cm_parse_json(content);
  if (!obj) {
    free((char*)content);
    return NULL;
  }

  // CM_TRACEERR("okay");

  return obj;
}

cm_object_t* cm_vec2_array_from_json(cJSON *const array) {
  if (!array) return NULL;
  cm_object_t *object = cm_create_array(NULL);


  cJSON *item = NULL;
  cJSON_ArrayForEach(item, array) {
    if (json_is_vec2(item)) {
      VEC2_TYPE vec = json_to_vec2(item);
      cm_object_t *v = cm_create_vec2(vec);
      cm_object_push(object, -1, v);
    }
  }

  return object;
}

cm_object_t* cm_object_from_json(cJSON* json) {
  if (!json) return NULL;
  cm_object_t *field = NULL;
  const char *typename = NULL;
  // if (meta_field) typename = cm_object_get_string(meta_field, CM_META_TYPE);

  int type = -1;
  if (typename) type = get_type_from_string(typename);

  if (type == -1) type = get_type_from_cjson_type(json->type, json);

  switch (type) {
    case CM_TNUMBER:
      field = cm_number_from_json(json);
      break;
    case CM_TSTRING:
      field = cm_string_from_json(json);
      break;
    case CM_TBOOLEAN:
      field = cm_boolean_from_json(json);
      break;
    case CM_TVEC2:
      field = cm_vec2_from_json(json);
      break;
    case CM_TVEC3:
      field = cm_vec3_from_json(json);
      break;
    case CM_TVEC4:
      field = cm_vec4_from_json(json);
      break;
    case CM_TARRAY:
      field = cm_array_from_json(json);
      break;
    case CM_TTABLE:
      field = cm_table_from_json(json);
      break;
    case CM_TCUSTOM:
      {
        field = cm_vm_get_type(typename);
        // if (type) {

        // }
        // if (field_type->load) field = field_type->load(json);
        // // TRACELOG("%p", field);
        // if (field) field->field_type = field_type;
      }
      break;
    default:
      CM_TRACELOG("Unknown type");
  }
  // if (field && field->type == CM_TVEC2) CM_TRACEERR("testew %f %f", field->type, field->vec2.data[0], field->vec2.data[1]);

  if (field) field->type = type;
  return field;
}

void cm_object_clear(cm_object_t *object) {
  if (!object) return;
  if (object->next) cm_object_delete(object->next);
  if (object->child) cm_object_delete(object->child);
  if (object->own_meta && object->meta) cm_object_delete(object->meta);
  // if (object->meta) meta_destroy(object->meta);
  object->child = NULL;
  // free(object);
}

void cm_object_delete(cm_object_t *object) {
  if (!object) return;
  cm_object_clear(object);
  CM_FREE(object);
}

cm_object_t *cm_object_get(cm_object_t *field, const char *name) {
  if (!field || !name) return NULL;
  // int sz = 32;
  // if (sz <= 0) return field;
  const char *part = strstr(name, "//");
  // CM_TRACELOG("%s", name);
  if (!part) {
    cm_object_t *item = NULL;
    cm_object_foreach(item, field) {
      if (!strcmp(item->name, name)) {
        return item;
      }
    }
    return NULL;
  }

  size_t sz = part - name;
  char key[sz+1];
  memset(key, 0, sz+1);

  // for (int i = 0; i < sz; i++) {
  //  CM_TRACELOG("%c", name[i]);
  // }

  memcpy(key, name, sz);
  // CM_TRACELOG("%s %d", key, sz);

  cm_object_t *item = NULL;
  cm_object_foreach(item, field) {
    if (!strcmp(item->name, key)) return cm_object_get(item, part+2);
  }

  return NULL;
}

cm_object_t *cm_object_index(cm_object_t *field, int index) {
  if (!field) return NULL;
  if (index < 0) index = cm_object_length(field) + index;
  int i = 0;
  cm_object_t *el = NULL;
  cm_object_foreach(el, field) {
    if (i == index) return el;
    i++;
  }

  return NULL;
}

cm_object_t *cm_object_set(cm_object_t *field, const char *name, cm_object_t *item) {
  if (!field || !name || !item) return NULL;

  cm_object_t *el = NULL;
  cm_object_foreach(el, field) {
    if (!strcmp(el->name, name)) return el;
  }

  strcpy(item->name, name);
  if (!field->child) {
    field->child = item;
    return field->child;
  }

  el = field->child;
  while (el->next) el = el->next;

  el->next = item;
  item->prev = el;
  item->parent = field;

  return item;
}

void cm_object_push(cm_object_t *field, int index, cm_object_t *item) {
  CM_ASSERT(field != NULL);
  if (field->type != CM_TARRAY) return;
  if (!item) return;

  if (!field->child) {
    field->child = item;
    return;
  }

  cm_object_t *el = field->child;
  while (el->next) {
    el = el->next;
  }

  el->next = item;
  item->prev = el;
  item->parent = field;
}

int cm_object_length(cm_object_t *field) {
  if (!field) return -1;

  int len = 0;
  cm_object_t *el = NULL;
  cm_object_foreach(el, field) len++;

  return len;
}

void cm_object_update_meta(cm_object_t *obj) {
  CM_ASSERT(obj != NULL);
  if (!obj->meta) return;
  
  cm_object_t *meta = obj->meta;
  cm_object_t *typef = cm_object_get(meta, CM_META_TYPE);
  if (typef) obj->type = get_type_from_string(typef->string);

  cm_object_t *el = NULL;
  cm_object_foreach(el, obj) {
    cm_object_t *metac = cm_object_get(meta, el->name);
    if (metac) cm_object_update_meta(el);
  }
}

void cm_object_set_meta(cm_object_t *obj, cm_object_t *meta) {
  CM_ASSERT(obj != NULL);
  if (!meta) return;
  // if (obj->meta) cm_object_add(vm->gc, meta->name, obj->meta);

  // obj->meta = meta;
  obj->meta = meta;
  cm_object_t *el = NULL;
  cm_object_foreach(el, obj) {
    cm_object_t *metac = cm_object_get(meta, el->name);
    if (metac) cm_object_set_meta(el, metac);
  }
}

cm_object_t* cm_object_get_meta(cm_object_t *obj) {
  CM_ASSERT(obj != NULL);

  return obj->meta;
}

/*================*
 *     Number     *
 *================*/

cm_object_t* cm_create_number(float number) {
  cm_object_t *field = cm_object_create(CM_TNUMBER);
  if (!field) return NULL;

  cm_object_set_number(field, number);

  return field;
}

cm_object_t *cm_number_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_Number) return NULL;

  float number = json_to_number(json);
  return cm_create_number(number);
}

void cm_object_set_number(cm_object_t *field, float number) {
  if (!field) return;
  if (field->type != CM_TNUMBER) return;

  field->number = number;
}

float cm_object_to_number(cm_object_t *object) {
  CM_ASSERT(object != NULL);
  CM_ASSERT(object->type != CM_TNUMBER);

  return cm_object_to_opt_number(object, -1);
}

float cm_object_to_opt_number(cm_object_t *field, float opt) {
  if (!field) return opt;
  if (field->type != CM_TNUMBER) return opt;

  return field->number;
}

float cm_object_get_number(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_number(field, name, -1);
}

float cm_object_get_opt_number(cm_object_t *field, const char *name, float opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_number(item, opt);
}

cm_object_t *cm_object_add_number(cm_object_t *field, const char *name, float number) {
  if (!field) return NULL;
  if (!name) return NULL;

  cm_object_t *n_field = cm_create_number(number);
  cm_object_set(field, name, n_field);

  return n_field;
}

/*****************
 * String
 *****************/

cm_object_t* cm_create_string(const char *string) {
  cm_object_t *field = cm_object_create(CM_TSTRING);
  if (!field) return NULL;

  cm_object_set_string(field, string);

  return field;
}

cm_object_t *cm_string_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_String) return NULL;

  const char *string = json_to_string(json);
  return cm_create_string(string);
}

void cm_object_set_string(cm_object_t *field, const char *string) {
  if (!field) return;
  if (field->type != CM_TSTRING) return;

  // field->string = string;
  strcpy(field->string, string);
}


const char* cm_object_to_string(cm_object_t *field) {
  CM_ASSERT(field != NULL);
  CM_ASSERT(field->type == CM_TSTRING);

  return cm_object_to_opt_string(field, NULL);
}

const char* cm_object_to_opt_string(cm_object_t *field, const char* opt) {
  if (!field) return opt;
  if (field->type != CM_TSTRING) return opt;

  return field->string;
}

const char* cm_object_get_string(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_string(field, name, NULL);
}

const char* cm_object_get_opt_string(cm_object_t *field, const char *name, const char* opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_string(item, opt);
}

cm_object_t *cm_object_add_string(cm_object_t *field, const char *name, const char *string) {
  if (!field) return NULL;
  if (!name) return NULL;

  cm_object_t *n_field = cm_create_string(string);
  cm_object_set(field, name, n_field);

  return n_field;
}


/*****************
 * Boolean
 *****************/

cm_object_t* cm_create_boolean(int boolean) {
  cm_object_t *field = cm_object_create(CM_TBOOLEAN);
  if (!field) return NULL;

  cm_object_set_boolean(field, boolean);

  return field;
}

cm_object_t *cm_boolean_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (!cJSON_IsBool(json)) return NULL;

  int boolean = json_to_boolean(json);
  return cm_create_boolean(boolean);
}

void cm_object_set_boolean(cm_object_t *obj, int boolean) {
  if (!obj) return;
  if (obj->type != CM_TBOOLEAN) return;

  obj->boolean = boolean;
}

int cm_object_to_boolean(cm_object_t *object) {
  CM_ASSERT(object != NULL);
  CM_ASSERT(object->type != CM_TBOOLEAN);

  return cm_object_to_opt_boolean(object, 0);
}

int cm_object_to_opt_boolean(cm_object_t *object, int opt) {
  if (!object) return opt;
  if (object->type != CM_TBOOLEAN) return opt;

  return object->boolean;
}

int cm_object_get_boolean(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_boolean(field, name, 0);
}

int cm_object_get_opt_boolean(cm_object_t *field, const char *name, int opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_boolean(item, opt);
}

cm_object_t *cm_object_add_boolean(cm_object_t *field, const char *name, int boolean) {
  if (!field) return NULL;
  if (!name) return NULL;

  cm_object_t *n_field = cm_create_boolean(boolean);
  cm_object_set(field, name, n_field);

  return n_field;
}

/*****************
 * Vec2
 *****************/

cm_object_t* cm_create_vec2(VEC2_TYPE vec) {
  cm_object_t *field = cm_object_create(CM_TVEC2);
  if (!field) return NULL;

  cm_object_set_vec2(field, vec);

  return field;
}

cm_object_t *cm_vec2_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_Array) return NULL;

  VEC2_TYPE vec = json_to_vec2(json);
  // cJSON *item = json->child;
  // CM_TRACELOG("%s %f %f", json->string, vec.data[0], vec.data[1]);

  return cm_create_vec2(vec);
}

void cm_object_set_vec2(cm_object_t *field, VEC2_TYPE vec) {
  if (!field) return;
  if (field->type != CM_TVEC2) return;

  field->vec2 = vec;
}

VEC2_TYPE cm_object_to_vec2(cm_object_t *field) {
  CM_ASSERT(field != NULL);
  CM_ASSERT(field->type == CM_TVEC2);

  return cm_object_to_opt_vec2(field, cm_vec2(-1, -1));
}

VEC2_TYPE cm_object_to_opt_vec2(cm_object_t *field, VEC2_TYPE opt) {
  if (!field) return opt;
  if (field->type != CM_TVEC2) return opt;

  return field->vec2;
}

VEC2_TYPE cm_object_get_vec2(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_vec2(field, name, cm_vec2(-1, -1));
}

VEC2_TYPE cm_object_get_opt_vec2(cm_object_t *field, const char *name, VEC2_TYPE opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_vec2(item, opt);
}

cm_object_t *cm_object_add_vec2(cm_object_t *field, const char *name, VEC2_TYPE vec2) {
  if (!field) return NULL;
  if (!name) return NULL;

  cm_object_t *n_field = cm_create_vec2(vec2);
  cm_object_set(field, name, n_field);

  return n_field;
}

/*****************
 * Vec3
 *****************/

cm_object_t* cm_create_vec3(VEC3_TYPE vec) {
  cm_object_t *field = cm_object_create(CM_TVEC3);
  if (!field) return NULL;

  cm_object_set_vec3(field, vec);

  return field;
}

cm_object_t *cm_vec3_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_Array) return NULL;

  VEC3_TYPE vec = json_to_vec3(json);
  return cm_create_vec3(vec);
}

void cm_object_set_vec3(cm_object_t *field, VEC3_TYPE vec) {
  if (!field) return;
  if (field->type != CM_TVEC3) return;

  field->vec3 = vec;
}

VEC3_TYPE cm_object_to_vec3(cm_object_t *field) {
  CM_ASSERT(field != NULL);
  CM_ASSERT(field->type != CM_TVEC3);

  return cm_object_to_opt_vec3(field, cm_vec3(-1, -1, -1));
}

VEC3_TYPE cm_object_to_opt_vec3(cm_object_t *field, VEC3_TYPE opt) {
  if (!field) return opt;
  if (field->type != CM_TVEC3) return opt;

  return field->vec3;
}

VEC3_TYPE cm_object_get_vec3(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_vec3(field, name, cm_vec3(-1, -1, -1));
}

VEC3_TYPE cm_object_get_opt_vec3(cm_object_t *field, const char *name, VEC3_TYPE opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_vec3(item, opt);
}

cm_object_t *cm_object_add_vec3(cm_object_t *field, const char *name, VEC3_TYPE vec3) {
  if (!field) return NULL;
  if (!name) return NULL;

  cm_object_t *n_field = cm_create_vec3(vec3);
  cm_object_set(field, name, n_field);

  return n_field;
}

/*****************
 * Vec4
 *****************/

cm_object_t* cm_create_vec4(VEC4_TYPE vec) {
  cm_object_t *field = cm_object_create(CM_TVEC4);
  if (!field) return NULL;

  cm_object_set_vec4(field, vec);

  return field;
}

cm_object_t *cm_vec4_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_Array) return NULL;

  VEC4_TYPE vec = json_to_vec4(json);
  return cm_create_vec4(vec);
}

void cm_object_set_vec4(cm_object_t *field, VEC4_TYPE vec) {
  if (!field) return;
  if (field->type != CM_TVEC4) return;

  field->vec4 = vec;
}

VEC4_TYPE cm_object_to_vec4(cm_object_t *field) {
  CM_ASSERT(field != NULL);

  return cm_object_to_opt_vec4(field, cm_vec4(-1, -1, -1, -1));
}

VEC4_TYPE cm_object_to_opt_vec4(cm_object_t *field, VEC4_TYPE opt) {
  if (!field) return opt;
  if (field->type != CM_TVEC4) return opt;

  return field->vec4;
}

VEC4_TYPE cm_object_get_vec4(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_vec4(field, name, cm_vec4(-1, -1, -1, -1));
}

VEC4_TYPE cm_object_get_opt_vec4(cm_object_t *field, const char *name, VEC4_TYPE opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_vec4(item, opt);
}

cm_object_t *cm_object_add_vec4(cm_object_t *field, const char *name, VEC4_TYPE vec4) {
  if (!field) return NULL;
  if (!name) return NULL;

  cm_object_t *n_field = cm_create_vec4(vec4);
  cm_object_set(field, name, n_field);

  return n_field;
}

/*****************
 * Array
 *****************/

cm_object_t* cm_create_array(cm_object_t *ref) {
  cm_object_t *field = cm_object_create(CM_TARRAY);
  if (!field) return NULL;

  // cm_object_set_vec2(field, vec);
  // if (meta)

  return field;
}

cm_object_t *cm_array_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_Array) return NULL;

  cm_object_t *field = cm_create_array(NULL);


  cJSON *item = NULL;
  cJSON_ArrayForEach(item, json) {
    cm_object_t *aux = cm_object_from_json(item);

    if (aux) cm_object_push(field, -1, aux);
  }

  return field;
}

cm_object_t *cm_object_to_array(cm_object_t *field) {
  CM_ASSERT(field != NULL);
  CM_ASSERT(field->type != CM_TARRAY);

  return cm_object_to_opt_array(field, NULL);
}

void cm_object_set_array(cm_object_t *field, cm_object_t *array) {

}

cm_object_t *cm_object_to_opt_array(cm_object_t *field, cm_object_t *opt) {
  if (!field) return opt;
  if (field->type != CM_TARRAY) return opt;

  return NULL;
}

cm_object_t* cm_object_get_array(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_array(field, name, NULL);
}

cm_object_t* cm_object_get_opt_array(cm_object_t *field, const char *name, cm_object_t* opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_array(item, opt);
}

cm_object_t* cm_object_add_array(cm_object_t *field, const char *name, cm_object_t *array) {
  if (!field) return NULL;
  cm_object_push(field, 0, array);

  return array;
}

/*****************
 * Object
 *****************/

cm_object_t* cm_create_table(cm_object_t *ref) {
  cm_object_t *field = cm_object_create(CM_TTABLE);
  if (!field) return NULL;

  return field;
}

cm_object_t *cm_table_from_json(cJSON *const json) {
  if (!json) return NULL;
  if (json->type != cJSON_Object) return NULL;

  cm_object_t *object = cm_create_table(NULL);
  cm_object_t *meta = NULL;

  cJSON *json_meta = cJSON_GetObjectItem(json, CM_META_NAME);
  if (json_meta) {
    meta = cm_object_from_json(json_meta);
    cm_object_set_meta(object, meta);
  }

  cJSON *item = NULL;
  cJSON_ArrayForEach(item, json) {
    if (!strcmp(item->string, CM_META_NAME)) continue;
    if (!strcmp(item->string, CM_HEADER_NAME)) continue;

    cm_object_t *meta_field = NULL;

    if (meta) meta_field = cm_object_get(meta, item->string);
    cm_object_t *aux = cm_object_from_json(item);

    if (aux) cm_object_set(object, item->string, aux);
  }

  return object;
}

void cm_object_set_table(cm_object_t *field, cm_object_t *object) {}

cm_object_t *cm_object_to_object(cm_object_t *field) {
  CM_ASSERT(field != NULL);
  CM_ASSERT(field->type != CM_TTABLE);

  return cm_object_to_opt_table(field, NULL);
}

cm_object_t *cm_object_to_opt_table(cm_object_t *field, cm_object_t *opt) {
  if (!field) return opt;
  if (field->type != CM_TTABLE) return opt;

  return field;
}

cm_object_t* cm_object_get_table(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_table(field, name, NULL);
}

cm_object_t* cm_object_get_opt_table(cm_object_t *field, const char *name, cm_object_t* opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_table(item, opt);
}

cm_object_t* cm_object_add_table(cm_object_t *field, const char *name, cm_object_t *object) {
  if (!field) return NULL;

  if (!object) object = cm_create_table(NULL);
  return cm_object_set(field, name, object);
}

/*=================
 *    Userdata
 *=================*/

cm_object_t* cm_create_userdata(void *userdata) {
  cm_object_t *field = cm_object_create(CM_TUSERDATA);
  if (!field) return NULL;

  field->userdata = userdata;

  return field;
}

cm_object_t *cm_userdata_from_json(cJSON *const json) {
  return NULL;
}

void cm_object_set_userdata(cm_object_t *field, void *userdata) {}

void *cm_object_to_userdata(cm_object_t *field) {
  CM_ASSERT(field != NULL);
  CM_ASSERT(field->type != CM_TUSERDATA);

  return cm_object_to_opt_userdata(field, NULL);
}

void *cm_object_to_opt_userdata(cm_object_t *field, void *opt) {
  if (!field) return opt;
  if (field->type != CM_TUSERDATA) return opt;

  return field;
}

void* cm_object_get_userdata(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_userdata(field, name, NULL);
}

void* cm_object_get_opt_userdata(cm_object_t *field, const char *name, void* opt) {
  if (!field) return opt;
  cm_object_t *item = cm_object_get(field, name);
  return cm_object_to_opt_userdata(item, opt);
}

cm_object_t* cm_object_add_userdata(cm_object_t *field, const char *name, void *userdata) {
  if (!field) return NULL;

  cm_object_t *userdataf = cm_create_userdata(userdata);

  return cm_object_set(field, name, userdataf);
}

/*================*
 *      JSON      *
 *================*/

static const char* ignore_chars(const char *str) {
  while (*str == '\r' || *str == 10 || *str == '\t' || *str == '\n' || *str == ' ') str++;
  return str;
}

static int next_char(const char *str, char ch) {
  if (!str) return -1;

  int i = 0;
  while (*str != ch && *str  != '\0') {
    str++;
    i++;
  }

  // CM_TRACELOG("%s", str);
  return (*str == '\0') ? -1 : i;
}

static char *check_content(const char *json_str, int *len) {
  if (!json_str) return NULL;

  char *init = json_str;
  init = ignore_chars(init);
  const char tokens[] = {',', ' ', '\n', '\r', ']', '}'};
  // if (i)

  // for (int i = 0; i < 3; i++) {
  //   int ind = next_char(p, tokens[i]);
  //   // CM_TRACELOG("%d", ind);
  //   if (ind > 0) {
  //     *len = ind;
  //     return p;
  //   }
  // }
  int index = 0;
  char *p = init;
  while (p) {
    for (int i = 0; i < 5; i++) {
      if (*p == tokens[i]) {
        if (len) *len = index;
        return init;
      }
    }
    p++;
    index++;
  }
  CM_TRACEERR("Bad formating: %s", p);
  exit(1);

  return NULL;
}

static int char_is_number(char p) {
  int n = p - 48;

  return n >= 0 || n <= 9;
}

static int str_is_boolean(char *p) {
  CM_ASSERT(p != NULL);
  char val[6];

  if (*p != 't') {
    memcpy(val, p, 4);
    return !strcmp(val, "true");
  }

  if (*p != 'f') {
    memcpy(val, p, 5);
    return !strcmp(val, "false");
  }

  return 0;
}

static char *check_number(const char *json_str, int *len) {
  if (!json_str) return NULL;
  char *p = (char*)json_str;

  p = (char*)ignore_chars(p);

  if (char_is_number(*p)) {
    int i = 0;
    char *val = check_content(p, &i);
    if (len) *len = i;
    return p;
  }

  return NULL;
}

static char *check_string(const char *json_str, int *len) {
  if (!json_str) return NULL;
  char *p = json_str;
  
  p = ignore_chars(p);
  if (*p == '"') {
    p++;
    int i = next_char(p, '"');
    // CM_TRACELOG("%s", p+i);
    if (i < 1) return NULL;
    if (len) *len = i;
    return p;
  }

  return NULL;
}

// static char *get_string
static cm_object_t *parse_json_null(const char **json_str) {
  CM_ASSERT(json_str != NULL);

}

static cm_object_t *parse_json_boolean(const char **json_str) {
  CM_ASSERT(json_str != NULL);

}



static cm_object_t *parse_json_number(const char **json_str) {
  CM_ASSERT(json_str != NULL);


  char *init = (char*)*json_str;
  int len = 0;


  char *num = check_number(init, &len);
  // num = ignore_chars(num);
  if (!num || len <= 0) return NULL;

  char val[len+1];
  memcpy(val, num, len);
  val[len] = '\0';
  // CM_TRACEERR("testando");
  // float v = strtof(val, NULL);

  cm_object_t *o_num = cm_object_create(CM_TNUMBER);
  if (strchr(val, '.')) {
    float v = atof(val);
    // CM_TRACELOG("%f", v);
    cm_object_set_number(o_num, v);
  } else {
    int v = atoi(val);
    o_num->number = v;
  }
  // CM_TRACELOG("%d %s", len, val);

  // o_num =
  // CM_TRACELOG("%f", o_num->number);
  *json_str += len+1;

  return o_num;
}

static cm_object_t *parse_json_string(const char **json_str) {
  CM_ASSERT(json_str != NULL);
  char *init = (char*)*json_str;
  int len = 0;

  char *str = check_string(init, &len);
  if (!str || len <= 0) return NULL;


  cm_object_t *o_str = cm_object_create(CM_TSTRING);
  memcpy(o_str->string, str, len);
  // CM_TRACELOG("%s", str);


  *json_str = str+len+2;

  return o_str;
}

static cm_object_t *parse_json_object(const char **json_str);
static cm_object_t *parse_json_array(const char **json_str);

cm_object_t *parse_json_array(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;
  cm_object_t *obj = NULL;

  char *p = init;
  if (*p == '[') {
    p++;

    obj = cm_object_create(CM_TARRAY);
    while (*p != ']') {
      p = ignore_chars(p);
      if (*p == ',') p++;

      // CM_TRACELOG("%s", p);

      cm_object_t *child = NULL;
      if (*p == '{') {
        child = parse_json_object(&p);
      } else if (*p == '"') {
        child = parse_json_string(&p);
      } else if (*p == '[') {
        child = parse_json_array(&p);
      } else if (char_is_number(*p)) {
        child = parse_json_number(&p);
      } else {

      }
      // CM_
      p = ignore_chars(p);

      // CM_TRACELOG("%p %s", child, p);
      CM_ASSERT(child != NULL);
      // cm_object_set(obj, key, child);
      cm_object_push(obj, -1, child);
    }
    p++;
    *json_str = p;
  }

  return obj;
}

cm_object_t *parse_json_object(const char **json_str) {
  CM_ASSERT(json_str != NULL);
  char *init = (char*)*json_str;
  cm_object_t *obj = NULL;

  char *p = init;
  if (*p == '{') {
    p++;
    obj = cm_object_create(CM_TTABLE);
    // p = (char*)
    while (*p != '}') {
      p = (char*)ignore_chars(p);
      if (*p == ',') p++;

      int len = 0;
      // CM_TRACELOG("%s", p);
      char *name = check_string(p, &len);
      CM_ASSERT(name != NULL);
      CM_ASSERT(len > 0);

      p = name + len + 1;

      if (*p != ':') {
        CM_TRACELOG("missing object key: %s", --name);
        exit(1);
      }
      char key[len+1];
      memcpy(key, name, len);
      key[len] = '\0';
      p++;

      p = (char*)ignore_chars(p);

      cm_object_t *child = NULL;
      if (*p == '{') {
        child = parse_json_object(&p);
      } else if (*p == '"') {
        child = parse_json_string(&p);
      } else if (*p == '[') {
        child = parse_json_array(&p);
      } else if (char_is_number(*p)) {
        child = parse_json_number(&p);
      } else {

      }

      // CM_TRACELOG("%p %s", child, p);
      CM_ASSERT(child != NULL);
      // CM_TRACELOG("addzando %s", p);
      cm_object_set(obj, key, child);

      p = ignore_chars(p);

      // if (title)
    }
    p++;
    *json_str = p;
  }

  return obj;
}

cm_object_t* cm_parse_json(const char *json_string) {
  CM_ASSERT(json_string != NULL);

  const char *str = json_string;
  cm_object_t *root = parse_json_object(&json_string);

  return root;
}

const char* cm_print_json(cm_object_t *object) {

}


int json_is_valid(const char *filename) {
  CM_ASSERT(vm->read_file != NULL);
  const char *content = vm->read_file(filename, "rb");

  cJSON *parsed = cJSON_Parse(content);
  free((char*)content);
  if (!parsed) return 0;

  return 1;
}

cJSON *json_open(const char *filename) {
  CM_ASSERT(vm->read_file != NULL);
  const char *content = vm->read_file(filename, "rb");
  cm_parse_json(content);
  cJSON *parsed = cJSON_Parse((const char*)content);
  free((char*)content);
  if (!parsed) {
    const char *err = cJSON_GetErrorPtr();
    if (err != NULL) {
      CM_TRACEERR("Failed to load json '%s': error before %s", filename, err);
    }
  }
  return parsed;
}
cJSON *json_parse(const char *jsonStr) {
  cJSON *parsed = cJSON_Parse(jsonStr);
  if (!parsed) {
    const char *err = cJSON_GetErrorPtr();
    if (err != NULL) {
      CM_TRACEERR("Failed to parse json: error before %s", err);
    }
  }
  return parsed;
}

cJSON* json_clone(cJSON *src) {
  CM_ASSERT(src != NULL);
  const char *json_str = json_print(src);
  if (!json_str) return NULL;

  return json_parse(json_str);
}

void json_save(const char *filename, cJSON* const json) {
  char *string = cJSON_Print(json);
  size_t size = strlen(string);
  if (!string) {
    CM_TRACEERR("Failed to save json: '%s'", filename);
    return;
  }
  vm->write_file(filename, string, size, "wb");
}

char *json_print(cJSON* const json) {
  return cJSON_Print(json);
}

cJSON* json_create() {
  return cJSON_CreateObject();
}

// cJSON* json_create_array(const cJSON*) {
//   return cJSON_CreateArray();
// }

void json_delete(cJSON* json) {
  cJSON_Delete(json);
}

int json_add_item(cJSON *const json, const char *name, cJSON *const item) {
  CM_ASSERT(json != NULL);
  // cJSON *string = cJSON_CreateString(value);
  if (json_is_object(json)) {
    CM_ASSERT(name != NULL);
    CM_ASSERT(item != NULL);
    cJSON_AddItemToObject(json, name, item);
    return 1;
  }
  if (json_is_array(json)) {
    CM_ASSERT(item != NULL);
    cJSON_AddItemToArray(json, item);
    return 1;
  }

  return 0;
}

cJSON* json_get_item(cJSON *const json, const char *name, int index) {
  CM_ASSERT(json != NULL);

  if (json_is_object(json)) {
    CM_ASSERT(name != NULL);
    cJSON *item = cJSON_GetObjectItem(json, name);
    return item;
  }

  if (json_is_array(json)) {
    CM_ASSERT(index >= 0);
    cJSON *item = cJSON_GetArrayItem(json, index);
    return item;
  }

  return NULL;
}

/***********
 * String
 ***********/

cJSON *json_create_string(const char *string) {
  CM_ASSERT(string != NULL);
  return cJSON_CreateString(string);
}

int json_is_string(cJSON* const json) {
  // cJSON *jsonName = cJSON_GetObjectItem(json, name);
  CM_ASSERT(json != NULL);
  return cJSON_IsString(json);
}

void json_set_string(cJSON* const json, const char* value) {
  if (!json_is_string(json)) return;

  cJSON_SetValuestring(json, value);
}

const char* json_to_string(cJSON* const json) {
  return json_opt_string(json, NULL);
}

const char* json_opt_string(cJSON* const json, const char* opt) {
  if (!json_is_string(json)) return opt;
  return json->valuestring;
}

cJSON *json_add_string(cJSON* const json, const char *name, const char* value) {
  cJSON *string = cJSON_CreateString(value);
  if (json_add_item(json, name, string)) return string;

  return NULL;
}

const char *json_get_string(cJSON* const json, const char *name, int index) {
  return json_get_opt_string(json, name, index, NULL);
}

const char *json_get_opt_string(cJSON* const json, const char *name, int index, const char *opt_string) {
  // if (!json_is_object(json)) return opt_string;
  // cJSON *jsonName = cJSON_GetObjectItem(json, name);
  // return json_opt_string(jsonName, opt_string);
  cJSON *item = json_get_item(json, name, index);
  if (!item) return opt_string;
  return json_opt_string(item, opt_string);
}


/**********
 * Number
 **********/

cJSON* json_create_number(float number) {
  return cJSON_CreateNumber(number);
}

int json_is_number(cJSON* const json) {
  // cJSON *value = cJSON_GetObjectItem(json, name);
  // if (value) return cJSON_IsNumber(value);
  // return tc_false;
  CM_ASSERT(json != NULL);
  return cJSON_IsNumber(json);
}

void json_set_number(cJSON* const json, float value) {
  if (!json_is_number(json)) return;
  cJSON_SetNumberValue(json, value);
}

float json_to_number(cJSON *const json) {
  return json_opt_number(json, -1);
}

float json_opt_number(cJSON *const json, float opt) {
  if (!json_is_number(json)) return opt;
  // CM_TRACELOG("qqqqqq %f", json->valuedouble);

  return json->valuedouble;
}

cJSON *json_add_number(cJSON *const json, const char *name, float number) {
  cJSON *value = cJSON_CreateNumber(number);
  if (json_add_item(json, name, value)) return value;

  return NULL;
}

float json_get_number(cJSON* const json, const char *name, int index) {
  return json_get_opt_number(json, name, index, -1);
}
float json_get_opt_number(cJSON* const json, const char *name, int index, float optVal) {
  cJSON *item = json_get_item(json, name, index);
  if (!item) return optVal;
  return json_opt_number(item, optVal);
}

/************
 * Vectors
 ************/

static cJSON *json_create_vecn(float *data, int n) {
  CM_ASSERT(data != NULL);
  CM_ASSERT(n > 0);
  cJSON *array = json_create_array(NULL);
  for (int i = 0; i < n; i++) {
    cJSON *item = json_create_number(data[i]);
    cJSON_AddItemToArray(array, item);
  }

  return array;
}

static int json_is_vecn(cJSON *const json, int n) {
  CM_ASSERT(json != NULL);
  CM_ASSERT(n > 0);
  if (!json_is_array(json)) return 0;
  if (cJSON_GetArraySize(json) < n) return 0;
  // CM_TRACELOG("testandow");

  return 1;
}

// static void json_set_vecn(cJSON *const json, float *data, int n) {
//   CM_ASSERT(json != NULL);
//   if (!json_is_vecn(json, n)) return;
//   // CM_ASSERT(json != NULL);
//   CM_ASSERT(data != NULL);

//   for (int i = 0; i < n; i++) {
//     // cJSON *item = json_get_item(json, NULL, i);
//     cJSON *item = cJSON_GetArrayItem(json, i);
//     if (!item) return;
//     // TRACELOG("%d: %d", i, data[i]);
//     // item->valuedouble = data[i];
//     json_set_number(item, data[i]);
//   }
// }

// static float* json_opt_vecn(cJSON *const json, int n, float *opt) {

// }

// TRACELOG("%d: %f %f", i, vec.data[i], item->valuedouble);

#define JSON_VECN(n) \
cJSON *json_create_vec##n (VEC##n##_TYPE vec) { \
  return json_create_vecn(vec.data, n); \
} \
int json_is_vec##n (cJSON *const json) { \
  return json_is_vecn(json, n); \
} \
void json_set_vec##n (cJSON *const json, VEC##n##_TYPE vec) { \
  CM_ASSERT(json != NULL); \
  if (!json_is_vec##n(json)) return; \
  for (int i = 0; i < n; i++) { \
    cJSON *item = json_get_item(json, NULL, i); \
    if (!item) return; \
    json_set_number(item, vec.data[i]); \
  } \
} \
VEC##n##_TYPE json_to_vec##n (cJSON *const json) { \
  CM_ASSERT(json != NULL); \
  VEC##n##_TYPE opt = {-1}; \
  return json_opt_vec##n (json, opt); \
} \
VEC##n##_TYPE json_opt_vec##n (cJSON *const json, VEC##n##_TYPE opt) { \
  if (!json) return opt; \
  if (!json_is_vecn(json, n)) return opt; \
  for (int i = 0; i < n; i++) { \
    opt.data[i] = json_get_number(json, NULL, i); \
  } \
  return opt; \
} \
cJSON *json_add_vec##n (cJSON *const json, const char *name, VEC##n##_TYPE vec) { \
  cJSON *value = json_get_item(json, name, 0); \
  if (value) { \
    json_set_vec##n(value, vec); \
    return value; \
  } \
  value = json_create_vec##n(vec); \
  if (json_add_item(json, name, value)) return value; \
  json_delete(value); \
  return NULL; \
} \
VEC##n##_TYPE json_get_vec##n (cJSON *const json, const char *name, int index) { \
  VEC##n##_TYPE opt; \
  memset(&opt, -1, sizeof(VEC##n##_TYPE)); \
  return json_get_opt_vec##n (json, name, index, opt); \
} \
VEC##n##_TYPE json_get_opt_vec##n (cJSON *const json, const char *name, int index, VEC##n##_TYPE opt) { \
  cJSON *item = json_get_item(json, name, index); \
  if (!item) return opt; \
  return json_opt_vec##n(item, opt); \
}

JSON_VECN(2);
JSON_VECN(3);
JSON_VECN(4);

/***********
 * Boolean
 ***********/

cJSON* json_create_boolean(int boolean) {
  return cJSON_CreateBool(boolean);
}

int json_is_boolean(cJSON* const json) {
  // cJSON *val = cJSON_GetObjectItem(json, name);
  CM_ASSERT(json != NULL);
  return cJSON_IsBool(json);
}

void json_set_boolean(cJSON *const json, int boolean) {
  if (!json_is_boolean(json)) return;

  json->valueint = boolean;
}

int json_to_boolean(cJSON* const json) {
  return json_opt_boolean(json, -1);
}

int json_opt_boolean(cJSON *const json, int opt) {
  if (!json_is_boolean(json)) return opt;

  return json->valueint;
}


cJSON *json_add_boolean(cJSON *const json, const char *name, int boolean) {
  cJSON *item = json_create_boolean(boolean);
  if (json_add_item(json, name, item)) return item;

  json_delete(item);
  return NULL;
}

int json_get_boolean(cJSON* const json, const char *name, int index) {
  return json_get_opt_boolean(json, name, index, -1);
}
int json_get_opt_boolean(cJSON* const json, const char *name, int index, int opt_boolean) {
  cJSON *item = json_get_item(json, name, index);
  if (!item) return opt_boolean;

  return json_opt_boolean(item, opt_boolean);
}

/***********
 * Array
 ***********/

cJSON* json_create_array(const cJSON *json) {
  cJSON *item = cJSON_CreateArray();
  return item;
}

int json_is_array(cJSON* const json) {
  CM_ASSERT(json != NULL);
  return cJSON_IsArray(json);
}

void json_set_array(cJSON* const json, const cJSON* jsonArray) {

}

const cJSON* json_to_array(cJSON *const json) {
  return json_opt_array(json, NULL);
}

const cJSON* json_opt_array(cJSON *const json, const cJSON* opt) {
  if (!json_is_array(json)) return opt;

  return json;
}

cJSON* json_add_array(cJSON *const json, const char *name, const cJSON *array) {
  if (!array) {
    cJSON *item = json_create_array(NULL);
    json_add_item(json, name, item);
    return item;
  }

  if (!json_is_array((cJSON*)array)) return NULL;
  json_add_item(json, name, (cJSON*)array);
  return (cJSON*)array;
}

const cJSON* json_get_array(cJSON* const json, const char *name, int index) {
  cJSON *array = cJSON_GetObjectItem(json, name);
  if (cJSON_IsArray(array)) {
    return array;
  }
  return NULL;
}

const cJSON* json_get_opt_array(cJSON *const json, const char *name, int index, const cJSON *opt) {
  cJSON *item = json_get_item(json, name, index);
  if (!item) return opt;

  return json_opt_array(item, opt);
}



int json_get_array_size(const cJSON *json) {
  return cJSON_GetArraySize(json);
}

/*************
 * Object
 *************/

cJSON *json_create_object(const cJSON* json) {
  cJSON *item = cJSON_CreateObject();
  return item;
}

int json_is_object(cJSON* const json) {
  // cJSON *obj = cJSON_GetObjectItem(json, name);
  // if (obj) return cJSON_IsObject(obj);
  // return tc_false;
  CM_ASSERT(json != NULL);
  return cJSON_IsObject(json);
}

void json_set_object(cJSON *const json, const cJSON *value) {

}

const cJSON *json_to_object(cJSON *const json) {
  return json_opt_object(json, NULL);
}

const cJSON *json_opt_object(cJSON *const json, const cJSON *opt) {
  if (!json_is_object(json)) return opt;

  return json;
}

cJSON *json_add_object(cJSON *const json, const char *name, const cJSON *value) {
  if (!value) {
    cJSON *item = json_create_object(NULL);
    json_add_item(json, name, item);
    return item;
  }

  if (!json_is_object((cJSON*)value)) return NULL;
  json_add_item(json, name, (cJSON*)value);
  return (cJSON*)value;
}

const cJSON* json_get_object(cJSON* const json, const char *name, int index) {
  // cJSON *obj = cJSON_GetObjectItem(json, name);
  // if (cJSON_IsObject(obj)) return obj;
  // return NULL;
  return json_get_opt_object(json, name, index, NULL);
}

const cJSON* json_get_opt_object(cJSON *const json, const char *name, int index, const cJSON *opt) {
  cJSON *item = json_get_item(json, name, index);
  if (!item) return opt;

  return json_opt_object(item, opt);
}

/*===============*
 *     Debug     *
 *===============*/

#include <time.h>
#include <stdarg.h>

void cm_log(int type, const char *fmt, ...) {
  time_t t = time(NULL);
  struct tm *tm_now;

  va_list args;

  char err[15] = "";
  if (type == 1) sprintf((char*)err, "ERROR: ");
  // char buffer[1024];
  // char bufmsg[512];

  tm_now = localtime(&t);
  char buf[10];
  strftime((char*)buf, sizeof(buf), "%H:%M:%S", tm_now);
  fprintf(stderr, "%s %s", buf, err);
  va_start(args, fmt);
  vfprintf(stderr, (const char*)fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void cm_tracelog(int type, const char *file, const char *function, int line, const char *fmt, ...) {
  time_t t = time(NULL);
  struct tm *tm_now;

  va_list args;

  char err[15] = "";
  if (type == 1) sprintf((char*)err, "ERROR in ");
  // char buffer[1024];
  // char bufmsg[512];

  tm_now = localtime(&t);
  char buf[10];
  strftime((char*)buf, sizeof(buf), "%H:%M:%S", tm_now);
  fprintf(stderr, "%s %s:%d %s%s(): ", buf, file, line, err, function);
  // sprintf(buffer, "%s %s:%d %s%s(): ", buf, file, line, err, function);
  va_start(args, fmt);
  // vsprintf(bufmsg, (const char*)fmt, args);
  vfprintf(stderr, (const char*)fmt, args);
  // fprintf(stderr, "%s", bufmsg);
  va_end(args);
  fprintf(stderr, "\n");
  // input_ok_internal
  // strcat(buffer, bufmsg);
  // tc_editor_write_log(buffer);
}
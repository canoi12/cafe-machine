#include "coffee.h"

// #define CM_META_TYPE "__type__"

// #define CM_HEADER_NAME "__header__"
// #define CM_DATA_NAME "__data__"
// #define CM_META_COLOR "__color__"
// #define CM_META_FORMAT "__format__"
// #define CM_META_V_MIN "__v_min__"
// #define CM_META_V_MAX "__v_max__"
// #define CM_META_SPEED "__speed__"
// #define CM_META_RES_TYPE "__res_type__"
// #define CM_META_LOCAL_POS "__local_pos__"

#define CHECK_VM(vm) \
  CM_ASSERT(vm != NULL); \
  CM_ASSERT(vm->root != NULL)

coffee_machine_t *vm;

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
  {"userdata", CM_TUSERDATA},
  {"function", CM_TFUNCTION},
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

void cm_object_get_uid(cm_object_t *object, char *uid) {
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

// #define cm_vecn_impl_old(n) \
// cm_object_t *cm_vec##n##_from_array(cm_object_t *array) { \
//   CM_ASSERT(array != NULL); \
//   int len = cm_object_length(array); \
//   if (len != n) return NULL; \
//   VEC##n##_TYPE v_data; \
//   cm_object_t *iter = NULL; \
//   int i = 0; \
//   cm_object_foreach(iter, array) { \
//   if (iter->type != CM_TNUMBER) return NULL; \
//     v_data.data[i++] = iter->number; \
//   }\
//   cm_object_t *vec = cm_object_create(CM_TVEC##n); \
//   cm_object_set_vec##n(vec, v_data); \
//   return vec; \
// }

#define cm_vecn_impl(n) \
void cm_array_to_vec##n(cm_object_t *array) { \
  CM_ASSERT(array != NULL); \
  int len = cm_object_length(array); \
  if (len != n) return; \
  cm_object_t *iter = NULL; \
  array->type = CM_TVEC##n; \
  int i = 0; \
  cm_object_foreach(iter, array) { \
  if (iter->type != CM_TNUMBER) return; \
    array->vec##n.data[i++] = iter->number; \
  } \
  cm_object_clear(array); \
}

cm_vecn_impl(2);
cm_vecn_impl(3);
cm_vecn_impl(4);

cm_object_t* cm_array_to_vec(cm_object_t *array) {
  return NULL;
}


/*===================*
 *        VM         *
 *===================*/

int cm_init() {
  vm = CM_CALLOC(1, sizeof(*vm));
  if (!vm) return 0;

  vm->root = cm_object_create(CM_TTABLE);
  vm->gc = cm_object_create(CM_TTABLE);
  vm->read_file = read_file;
  vm->write_file = write_file;
  vm->stack_index = 0;
  vm->fp = 0;
  cm_object_add_table(vm->root, CM_META_TYPE, NULL);

  return 1;
}

void cm_deinit() {
  cm_object_delete(vm->root);
  cm_object_delete(vm->gc);
  CM_FREE(vm);
}

coffee_machine_t* cm_get_vm() {
  CHECK_VM(vm);
  return vm;
}

void cm_gc() {
  CHECK_VM(vm);
  cm_object_clear(vm->gc);
}

cm_object_t *cm_set_type(const char *typename, cm_object_t* type) {
  CHECK_VM(vm);
  if (!typename) return NULL;
  cm_object_t *types = cm_object_get(vm->root, typename);
  if (types) return NULL;

  return cm_object_set(types, typename, type);
}

cm_object_t *cm_get_type(const char *typename) {
  CHECK_VM(vm);
  if (!typename) return NULL;

  cm_object_t *types = cm_object_get(vm->root, CM_META_TYPE);
  cm_object_t *type = cm_object_get(types, typename);

  return type;
}

cm_object_t* cm_get_object(const char *name) {
  if (!name) return NULL;
  const char *part = strstr(name, "//");
  int sz = strlen(name)+1;
  if (part) sz = part - name + 1;

  char key[sz];
  memcpy(key, name, sz);
  key[sz] = '\0';
  cm_object_t *obj = cm_object_get(vm->root, key);
  if (part) return cm_object_get(obj, part+2);

  return obj;
}

cm_object_t* cm_set_object(const char *name, cm_object_t *object) {
  return NULL;
}

int cm_call(coffee_machine_t *vm, int nargs, int nres) {
  // cm_object_t *fn = cm_pop(vm);
  // if (!fn) return 0;
  int top = vm->stack_index;
  vm->fp = top-nargs-1;
  // if (top < nargs) return 0;

  // vm->stack_index = 0;
  // for (int i = 0; i < nargs; i++) {
  //   cm_pushvalue(vm, top-i);
  // }

  // fn->((cm_function)func)(vm);
  // cm_function *func = fn->func;
  // stack
  cm_object_t *fn = cm_get(vm, 0);


  // if (func) func(vm);
  cm_object_t *res = NULL;
  if (fn && fn->func) {
    cm_function *func = fn->func;
    ((cm_function)func)(vm);
    res = cm_pop(vm);
  }
  vm->stack_index = vm->fp;
  if (res) cm_push(vm, res);
  return nres;
}

void cm_settop(coffee_machine_t *vm, int top) {
  vm->fp = vm->fp + top;
}
int cm_gettop(coffee_machine_t *vm) {
  return vm->stack_index - vm->fp;
}

void cm_push(coffee_machine_t *vm, cm_object_t *obj) {
  CM_ASSERT(vm != NULL);
  CM_ASSERT(vm->stack_index < MAX_STACK);
  // CM_TRACELOG("%p", obj->func);
  vm->stack[vm->stack_index++] = obj;
}

cm_object_t *cm_pop(coffee_machine_t *vm) {
  CM_ASSERT(vm != NULL);
  CM_ASSERT(vm->stack_index > 0);
  return vm->stack[--vm->stack_index];
}

cm_object_t *cm_get(coffee_machine_t *vm, int index) {
  CM_ASSERT(vm != NULL);
  int i = vm->fp + index;
  if (index < 0) i = vm->fp - index;
  CM_ASSERT(i >= 0 && i < MAX_STACK);

  return vm->stack[i];
}

void cm_newtable(coffee_machine_t *vm) {
  cm_object_t *object = cm_create_table(NULL);
  cm_push(vm, object);
}

void cm_setmetatable(coffee_machine_t *vm) {
  // cm_checktable
}

void cm_getmetatable(coffee_machine_t *vm) {}

/*********
 * Push
 *********/

void cm_pushnull(coffee_machine_t *vm) {
  cm_object_t *null = cm_create_null();
  cm_push(vm, null);
}

void cm_pushnumber(coffee_machine_t *vm, float value) {
  cm_object_t *number = cm_create_number(value);
  cm_push(vm, number);
}
void cm_pushstring(coffee_machine_t *vm, const char *string) {
  cm_object_t *str = cm_create_string(string);
  cm_push(vm, str);
}
void cm_pushboolean(coffee_machine_t *vm, int value) {
  cm_object_t *boolean = cm_create_boolean(value);
  cm_push(vm, boolean);
}
void cm_pushvec2(coffee_machine_t *vm, VEC2_TYPE value) {
  cm_object_t *vec2 = cm_create_vec2(value);
  cm_push(vm, vec2);
}
void cm_pushvec3(coffee_machine_t *vm, VEC3_TYPE value) {
  cm_object_t *vec3 = cm_create_vec3(value);
  cm_push(vm, vec3);
}
void cm_pushvec4(coffee_machine_t *vm, VEC4_TYPE value) {
  cm_object_t *vec4 = cm_create_vec4(value);
  cm_push(vm, vec4);
}
// void cm_pushtable(coffee_machine_t *vm);
void cm_pushvalue(coffee_machine_t *vm, int index) {
  cm_object_t *val = cm_get(vm, index);
  if (!val) val = cm_create_null();
  cm_push(vm, val);
}
void cm_pushcfunction(coffee_machine_t *vm, cm_function* fn) {
  cm_object_t *val = cm_create_function(fn);
  if (!val) val = cm_create_null();
  // CM_TRACELOG("%p", val);
  cm_push(vm, val);
}

/**********
 * To
 **********/

float cm_tonumber(coffee_machine_t *vm, int index) {
  return cm_optnumber(vm, index, -1);
}
const char* cm_tostring(coffee_machine_t *vm, int index) {
  return cm_optstring(vm, index, NULL);
}
int cm_toboolean(coffee_machine_t *vm, int index) {
  return cm_optboolean(vm, index, -1);
}
VEC2_TYPE cm_tovec2(coffee_machine_t *vm, int index) {
  return cm_optvec2(vm, index, (VEC2_TYPE){-1, -1});
}
VEC3_TYPE cm_tovec3(coffee_machine_t *vm, int index) {
  return cm_optvec3(vm, index, (VEC3_TYPE){-1, -1, -1});
}
VEC4_TYPE cm_tovec4(coffee_machine_t *vm, int index) {
  return cm_optvec4(vm, index, (VEC4_TYPE){-1, -1, -1, -1});
}
cm_object_t* cm_toobject(coffee_machine_t *vm, int index) {
  return cm_optobject(vm, index, NULL);
}
void *cm_touserdata(coffee_machine_t *vm, int index) {
  return cm_optudata(vm, index, NULL);
}
cm_object_t* cm_toref(coffee_machine_t *vm, int index) {
  return cm_optref(vm, index, NULL);
}
cm_function* cm_tofunction(coffee_machine_t *vm, int index) {
  return cm_optfunction(vm, index, NULL);
}

cm_object_t *cm_checktype(coffee_machine_t *vm, int index, CM_T_ type) {
  cm_object_t *obj = cm_get(vm, index);
  if (!obj) return NULL;
  // if (obj->type != type) return NULL;
  CM_ASSERT(obj->type != type);

  return obj;
}

/**********
 * Check
 **********/

float cm_checknumber(coffee_machine_t *vm, int index) {
  cm_object_t *n = cm_checktype(vm, index, CM_TNUMBER);
  return n->number;
}
const char* cm_checkstring(coffee_machine_t *vm, int index) {
  cm_object_t *s = cm_checktype(vm, index, CM_TSTRING);
  return s->string;
}
int cm_checkboolean(coffee_machine_t *vm, int index) {
  cm_object_t *b = cm_checktype(vm, index, CM_TBOOLEAN);
  return b->boolean;
}
VEC2_TYPE cm_checkvec2(coffee_machine_t *vm, int index) {
  cm_object_t *v = cm_checktype(vm, index, CM_TVEC2);
  return v->vec2;
}
VEC3_TYPE cm_checkvec3(coffee_machine_t *vm, int index) {
  cm_object_t *v = cm_checktype(vm, index, CM_TVEC3);
  return v->vec3;
}
VEC4_TYPE cm_checkvec4(coffee_machine_t *vm, int index) {
  cm_object_t *v = cm_checktype(vm, index, CM_TVEC4);
  return v->vec4;
}
cm_object_t* cm_checkobject(coffee_machine_t *vm, int index) {
  cm_object_t *obj = cm_get(vm, index);
  CM_ASSERT(obj != NULL);
  return obj;
}
void *cm_checkudata(coffee_machine_t *vm, int index) {
  cm_object_t *u = cm_checktype(vm, index, CM_TUSERDATA);
  return u->userdata;
}
cm_object_t* cm_checkref(coffee_machine_t *vm, int index) {
  cm_object_t *r = cm_checktype(vm, index, CM_TREFERENCE);
  return r->ref;
}
cm_function* cm_checkfunction(coffee_machine_t *vm, int index) {
  cm_object_t *f = cm_checktype(vm, index, CM_TFUNCTION);
  return f->func;
}

/*********
 * Opt
 *********/

float cm_optnumber(coffee_machine_t *vm, int index, float opt) {
  cm_object_t *number = cm_get(vm, index);
  if (!number) return opt;
  if (number->type != CM_TNUMBER) return opt;

  return number->number;
}
const char* cm_optstring(coffee_machine_t *vm, int index, const char *opt) {
  cm_object_t *str = cm_get(vm, index);
  if (!str) return opt;
  if (str->type != CM_TSTRING) return opt;

  return str->string;
}
int cm_optboolean(coffee_machine_t *vm, int index, int opt) {
  cm_object_t *boolean = cm_get(vm, index);
  if (!boolean) return opt;
  if (boolean->type != CM_TBOOLEAN) return opt;

  return boolean->boolean;
}
VEC2_TYPE cm_optvec2(coffee_machine_t *vm, int index, VEC2_TYPE opt) {
  cm_object_t *vec2 = cm_get(vm, index);
  if (!vec2) return opt;
  if (vec2->type != CM_TBOOLEAN) return opt;

  return vec2->vec2;
}
VEC3_TYPE cm_optvec3(coffee_machine_t *vm, int index, VEC3_TYPE opt) {
  cm_object_t *vec3 = cm_get(vm, index);
  if (!vec3) return opt;
  if (vec3->type != CM_TBOOLEAN) return opt;

  return vec3->vec3;
}
VEC4_TYPE cm_optvec4(coffee_machine_t *vm, int index, VEC4_TYPE opt) {
  cm_object_t *vec4 = cm_get(vm, index);
  if (!vec4) return opt;
  if (vec4->type != CM_TBOOLEAN) return opt;

  return vec4->vec4;
}
cm_object_t* cm_optobject(coffee_machine_t *vm, int index, cm_object_t *opt) {
  cm_object_t *obj = cm_get(vm, index);
  if (!obj) return opt;
  // if (vec2->type != CM_TBOOLEAN) return opt;

  return obj;
}
void *cm_optudata(coffee_machine_t *vm, int index, void *opt) {
  cm_object_t *udata = cm_get(vm, index);
  if (!udata) return opt;
  if (udata->type != CM_TUSERDATA) return opt;

  return udata->userdata;
}
cm_object_t* cm_optref(coffee_machine_t *vm, int index, cm_object_t *opt) {
  cm_object_t *ref = cm_get(vm, index);
  if (!ref) return opt;
  if (ref->type != CM_TREFERENCE) return opt;

  return ref->ref;
}
cm_function* cm_optfunction(coffee_machine_t *vm, int index, cm_function *opt) {
  cm_object_t *fn = cm_get(vm, index);
  if (!fn) return opt;
  if (fn->type != CM_TREFERENCE) return opt;

  return fn->func;
}

/*====================*
 *       Coffee       *
 *====================*/

const char *typenames[] = {
  "null", "number", "string", "boolean",
  "vec2", "vec3", "vec4", "array", "table",
  "userdata"
};

static const char *typename_from_object(cm_object_t *obj) {
  if (!obj) return "";
  if (obj->type < 0 || obj->type >= 10) return "";
  return typenames[obj->type];
}

CM_API void cm_object_print(cm_object_t *obj) {
  if (!obj) return;
  const char *typename = typename_from_object(obj);
  const char *prev_typename = typename_from_object(obj->prev);
  const char *next_typename = typename_from_object(obj->next);
  const char *parent_typename = typename_from_object(obj->parent);
  const char *child_typename = typename_from_object(obj->child);
  CM_LOG("<%s>%p -> prev: <%s>%p, next: <%s>%p, parent: <%s>%p, child: <%s>%p", typename, obj, prev_typename, obj->prev, next_typename, obj->next, parent_typename, obj->parent, child_typename, obj->child);
}

cm_object_t* cm_object_create(CM_T_ type) {
  cm_object_t *object = malloc(sizeof(*object));
  memset(object, 0, sizeof(*object));

  object->marked = 0;
  object->name = NULL;
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
    CM_FREE((char*)content);
    return NULL;
  }

  CM_FREE((char*)content);
  // CM_TRACEERR("okay");

  return obj;
}

cm_object_t* cm_object_clone(cm_object_t *obj) {
  CM_ASSERT(obj != NULL);
  cm_object_t *clone = NULL;

  switch(obj->type) {
    case CM_TNULL:
      clone = cm_object_create(CM_TNULL);
      break;
    case CM_TNUMBER:
      clone = cm_create_number(obj->number);
      break;
    case CM_TSTRING:
      clone = cm_create_string(obj->string);
      break;
    case CM_TBOOLEAN:
      clone = cm_create_boolean(obj->boolean);
      break;
    case CM_TVEC2:
      clone = cm_create_vec2(obj->vec2);
      break;
    case CM_TVEC3:
      clone = cm_create_vec3(obj->vec3);
      break;
    case CM_TVEC4:
      clone = cm_create_vec4(obj->vec4);
      break;
    case CM_TARRAY:
      {
        clone = cm_create_array(NULL);
        cm_object_t *iter = NULL;
        cm_object_foreach(iter, obj) {
          cm_object_t *item_clone = cm_object_clone(iter);
          if (item_clone) cm_object_push(clone, -1, item_clone);
        }
        break;
      }
    case CM_TTABLE:
      {
        clone = cm_create_table(NULL);
        cm_object_t *iter = NULL;
        cm_object_foreach(iter, obj) {
          cm_object_t *item_clone = cm_object_clone(iter);
          if (item_clone) cm_object_set(clone, iter->name, item_clone);
        }
        break;
      }
    default:
      CM_TRACELOG("Unkown Coffee type: %d", obj->type);
  }

  if (obj->name) cm_object_set_name(clone, obj->name);

  return clone;
}

cm_object_t* cm_object_replace(cm_object_t *obj, const char *name, cm_object_t *new_item) {
  CM_ASSERT(obj != NULL);
  CM_ASSERT(new_item != NULL);
  if (!name) return NULL;

  cm_object_t *old = cm_object_get(obj, name);
  if (!old) return NULL;

  cm_object_t *prev = old->prev;
  cm_object_t *next = old->next;
  // cm_object_t *parent = old->parent;

  if (prev) prev->next = new_item;
  if (next) next->prev = new_item;

  new_item->prev = prev;
  new_item->next = next;
  new_item->parent = obj;

  if (obj->child == old) obj->child = new_item;
  cm_object_set_name(new_item, name);

  return old;
}

void cm_object_remove(cm_object_t *object, const char *name) {
  CM_ASSERT(object != NULL);
  if (!name) return;

  cm_object_t *item = cm_object_get(object, name);
  if (!item) return;

  cm_object_delete(item);
}

void cm_object_clear(cm_object_t *object) {
  if (!object) return;
  if (!object->child) return;

  cm_object_t *iter = NULL;
  cm_object_foreach(iter, object) {
    cm_object_clear(iter);
  }
  object->child = NULL;
}

void cm_object_delete(cm_object_t *object) {
  if (!object) return;
  // if (object->next) cm_object_delete(object->next)
  cm_object_t *prev = object->prev;
  cm_object_t *next = object->next;
  cm_object_t *parent = object->parent;
  if (object->child) cm_object_clear(object);

  if (prev) prev->next = object->next;
  if (next) next->prev = object->prev;
  if (parent && parent->child == object) parent->child = next;

  // cm_object_clear(object);
  CM_FREE(object);
}

void cm_object_set_name(cm_object_t *object, const char *name) {
  CM_ASSERT(object != NULL);
  if (!name) return;

  int name_len = strlen(name);
  if (!object->name) object->name = CM_MALLOC(name_len + 5);
  else object->name= CM_REALLOC(object->name, name_len + 5);
  
  strcpy(object->name, name);
}

const char* cm_object_get_name(cm_object_t *object) {
  CM_ASSERT(object != NULL);
  return object->name;
}

cm_object_t *cm_object_get(cm_object_t *field, const char *name) {
  if (!field || !name) return NULL;
  const char *part = strstr(name, "//");
  // CM_TRACELOG("%s", name);
  if (!part) {
    cm_object_t *item = NULL;
    cm_object_foreach(item, field) {
      if (item->name && !strcmp(item->name, name)) {
        return item;
      }
    }
    return NULL;
  }

  size_t sz = part - name;
  char key[sz+1];
  memset(key, 0, sz+1);

  memcpy(key, name, sz);

  cm_object_t *item = NULL;
  cm_object_foreach(item, field) {
    if (item->name && !strcmp(item->name, key)) return cm_object_get(item, part+2);
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

cm_object_t *cm_object_set(cm_object_t *obj, const char *name, cm_object_t *item) {
  CM_ASSERT(obj != NULL);
  if (!name || !item) return NULL;


  cm_object_t *el = NULL;
  cm_object_foreach(el, obj) {
    if (el->name && !strcmp(el->name, name)) return el;
  }

  // strcpy(item->name, name);
  cm_object_set_name(item, name);
  if (!obj->child) {
    obj->child = item;
    item->parent = obj;
    return obj->child;
  }


  el = obj->child;
  while (el->next) el = el->next;

  el->next = item;
  item->prev = el;
  item->parent = obj;

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
 *      NULL      *
 *================*/

cm_object_t* cm_create_null() {
  return cm_object_create(CM_TNULL);
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

void cm_object_set_string(cm_object_t *obj, const char *string) {
  if (!obj) return;
  if (!string) return;
  if (obj->type != CM_TSTRING) return;
  int len = strlen(string);

  if (obj->string && strlen(obj->string) > len) obj->string = CM_REALLOC(obj->string, len+10);
  else if (!obj->string) obj->string = CM_MALLOC(len+10);

  // obj->string = string;
  strcpy(obj->string, string);
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

cm_object_t* cm_object_add_table(cm_object_t *obj, const char *name, cm_object_t *object) {
  if (!obj) return NULL;

  if (!object) object = cm_create_table(NULL);
  return cm_object_set(obj, name, object);
}

/*=================
 *    Userdata
 *=================*/

cm_object_t* cm_create_userdata(void *userdata) {
  cm_object_t *obj = cm_object_create(CM_TUSERDATA);
  if (!obj) return NULL;

  obj->userdata = userdata;

  return obj;
}

void cm_object_set_userdata(cm_object_t *obj, void *userdata) {
  CM_ASSERT(obj != NULL);
  if (!userdata) return;
  if (obj->type == CM_TTABLE || obj->type == CM_TARRAY) return;

  if (obj->type == CM_TSTRING) CM_FREE(obj->string);
  obj->type = CM_TUSERDATA;
  obj->userdata = userdata;
}

void *cm_object_to_userdata(cm_object_t *obj) {
  CM_ASSERT(obj != NULL);
  return cm_object_to_opt_userdata(obj, NULL);
}

void *cm_object_to_opt_userdata(cm_object_t *obj, void *opt) {
  if (!obj) return opt;
  if (obj->type != CM_TUSERDATA) return opt;

  return obj->userdata;
}

void* cm_object_get_userdata(cm_object_t *obj, const char *name) {
  CM_ASSERT(obj != NULL);
  return cm_object_get_opt_userdata(obj, name, NULL);
}

void* cm_object_get_opt_userdata(cm_object_t *obj, const char *name, void* opt) {
  if (!obj) return opt;
  cm_object_t *item = cm_object_get(obj, name);
  return cm_object_to_opt_userdata(item, opt);
}

cm_object_t* cm_object_add_userdata(cm_object_t *obj, const char *name, void *userdata) {
  if (!obj) return NULL;

  cm_object_t *userdataf = cm_create_userdata(userdata);

  return cm_object_set(obj, name, userdataf);
}

/*=================
 *    Function
 *=================*/

cm_object_t* cm_create_function(cm_function* function) {
  cm_object_t *field = cm_object_create(CM_TFUNCTION);
  if (!field) return NULL;

  field->func = function;

  return field;
}

cm_function* cm_object_to_opt_function(cm_object_t *obj, cm_function* opt) {
  if (!obj) return opt;
  if (obj->type != CM_TFUNCTION) return opt;

  return obj->func;
}

cm_function* cm_object_get_function(cm_object_t *field, const char *name) {
  CM_ASSERT(field != NULL);
  return cm_object_get_opt_function(field, name, NULL);
}

cm_function* cm_object_get_opt_function(cm_object_t *obj, const char *name, cm_function* opt) {
  if (!obj) return opt;
  cm_object_t *item = cm_object_get(obj, name);
  return cm_object_to_opt_function(item, opt);
}

cm_object_t* cm_object_add_function(cm_object_t *obj, const char *name, cm_function* func) {
  if (!obj) return NULL;

  cm_object_t *funcf = cm_create_function(func);

  return cm_object_set(obj, name, funcf);
}

/*=================*
 *    Reference    *
 *=================*/

cm_object_t* cm_create_reference(cm_object_t *ref) {
  cm_object_t *obj = cm_object_create(CM_TREFERENCE);
  if (!obj) return NULL;

  cm_object_set_reference(obj, ref);

  return obj;
}

void cm_object_set_reference(cm_object_t *obj, cm_object_t *object) {
  if (!obj) return;
  if (!object) return;
  if (obj->type != CM_TREFERENCE) return;

  obj->ref = object;
}

cm_object_t *cm_object_to_reference(cm_object_t *obj) {
  CM_ASSERT(obj != NULL);
  CM_ASSERT(obj->type != CM_TREFERENCE);

  return cm_object_to_opt_reference(obj, NULL);
}

cm_object_t *cm_object_to_opt_reference(cm_object_t *obj, cm_object_t *opt) {
  if (!obj) return opt;
  if (obj->type != CM_TREFERENCE) return opt;

  return obj->ref;
}

cm_object_t* cm_object_get_reference(cm_object_t *obj, const char *name) {
  CM_ASSERT(obj != NULL);
  return cm_object_get_opt_reference(obj, name, NULL);
}

cm_object_t* cm_object_get_opt_reference(cm_object_t *obj, const char *name, cm_object_t* opt) {
  if (!obj) return opt;
  cm_object_t *item = cm_object_get(obj, name);
  return cm_object_to_opt_reference(item, opt);
}

cm_object_t* cm_object_add_reference(cm_object_t *obj, const char *name, cm_object_t *object) {
  if (!obj) return NULL;
  if (!object) return NULL;

  // if (!object) object = cm_create_reference(object);
  cm_object_t *ref = cm_create_reference(object);
  return cm_object_set(obj, name, ref);
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
  // CM_TRACELOG("%c", p);
  if (p == '-') return 1;
  int n = p - 48;

  return (n >= 0 && n <= 9);
}

static int str_is_boolean(char *p) {
  CM_ASSERT(p != NULL);
  char val[6];
  memset(val, 0, 6);
  p = ignore_chars(p);

  if (*p == 't') {
    memcpy(val, p, 4);
    return !strcmp(val, "true");
  }

  if (*p == 'f') {
    memcpy(val, p, 5);
    return !strcmp(val, "false");
  }

  return 0;
}

static char *check_null(const char *json_str) {
  if (!json_str) return NULL;

  char *p = (char*)json_str;
  p = (char*)ignore_chars(p);
  char val[5];

  if (*p == 'n') {
    memcpy(val, p, 4);
    val[4] = '\0';
    if (!strcmp(val, "null")) return p;
  }

  return NULL;
}

static char *check_boolean(const char *json_str, int *len) {
  if (!json_str) return NULL;

  char *p = (char*)json_str;
  p = (char*)ignore_chars(p);

  if (str_is_boolean(p)) {
    int i = 0;
    char *boolean = check_content(p, &i);
    if (len) *len = i;
    return p;
  }

  return NULL;
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
    if (i < 0) return NULL;
    // if (i == 0)
    if (len) *len = i;
    return p;
  }

  return NULL;
}

// static char *get_string
static cm_object_t *parse_json_null(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;

  char *null = check_null(init);
  if (!null) return NULL;

  cm_object_t *o_null = cm_object_create(CM_TNULL);
  *json_str += 5;

  return o_null;
}

static int str_to_bool(const char *str) {
  if (strstr(str, "true")) return 1;
  return 0;
}

static cm_object_t *parse_json_boolean(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;

  // char *p = init;
  int len = 0;
  char *boolean = check_boolean(init, &len);
  if (!boolean || len <= 0) return NULL;

  char val[len+1];
  memcpy(val, boolean, len);
  val[len] = '\0';

  cm_object_t *o_bool = cm_object_create(CM_TBOOLEAN);
  o_bool->boolean = str_to_bool(val);
  *json_str += len+1;

  return o_bool;
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
  *json_str += len;

  return o_num;
}

static cm_object_t *parse_json_string(const char **json_str) {
  CM_ASSERT(json_str != NULL);
  char *init = (char*)*json_str;
  int len = 0;

  char *str = check_string(init, &len);
  if (!str || len < 0) return NULL;
  char strp[len+1];
  memcpy(strp, str, len);
  strp[len] = '\0';

  // const char *strp = len > 0 ? str : "";
  cm_object_t *o_str = cm_create_string(strp);


  *json_str = str+len+1;

  return o_str;
}

static cm_object_t *parse_json_object(const char **json_str);
static cm_object_t *parse_json_array(const char **json_str);

cm_object_t *parse_json_array(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;
  cm_object_t *obj = NULL;

  char *p = init;
  p = ignore_chars(p);
  if (*p == '[') {
    p++;
    // CM_TRACELOG("%s", p);
    obj = cm_object_create(CM_TARRAY);
    while (*p != ']') {
      p = ignore_chars(p);
      if (*p == ',') {
        p++;
        p = ignore_chars(p);
      }

      // CM_TRACELOG("%s", p);

      cm_object_t *child = NULL;
      // CM_TRACELOG("%s", p);
      if (*p == '{') {
        child = parse_json_object(&p);
        // CM_TRACELOG("%p %s", child, p);
      } else if (*p == '"') {
        child = parse_json_string(&p);
      } else if (*p == '[') {
        child = parse_json_array(&p);
      } else if (char_is_number(*p)) {
        child = parse_json_number(&p);
      } else if (str_is_boolean(p)) {
        child = parse_json_boolean(&p);
      } else {
        child = parse_json_null(&p);
      }
      // CM_

      // CM_TRACELOG("%p %s", child, p);
      if (!child) {
        CM_TRACEERR("Failed to parse: %s", p);
        exit(1);
      }
      p = ignore_chars(p);
      // cm_object_set(obj, key, child);
      cm_object_push(obj, -1, child);
      // CM_TRACELOG("%s", p);
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
  p = ignore_chars(p);
  if (*p == '{') {
    p++;
    obj = cm_object_create(CM_TTABLE);
    // p = (char*)
    while (*p != '}') {
      p = (char*)ignore_chars(p);
      if (*p == ',') {
        p++;
        p = ignore_chars(p);
      }

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
        // CM_TRACELOG("%s", p);
        child = parse_json_string(&p);
      } else if (*p == '[') {
        child = parse_json_array(&p);
      } else if (char_is_number(*p)) {
        child = parse_json_number(&p);
      } else if (str_is_boolean(p)) {
        child = parse_json_boolean(&p);
      } else {
        child = parse_json_null(&p);
      }

      if (!child) {
        CM_TRACEERR("Failed to parse: %s", p);
        exit(1);
      }
      // CM_TRACEERR("%s %d %s", key, child->type, p);
      cm_object_set(obj, key, child);
      p = ignore_chars(p);

      // if (title)
    }
    p++;
    // p = ignore_chars(p);
    *json_str = p;
  }

  return obj;
}

cm_object_t* cm_parse_json(const char *json_string) {
  CM_ASSERT(json_string != NULL);

      // CM_TRACELOG("%s", json_string);
  const char *str = json_string;
  cm_object_t *root = parse_json_object(&json_string);

  return root;
}

const char* print_json_null(cm_object_t *obj, int *char_count) {
  if (obj->type != CM_TNULL) return NULL;
  if (char_count) *char_count += 4;

  char *str = CM_CALLOC(1, 5);
  strcpy(str, "null");

  return str;
}

const char* print_json_boolean(cm_object_t *obj, int *char_count) {
  if (obj->type != CM_TBOOLEAN) return NULL;
  int sz = obj->boolean ? 5 : 6;
  if (char_count) *char_count += sz;

  char *str = CM_CALLOC(1, sz);

  if (obj->boolean) strcpy(str, "true");
  else strcpy(str, "false");

  return str;
}

const char* print_json_number(cm_object_t *obj, int *char_count) {
  if (obj->type != CM_TNUMBER) return NULL;
  char lstr_n[32];
  sprintf(lstr_n, "%f", obj->number);

  int sz = strlen(lstr_n);

  char *str = CM_CALLOC(1, sz+1);

  if (char_count) *char_count += sz;
  strcpy(str, lstr_n);

  return str;
}

const char *print_json_vec2(cm_object_t *obj, int *char_count) {
  CM_ASSERT(obj != NULL);
  if (obj->type != CM_TVEC2) return NULL;
  char lstr_n[128];
  memset(lstr_n, 0, 128);
  // sprintf(lstr_n, "%f", obj->number);
  VEC2_TYPE *vec = &obj->vec2;

  int sz = 0;
  // strlen(const char *__s)
  // lstr
  *lstr_n = '(';
  for (int i = 0; i < 2; i++) {
    sprintf(lstr_n, "%s%f", lstr_n, vec->data[i]);
    if (i < 1) strcat(lstr_n, ", ");
  }
  strcat(lstr_n, ")");
  sz = strlen(lstr_n);


  char *str = CM_CALLOC(1, sz+1);
  // *str = '(';

  // for (int i = 0; i < 2; i++) {
  //   sprintf(lstr_n, "%f", vec->data[i]);
  // }

  if (char_count) *char_count += sz;
  strcpy(str, lstr_n);
  // strcat(str, ")");

  return str;
}

const char* print_json_string(cm_object_t *obj, int *char_count) {
  if (obj->type != CM_TSTRING) return NULL;
  int len = strlen(obj->string);
  char *str = CM_CALLOC(1, len+3);

  if (char_count) *char_count += len+3;

  sprintf(str, "\"%s\"", obj->string);

  return str;
}

const char *print_json_array(cm_object_t *obj, int *char_count);
const char *print_json_object(cm_object_t *obj, int *char_count);

const char* print_json_array(cm_object_t *obj, int *char_count) {
  if (obj->type != CM_TARRAY) return NULL;
  // int str_len = 3;
  // char *str = CM_CALLOC(1, str_len);
  // *str = '[';

  // cm_object_t *el = NULL;
  // cm_object_foreach(el, obj) {
  //   const char *el_str = NULL;
  //   int el_str_len = 0;
  //   switch (el->type) {
  //     case CM_TNULL:
  //       el_str = print_json_null(el, &el_str_len);
  //       break;
  //     case CM_TBOOLEAN:
  //       el_str = print_json_boolean(el, &el_str_len);
  //       break;
  //     case CM_TNUMBER:
  //       el_str = print_json_number(el, &el_str_len);
  //       break;
  //     case CM_TSTRING:
  //       el_str = print_json_string(el, &el_str_len);
  //       break;
  //     case CM_TARRAY:
  //       el_str = print_json_array(el, &el_str_len);
  //       break;
  //     case CM_TTABLE:
  //       el_str = print_json_object(el, &el_str_len);
  //       break;
  //     default:
  //       CM_TRACELOG("Invalid json type: %d", el->type);
  //   }
  //   if (el_str && el_str_len > 0) {

  //     str_len += el_str_len + 2;
  //     str = realloc(str, str_len);
  //     strcat(str, el_str);
  //     if (el->next) strcat(str, ",");
  //     CM_FREE((char*)el_str);
  //   }
  // }
  // strcat(str, "]");

  // if (char_count) *char_count += str_len;

  // return str;

  int str_len = 3;
  char *str = CM_CALLOC(1, str_len);
  *str = '[';

  cm_object_t *iter = NULL;
  cm_object_foreach(iter, obj) {
    int iter_str_len = 0;
    const char *iter_str = cm_print_json(iter, &iter_str_len);
    if (iter_str) {
      str_len += iter_str_len + 2;
      str = realloc(str, str_len);
      strcat(str, iter_str);
      if (iter->next) strcat(str, ",");
      CM_FREE((char*)iter_str);
    }
  }

  strcat(str, "]");

  if (char_count) *char_count += str_len;
  // CM_TRACELOG("%s %d", obj->name, str_len);

  return str;
}

const char* print_json_object(cm_object_t *obj, int *char_count) {
  if (obj->type != CM_TTABLE) return NULL;
  int str_len = 3;
  char *str = CM_CALLOC(1, str_len);
  *str = '{';

  cm_object_t *iter = NULL;
  cm_object_foreach(iter, obj) {
    int iter_str_len = 0;
    const char *iter_str = cm_print_json(iter, &iter_str_len);
    if (iter_str) {
      int key_size = strlen(iter->name) + 3;
      char key[key_size+2];
      sprintf(key, "\"%s\": ", iter->name);

      // CM_

      str_len += key_size + iter_str_len + 2;
      str = realloc(str, str_len);

      strcat(str, key);
      strcat(str, iter_str);
      if (iter->next) strcat(str, ",");
  //     // CM_TRACELOG("%s %s %s", el->name, str, el_str);
      CM_FREE((char*)iter_str);
    }
  }

  strcat(str, "}");

  if (char_count) *char_count += str_len;
  // CM_TRACELOG("%s %d", obj->name, str_len);

  return str;

  //   const char *el_str = NULL;
  //   int el_str_len = 0;
  //   switch (el->type) {
  //     case CM_TNULL:
  //       el_str = print_json_null(el, &el_str_len);
  //       break;
  //     case CM_TBOOLEAN:
  //       el_str = print_json_boolean(el, &el_str_len);
  //       break;
  //     case CM_TNUMBER:
  //       el_str = print_json_number(el, &el_str_len);
  //       break;
  //     case CM_TSTRING:
  //       el_str = print_json_string(el, &el_str_len);
  //       break;
  //     case CM_TARRAY:
  //       el_str = print_json_array(el, &el_str_len);
  //       break;
  //     case CM_TTABLE:
  //       el_str = print_json_object(el, &el_str_len);
  //       break;
  //     default:
  //       CM_TRACELOG("Invalid json type: %d", el->type);
  //   }

  //   if (el_str && el_str_len > 0) {
  //     int key_size = strlen(el->name) + 3;
  //     char key[key_size+2];
  //     sprintf(key, "\"%s\": ", el->name);

  //     // CM_

  //     str_len += key_size + el_str_len + 2;
  //     str = realloc(str, str_len);

  //     strcat(str, key);
  //     strcat(str, el_str);
  //     if (el->next) strcat(str, ",");
  //     // CM_TRACELOG("%s %s %s", el->name, str, el_str);
  //     CM_FREE((char*)el_str);
  //   }
  // }
  
}

const char* cm_print_json(cm_object_t *object, int *char_count) {
  // if (!object) return "{}";

  // // const char *str = print_json_object(object, NULL);
  // // CM_TRACELOG("%s", str);
  // return str;

  // int char_count = 0;
  int str_len = 0;
  const char *str = NULL;
  // int count = char_count ? *char_count : 0;
  // char *str = calloc(1, sizeof(char)*2);
  // *str = '{';

  cm_object_t *item = object;
  switch (object->type) {
    case CM_TNULL:
      str = print_json_null(item, char_count);
      break;
    case CM_TBOOLEAN:
      str = print_json_boolean(item, char_count);
      break;
    case CM_TNUMBER:
      str = print_json_number(item, char_count);
      break;
    case CM_TSTRING:
      str = print_json_string(item, char_count);
      break;
    case CM_TVEC2:
      str = print_json_vec2(item, char_count);
    case CM_TARRAY:
      str = print_json_array(item, char_count);
      break;
    case CM_TTABLE:
      str = print_json_object(item, char_count);
      break;
    default:
      CM_TRACELOG("Invalid json type: %d", item->type);
  }

  return str;
}

cm_object_t *cm_json_to_coffee(cm_object_t *obj) {
  CM_ASSERT(obj != NULL);
  cm_object_t *out = obj;

  // if ()

  cm_object_t *iter = NULL;
  cm_object_foreach(iter, out) {
    if (iter->type == CM_TARRAY) {
      int len = cm_object_length(iter);
      // cm_object_t *vec = NULL;
      if (len == 2) cm_array_to_vec2(iter);
      else if (len == 3) cm_array_to_vec3(iter);
      else if (len == 4) cm_array_to_vec4(iter);

    } else if (iter->type == CM_TTABLE) {
      iter = cm_json_to_coffee(iter);
    }
  }

  return out;
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
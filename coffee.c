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
  // CM_ASSERT(vm != NULL);
  // cm_object_t *types = cm_object_get(vm->root, CM_META_TYPE);
  // int i;
  // for (i = 0; i < CM_TMAX; i++) {
  //   if (!strcmp(type_names[i].name, string)) return type_names[i].type;
  // }

  // return CM_TNULL;
}

// void cm_object_get_uid(cm_object_t *object, char *uid) {
//   CHECK_VM(vm);
//   sprintf(uid, "%s##", object->name);

//   cm_object_t *parent = object->parent;
//   while (parent) {
//     strcat(uid, "_");
//     strcat(uid, parent->name);
//     parent = parent->parent;
//   }
// }

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

  vm->root = coffee_create_table();
  vm->gc = coffee_create_table();
  vm->read_file = read_file;
  vm->write_file = write_file;
  vm->stack_index = 0;
  vm->fp = 0;
  // cm_object_add_table(vm->root, CM_META_TYPE, NULL);
  coffee_t *meta = coffee_create_table();
  coffee_table_set(vm->root->table, CM_META_TYPE, meta);

  return 1;
}

void cm_deinit() {
  coffee_delete(vm->root);
  coffee_delete(vm->gc);
  CM_FREE(vm);
}

coffee_machine_t* cm_get_vm() {
  CHECK_VM(vm);
  return vm;
}

void cm_gc() {
  CHECK_VM(vm);
  coffee_clear(vm->gc);
}

// cm_object_t *cm_set_type(const char *typename, cm_object_t* type) {
//   CHECK_VM(vm);
//   if (!typename) return NULL;
//   cm_object_t *types = cm_object_get(vm->root, typename);
//   if (types) return NULL;

//   return cm_object_set(types, typename, type);
// }

// cm_object_t *cm_get_type(const char *typename) {
//   CHECK_VM(vm);
//   if (!typename) return NULL;

//   cm_object_t *types = cm_object_get(vm->root, CM_META_TYPE);
//   cm_object_t *type = cm_object_get(types, typename);

//   return type;
// }

coffee_t* cm_get_coffee(const char *name) {
  if (!name) return NULL;
  const char *part = strstr(name, "//");
  int sz = strlen(name)+1;
  if (part) sz = part - name + 1;

  char key[sz];
  memcpy(key, name, sz);
  key[sz] = '\0';
  coffee_t *obj = coffee_table_get(vm->root->table, key);
  if (part) return coffee_table_get(obj->table, part+2);

  return obj;
}

cm_object_t* cm_set_coffee(const char *name, cm_object_t *object) {
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
  coffee_t *fn = cm_get(vm, 0);


  // if (func) func(vm);
  coffee_t *res = NULL;
  if (fn && fn->func) {
    coffee_fn *func = fn->func;
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

void cm_push(coffee_machine_t *vm, coffee_t *obj) {
  CM_ASSERT(vm != NULL);
  CM_ASSERT(vm->stack_index < MAX_STACK);
  // CM_TRACELOG("%p", obj->func);
  vm->stack[vm->stack_index++] = obj;
}

coffee_t *cm_pop(coffee_machine_t *vm) {
  CM_ASSERT(vm != NULL);
  CM_ASSERT(vm->stack_index > 0);
  return vm->stack[--vm->stack_index];
}

coffee_t *cm_get(coffee_machine_t *vm, int index) {
  CM_ASSERT(vm != NULL);
  int i = vm->fp + index;
  if (index < 0) i = vm->fp - index;
  CM_ASSERT(i >= 0 && i < MAX_STACK);

  return vm->stack[i];
}

void cm_newtable(coffee_machine_t *vm) {
  coffee_t *object = coffee_create_table();
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
  coffee_t *null = cm_create_null();
  cm_push(vm, null);
}

void cm_pushnumber(coffee_machine_t *vm, float value) {
  coffee_t *number = coffee_create_float(value);
  cm_push(vm, number);
}
void cm_pushstring(coffee_machine_t *vm, const char *string) {
  coffee_t *str = coffee_create_string(string);
  cm_push(vm, str);
}
void cm_pushboolean(coffee_machine_t *vm, int value) {
  coffee_t *boolean = coffee_create_boolean(value);
  cm_push(vm, boolean);
}
void cm_pushvec2(coffee_machine_t *vm, double data[2]) {
  coffee_t *vec2 = coffee_create_vec2(data[0], data[1]);
  cm_push(vm, vec2);
}
void cm_pushvec3(coffee_machine_t *vm, double data[3]) {
  coffee_t *vec3 = coffee_create_vec3(data[0], data[1], data[2]);
  cm_push(vm, vec3);
}
void cm_pushvec4(coffee_machine_t *vm, double data[4]) {
  coffee_t *vec4 = coffee_create_vec4(data[0], data[1], data[2], data[3]);
  cm_push(vm, vec4);
}
// void cm_pushtable(coffee_machine_t *vm);
void cm_pushvalue(coffee_machine_t *vm, int index) {
  coffee_t *val = cm_get(vm, index);
  if (!val) val = coffee_create_null();
  cm_push(vm, val);
}
void cm_pushcfunction(coffee_machine_t *vm, cm_function* fn) {
  coffee_t *val = coffee_create_function((coffee_fn*)fn);
  if (!val) val = coffee_create_null();
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
double* cm_tovec2(coffee_machine_t *vm, int index) {
  double data[2] = {-1, -1};
  return cm_optvec2(vm, index, data);
}
double* cm_tovec3(coffee_machine_t *vm, int index) {
  double data[3] = {-1, -1, -1};
  return cm_optvec3(vm, index, data);
}
double* cm_tovec4(coffee_machine_t *vm, int index) {
  return cm_optvec4(vm, index, (double[4]){-1, -1, -1, -1});
}
coffee_t* cm_toobject(coffee_machine_t *vm, int index) {
  return cm_optobject(vm, index, NULL);
}
void *cm_touserdata(coffee_machine_t *vm, int index) {
  return cm_optudata(vm, index, NULL);
}
coffee_t* cm_toref(coffee_machine_t *vm, int index) {
  return cm_optref(vm, index, NULL);
}
cm_function* cm_tofunction(coffee_machine_t *vm, int index) {
  return cm_optfunction(vm, index, NULL);
}

/**********
 * Check
 **********/

coffee_t *cm_checktype(coffee_machine_t *vm, int index, COFFEE_T_ type) {
  coffee_t *obj = cm_get(vm, index);
  CHECK_TYPE(obj, type);
  return obj;
}

float cm_checknumber(coffee_machine_t *vm, int index) {
  coffee_t *n = cm_checktype(vm, index, COFFEE_TNUMBER);
  return n->number.value;
}
const char* cm_checkstring(coffee_machine_t *vm, int index) {
  coffee_t *s = cm_checktype(vm, index, COFFEE_TSTRING);
  return s->string.value;
}
int cm_checkboolean(coffee_machine_t *vm, int index) {
  coffee_t *b = cm_checktype(vm, index, COFFEE_TBOOL);
  return b->number.value;
}
double* cm_checkvec2(coffee_machine_t *vm, int index) {
  coffee_t *v = cm_checktype(vm, index, COFFEE_TVEC2);
  return v->vec2.data;
}
double* cm_checkvec3(coffee_machine_t *vm, int index) {
  coffee_t *v = cm_checktype(vm, index, COFFEE_TVEC3);
  return v->vec3.data;
}
double* cm_checkvec4(coffee_machine_t *vm, int index) {
  coffee_t *v = cm_checktype(vm, index, COFFEE_TVEC4);
  return v->vec4.data;
}
coffee_t* cm_checkobject(coffee_machine_t *vm, int index) {
  coffee_t *obj = cm_get(vm, index);
  CM_ASSERT(obj != NULL);
  return obj;
}
void *cm_checkudata(coffee_machine_t *vm, int index) {
  coffee_t *u = cm_checktype(vm, index, COFFEE_TUSERDATA);
  return u->userdata.data;
}
coffee_t* cm_checkref(coffee_machine_t *vm, int index) {
  coffee_t *r = cm_checktype(vm, index, COFFEE_TREF);
  return r->ref;
}
cm_function* cm_checkfunction(coffee_machine_t *vm, int index) {
  coffee_t *f = cm_checktype(vm, index, COFFEE_TFUNCTION);
  return (cm_function*)f->func;
}

/*********
 * Opt
 *********/

float cm_optnumber(coffee_machine_t *vm, int index, float opt) {
  coffee_t *number = cm_get(vm, index);
  if (!coffee_isnumber(number)) return opt;
  return number->number.value;
}
const char* cm_optstring(coffee_machine_t *vm, int index, const char *opt) {
  coffee_t *str = cm_get(vm, index);
  if (!coffee_isstring(str)) return opt;

  return str->string.value;
}
int cm_optboolean(coffee_machine_t *vm, int index, int opt) {
  coffee_t *boolean = cm_get(vm, index);
  if (!coffee_isboolean(boolean)) return opt;
  return boolean->number.value;
}
double* cm_optvec2(coffee_machine_t *vm, int index, double opt[2]) {
  coffee_t *vec2 = cm_get(vm, index);
  if (!coffee_isboolean(vec2)) return opt;

  return vec2->vec2.data;
}
double* cm_optvec3(coffee_machine_t *vm, int index, double opt[3]) {
  coffee_t *vec3 = cm_get(vm, index);
  if (!coffee_isvec3(vec3)) return opt;

  return vec3->vec3.data;
}
double* cm_optvec4(coffee_machine_t *vm, int index, double opt[4]) {
  coffee_t *vec4 = cm_get(vm, index);
  if (!coffee_isvec4(vec4)) return opt;

  return vec4->vec4.data;
}
coffee_t* cm_optobject(coffee_machine_t *vm, int index, coffee_t *opt) {
  coffee_t *obj = cm_get(vm, index);
  if (!obj) return opt;
  // if (vec2->type != COFFEE_TBOOLEAN) return opt;
  return obj;
}
void *cm_optudata(coffee_machine_t *vm, int index, void *opt) {
  coffee_t *udata = cm_get(vm, index);
  if (!coffee_isuserdata(udata)) return opt;

  return udata->userdata.data;
}

coffee_t* cm_optref(coffee_machine_t *vm, int index, coffee_t *opt) {
  coffee_t *ref = cm_get(vm, index);
  if (!coffee_isref(ref)) return opt;
  return ref->ref;
}

cm_function* cm_optfunction(coffee_machine_t *vm, int index, cm_function *opt) {
  coffee_t *fn = cm_get(vm, index);
  if (coffee_isfunction(fn)) return opt;
  return (cm_function*)fn->func;
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

  char *init = (char*)json_str;
  init = (char*)ignore_chars(init);
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
  p = (char*)ignore_chars(p);

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
  char *p = (char*)json_str;
  
  p = (char*)ignore_chars(p);
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
static coffee_t* parse_json_null(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;

  char *null = check_null(init);
  if (!null) return NULL;

  // cm_object_t *o_null = cm_object_create(CM_TNULL);
  coffee_t *o_null = coffee_create(COFFEE_TNULL);
  *json_str += 5;

  return o_null;
}

static int str_to_bool(const char *str) {
  if (strstr(str, "true")) return 1;
  return 0;
}

static coffee_t* parse_json_boolean(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;

  // char *p = init;
  int len = 0;
  char *boolean_str = check_boolean(init, &len);
  if (!boolean_str || len <= 0) return NULL;

  char val[len+1];
  memcpy(val, boolean_str, len);
  val[len] = '\0';

  int boolean = str_to_bool(boolean_str);

  // cm_object_t *o_bool = cm_object_create(CM_TBOOLEAN);
  // o_bool->boolean = str_to_bool(val);
  coffee_t *o_bool = coffee_create_boolean(boolean);
  *json_str += len+1;

  return o_bool;
}



static coffee_t* parse_json_number(const char **json_str) {
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

  // cm_object_t *o_num = cm_object_create(CM_TNUMBER);
  // coffee_t *o_num = coffee_create_number(double value, int type)
  coffee_t *o_num = NULL;
  if (strchr(val, '.')) {
    float v = atof(val);
    // CM_TRACELOG("%f", v);
    // cm_object_set_number(o_num, v);
    o_num = coffee_create_float(v);
  } else {
    int v = atoi(val);
    // o_num->number = v;
    o_num = coffee_create_int(v);
  }
  // CM_TRACELOG("%s", val);
  // CM_TRACELOG("%d %s", len, val);

  // o_num =
  // CM_TRACELOG("%f", o_num->number);
  *json_str += len;

  return o_num;
}

static coffee_t *parse_json_string(const char **json_str) {
  CM_ASSERT(json_str != NULL);
  char *init = (char*)*json_str;
  int len = 0;

  char *str = check_string(init, &len);
  if (!str || len < 0) return NULL;
  char strp[len+1];
  memcpy(strp, str, len);
  strp[len] = '\0';

  // const char *strp = len > 0 ? str : "";
  coffee_t *o_str = coffee_create_string(strp);


  *json_str = str+len+1;

  return o_str;
}

static coffee_t *parse_json_object(const char **json_str);
static coffee_t *parse_json_array(const char **json_str);

coffee_t *parse_json_array(const char **json_str) {
  CM_ASSERT(json_str != NULL);

  char *init = (char*)*json_str;
  coffee_t *obj = NULL;

  char *p = init;
  p = (char*)ignore_chars(p);
  if (*p == '[') {
    p++;
    // CM_TRACELOG("%s", p);
    // obj = cm_object_create(CM_TARRAY);
    obj = coffee_create_array();
    while (*p != ']') {
      p = (char*)ignore_chars(p);
      if (*p == ',') {
        p++;
        p = (char*)ignore_chars(p);
      }

      // CM_TRACELOG("%s", p);

      coffee_t *child = NULL;
      if (*p == '{') {
        child = parse_json_object((const char**)&p);
        // CM_TRACELOG("%p %s", child, p);
      } else if (*p == '"') {
        child = parse_json_string((const char**)&p);
      } else if (*p == '[') {
        child = parse_json_array((const char**)&p);
      } else if (char_is_number(*p)) {
        child = parse_json_number((const char**)&p);
      } else if (str_is_boolean(p)) {
        child = parse_json_boolean((const char**)&p);
      } else {
        child = parse_json_null((const char**)&p);
      }

      if (!child) {
        CM_TRACEERR("Failed to parse: %s", p);
        exit(1);
      }
      p = (char*)ignore_chars(p);
      // cm_object_set(obj, key, child);
      coffee_array_push(obj->table, child);
      // coffee_ata
      // CM_TRACELOG("%s", p);
    }
    p++;
    *json_str = p;
  }

  return obj;
}

coffee_t *parse_json_object(const char **json_str) {
  CM_ASSERT(json_str != NULL);
  char *init = (char*)*json_str;
  coffee_t *obj = NULL;


  char *p = init;
  p = (char*)ignore_chars(p);
  if (*p == '{') {
    p++;
    obj = coffee_create_table();
    // p = (char*)
    while (*p != '}') {
      p = (char*)ignore_chars(p);
      if (*p == ',') {
        p++;
        p = (char*)ignore_chars(p);
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

      coffee_t *child = NULL;
      // CM_TRACELOG("%s", p);
      if (*p == '{') {
        child = parse_json_object((const char**)&p);
      } else if (*p == '"') {
        // CM_TRACELOG("%s", p);
        child = parse_json_string((const char**)&p);
      } else if (*p == '[') {
        child = parse_json_array((const char**)&p);
      } else if (char_is_number(*p)) {
        child = parse_json_number((const char**)&p);
      } else if (str_is_boolean(p)) {
        child = parse_json_boolean((const char**)&p);
      } else {
        child = parse_json_null((const char**)&p);
      }

      if (!child) {
        CM_TRACEERR("Failed to parse: %s", p);
        exit(1);
      }
      // CM_TRACELOG("%p %d", key, child->type);
      // CM_TRACEERR("%s %d %s", key, child->type, p);
      coffee_table_set(obj->table, key, child);
      p = (char*)ignore_chars(p);

      // if (title)
    }
    p++;
    // p = ignore_chars(p);
    *json_str = p;
  }

  return obj;
}

const char* print_json_null(coffee_t *obj, int *char_count) {
  if (obj->type != COFFEE_TNULL) return NULL;
  if (char_count) *char_count += 4;

  char *str = CM_CALLOC(1, 5);
  strcpy(str, "null");

  return str;
}

const char* print_json_boolean(coffee_t *obj, int *char_count) {
  if (obj->type != COFFEE_TBOOL) return NULL;
  int sz = obj->number.value ? 5 : 6;
  if (char_count) *char_count += sz;

  char *str = CM_CALLOC(1, sz);

  if (obj->number.value) strcpy(str, "true");
  else strcpy(str, "false");

  return str;
}

const char* print_json_number(coffee_t *obj, int *char_count) {
  if (obj->type != COFFEE_TNUMBER) return NULL;
  char lstr_n[32];
  float val = obj->number.value;
  sprintf(lstr_n, "%f", val);

  int sz = strlen(lstr_n);

  char *str = CM_CALLOC(1, sz+1);

  if (char_count) *char_count += sz;
  strcpy(str, lstr_n);

  return str;
}

const char *print_json_vec2(coffee_t *obj, int *char_count) {
  CM_ASSERT(obj != NULL);
  if (obj->type != COFFEE_TVEC2) return NULL;
  char lstr_n[128];
  memset(lstr_n, 0, 128);
  // sprintf(lstr_n, "%f", obj->number);
  // VEC2_TYPE *vec = &obj->vec2;
  double *data = obj->vec2.data;

  int sz = 0;
  // strlen(const char *__s)
  // lstr
  *lstr_n = '(';
  for (int i = 0; i < 2; i++) {
    sprintf(lstr_n, "%s%f", lstr_n, data[i]);
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

const char* print_json_string(coffee_t *obj, int *char_count) {
  if (obj->type != COFFEE_TSTRING) return NULL;
  int len = strlen(obj->string.value);
  char *str = CM_CALLOC(1, len+3);

  if (char_count) *char_count += len+3;

  sprintf(str, "\"%s\"", obj->string.value);

  return str;
}

const char *print_json_array(coffee_t *obj, int *char_count);
const char *print_json_object(coffee_t *obj, int *char_count);

const char* print_json_array(coffee_t *obj, int *char_count) {
  if (obj->type != COFFEE_TARRAY) return NULL;
  int str_len = 3;
  char *str = CM_CALLOC(1, str_len);
  *str = '[';

  coffee_t *iter = NULL;
  coffee_foreach(iter, obj) {
    int iter_str_len = 0;
    const char *iter_str = coffee_print_json(iter, &iter_str_len);
    if (iter_str) {
      str_len += iter_str_len + 2;
      str = CM_REALLOC(str, str_len);
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

const char* print_json_object(coffee_t *obj, int *char_count) {
  if (obj->type != COFFEE_TTABLE) return NULL;
  int str_len = 3;
  char *str = CM_CALLOC(1, str_len);
  *str = '{';

  coffee_t *iter = NULL;
  coffee_foreach(iter, obj) {
    int iter_str_len = 0;
    const char *iter_str = coffee_print_json(iter, &iter_str_len);
    if (iter_str) {
      int key_size = strlen(iter->name) + 3;
      char key[key_size+2];
      sprintf(key, "\"%s\": ", iter->name);

      // CM_

      str_len += key_size + iter_str_len + 2;
      str = CM_REALLOC(str, str_len);

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
}

// const char* cm_print_json(cm_object_t *object, int *char_count) {
//   return NULL;
// }

// cm_object_t *cm_json_to_coffee(cm_object_t *obj) {
//   CM_ASSERT(obj != NULL);
//   cm_object_t *out = obj;

//   // if ()

//   cm_object_t *iter = NULL;
//   cm_object_foreach(iter, out) {
//     if (iter->type == CM_TARRAY) {
//       int len = cm_object_length(iter);
//       // cm_object_t *vec = NULL;
//       if (len == 2) cm_array_to_vec2(iter);
//       else if (len == 3) cm_array_to_vec3(iter);
//       else if (len == 4) cm_array_to_vec4(iter);

//     } else if (iter->type == CM_TTABLE) {
//       iter = cm_json_to_coffee(iter);
//     }
//   }

//   return out;
// }


/*==========================*
 *          Coffee          *
 *==========================*/

const char *_typenames[] = {
  "null", "number", "string", "bool",
  "vec2", "vec3", "vec4", "array", "table",
  "userdata", "function", "ref"
};

void coffee_print(coffee_t *coffee) {

}

coffee_t* coffee_create(COFFEE_T_ type) {
  CM_ASSERT(type >= 0);
  CM_ASSERT(type < COFFEE_TMAX);
  coffee_t *coffee = malloc(sizeof(*coffee));
  memset(coffee, 0, sizeof(*coffee));

  coffee->marked = 1;
  coffee->name = NULL;
  coffee->next = NULL;
  coffee->prev = NULL;
  coffee->ref = NULL;
  coffee->name = NULL;

  coffee->type = type;

  return coffee;  
}

coffee_t* coffee_load(const char *filename) {
  return coffee_load_json(filename); 
}
coffee_t* coffee_load_json(const char *filename) {
  CM_ASSERT(filename);
  const char *content = vm->read_file(filename, "rb");
  coffee_t *coffee = coffee_parse_json(content);

  if (!coffee) {
    CM_FREE((char*)content);
    return NULL;
  }
  CM_FREE((char*)content);
  return coffee;
}

// coffee_t *

coffee_t* coffee_clone(coffee_t *coffee) {
  CM_ASSERT(coffee != NULL);
  coffee_t *clone = NULL;

  switch(coffee->type) {
    case COFFEE_TNULL:
      clone = coffee_create_null();
      break;
    case COFFEE_TNUMBER:
      clone = coffee_clone_number(coffee);
      break;
    case COFFEE_TSTRING:
      clone = coffee_clone_string(coffee);
      break;
    case COFFEE_TBOOL:
      clone = coffee_clone_boolean(coffee);
      break;
    case COFFEE_TVEC2:
      clone = coffee_clone_vec2(coffee);
      break;
    case COFFEE_TVEC3:
      clone = coffee_clone_vec3(coffee);
      break;
    case COFFEE_TVEC4:
      clone = coffee_clone_vec4(coffee);
      break;
    case COFFEE_TARRAY:
      clone = coffee_clone_array(coffee);
      break;
    case COFFEE_TTABLE:
      clone = coffee_clone_table(coffee);
      break;
    case COFFEE_TFUNCTION:
      clone = coffee_clone_function(coffee);
      break;
    case COFFEE_TUSERDATA:
      clone = coffee_clone_userdata(coffee);
      break;
    case COFFEE_TREF:
      clone = coffee_clone_ref(coffee);
      break;
    default:
      CM_TRACELOG("Undefined type: %d", coffee->type);
  }

  return clone;
}
void coffee_delete(coffee_t *coffee) {
  CM_ASSERT(coffee != NULL);
  coffee_t *prev = coffee->prev;
  coffee_t *next = coffee->next;

  if (prev) prev->next = next;
  if (next) next->prev = prev;

  if (coffee_isarray(coffee) || coffee_istable(coffee)) {
    coffee_clear(coffee);
  }

  CM_FREE(coffee);
}

void coffee_clear(coffee_t *coffee) {
  CM_ASSERT(coffee != NULL);

  coffee_t *iter = NULL;
  coffee_foreach(iter, coffee) {
    coffee_delete(iter);
  }
}

coffee_t* coffee_parse(const char *coffee_string) {
  return NULL;
}
coffee_t* coffee_parse_json(const char *json_string) {
  CM_ASSERT(json_string != NULL);

      // CM_TRACELOG("%s", json_string);
  const char *str = json_string;
  coffee_t *root = parse_json_object(&json_string);

  return root;
}

coffee_t* coffee_from_json(coffee_t *coffee_json) {
  CM_ASSERT(coffee_json != NULL);
  coffee_t *out = coffee_json;

  // if ()

  coffee_t *iter = NULL;
  coffee_foreach(iter, out) {
    if (iter->type == CM_TARRAY) {
      int len = coffee_length(iter);
      // cm_object_t *vec = NULL;
      if (len == 2) cm_array_to_vec2(iter);
      else if (len == 3) cm_array_to_vec3(iter);
      else if (len == 4) cm_array_to_vec4(iter);

    } else if (iter->type == CM_TTABLE) {
      iter = coffee_from_json(iter);
    }
  }

  return out;
}

const char* coffee_print_json(coffee_t *coffee_json, int *char_count) {
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

  coffee_t *item = coffee_json;
  switch (item->type) {
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
const char* coffee_print_coffee(coffee_t *coffee);


void coffee_setmetatable(coffee_table_t *table, coffee_table_t *metatable) {}

coffee_table_t* coffee_getmetatable(coffee_table_t *table) {
  return NULL;
}

void coffee_setname(coffee_t *coffee, const char *name) {
  if (!coffee) return;
  if (!name) return;
  int len = strlen(name);
  if (!coffee->name) coffee->name = CM_MALLOC(len);
  else if (len > strlen(coffee->name)) coffee->name = CM_REALLOC(coffee->name, len);
  strcpy(coffee->name, name);
}

const char* coffee_getname(coffee_t *coffee) {
  CM_ASSERT(coffee != NULL);
  return coffee->name;
}

coffee_t* coffee_get(coffee_t *coffee, const char *name) {
  if (!coffee_istable(coffee)) return NULL;
  return coffee_table_get(coffee->table, name);
}

coffee_t* coffee_index(coffee_t *coffee, int index) {
  if (!coffee_istable(coffee) && !coffee_isarray(coffee)) return NULL;
  return coffee_array_get(coffee->table, index);
}

int coffee_length(coffee_t *coffee) {
  if (!coffee_istable(coffee) && !coffee_isarray(coffee)) return -1;
  return coffee->table->len;
}

coffee_table_t* coffee_newtable() {
  coffee_table_t *table = CM_MALLOC(sizeof(*table));
  table->meta = NULL;
  table->child = NULL;
  table->len = 0;
  return table;
}

/*==========*
 *   NULL   *
 *==========*/

int coffee_isnull(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TNULL;
}
coffee_t* coffee_create_null() {
  return coffee_create(COFFEE_TNULL);
}


/*============*
 *   Number   *
 *============*/

coffee_t* coffee_create_number(double value, int type) {
  coffee_t *number = NULL;
  if (type == COFFEE_NUMBER_FLOAT) number = coffee_create_float((float)value);
  else number = coffee_create_int((int)value);
  return number;
}
int coffee_isnumber(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TNUMBER;
}
double coffee_tonumber(coffee_t *coffee) {
  if (!coffee_isnumber(coffee)) return -1;
  return coffee->number.value;
}
double coffee_checknumber(coffee_t *coffee) {
  CM_ASSERT(coffee != NULL);
  CM_ASSERT(coffee->type == COFFEE_TNUMBER);

  return coffee->number.value;
}
void coffee_setnumber(coffee_t *coffee, double number) {
  if (!coffee_isnumber(coffee)) return;

  coffee->number.value = number;
}
coffee_t* coffee_clone_number(coffee_t *coffee) {
  CM_ASSERT(coffee != NULL);
  coffee_t *clone = coffee_create_number(coffee->number.value, coffee->number.type);
  coffee_setname(clone, coffee->name);
  return clone;
}

coffee_t* coffee_create_float(float value) {
  coffee_t *number = coffee_create(COFFEE_TNUMBER);
  number->number.type = COFFEE_NUMBER_FLOAT;
  number->number.value = value;
  return number;
}
int coffee_isfloat(coffee_t *coffee) {
  if (!coffee) return 0;
  if (coffee->type != COFFEE_TNUMBER) return 0;;
  coffee_number_t* number = &coffee->number;
  return number->type == COFFEE_NUMBER_FLOAT;
}
float coffee_tofloat(coffee_t *coffee) {
  if (!coffee) return -1;
  if (coffee->type != COFFEE_TNUMBER) return -1;
  return (float)coffee->number.value;
}
float coffee_checkfloat(coffee_t *coffee) {
  CM_ASSERT(coffee);
  CM_ASSERT(coffee->type == COFFEE_TNUMBER);
  CM_ASSERT(coffee->number.type == COFFEE_NUMBER_FLOAT);

  return (float)coffee->number.value;
}
void coffee_setfloat(coffee_t *coffee, float number) {
  if (!coffee) return;
  if (coffee->type != COFFEE_TNUMBER) return;
  if (coffee->number.type != COFFEE_NUMBER_FLOAT) return;

  coffee->number.value_float.value = number;
}

coffee_t* coffee_create_int(int value) {
  coffee_t *number = coffee_create(COFFEE_TNUMBER);
  number->number.type = COFFEE_NUMBER_INT;
  number->number.value = value;

  return number;
}
int coffee_isint(coffee_t *coffee) {
  if (!coffee) return 0;
  if (coffee->type != COFFEE_TNUMBER) return 0;
  coffee_number_t* number = &coffee->number;
  return number->type == COFFEE_NUMBER_INT;
}
int coffee_toint(coffee_t *coffee) {
  if (!coffee) return -1;
  if (coffee->type != COFFEE_TNUMBER) return -1;
  return (int)coffee->number.value;
}
int coffee_checkint(coffee_t *coffee) {
  CM_ASSERT(coffee);
  CM_ASSERT(coffee->type == COFFEE_TNUMBER);
  CM_ASSERT(coffee->number.type == COFFEE_NUMBER_INT);

  return (float)coffee->number.value;
}
void coffee_setint(coffee_t *coffee, int number) {
  if (!coffee) return;
  if (coffee->type != COFFEE_TNUMBER) return;
  if (coffee->number.type != COFFEE_NUMBER_INT) return;

  coffee->number.value_int.value = number;
}


/*============*
 *   String   *
 *============*/

coffee_t* coffee_create_string(const char *string) {
  coffee_t *coffee = coffee_create(COFFEE_TSTRING);
  if (!string) {
    coffee->string.len = 0;
    coffee->string.value = NULL;
    return coffee;
  }
  int len = strlen(string);
  coffee->string.len = len;
  coffee->string.value = CM_MALLOC(len);
  strcpy(coffee->string.value, string);
  return coffee;
}
int coffee_isstring(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TSTRING;
}
const char* coffee_tostring(coffee_t *coffee) {
  if (!coffee_isstring(coffee)) return NULL;
  return coffee->string.value;
}
const char* coffee_checkstring(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TSTRING);
  return coffee->string.value;
}
void coffee_setstring(coffee_t *coffee, const char *string) {
  if (!coffee_isstring(coffee)) return;
  if (!string) return;

  int len = strlen(string);
  if (len > coffee->string.len) {
    coffee->string.len = len+10;
    coffee->string.value = CM_REALLOC(coffee->string.value, len+10);
  }

  strcpy(coffee->string.value, string);
}
coffee_t* coffee_clone_string(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TSTRING);
  coffee_t *clone = coffee_create_string(NULL);
  if (coffee->string.value) {
    clone->string.len = coffee->string.len;
    clone->string.value = CM_MALLOC(clone->string.len);
    strcpy(clone->string.value, coffee->string.value);
  }
  coffee_setname(clone, coffee->name);

  return clone;
}


/*============*
 *    Bool    *
 *============*/

coffee_t* coffee_create_boolean(int boolean) {
  coffee_t *coffee = coffee_create(COFFEE_TBOOL);
  coffee->number.value = (int)boolean;
  return coffee;
}
int coffee_isboolean(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TBOOL;
}
int coffee_toboolean(coffee_t *coffee) {
  if (!coffee_isboolean(coffee)) return -1;
  return (int)coffee->number.value;
}
int coffee_checkboolean(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TBOOL);
  return (int)coffee->number.value;
}
void coffee_setboolean(coffee_t *coffee, int boolean) {
  if (!coffee_isboolean(coffee)) return;
  coffee->number.value = boolean;
}
coffee_t* coffee_clone_boolean(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TBOOL);
  coffee_t *clone = coffee_create_boolean(coffee->number.value);
  return clone;
}


/*============*
 *    Vec2    *
 *============*/

coffee_t* coffee_create_vec2(double x, double y) {
  coffee_t *coffee = coffee_create(COFFEE_TVEC2);
  coffee_vec2_t *vec = &coffee->vec2;

  vec->x = x;
  vec->y = y;

  return coffee;
}
int coffee_isvec2(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TVEC2;
}
double* coffee_tovec2(coffee_t *coffee) {
  if (!coffee_isvec2(coffee)) return NULL;
  return coffee->vec2.data;
}
double* coffee_checkvec2(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TVEC2);
  return coffee->vec2.data;
}
void coffee_setvec2(coffee_t *coffee, double x, double y) {
  if (!coffee_isvec2(coffee)) return;
  coffee->vec2.x = x;
  coffee->vec2.y = y;
}
coffee_t* coffee_clone_vec2(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TVEC2);
  coffee_vec2_t *vec = &coffee->vec2;
  coffee_t *clone = coffee_create_vec2(vec->x, vec->y);
  coffee_setname(coffee, coffee->name);
  return clone;
}

/*============*
 *    Vec3    *
 *============*/

coffee_t* coffee_create_vec3(double x, double y, double z) {
  coffee_t *coffee = coffee_create(COFFEE_TVEC3);
  coffee_vec3_t *vec = &coffee->vec3;

  vec->x = x;
  vec->y = y;
  vec->z = z;

  return coffee;
}
int coffee_isvec3(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TVEC3;
}
double* coffee_tovec3(coffee_t *coffee) {
  if (!coffee_isvec3(coffee)) return NULL;
  return coffee->vec3.data;
}
double* coffee_checkvec3(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TVEC3);
  return coffee->vec3.data;
}
void coffee_setvec3(coffee_t *coffee, double x, double y, double z) {
  if (!coffee_isvec3(coffee)) return;
  coffee->vec3.x = x;
  coffee->vec3.y = y;
  coffee->vec3.z = z;
}
coffee_t* coffee_clone_vec3(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TVEC3);
  coffee_vec3_t *vec = &coffee->vec3;
  coffee_t *clone = coffee_create_vec3(vec->x, vec->y, vec->z);
  coffee_setname(coffee, coffee->name);
  return clone;
}

/*============*
 *    Vec4    *
 *============*/

coffee_t* coffee_create_vec4(double x, double y, double z, double w) {
  coffee_t *coffee = coffee_create(COFFEE_TVEC4);
  coffee_vec4_t *vec = &coffee->vec4;

  vec->x = x;
  vec->y = y;
  vec->z = z;
  vec->w = w;

  return coffee;
}
int coffee_isvec4(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TVEC4;
}
double* coffee_tovec4(coffee_t *coffee) {
  if (!coffee_isvec4(coffee)) return NULL;
  return coffee->vec4.data;
}
double* coffee_checkvec4(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TVEC4);
  return coffee->vec4.data;
}
void coffee_setvec4(coffee_t *coffee, double x, double y, double z, double w) {
  if (!coffee_isvec4(coffee)) return;
  coffee->vec4.x = x;
  coffee->vec4.y = y;
  coffee->vec4.z = z;
  coffee->vec4.w = w;
}
coffee_t* coffee_clone_vec4(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TVEC4);
  coffee_vec4_t *vec = &coffee->vec4;
  coffee_t *clone = coffee_create_vec4(vec->x, vec->y, vec->z, vec->w);
  coffee_setname(coffee, coffee->name);
  return clone;
}

/*=============*
 *    Array    *
 *=============*/

coffee_t* coffee_create_array() {
  coffee_t *coffee = coffee_create(COFFEE_TARRAY);
  coffee->table = coffee_newtable();
  return coffee;
}
int coffee_isarray(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TARRAY;
}
coffee_table_t* coffee_toarray(coffee_t *coffee) {
  if (!coffee_isarray(coffee)) return NULL;
  return coffee->table;
}
coffee_table_t* coffee_checkarray(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TARRAY);
  return coffee->table;
}
coffee_table_t* coffee_setarray(coffee_t *coffee, coffee_table_t *array) {
  if (!coffee_isarray(coffee)) return NULL;
  if (!array) return NULL;

  coffee_table_t *old = coffee->table;
  coffee->table = array;
  return old;
}

// int coffee_length(coffee_t *coffee) {
//   if (!coffee_isarray(coffee) && !coffee_istable(coffee)) return -1;
//   return coffee->table->len;
// }

int coffee_array_length(coffee_table_t *array) {
  CM_ASSERT(array != NULL);
  return array->len;
}
coffee_t* coffee_array_get(coffee_table_t *array, int index) {
  CM_ASSERT(array != NULL);

  coffee_t *iter = NULL;
  int i = 0;
  coffee_table_foreach(iter, array) {
    if (index == i) return iter;
  }

  return NULL;
}
void coffee_array_set(coffee_table_t *array, int index, coffee_t *value) {
  CM_ASSERT(array != NULL);
  int i = 0;
  if (index < 0 && array->len > 0) i = array->len - i;

  coffee_t *iter = NULL;
  coffee_table_foreach(iter, array) {
    if (i == index) {
      if (i == 0) array->child = value;
      coffee_t *prev = iter->prev;

      iter->prev = value;
      if (prev) prev->next = value;

      value->next = iter;
      value->prev = prev;
      array->len++;
      return;
    }
  }
}
void coffee_array_push(coffee_table_t *array, coffee_t *value) {
  CM_ASSERT(array != NULL);
  if (!value) return;
  coffee_t *iter = array->child;
  if (!iter) {
    array->child = value;
    return;
  }
  while (iter->next) {
    iter = iter->next;
  }

  iter->next = value;
  value->prev = iter;
  array->len++;
}
coffee_t* coffee_array_pop(coffee_table_t *array) {
  CM_ASSERT(array != NULL);
  coffee_t *iter = array->child;
  if (!iter) return NULL;
  while (iter->next) {
    iter = iter->next;
  }
  coffee_t *prev = iter->prev;
  prev->next = NULL;

  iter->prev = NULL;
  array->len--;

  return iter;
}
coffee_t* coffee_clone_array(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TARRAY);

  coffee_t *clone = coffee_create_array();
  coffee_t *iter = NULL;
  coffee_table_foreach(iter, coffee->table) {
    coffee_t *item_clone = coffee_clone(iter);
    if (item_clone) coffee_array_push(clone->table, item_clone);
  }
  coffee_setname(coffee, coffee->name);

  return clone;
}

/*=============*
 *    Table    *
 *=============*/


coffee_t* coffee_create_table() {
  coffee_t *coffee = coffee_create(COFFEE_TTABLE);
  coffee->table = coffee_newtable();
  return coffee;
}
int coffee_istable(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TTABLE;
}
coffee_table_t* coffee_totable(coffee_t *coffee) {
  if (!coffee_istable(coffee)) return NULL;
  return coffee->table;
}
coffee_table_t* coffee_checktable(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TTABLE);
  return coffee->table;
}
coffee_table_t* coffee_settable(coffee_t *coffee, coffee_table_t *table) {
  if (!coffee_istable(coffee)) return NULL;
  if (!table) return NULL;

  coffee_table_t *old = coffee->table;
  coffee->table = table;
  return old;
}
int coffee_table_length(coffee_table_t *table) {
  CM_ASSERT(table != NULL);
  return table->len;
}
coffee_t* coffee_table_get(coffee_table_t *table, const char *name) {
  CM_ASSERT(table != NULL);
  if (!name) return NULL;

  coffee_t *iter = NULL;
  int i = 0;
  coffee_table_foreach(iter, table) {
    CM_ASSERT(iter->name != NULL);
    if (!strcmp(iter->name, name)) return iter;
  }

  return NULL;
}
void coffee_table_set(coffee_table_t *table, const char *name, coffee_t *value) {
  CM_ASSERT(table != NULL);
  if (!name) return;
  if (!value) return;

  coffee_t *val = coffee_table_get(table, name);
  if (val) return; 

  coffee_setname(value, name);
  coffee_array_push(table, value);
  table->len++;
}
void coffee_table_remove(coffee_table_t *table, const char *name) {
  CM_ASSERT(table != NULL);
  coffee_t *iter = coffee_table_get(table, name);
  if (!iter) return;

  if (table->child == iter) table->child = iter->next;
  coffee_delete(iter);
}
coffee_t* coffee_clone_table(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TARRAY);

  coffee_t *clone = coffee_create_table();
  coffee_t *iter = NULL;
  coffee_table_foreach(iter, coffee->table) {
    coffee_t *item_clone = coffee_clone(iter);
    if (item_clone) coffee_array_push(clone->table, item_clone);
  }
  coffee_setname(coffee, coffee->name);

  return clone;
}

/*================*
 *    Userdata    *
 *================*/

coffee_t* coffee_create_userdata(void *udata) {
  CM_ASSERT(udata != NULL);
  coffee_t *coffee = coffee_create(COFFEE_TUSERDATA);
  coffee->userdata.data = udata;
  return coffee;
}
int coffee_isuserdata(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TUSERDATA;
}
void* coffee_touserdata(coffee_t *coffee) {
  if (!coffee_isuserdata(coffee)) return NULL;
  return coffee->userdata.data;
}
void* coffee_checkudata(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TUSERDATA);
  return coffee->userdata.data;
}
void* coffee_setuserdata(coffee_t *coffee, void *userdata) {
  void *old = coffee_touserdata(coffee);
  coffee->userdata.data = userdata;
  return old;
}
coffee_t* coffee_clone_userdata(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TUSERDATA);
  coffee_t *clone = coffee_create_userdata(coffee->userdata.data);
  coffee_setname(clone, coffee->name);
  return clone;
}

/*================*
 *    Function    *
 *================*/

coffee_t* coffee_create_function(coffee_fn *func) {
  CM_ASSERT(func != NULL);
  coffee_t *coffee = coffee_create(COFFEE_TFUNCTION);
  coffee->func = func;
  return coffee;
}
int coffee_isfunction(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TFUNCTION;
}
coffee_fn* coffee_tofunction(coffee_t *coffee) {
  if (!coffee_isfunction(coffee)) return NULL;

  return coffee->func;
}
coffee_fn* coffee_checkfunction(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TFUNCTION);
  return coffee->func;
}
void coffee_setfunction(coffee_t *coffee, coffee_fn* fn) {
  if (!coffee_isfunction(coffee)) return;
  coffee->func = fn;
}
int coffee_call_function(coffee_t *coffee) {
  return 0;
}
coffee_t* coffee_clone_function(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TFUNCTION);
  coffee_t *clone = coffee_create_function(coffee->func);
  coffee_setname(clone, coffee->name);
  return clone;
}

/*===========*
 *    Ref    *
 *===========*/

coffee_t* coffee_create_ref(coffee_t *ref) {
  CM_ASSERT(ref != NULL);
  coffee_t *coffee = coffee_create(COFFEE_TREF);
  coffee->ref = ref;
  return coffee;
}
int coffee_isref(coffee_t *coffee) {
  if (!coffee) return 0;
  return coffee->type == COFFEE_TREF;
}
coffee_t* coffee_toref(coffee_t *coffee) {
  if (!coffee_isref(coffee)) return NULL;
  return coffee->ref;
}
coffee_t* coffee_checkref(coffee_t *coffee) {
  CHECK_TYPE(coffee, COFFEE_TREF);
  return coffee->ref;
}
coffee_t* coffee_setref(coffee_t *coffee, coffee_t *ref) {
  if (!coffee_isref(coffee)) return NULL;
  if (!ref) return NULL;

  coffee_t *old_ref = coffee->ref;
  coffee->ref = ref;

  return old_ref;
}
coffee_t* coffee_clone_ref(coffee_t* coffee) {
  CHECK_TYPE(coffee, COFFEE_TREF);
  coffee_t *clone = coffee_create_ref(coffee->ref);
  coffee_setname(clone, coffee->name);
  return clone;
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



/*=====================================*
 *            DON'T USE                *
 *=====================================*/

cm_object_t* cm_parse_json(const char *json_string) {
  CM_ASSERT(json_string != NULL);

      // CM_TRACELOG("%s", json_string);
  const char *str = json_string;
  cm_object_t *root = parse_json_object(&json_string);

  return root;
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
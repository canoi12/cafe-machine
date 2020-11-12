# coffee-machine

coffee machine is a tiny c vm

I'm still working on the vm, but can use the `coffee_` lib to create data structures, load from json, etc.

main.c
```c
#include "coffee.h"

const char *cat_json = "{\"name\":\"Yoda\",\"type\":\"Cat\"}";

int check_cat(coffee_t *cat) {
  const char *type = coffee_table_getstring(cat->table, "type");
  if (!type) return 0;
  if (strcmp(type, "Cat")) return 0;

  const char *name = coffee_table_getstring(cat->table, "name");
  coffee_table_setboolean(cat->table, "check", CM_TRUE);
  coffee_table_setboolean(cat->table, "fofin", CM_TRUE);
  
  CM_TRACELOG("name: %s", name);
  return 1;
}

int main(int argc, char ** argv) {
  cm_init();
  coffee_t *cat = coffee_parse_json(cat_json);
  check_cat(cat); // print "name: Yoda"

  const char *str = coffee_print_json(cat, NULL); 
  CM_TRACELOG("%s", str); // print {"name": "Yoda","type": "Cat","check": true,"fofin": true}

  
  cm_deinit();
  return 0;
}
```

## TODOS

Later i plan to create a format that the vm can interpret, and can convert to json:

```c
// components/transform.coffee
{
  "position": <vec2>(10, 20),
  "scale": <vec2>,
  "angle": 0
}
// player.coffee
{
  "transform": <require>("components.transform"){
    "scale": <vec2>(1, 1)
  },
  "speed": <vec2>
}
```

```json
// components/transform.json
{
  "position": [10, 20],
  "scale": [0, 0],
  "angle": 0
}
// player.json
{
  "transform": {
    "position": [10, 20],
    "scale": [1, 1],
    "angle": 0
  },
  "speed": [0, 0]
}
```

Or referenciate just for use in development, it will not be added in the final json

```c
// options.coffee
[
  "opt1",
  "opt2",
  "opt3"
]

// menu.coffee
{
  "option": "opt1",
  "options": <ref>("options")
}

```

```c
// menu.json
{
  "option": "opt1"
}
```

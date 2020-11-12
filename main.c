#include "coffee.h"

const char *cat_json = "{\"name\":\"Yoda\",\"type\":\"Cat\"}";

int check_cat(coffee_t *cat) {
  const char *type = coffee_table_getstring(cat->table, "type");
  if (!type) return 0;
  if (strcmp(type, "Cat")) return 0;

  // coffee_t *name = coffee_get(cat, "name");
  const char *name = coffee_table_getstring(cat->table, "name");
  // coffee_set()
  coffee_table_setboolean(cat->table, "check", CM_TRUE);
  coffee_table_setboolean(cat->table, "fofin", CM_TRUE);
  
  CM_TRACELOG("%s", name);
  return 1;
}

int main(int argc, char ** argv) {
	cm_init();
  coffee_t *cat = coffee_parse_json(cat_json);
  check_cat(cat); // print "Yoda"

  const char *str = coffee_print_json(cat, NULL); 
  CM_TRACELOG("%s", str); // print {\"name\": \"Yoda\",\"type\": \"Cat\",\"check\": true,\"fofin\": true}

  
  cm_deinit();
	return 0;
}
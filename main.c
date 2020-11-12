#include "coffee.h"

int main(int argc, char ** argv) {
	cm_init();
	coffee_t *coffee = coffee_load_json("json/sum.json");

	const char *str = coffee_print_json(coffee, NULL);
	CM_TRACELOG("%s", str);
	
	CM_FREE((char*)str);
	coffee_delete(coffee);
	
	cm_deinit();
	return 0;
}
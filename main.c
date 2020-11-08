#include "coffee.h"

int check_player(cm_object_t *object) {
	cm_vec2_t vec = cm_object_get_vec2(object, "transform//position");
	// cm_object_t *str = cm_object_get(vec, "string");

	// CM_TRACELOG("teste");

	CM_TRACELOG("%f %f", vec.data[0], vec.data[1]);
	return 0;
}

int main(int argc, char ** argv) {
	cm_vm_init();

	cm_object_t *obj = cm_object_load("teste.json");
	check_player(obj);

	cm_vm_deinit();
	return 0;
}
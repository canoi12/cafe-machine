#include "coffee.h"

int check_player(cm_object_t *object) {
	cm_object_t *array = cm_object_get(object, "qqq");
	cm_object_t *item = NULL;
	cm_object_foreach(item, array) {
		const char *str = cm_object_to_opt_string(item, "tem nada aqui");
		CM_TRACELOG("item %p", item);
		if (item->type == CM_TNUMBER) CM_TRACELOG("\t<number> %f", item->number); 
		else if (item->type == CM_TSTRING) CM_TRACELOG("\t<string> %s", item->string);
		else if (item->type == CM_TTABLE) CM_TRACELOG("\t<table> %p", item);
	}
	return 0;
}

int main(int argc, char ** argv) {
	cm_vm_init();

	cm_object_t *obj = cm_object_load("teste.json");
	check_player(obj);

	cm_object_t *el = NULL;
	cm_object_foreach(el, obj) {
		CM_TRACELOG("%s %d", el->name, el->type);
	}

	cm_vm_deinit();
	return 0;
}
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

int sum(coffee_machine_t *vm) {
	int top = cm_gettop(vm);
	float total = 0;

	for (int i = 1; i < top; i++) {
		total += cm_tonumber(vm, i);
	}

	cm_pushnumber(vm, total);
	return 1;
}

enum {
	OP_NONE = 0,
	OP_SUM,
	OP_MUL,
	OP_SUB
};

int sum_json(coffee_machine_t *vm) {
	float n1 = cm_tonumber(vm, 1);
	int op = cm_tonumber(vm, 2);
	float n2 = cm_tonumber(vm, 3);
	float res = 0;
	switch (op) {
		case OP_SUM:
			res = n1 + n2;
			break;
		case OP_MUL:
			res = n1 * n2;
			break;
		case OP_SUB:
			res = n1 - n2;
			break;
	}

	cm_pushnumber(vm, res);
	return 1;
}

int main(int argc, char ** argv) {
	cm_init();
	coffee_machine_t *vm = cm_get_vm();

	cm_object_t *tst = cm_object_load("json/sum.json");

	cm_pushcfunction(vm, &sum_json);
	cm_pushnumber(vm, cm_object_get_number(tst, "arg0"));
	cm_pushnumber(vm, cm_object_get_number(tst, "op"));
	cm_pushnumber(vm, cm_object_get_number(tst, "arg1"));

	cm_call(vm, 3, 1);

	float n = cm_tonumber(vm, 0);
	CM_TRACELOG("%.0f", n);
	cm_deinit();
	return 0;
}
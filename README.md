# coffee-machine

cat.json
```json
{
  "name": "Yoda",
  "type": "Cat"
}
```

main.c
```c
 
int check_cat(cm_object_t *cat) {
  cm_object_t *type = cm_object_get(cay, "type");
  if (!type) return 0;
  cm_object_t *name = cm_object_get(cat, "name");
  
  CM_TRACELOG("%s", cm_object_to_string(name));
  return 1;
}
 

int main(int argc, char ** argv) {
  cm_vm_init();
  cm_object_t *cat = cm_object_load("cat.json");
  
  
  cm_vm_deinit();
}

```


#include <stdint.h>

uint32_t lang_integer_create(int32_t value) {
	ObjectHeader* object = lang_alloc_object(0, sizeof(int32_t));
	int32_t* data = lang_object_data(handle);
	*data = value;
	lang_return_object(handle);
}

uint32_t lang_integer_add(ObjectHeader *a, ObjectHeader *b) {
	int32_t* a_data = lang_object_data(a);
	int32_t* b_data = lang_object_data(b);
	return lang_integer_create(*a_data + *b_data);
}
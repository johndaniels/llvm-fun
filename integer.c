
#include <stdint.h>

uint32_t lang_integer_create(int32_t value) {
	uint32_t handle = lang_alloc_object(0, sizeof(int32_t));
	int32_t* data = lang_object_data(lang_checkout_object(handle));
	*data = value;
	lang_return_object(handle);
}

uint32_t lang_integer_add(uint32_t a, uint32_t b) {
	int32_t* a_data = lang_object_data(lang_checkout_object(a));
	int32_t* b_data = lang_object_data(lang_checkout_object(b));
	uint32_t result_handle = lang_integer_create(*a_data + *b_data);
	lang_return_object(a);
	lang_return_object(b);

	return result_handle;
}
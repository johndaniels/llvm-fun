#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
	uint32_t size;
	uint32_t reference_len;
} ObjectHeader;

#define LANG_SYSTEM_LOCK 1

typedef struct {
	pthread_mutex_t mutex;
	uint16_t refcount;
	uint16_t flags;
	uint32_t location;
} ObjectEntry;

static ObjectEntry* object_data;
static size_t object_data_len;

static unsigned char* object_heap;
static size_t object_heap_len;
static size_t object_heap_used;

void lang_alloc_object(size_t references, size_t size) {
	object_heap_used += sizeof(ObjectHeader) + sizeof(uint32_t) * references + size;
}

void lang_initialize() {
	object_data = malloc(sizeof(ObjectEntry) * 1024);
	if (!object_data) {
		exit(1);
	}
	object_heap = malloc(10000);
	if (!object_data) {
		exit(1);
	}
}

int32_t print(int32_t param) {
	printf("%d\n", param);
}

/*int main() {
	printf("%lu", sizeof(pthread_spinlock_t));
	printf("%d", atomic_is_lock_free(ATOMIC_INT_LOCK_FREE));
}*/
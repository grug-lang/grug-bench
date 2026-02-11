#include <stdio.h>
#include <assert.h>
#include <profileapi.h>
#include "bench.h"

#ifdef WIN32
static uint64_t get_timestamp_resolution() {
	uint64_t resolution = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&resolution);
	return resolution;
}

static uint64_t get_timestamp() {
	uint64_t time_stamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&time_stamp);
	return time_stamp;
}
#endif /* WIN32 */

static double print_value = 0;
void game_fn_print_number(void* state, union grug_value* arguments) {
	(void)(state);
	print_value = arguments[0].number;
}

static double get_1_call_count = 0;
union grug_value game_fn_get_1(void* state) {
	(void)(state);
	get_1_call_count++;
	return (union grug_value) {.number = 1.};
}

void runtime_error_handler(
	char const* reason, 
	enum grug_error_type type, 
	char const* on_fn_name, 
	char const* on_fn_path
) {
	(void)(type);
	printf("unexpected runtime error: %s in file %s in function %s\n", reason, on_fn_name, on_fn_path);
	exit(1);
}

void grug_bench_run(
	const char* mod_api_path,
	const char* mods_dir,
	struct grug_state_vtable* grug_state_vtable
) {
	void* state = grug_state_vtable->create_grug_state(mod_api_path, mods_dir);

	void* prnt_fn_id = grug_state_vtable->get_on_fn_id(state, "Bench", "on_print");
	void* incr_fn_id = grug_state_vtable->get_on_fn_id(state, "Bench", "on_increment");

	void* file = grug_state_vtable->compile_grug_file(state, "bench/basic-Bench.grug");
	void* entity = grug_state_vtable->create_entity(state, file);

	// run both
	grug_state_vtable->call_entity_on_fn(state, entity, incr_fn_id);
	grug_state_vtable->call_entity_on_fn(state, entity, prnt_fn_id);
	assert(print_value == 1.0);
	assert(get_1_call_count == 1);

	uint64_t start_time = get_timestamp();
	// run 1B times; 
	for (size_t i = 0; i < 1000 * 1000 * 1000; i++) {
		grug_state_vtable->call_entity_on_fn(state, entity, incr_fn_id);
	}
	uint64_t end_time = get_timestamp();
	uint64_t resolution = get_timestamp_resolution();

	grug_state_vtable->call_entity_on_fn(state, entity, prnt_fn_id);
	assert(print_value == 1000 * 1000 * 1000 + 1);
	assert(get_1_call_count == 1000 * 1000 * 1000 + 1);

	printf("time taken: %lf seconds\n", ((double)(end_time) - (double)(start_time)) / (double)(resolution));
	
	grug_state_vtable->destroy_entity(state, entity);
	grug_state_vtable->destroy_grug_state(state);
	
	return;
}

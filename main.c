#include <stdio.h>
#include <assert.h>
#include "main.h"

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

	// run 1B times; 
	for (size_t i = 0; i < 1000 * 1000 * 1000; i++) {
		grug_state_vtable->call_entity_on_fn(state, entity, incr_fn_id);
	}

	grug_state_vtable->call_entity_on_fn(state, entity, prnt_fn_id);
	assert(print_value == 1000 * 1000 * 1000 + 1);
	assert(get_1_call_count == 1000 * 1000 * 1000 + 1);

	grug_state_vtable->destroy_entity(state, entity);
	grug_state_vtable->destroy_grug_state(state);
	
	return;
}

/* static void runtime_error_handler(const char *reason, enum grug_runtime_error_type type, const char *on_fn_name, const char *on_fn_path) { */
/* 	(void)type; */
/* 	fprintf(stderr, "grug runtime error in %s(): %s, in %s\n", on_fn_name, reason, on_fn_path); */
/* } */

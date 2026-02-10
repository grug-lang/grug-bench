#include <stdio.h>
#include <libloaderapi.h>
#include <assert.h>
#include <string.h>
#include "main.h"

/* platform specific code */
void* load_library(const char* path) {
	return LoadLibraryExA(path, NULL, 0);
}

void* load_symbol(void* dll, const char* proc_name) {
	return GetProcAddress(dll, proc_name);
}
/* platform specific code */

/* game functions */
static void             (*game_fn_print_number)(void* state, union grug_value* arguments) = {0};
static union grug_value (*game_fn_get_1       )(void* state                             ) = {0};
/* game functions */

typedef typeof(&grug_bench_run) p_grug_bench_run;

/* on functions */
typedef void (*on_fn_ptr)(void* state, double* entity_data);

void on_print(void* state, double* entity_data) {
	game_fn_print_number(state, &(union grug_value) {.number = *entity_data});
}

void on_increment(void* state, double* entity_data) {
	*entity_data += game_fn_get_1(state).number;
}
/* on functions */

/* vtable functions */
void* create_grug_state(const char* mod_api_path, const char* mods_dir) {
	printf("Creating state\n");
	printf("mod_api.json: %s\n", mod_api_path);
	printf("mods_dir: %s\n", mods_dir);
	return NULL;
}

void destroy_grug_state(void* state) {
	printf("Destroying state\n");
	(void)(state);
}

void* compile_grug_file(void* state, const char* file_path) {
	(void)state;
	(void)file_path;
	return (void*)1;
}

void* create_entity(void* state, void* grug_script_id) {
	(void)state;
	assert((size_t)grug_script_id == 1);
	
	double* entity_data = malloc(sizeof(double));
	*entity_data = 0.0;
	return entity_data;
}

void destroy_entity(void* state, void* entity) {
	(void)state;
	free(entity);
}

void* get_on_fn_id(void* state, const char* entity_type, const char* function_name) {
	(void)(state);
	assert(strcmp(entity_type, "Bench") == 0);
	if (strcmp(function_name, "on_print") == 0) {
		return (void*)on_print;
	} else if (strcmp(function_name, "on_increment") == 0) {
		return (void*)on_increment;
	} else {
		return NULL;
	}
}

void call_entity_on_fn(void* state, void* entity_data, void* on_fn_id) {
	((on_fn_ptr)on_fn_id)(state, entity_data);
}
/* vtable functions */

int main () {
	void* dll = load_library("main.dll");
	if (!dll) {
		fprintf(stderr, "could not open dll");
		exit(1);
	}
	p_grug_bench_run grug_bench_run = load_symbol(dll, "grug_bench_run"      );
	game_fn_print_number            = load_symbol(dll, "game_fn_print_number");
	game_fn_get_1                   = load_symbol(dll, "game_fn_get_1"       );
	if (!(grug_bench_run && game_fn_print_number && game_fn_print_number)) {
		fprintf(stderr, "could not load symbols\n");
		return 1;
	}

	struct grug_state_vtable vtable = {
		.create_grug_state   = create_grug_state,
		.destroy_grug_state  = destroy_grug_state,
		.compile_grug_file   = compile_grug_file,
		.create_entity       = create_entity,
		.destroy_entity      = destroy_entity,
		.get_on_fn_id        = get_on_fn_id,
		.call_entity_on_fn   = call_entity_on_fn,
	};
	grug_bench_run(
		"./mod_api.json",
		"./mods",
		&vtable
	);
	printf("hello from smoketest.c");
	return 0;
}

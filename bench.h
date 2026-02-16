#pragma once

#include<stdint.h>

enum grug_error_type {
	stack_overflow = 0,
	runtime_exceeded = 1,
	game_fn_error = 2
};

union grug_value {
	double number;
	uint64_t id;
};

typedef void* (*create_grug_state_t)(const char* mod_api_path, const char* mods_dir);
typedef void (*destroy_grug_state_t)(void* state);

typedef void* (*compile_grug_file_t)(void* state, const char* file_path);
typedef void* (*create_entity_t)(void* state, void* grug_script_id);
typedef void* (*get_on_fn_id_t)(void* state, const char* entity_type, const char* function_name);
typedef void  (*call_entity_on_fn_t)(void* state, void* entity, void* on_fn_id, union grug_value* value, size_t values_len);
typedef void  (*destroy_entity_t)(void* state, void* entity);

struct grug_state_vtable {
	create_grug_state_t create_grug_state;
	destroy_grug_state_t destroy_grug_state;

	compile_grug_file_t compile_grug_file;
	create_entity_t create_entity;
	get_on_fn_id_t get_on_fn_id;
	call_entity_on_fn_t call_entity_on_fn;
	destroy_entity_t destroy_entity;
};

/* call this function to run the benchmarks */
void grug_bench_run(
	const char* mod_api_path,
	const char* mods_dir,
	struct grug_state_vtable* vtable
);

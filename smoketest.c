#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bench.h"

/* platform specific code */
#if defined(_WIN32)
#include <windows.h>
void* load_library(const char* path) {
	return LoadLibraryExA(path, NULL, 0);
}

void* load_symbol(void* dll, const char* proc_name) {
	return (void*)GetProcAddress(dll, proc_name);
}
#elif defined(__linux__) /* end _WIN32 */
#include <dlfcn.h>
void* load_library(const char* path) {
	return dlopen(path, RTLD_NOW);
}

void* load_symbol(void* dll, const char* proc_name) {
	return (void*)dlsym(dll, proc_name);
}
#endif /* end __linux__ */
/* platform specific code */

/* game functions */
static void             (*game_fn_print_number)(void* state, union grug_value* arguments) = {0};
static union grug_value (*game_fn_get_1       )(void* state                             ) = {0};
/* game functions */

typedef __typeof__(&grug_bench_run) p_grug_bench_run;

/* on functions */
typedef void (*on_fn_ptr)(void* state, double* entity_data, union grug_value* values, size_t values_len);

void on_print(void* state, double* entity_data, union grug_value* values, size_t values_len) {
	(void)(values);
	(void)(values_len);
	game_fn_print_number(state, &(union grug_value) {.number = *entity_data});
}

void on_increment(void* state, double* entity_data, union grug_value* values, size_t values_len) {
	(void)(values);
	(void)(values_len);
	*entity_data += game_fn_get_1(state).number;
}

double calc_fib(double i) {
	if (i < 0) return 0;
	if (i <= 2) return 1;
	return calc_fib(i - 1) + calc_fib(i - 2);
}

void on_fib(void* state, double* entity_data, union grug_value* values, size_t values_len) {
	(void)(entity_data);
	(void)(values);
	(void)(values_len);
	assert(values_len == 1);
	double i = values->number;
	game_fn_print_number(state, &(union grug_value) {.number = calc_fib(i)});
	
	/* if (i < 0.0) { */
	/* 	game_fn_print_number(state, &(union grug_value) {.number = 0.0}); */
	/* } else if (i <= 2.) { */
	/* 	game_fn_print_number(state, &(union grug_value) {.number = 1.0}); */
	/* } else { */
	/* 	double a = 1; */
	/* 	double b = 1; */
	/* 	while (i > 2) { */
	/* 		double temp = a + b; */
	/* 		b = a; */
	/* 		a = temp; */
	/* 		i -= 1; */
	/* 	} */
	/* 	game_fn_print_number(state, &(union grug_value) {.number = a}); */
	/* } */
}
/* on functions */

/* vtable functions */
void* create_grug_state(const char* mod_api_path, const char* mods_dir) {
	(void)(mod_api_path);
	(void)(mods_dir);
	return NULL;
}

void destroy_grug_state(void* state) {
	(void)(state);
}

void* compile_grug_file(void* state, const char* file_path) {
	(void)state;
	(void)file_path;
	if (strcmp(file_path, "bench/basic-Bench.grug") == 0) {
		return (void*)1;
	} else if (strcmp(file_path, "bench/fib-FibBench.grug") == 0) {
		return (void*)2;
	}
	exit(2);
}

void* create_entity(void* state, void* grug_script_id) {
	(void)state;
	if ((size_t)grug_script_id == 1) {
		double* entity_data = malloc(sizeof(double));
		*entity_data = 0.0;
		return entity_data;
	} else if ((size_t)grug_script_id == 2) {
		return NULL;
	}
	exit(2);
}

void destroy_entity(void* state, void* entity) {
	(void)state;
	if (!entity) free(entity);
}

void* get_on_fn_id(void* state, const char* entity_type, const char* function_name) {
	(void)(state);
	(void)(entity_type);
	if (strcmp(entity_type, "Bench") == 0) {
		if (strcmp(function_name, "on_print") == 0) {
			return (void*)on_print;
		} else if (strcmp(function_name, "on_increment") == 0) {
			return (void*)on_increment;
		} else {
			return NULL;
		}
	} else if (strcmp(entity_type, "FibBench") == 0) {
		if (strcmp(function_name, "on_fib") == 0) {
			return (void*)on_fib;
		} else {
			return NULL;
		}
	}
	exit(2);
}

void call_entity_on_fn(void* state, void* entity_data, void* on_fn_id, union grug_value* values, size_t values_len) {
	((on_fn_ptr)on_fn_id)(state, entity_data, values, values_len);
}
/* vtable functions */

int main () {
	void* dll;
	(dll = load_library("bench.dll"))? (void)(0): 
	(dll = load_library("libbench.dll"))? (void)(0): 
	(dll = load_library("libbench.so"))? (void)(0): NULL;
	if (!dll) {
		fprintf(stderr, "could not open dll\n");
		exit(1);
	}
	p_grug_bench_run grug_bench_run = (__typeof__(p_grug_bench_run    ))load_symbol(dll, "grug_bench_run"      );
	game_fn_print_number            = (__typeof__(game_fn_print_number))load_symbol(dll, "game_fn_print_number");
	game_fn_get_1                   = (__typeof__(game_fn_get_1       ))load_symbol(dll, "game_fn_get_1"       );
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
	return 0;
}

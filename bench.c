#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bench.h"

#if defined(_WIN32)
#include <windows.h>
static uint64_t get_timestamp_frequency() {
	uint64_t resolution = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&resolution);
	return resolution;
}

static uint64_t get_timestamp() {
	uint64_t time_stamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&time_stamp);
	return time_stamp;
}
#elif defined(__linux__) /* end WIN32 */
#include <time.h>
#define BIL(n) ((n) * 1000 * 1000 * 1000)
static uint64_t get_timestamp_resolution() {
	return BIL(1);
}

static uint64_t get_timestamp() {
	struct timespec time = {0};
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return (uint64_t)(time.tv_sec * BIL(1)) + (uint32_t)(time.tv_nsec);
}
#endif /* linux */

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

void run_on_function_test(
	void* state,
	struct grug_state_vtable* grug_state_vtable
) {
	void* prnt_fn_id = grug_state_vtable->get_on_fn_id(state, "Bench", "on_print");
	void* incr_fn_id = grug_state_vtable->get_on_fn_id(state, "Bench", "on_increment");

	void* file = grug_state_vtable->compile_grug_file(state, "bench/basic-Bench.grug");
	void* entity = grug_state_vtable->create_entity(state, file);
	
	printf("Running on function test\n");
	fflush(stdout);

	// run both
	grug_state_vtable->call_entity_on_fn(state, entity, incr_fn_id, NULL, 0);
	grug_state_vtable->call_entity_on_fn(state, entity, prnt_fn_id, NULL, 0);
	assert(print_value == 1.0);
	assert(get_1_call_count == 1);

	uint64_t start_time = get_timestamp();
	// run 1B times; 
	#define NUM_ITERATIONS 1000 * 1000 * 1
	for (size_t i = 0; i < NUM_ITERATIONS; i++) {
		grug_state_vtable->call_entity_on_fn(state, entity, incr_fn_id, NULL, 0);
	}
	uint64_t end_time = get_timestamp();
	uint64_t frequency = get_timestamp_frequency();

	grug_state_vtable->call_entity_on_fn(state, entity, prnt_fn_id, NULL, 0);
	assert(print_value == NUM_ITERATIONS + 1);
	assert(get_1_call_count == NUM_ITERATIONS + 1);

	printf("time taken: %lf seconds\n", ((double)(end_time) - (double)(start_time)) / (double)(frequency));
	
	grug_state_vtable->destroy_entity(state, entity);
}

double calc_fib (double i) {
	if (i < 0.0) {
		return 0.0;
	} else if (i <= 2.) {
		return 1.0;
	} else {
		double a = 1;
		double b = 1;
		while (i > 2) {
			double temp = a + b;
			b = a;
			a = temp;
			i -= 1;
		}
		return a;
	}
}

void run_fibonacci_test(
	void* state,
	struct grug_state_vtable* grug_state_vtable
) {
	void* on_fib_id = grug_state_vtable->get_on_fn_id(state, "FibBench", "on_fib");

	void* file = grug_state_vtable->compile_grug_file(state, "bench/fib-FibBench.grug");
	void* entity = grug_state_vtable->create_entity(state, file);
	
	printf("Running fibonacci function test\n");
	fflush(stdout);

	uint64_t frequency = get_timestamp_frequency();

	for (size_t i = 0;; i++) {
		uint64_t start = get_timestamp();
		grug_state_vtable->call_entity_on_fn(
			state,
			entity, 
			on_fib_id, 
			(union grug_value[]){
				(union grug_value){.number = (double)i}
			}, 
			1
		);
		assert(print_value == calc_fib((double)i) && "mismatched output");
		uint64_t end = get_timestamp();
		// if the calculation takes more than 1 second
		if (end - start > frequency) {
			printf("Maximum fibonacci number calculated within 1 second: %zu", i);
			break;
		}
	}
	
	grug_state_vtable->destroy_entity(state, entity);
}

void grug_bench_run(
	const char* mod_api_path,
	const char* mods_dir,
	struct grug_state_vtable* grug_state_vtable
) {
	void* state = grug_state_vtable->create_grug_state(mod_api_path, mods_dir);

	run_on_function_test(state, grug_state_vtable);
	run_fibonacci_test(state, grug_state_vtable);

	grug_state_vtable->destroy_grug_state(state);
	return;
}

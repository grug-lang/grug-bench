#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "bench.h"

#if defined(_WIN32)
#include <windows.h>
static uint64_t get_timestamp_frequency() {
	uint64_t frequency = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	return frequency;
}

static uint64_t get_timestamp() {
	uint64_t time_stamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&time_stamp);
	return time_stamp;
}
#elif defined(__linux__) /* end WIN32 */
#include <time.h>
#define BIL(n) ((n) * 1000 * 1000 * 1000)
static uint64_t get_timestamp_frequency() {
	return BIL(1);
}

static uint64_t get_timestamp() {
	struct timespec time = {0};
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return (uint64_t)(time.tv_sec * BIL(1)) + (uint32_t)(time.tv_nsec);
}
#endif /* linux */

/* Game functions */
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

union grug_value game_fn_get_number(void* state) {
	(void)(state);
	static size_t count = 0;
	return (union grug_value){.number = (double)(count++)};
}

struct ParticleData {
	double mass;
	double x;
	double y;
};

static struct ParticleData* particles     = NULL;
static size_t               particles_len = 0   ;

union grug_value game_fn_get_mass(void* state, union grug_value* values) {
	(void)(state);

	size_t index = (size_t)values[0].number;
	if (index > particles_len) {
		printf("Particle index %zu out of bounds", index);
		exit(1);
	}
	return (union grug_value){.number = particles[index].mass};
}
union grug_value game_fn_x(void* state, union grug_value* values) {
	(void)(state);

	size_t index = (size_t)values[0].number;
	if (index > particles_len) {
		printf("Particle index %zu out of bounds", index);
		exit(1);
	}
	return (union grug_value){.number = particles[index].x};
}
union grug_value game_fn_y(void* state, union grug_value* values) {
	(void)(state);

	size_t index = (size_t)values[0].number;
	if (index > particles_len) {
		printf("Particle index %zu out of bounds", index);
		exit(1);
	}
	return (union grug_value){.number = particles[index].y};
}
void game_fn_set_x(void* state, union grug_value* values) {
	(void)(state);

	size_t index = (size_t)values[0].number;
	double value = values[1].number;
	if (index > particles_len) {
		printf("Particle index %zu out of bounds", index);
		exit(1);
	}
	particles[index].x = value;
}
void game_fn_set_y(void* state, union grug_value* values) {
	(void)(state);

	size_t index = (size_t)values[0].number;
	double value = values[1].number;
	if (index > particles_len) {
		printf("Particle index %zu out of bounds", index);
		exit(1);
	}
	particles[index].y = value;
}
union grug_value game_fn_sqrt(void* state, union grug_value* values) {
	(void)(state);

	double value = values[0].number;
	return (union grug_value){.number = sqrt(value)};
}
/* Game functions */

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
		// if the calculation takes more than 10 ms
		if (end - start > (frequency / 100)) {
			printf("Maximum fibonacci number calculated within 1 second: %zu\n", i);
			break;
		}
	}
	
	grug_state_vtable->destroy_entity(state, entity);
}

void run_nbody_test(
	void* state,
	struct grug_state_vtable* grug_state_vtable
) {
	void* on_tick_id = grug_state_vtable->get_on_fn_id(state, "Particle", "on_tick");

	void* file = grug_state_vtable->compile_grug_file(state, "bench/light-Particle.grug");
	
	assert(!particles);
	particles_len = 100;
	particles = malloc(sizeof(struct ParticleData) * particles_len);

	void** entities = malloc(sizeof(void*) * particles_len);
	
	for (size_t i = 0; i < particles_len; i++) {
		void* entity = grug_state_vtable->create_entity(state, file);
		entities[i] = entity;
		particles[i] = (struct ParticleData) {
			.mass = (double)i,
			.x    = (double)((i % 25) * 40),
			.y    = (double)((i / 25) * 40),
		};
	}

	printf("Running n body simulation\n");
	fflush(stdout);

	size_t counter = 0;

	uint64_t frequency = get_timestamp_frequency();
	uint64_t start = get_timestamp();
	// run for a maximum of 1 second
	while ((get_timestamp() - start) < (frequency)) {
		for (size_t i = 0; i < particles_len; i++) {
			grug_state_vtable->call_entity_on_fn(
				state,
				entities[i],
				on_tick_id,
				(union grug_value[]) {
					{.number = (double)particles_len}
				},
				1
			);
		}
		counter += 1;
	}

	printf("number of iterations completed: %zu\n", counter);

	for (size_t i = 0; i < particles_len; i++) {
		grug_state_vtable->destroy_entity(state, entities[i]);
	}
	free(entities);
	free(particles);
	particles = NULL;
	particles_len = 0;
}

void grug_bench_run(
	const char* mod_api_path,
	const char* mods_dir,
	struct grug_state_vtable* grug_state_vtable
) {
	void* state = grug_state_vtable->create_grug_state(mod_api_path, mods_dir);

	run_on_function_test(state, grug_state_vtable);
	run_fibonacci_test(state, grug_state_vtable);
	run_nbody_test(state, grug_state_vtable);

	grug_state_vtable->destroy_grug_state(state);
	return;
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
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
	double a_x, a_y;
	double v_x, v_y;
	double x, y;
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
union grug_value game_fn_sqrt(void* state, union grug_value* values) {
	(void)(state);

	double value = values[0].number;
	return (union grug_value){.number = sqrt(value)};
}
void game_fn_set_acc(void* state, union grug_value* values) {
	(void)(state);
	static size_t count = 0;

	size_t index = (size_t)values[0].number;
	double a_x = values[1].number;
	double a_y = values[2].number;
	particles[index].a_x = a_x;
	particles[index].a_y = a_y;
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

#define WIDTH 70
#define HEIGHT 70
#define SCALE 10
#define NUM_PARTICLES 1000
#define COLOR_SCALE 0.05

// Unicode blocks for density visualization
const char *density_blocks[] = {"  ", "░░", "▒▒", "▓▓", "██"};
#define NUM_BLOCKS (sizeof(density_blocks) / sizeof(density_blocks[0]))

const char *color_palette[] = {
	"\033[38;5;196m", "\033[38;5;202m", "\033[38;5;208m", "\033[38;5;214m",
	"\033[38;5;220m", "\033[38;5;226m", "\033[38;5;190m", "\033[38;5;154m",
	"\033[38;5;118m", "\033[38;5;82m",  "\033[38;5;46m",  "\033[38;5;47m",
	"\033[38;5;48m",  "\033[38;5;49m",  "\033[38;5;51m",  "\033[38;5;45m",
	"\033[38;5;39m",  "\033[38;5;33m",  "\033[38;5;27m",  "\033[38;5;21m",
	"\033[38;5;57m",  "\033[38;5;93m",  "\033[38;5;129m", "\033[38;5;165m",
	"\033[38;5;201m", "\033[38;5;200m", "\033[38;5;199m", "\033[38;5;198m",
	"\033[38;5;197m", "\033[38;5;196m", "\033[38;5;160m", "\033[38;5;124m",
	"\033[38;5;88m",  "\033[38;5;52m",  "\033[38;5;53m",  "\033[38;5;54m",
	"\033[38;5;55m"
};
#define NUM_COLOR_SHADES (sizeof(color_palette) / sizeof(color_palette[0]))

// Map number of particles to block & total speed to color
void density_velocity_to_block_color(
	double n_particles, double speed_sum,
	const char **block, const char **color
) {
	// Block based on particle count
	size_t idx_block = (size_t)n_particles;
	if (idx_block >= NUM_BLOCKS) idx_block = NUM_BLOCKS - 1;
	if (n_particles > 0.0 && idx_block < 1) idx_block = 1;

	// Color based on speed sum (linear scale, clamp to palette)
	size_t idx_color = (size_t)(speed_sum * COLOR_SCALE);
	if (idx_color >= NUM_COLOR_SHADES) idx_color = NUM_COLOR_SHADES - 1;

	*block = density_blocks[idx_block];
	*color = color_palette[idx_color];
}

void render_frame(double grid_count[HEIGHT][WIDTH], double grid_speed[HEIGHT][WIDTH], char *buffer, size_t buffer_size) {
	printf("\033[H"); // Move cursor to top-left
	char *p = buffer;
	size_t remaining = buffer_size;
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			char *block, *color;
			density_velocity_to_block_color(grid_count[y][x], grid_speed[y][x], &block, &color);
			int wrote = snprintf(p, remaining, "%s%s\033[0m", color, block);
			if (wrote < 0 || (size_t)wrote >= remaining) break;
			p += wrote;
			remaining -= wrote;
		}
		int wrote = snprintf(p, remaining, "\n");
		if (wrote < 0 || (size_t)wrote >= remaining) break;
		p += wrote;
		remaining -= wrote;
	}
	printf("%s", buffer);
	fflush(stdout);
}

void run_nbody_test(
	void* state,
	struct grug_state_vtable* grug_state_vtable
) {
	srand((unsigned)time(NULL));
	void* on_tick_id = grug_state_vtable->get_on_fn_id(state, "Particle", "on_tick");

	void* file = grug_state_vtable->compile_grug_file(state, "bench/light-Particle.grug");

	size_t buffer_size = HEIGHT * WIDTH * 32 + HEIGHT * 16;
	char* buffer = malloc(buffer_size);
	
	assert(!particles);
	particles_len = NUM_PARTICLES;
	particles = malloc(sizeof(struct ParticleData) * particles_len);

	void** entities = malloc(sizeof(void*) * particles_len);

	
	// initialize particles
	for (size_t i = 0; i < particles_len; i++) {
		void* entity = grug_state_vtable->create_entity(state, file);
		entities[i] = entity;
		particles[i] = (struct ParticleData) {
			.mass = ((double)rand() / RAND_MAX) * 10,
			.x = ((double)rand() / RAND_MAX) * WIDTH * SCALE,
			.y = ((double)rand() / RAND_MAX) * HEIGHT * SCALE,
			.v_x = (((double)rand() / RAND_MAX) - 0.5) / 2.0 * SCALE,
			.v_y = (((double)rand() / RAND_MAX) - 0.5) / 2.0 * SCALE,
			.a_x = 0,
			.a_y = 0
		};
	}

	printf("Running n body simulation\n");
	fflush(stdout);

	size_t counter = 0;

	uint64_t frequency = get_timestamp_frequency();
	uint64_t start = get_timestamp();
	// run for a maximum of 1 second
	while ((get_timestamp() - start) < (10 * frequency)) {
		// reset grid count
		double grid_count[HEIGHT][WIDTH] = {0};
		double grid_speed[HEIGHT][WIDTH] = {0};
		// run simulation tick 
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

		// update positions
		for (size_t i = 0; i < particles_len; i++) {
			particles[i].v_x += particles[i].a_x * 0.01;
			particles[i].v_y += particles[i].a_y * 0.01;
			
			particles[i].v_x;
			particles[i].v_y;

			particles[i].x += particles[i].v_x;
			particles[i].y += particles[i].v_y;

			int gx = (int)(particles[i].x / SCALE);
			int gy = (int)(particles[i].y / SCALE);

			if (gx >= 0 && gx < WIDTH && gy >= 0 && gy < HEIGHT) {
				grid_count[gy][gx] += 1.0;
				grid_speed[gy][gx] += sqrt(particles[i].v_x * particles[i].v_x +
										   particles[i].v_y * particles[i].v_y);
			}
		}

		render_frame(grid_count, grid_speed, buffer, buffer_size);
		counter += 1;
	}

	printf("number of iterations completed: %zu\n", counter);

	for (size_t i = 0; i < particles_len; i++) {
		grug_state_vtable->destroy_entity(state, entities[i]);
	}
	free(entities);
	free(particles);
	/* free(buffer); */
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

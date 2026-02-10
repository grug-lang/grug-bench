#ifndef MAIN_H
#define MAIN_H

typedef void* (*create_grug_state_t)(const char* mod_api_path, const char* mods_dir);
typedef void (*destroy_grug_state_t)(void* state);

/* call this function to run the benchmarks */
void grug_bench_run(
	create_grug_state_t,
	destroy_grug_state_t
);
#endif /* MAIN_H */

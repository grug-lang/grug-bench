#include <stdio.h>
#include "main.h"

void grug_bench_run(
	create_grug_state_t create_grug_state,
	destroy_grug_state_t destroy_grug_state
) {
	void* state = create_grug_state("nothing", "true");
	destroy_grug_state(state);
	printf("hello from main\n");
	return;
}

/* static void runtime_error_handler(const char *reason, enum grug_runtime_error_type type, const char *on_fn_name, const char *on_fn_path) { */
/* 	(void)type; */
/* 	fprintf(stderr, "grug runtime error in %s(): %s, in %s\n", on_fn_name, reason, on_fn_path); */
/* } */

#include<stdio.h>
#include<libloaderapi.h>
#include "main.h"

typedef typeof(&grug_bench_run) p_grug_bench_run;

void* load_library(const char* path) {
	return LoadLibraryExA(path, NULL, 0);
}

void* load_symbol(void* dll, const char* proc_name) {
	return GetProcAddress(dll, proc_name);
}

void* create_grug_state(const char* mod_api_path, const char* mods_dir) {
	printf("Creating state\n");
	(void)(mod_api_path);
	(void)(mods_dir);
	return NULL;
}

void destroy_grug_state(void* state) {
	printf("Destroying state\n");
	(void)(state);
}

int main () {
	void* dll = load_library("main.dll");
	if (!dll) {
		fprintf(stderr, "could not open dll");
		exit(1);
	}
	p_grug_bench_run grug_bench_run = load_symbol(dll, "grug_bench_run");
	if (!grug_bench_run) {
		fprintf(stderr, "could not load symbols\n");
		return 1;
	}
	grug_bench_run(create_grug_state, destroy_grug_state);
	printf("hello from smoketest.c");
	return 0;
}

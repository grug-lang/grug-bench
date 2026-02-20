# grug bench

Benchmarks for implementations of the [grug](https://github.com/grug-lang/grug) language.

`bench.c` is built into a shared library. Your library is supposed to load
`bench.dll` or `bench.so` and call `grug_bench_run` with the path to the
`mod_api.json` and the mods directory. 

The only benchmark available right now is the benchmark for calling game
functions, but other benchmarks are also planned
1. State initialization and deinitialization
2. Compilation time
3. grug execution time (for math)

If you have more ideas for benchmarks, please [create an issue](https://github.com/grug-lang/grug-bench/issues).

# Running the smoketest

1. Clone the git repository and `cd` into it.
2. Configure with `cmake -S . -B ./build`
3. Build with `cmake --build build`
4. Run with `build/smoketest`

# Running the benchmark on your implementation

bench.dll exports two main symbols, `grug_bench_run` and `runtime_error_handler` 

it also exports two game functions `game_fn_print_number` and `game_fn_get_1`,
which correspond to the game functions in the `mod_api`

Link against bench.so or bench.lib and call `grug_bench_run` with the `mod_api`
path, mods directory path, and a vtable for `grug_state`. `grug_bench_run` will:
1. Initialize the `grug_state`
2. Compile the grug files 
3. Create an entity from that grug file
4. Call that entity's `on_increment` 1 billion times
5. Print the time taken to do the increments

# N-body simulation benchmark

The benchmark `nbody.c` performs a naive [N-body simulation](
https://en.wikipedia.org/wiki/N-body_simulation) of 1000 grug
`Particle` entities, printing to the terminal:

<img src="./nbody.gif" width="250"/>

## Running

Run with `build/nbody`.

## Headless mode

If you want to run the benchmark as fast as possible without any graphics, pass `--headless`.

In headless mode, the benchmark stops after 10 seconds and prints the number of elapsed ticks.

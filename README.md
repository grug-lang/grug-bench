# grug bench
Benchmarks for implementations of the [grug](https://github.com/grug-lang/grug) language.

`bench.c` is built into a shared library. Your library is supposed to load
`bench.dll` or `bench.so` and call `grug_bench_run` with the path to the
`mod_api.json` and the mods directory. 

# running the smoketest
1. clone the git repository and `cd` into it.
2. run `make`
3. run `./smoketest.exe`

# disclaimer
This repository has only been tested on windows. It likely will not compile on
other platforms. The makefile especially is highly platform dependent. Any help
in porting this to other platforms is appreciated

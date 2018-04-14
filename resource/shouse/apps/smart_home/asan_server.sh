ASAN_OPTIONS=verbosity=2:symbolize=1:abort_on_error=1 ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer-3.8) gdb  ./smart_home_server

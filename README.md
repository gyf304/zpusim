# zpusim
Another ZPU Simulator

# Usage

    zpusim [OPTIONS]
    Example: zpusim -p 0x8000 -s 5 -f tests/fib/fib.bin
    -h, --help             Print help and exit
    -V, --version          Print version and exit
    -f, --filename=STRING  Binary file to execute
    -m, --mem=LONG         Memory size in bytes, if less than practical will use
                             default value
    -s, --stack=LONG       How deep should zpusim print stack.
    -c, --cycles=LONG      How many cycles should zpusim execute.
    -p, --peek=LONGLONG    A memory address zpusim checks for each cycle. Common
                             value is 0x8000.
    -S, --silent           Suppresses all output. Useful for profiling. Ignored
                             if cycles not set.  (default=off)
# Building
## Required Tools
 - C compiler
 - make
 - gengetopt

Then you simply type `make`


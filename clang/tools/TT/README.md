# `TT`: Sandbox-theistic S2S Transformer

The `TT` tool is a command-line interface to the TT software for
generating sandbox specific definitions required by CheckCBox. See the [TT
readme](../../docs/checkedc/3C/README.md) (for general information
about TT) and the [build
instructions](../../docs/checkedc/3C/INSTALL.md). This document
describes how to use `TT`.

As mentioned in the build instructions, the `TT` executable is in the
`build/bin` directory of your working tree. To use the commands below,
either add that directory to your `$PATH` or replace `TT` with
whatever other means you wish to use to run the executable (e.g., an
absolute path or a wrapper script).

## Workflow

`TT` generates Sandbox Specific Definitions as below:

1. Run `TT`. It reads a set of `.c` files and the header files they
transitively include (these files may be written in CheckCBox or, as a
special case, CheckedC), performs a whole-program static analysis to
infer and insert instrumentation for "_Tainted" marked functions,
and writes out the updated files with the "sandbox-specific" definitions
for the same. `TT` accepts various
options to choose, such as Sandbox-Type.
2. Definitions for each of the _Tainted functions is instrumented to call respective
sandbox definitions. The Passed arguments and return values are instrumented according
to the sandbox definition's requirement.
3. The true function definitions for "_Tainted" functions are commented out following which sandbox 
instrumentation is inserted, following which an actual call to the "sandbox-specific-version" of the
tainted functions is written. **WARNING**: Make sure you have the "original" tainted function definition symbols 
linked during your compilation process.
4. Compile your program with the
CHECKCBOX C compiler (`clang`) alongside the required static libraries to check your work.


For a complete, worked example, see [our
tutorial](https://github.com/correctcomputation/checkedc-tiny-bignum-c).

## Basic usage

The task of `TT` is complicated by the fact that the build system for
a typical C codebase will call the C compiler once per `.c` file,
possibly with different options for each file (include directories,
preprocessor definitions, etc.), and then link all the object files at
the end. A Clang-based whole-program analysis like TT needs to process
all `.c` files at once _with the correct options for each_. To achieve
this, you get your build system to produce a Clang "compilation
database" (a file named `compile_commands.json`) containing the list
of `.c` files and the options for each ([how to do this depends on the
build system](../../docs/JSONCompilationDatabase.rst)), and then TT
reads this database.

For simple tests, you can run `TT` on one or more source files without
a compilation database. For example, the following command will
instrument `foo.c` with sandbox-specific tainted instrumentation
and print the new version to standard output:

```
TT -addcr -alltypes foo.c --
```

The `--` at the end of the command line indicates that you _do not_
want to use a compilation database. This is important to ensure `TT`
doesn't automatically detect a compilation database that you don't
want to use. You can add compiler options that you want to use for all
files after the `--`, such as `-I` for include directories, etc.

This "stdout mode" only supports a single source file. If you have
multiple files, you must use one of the modes that writes the output
to files. You can specify either a string to be inserted into the
names of the new files or a directory under which they should be
written. For example, this command:

```
TT -addcr -alltypes -output-postfix=checked foo.c bar.c --
```

will write the new versions to `foo.checkcbox.c` and `bar.checkcbox.c`. If
one of these files is not created, it means the original file needs no
changes. If `foo.c` includes a header file `foo.h`, then `TT` will
automatically include it in the conversion and write the new version
of it to `foo.checkcbox.h` if there are changes; you don't need to
specify header files separately on the command line, just as they
wouldn't have their own entries in a compilation database.

The `-output-postfix` mode may be convenient for running tools such as
`diff` on individual files by hand. Alternatively, this command:

```
TT -addcr -alltypes -output-dir=/path/to/new foo.c bar.c --
```

will write the new versions to `/path/to/new/foo.c` and
`/path/to/new/bar.c`. You can then run something like `diff -ru .
/path/to/new` to diff all the files at once (_without_ the `-N` option
because many files in your starting directory may not have new
versions written out).

You need to Pass the Sandbox-Type Argument `-sbx`

- `-sbx` Specifies the Sandbox type that allows for sandbox-specific
instrumentation of tainted definitions

See `TT -help` for more.

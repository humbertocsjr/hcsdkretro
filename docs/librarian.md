# HC Librarian — `hclib`

Create and manage object code libraries.

```sh
hclib <library.lib> <object files...>
```

## Usage

Create a library from object files:

```sh
hclib mylib.lib putchar.obj getchar.obj exit.obj
```

The librarian concatenates all object files into a single `.lib` archive. The linker automatically extracts only the objects that resolve undefined symbols.

# RetroLang Programming Language Documentation

## Introduction
RetroLang is a simple, block-structured programming language with explicit pointer operations and a clean syntax. It supports variables, functions, conditional execution, and loops. Each statement is written on its own line, and multi-line blocks are terminated with the keyword `end`.

## General Syntax
- One command per line.
- Multi-line blocks (functions, if/else, while, until) are closed with `end`.
- Comments start with `;` and can only appear at the end of a line.

## Comments
Comments begin with a semicolon (`;`) and continue to the end of the line.

```retrolang
var x as int   ; this is a comment
x := 10        ; another comment
```

## Variables

### Declaration
Variables are declared using the `var` keyword, followed by a comma-separated list of variable names, each with an optional type specification. The type is introduced by the keyword `as`. For pointer types, one or more `@` symbols are placed before the base type to indicate the level of indirection. The generic pointer type `pointer` can also be used.

Syntax:
```
var name as type
var name as @type       ; pointer to type
var name as @@type      ; pointer to pointer to type
var name1 as type1, name2 as type2, ...
decl external_variable as int ; declaration of variable in another object
```

Examples:
```retrolang
var a as int            ; integer variable
var b as @@int          ; pointer to pointer to integer
var c as char, d as @char, e as int   ; multiple declarations
var ptr as pointer      ; generic pointer
```

### Assignment
Assignment is performed with the `:=` operator. The right-hand side can be an expression or the result of `addressof`.

```retrolang
a := 1 + 2 + 3
ptr := addressof(main)
```

## Functions
Functions are defined with the `def` keyword, followed by the function name, a parenthesized list of parameters, an optional return type, and a body terminated by `end`. Parameters follow the same syntax as variable declarations.

Syntax:
```
def function_name(param1 as type1, param2 as type2, ...) as return_type
    ; function body
end

; declaration of function in another object
decl external_name(param1 as type1, param2 as type2, ...) as return_type
```

Example:
```retrolang
decl print(text as @char)

def main(a as int, b as int) as int
    var c as int
    c := a + b
    return c           ; hypothetical return statement
end
```

*Note: The documentation assumes a `return` statement exists, though it wasn't explicitly mentioned in the rules.*

## Pointers and Addresses

### Taking an Address
The pseudo-function `addressof` returns the address of a variable or function. It can be used to initialize pointer variables.

```retrolang
var ptr as pointer
ptr := addressof(main)

var p as @int
var x as int
p := addressof(x)
```

### Dereferencing
To access the value pointed to by a pointer, prefix the pointer variable with `@`.

```retrolang
var ptr as @int
var src as int
var dst as int
src := 10
ptr := addressof(src)
dst := @ptr            ; dst now equals 10
```

## Conditional Execution

### Single‑line `if`
A statement can be conditionally executed by appending `if` followed by a condition at the end of the line.

```retrolang
a := 1 if b > 0
```

### Multi‑line `if`
For multi‑line conditional blocks, use `if` followed by the condition, then the body, optionally an `else` part, and close with `end`.

```retrolang
if a < 10
    a := 1
else
    a := 2
end
```

## Loops

### `while` Loop
Executes a block repeatedly as long as the condition evaluates to true.

```retrolang
while a < 1
    a += 1            ; note: += is used, though not formally defined
end
```

### `until` Loop
Executes a block repeatedly until the condition becomes true (i.e., while the condition is false).

```retrolang
until a == 10
    a += 1
end
```

## Expressions
Expressions can include arithmetic operators (`+`, `-`, `*`, `/`, etc.) and comparison operators (`<`, `>`, `==`, etc.). The exact operator set is not fully specified, but typical arithmetic and relational operators are assumed.

## Complete Example
A small program demonstrating various features:

```retrolang
; Example Retrolang program
def main() as int
    var x as int, y as @int
    var ptr as pointer

    x := 42
    y := addressof(x)

    if x > 40
        ptr := addressof(main)
    else
        ptr := addressof(x)
    end

    while @y > 0
        @y := @y - 1
    end

    until @y == 0
        @y := @y + 1
    end

    return 0
end
```

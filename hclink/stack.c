#include "link.h"

static int _stack[STACK_MAX];
static int _stack_pointer = STACK_MAX;

// [English] Reset the expression stack to its initial empty state
// [Portuguese] Reinicia a pilha de expressão para seu estado vazio inicial
void stack_reset()
{
    _stack_pointer = STACK_MAX;
}

// [English] Push a value onto the expression stack, checking for overflow/underflow
// [Portuguese] Empurra um valor na pilha de expressão, verificando overflow/underflow
void stack_push(int value)
{
    _stack_pointer--;
    if (_stack_pointer < 0)
        process_error("expression stack overflow. [PUSH]");
    if (_stack_pointer >= STACK_MAX)
        process_error("expression stack underflow. [PUSH]");
    _stack[_stack_pointer] = value;
}

// [English] Pop a value from the expression stack, checking for overflow/underflow
// [Portuguese] Remove um valor da pilha de expressão, verificando overflow/underflow
int stack_pop()
{
    if (_stack_pointer < 0)
        process_error("expression stack overflow. [POP]");
    if (_stack_pointer >= STACK_MAX)
        process_error("expression stack underflow. [POP]");
    return _stack[_stack_pointer++];
}

#include "link.h"

static int _stack[STACK_MAX];
static int _stack_pointer = STACK_MAX;

void stack_reset()
{
    _stack_pointer = STACK_MAX;
}

void stack_push(int value)
{
    _stack_pointer--;
    if(_stack_pointer < 0) process_error("expression stack overflow. [PUSH]");
    if(_stack_pointer >= STACK_MAX) process_error("expression stack underflow. [PUSH]");
    _stack[_stack_pointer] = value;
}

int stack_pop()
{
    if(_stack_pointer < 0) process_error("expression stack overflow. [POP]");
    if(_stack_pointer >= STACK_MAX) process_error("expression stack underflow. [POP]");
    return _stack[_stack_pointer++];
}
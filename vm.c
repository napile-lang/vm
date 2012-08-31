#include <stdio.h>
#include <assert.h>
#include "runtime.h"
#include "opcode.h"
#include "bytecode.h"

// Helpers to play with the stack
#define STACK_MAX      10
#define STACK_PUSH(I)  do {                             \
                          assert(sp-stack < STACK_MAX); \
                          *(++sp) = (I);                \
                       } while(0)
#define STACK_POP()    (*sp--)


void run(void *literals[], byte instructions[]) {
  byte    *ip = instructions;      // instruction pointer
  
  Object  *stack[STACK_MAX];       // the famous stack
  Object **sp = stack;             // stack pointer
  
  Object  *locals[STACK_MAX] = {}; // where we store our local variables
  
  // Setup the runtime
  Object *self = Object_new();
  
  // Start processing instructions
  while (1) {
    switch (*ip) {
      case CALL: {
        ip++; // advance to operand (method)
        char *method = (char *)literals[*ip];
        ip++; // # of arguments
        int argc = *ip;
        
        Object *argv[10];
        int i;
        for(i = 0; i < argc; ++i) argv[i] = STACK_POP();
        Object *receiver = STACK_POP();
        
        Object *result = call(receiver, method, argv, argc);
        STACK_PUSH(result);
        
        break;
      }
      case PUSH_NUMBER: {
        ip++; // advance to operand (literal index)
        STACK_PUSH(Number_new((long)literals[*ip]));
        break;
      }
      case PUSH_STRING: {
        ip++; // advance to operand (literal index)
        STACK_PUSH(String_new((char *)literals[*ip]));
        break;
      }
      case PUSH_SELF: {
        STACK_PUSH(self);
        break;
      }
      case PUSH_NIL: {
        STACK_PUSH(NilObject);
        break;
      }
      case PUSH_BOOL: {
        ip++;
        if (*ip == 0) {
          STACK_PUSH(FalseObject);
        } else {
          STACK_PUSH(TrueObject);
        }
        break;
      }
      case GET_LOCAL: {
        ip++;
        STACK_PUSH(locals[*ip]);
        break;
      }
      case SET_LOCAL: {
        ip++;
        locals[*ip] = STACK_POP();
        break;
      }
      case ADD: {
        Object *a = STACK_POP();
        Object *b = STACK_POP();
        
        STACK_PUSH(Number_new(Number_value(a) + Number_value(b)));
        
        break;
      }
      case JUMP_UNLESS: {
        ip++; // offset
        byte offset = *ip;
        Object *condition = STACK_POP();
        
        if (!Object_is_true(condition)) ip += offset;
        
        break;
      }
      case RETURN: {
        return;
      }
    }
    ip++;
  }
}

int main (int argc, char const *argv[]) {
  void *literals[] = LITERALS;
  byte instructions[] = INSTRUCTIONS;
  
  init_runtime();
  run(literals, instructions);
  destroy_runtime();
  
  return 0;
}
//
// setjmp.h
//

#ifndef _SETJMP_H
#define _SETJMP_H


#define JMP_BUF_SIZE 16


/// Container for saved execution environment
typedef int jmp_buf[ JMP_BUF_SIZE ];


int setjmp(jmp_buf env);

void longjmp(jmp_buf env, int value);


#endif

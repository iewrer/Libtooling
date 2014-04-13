#include "test2.h"
void do_nothing(int *x) {
    *x = 5;
}
void do_math(int *x) {
    *x += 5;
    do_nothing(x);
}
void do_test(int *x,void (*func)(int *)) {
    (*func)(x);
}
int main(void) {
    int result = -1, val = 4;
    do_test(&val,do_math);
    do_math(&val);
    do_little(&val);
    do_something();
    return result;
}

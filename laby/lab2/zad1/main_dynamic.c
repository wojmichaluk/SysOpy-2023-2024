#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    void *handler = dlopen("./liblibrary.so", RTLD_LAZY);
    if(!handler) {
        printf("Blad otwarcia biblioteki\n");
        return 1;
    }

    int(*fun)(int, int);
    fun = dlsym(handler, "test_collatz_convergence");
    if(dlerror() != 0) {
        printf("Blad funkcji\n");
        return 1;
    }

    int input, limit;
    char test;

     printf("Podaj liczbe calkowita: ");
    // testowanie poprawności danych
    if(scanf("%d%c", &input, &test) != 2 || test != '\n') {
        printf("Podano nieprawidlowe dane wejsciowe.\n");
        return 1;
    }

    printf("Podaj limit iteracji: ");
    // testowanie poprawności danych
    if(scanf("%d%c", &limit, &test) != 2 || test != '\n') {
        printf("Podano nieprawidlowe dane wejsciowe.\n");
        return 1;
    }

    printf("Jeżeli liczba iteracji przekroczy podany limit, wyswietle -1.\n");
    printf("Dla liczby %d liczba iteracji wynosi %d\n", input, fun(input, limit));

    dlclose(handler);
    return 0;
}

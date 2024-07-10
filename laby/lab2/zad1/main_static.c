#include <stdio.h>

extern int collatz_conjecture(int);
extern int test_collatz_convergence(int, int);

int main(void) {
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
    printf("Dla liczby %d liczba iteracji wynosi %d\n", input, test_collatz_convergence(input, limit));
    
    return 0;
}

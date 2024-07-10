int collatz_conjecture(int input) {
    return input % 2 == 0 ? input / 2 : 3 * input + 1;
}

int test_collatz_convergence(int input, int max_iter) {
    int result = collatz_conjecture(input);
    int counter = 0;

    while (++counter <= max_iter && result != 1) {
        result = collatz_conjecture(result);
    }

    return counter <= max_iter ? counter : -1;
}

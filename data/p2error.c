int ok() {
    /* Empty functions are allowed! */
}

int too_many_elses(int a, int b) {
    if (a < b) {
        return 1;
    } else {
        return 2;
    } else {
        return 3;
    }
}
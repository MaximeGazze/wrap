#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    /* sleep(5); */
    const ssize_t BUFFER_SIZE = 200;
    char buffer[BUFFER_SIZE];

    printf("Write some random shit: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    /* sleep(2); */
    printf("buffer: %s", buffer);
    
    return 0;
}

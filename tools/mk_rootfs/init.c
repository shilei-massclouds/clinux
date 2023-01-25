/* Init process */

#include <unistd.h>

int main()
{
    char buf[] = "Hello, world!";

    write(STDOUT_FILENO, buf, sizeof(buf));
    return 0;
}

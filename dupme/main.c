#include <unistd.h>
#include <stdlib.h>
int main (int argc, char * argv[]) {
	char * string = argv[1];
	write(1, "Hello", 5);
}

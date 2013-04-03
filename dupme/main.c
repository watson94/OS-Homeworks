#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int getInt(char * s) {
	int i = 0;
	int ans = 0;
	while (s[i] !='\0') {
		ans *= 10;
		ans += s[i] - '0';
		i++;
	}
	return ans;
}

int main (int argc, char * argv[]) {
	int k = getInt(argv[1]) ;		
	printf("%i", k);
}

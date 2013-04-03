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

char *buffer;

int writeStr (int begin, int end) {
	int writeRes = 0;
	while (begin < end) {
		writeRes = write(1, buffer + begin, end - begin);
		if (writeRes > 0) {
			begin += writeRes;
		}
	}
	return writeRes;
}


int main (int argc, char * argv[]) {
	int k = getInt(argv[1]) ;		
	int len = 0;
	int lastindex, curindex, i;
	int isWrite = 1;
	buffer = malloc(k + 1);
	while (1) {
         	int r = read(0, buffer + len, k - len + 1);
		if (r == 0) {
			if ((len > 0) && (isWrite == 1) && (len < k + 1)) {
				writeStr(0, len);
				write(1, "\n", 1);
				writeStr(0, len);
				write(1, "\n", 1);
			}
			free(buffer);
			return 0;
		}
		if (r < 0) {
			free(buffer);
			return 1;
		}
		lastindex = 0;
		for (i = 0; i < r; i++) {
			curindex = i + len;
			if (buffer[curindex] == '\n') {
				if (isWrite) {
		 			writeStr(lastindex, curindex + 1);
					writeStr(lastindex, curindex + 1);
					
				} else {
					isWrite = 1;
				}
				lastindex = curindex + 1;
			} 
		}
		if ((lastindex == 0) &&(curindex == k)) {
			isWrite = 0;
			len = 0;	
		} else {
			memmove(buffer, buffer + lastindex, curindex - lastindex + 1);
			len = curindex - lastindex + 1;
		}
	}
	free(buffer);
}

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int writestr (int begin, int end, char * buffer) {
	int writeRes = 0;
	while (begin < end) {
		writeRes = write(1, buffer + begin, end - begin);
		if (writeRes > 0) {
			begin += writeRes;
		}
	}
	return writeRes;
}


void writetwice(int begin, int end, char * buffer) {
    writestr(begin, end, buffer);
    writestr(begin, end, buffer);
}

int main (int argc, char * argv[]) {
    if (argc < 1) {
        return 2;
    }
	int k = atoi(argv[1]) ;		
	int len = 0;
	int lastindex, curindex, i;
	int isWrite = 1;
	char * buffer = malloc(k + 1);
	while (1) {
         	int r = read(0, buffer + len, k - len + 1);
		if (r == 0) {
			if ((len > 0) && (isWrite == 1) && (len < k + 1)) {
                buffer[len] = '\n'; 
				writetwice(0, len + 1, buffer);
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
                    writetwice(lastindex, curindex + 1, buffer);
					
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

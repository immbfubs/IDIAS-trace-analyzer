#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>

int main(int argc, char* argv[])
{
    FILE* f1 = fopen("./input.txt",  "r");
    FILE* f2 = fopen("./output.txt", "w");

// TEST 1

    fprintf(f2, "%s %s %s %d", "We", "are", "in", 2012);

// TEST 2

    char line[256];
    while (fgets(line, sizeof(line), f1)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        fprintf(f2, "%s", line);
    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

// TEST 3

    rewind(f1);

    char c;
    while (c = fgetc(f1)) {

        if( feof(f1) ) {
           break;
        }

        if(isdigit(c)){
            fprintf(f2, "%c", c);
        }
        else if(isalpha(c)){
            fprintf(f2, "%c", c);
        }
    }

    fclose(f1);
    fclose(f2);

    return 0;
}
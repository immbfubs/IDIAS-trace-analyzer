//----------------------------------------------------------------------------------------------------------
//   HOW IT WORKS
//----------------------------------------------------------------------------------------------------------
// 1. MAIN proc prompts for the desired function and calls the main procedure CONCISE that does the job
// 2. CONCISE iterates through all the rows and calls MODULE_CHANGE when the row beggins with a module name
//
// ATTENTION should be paid to the sequence strtok is used ! 
//
//----------------------------------------------------------------------------------------------------------
//   TODO
//----------------------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int count_file_rows(FILE* fp)
{
    int lines = 1;

    char buf[256] = "Garbage";
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        lines++;
    }

    rewind(fp);
    return lines;
}

char* get_module_name(char* row)
{
    char delim[2] = {' ', 0};
    char* cp1;
    char* cp2;

    cp1 = strtok(row, delim);
    delim[0] = '+';
    cp2 = strtok(cp1, delim);

    if (row == NULL || cp1 == NULL || cp2 == NULL ){
        printf("NULL POINTER ERROR\n");
    }

    return cp2;
}

char* module_changed(FILE* f2, char *first_occurence, char *addr_first, char *addr_last, char* mode)
{
    char* all = "all";
    char space1[] = "          ";
    char space2[] = "       ";
    char space3[] = "       ";

    space1[11 - strlen(first_occurence)] = 0;
    space2[8 - strlen(addr_first)] = 0;
    space3[8 - strlen(addr_last)] = 0;

    if (!strcmp(mode, all)){
        if ((strstr(first_occurence, "NL") != first_occurence) && !strstr(first_occurence, "ABSOLUT")){
            fprintf(f2, "%s%s  %s%s  %s%s      \n", first_occurence, space1, addr_first, space2, addr_last, space3);
        }
    } else {
        if (strstr(first_occurence, mode)){
            fprintf(f2, "%s%s  %s%s  %s%s      \n", first_occurence, space1, addr_first, space2, addr_last, space3);
        }
    }
}

//in dev
int concise(FILE* f1, FILE* f2, char* mode)
{
    // int nr_rows_f1 = count_file_rows(f1);
    char row[256] = "";
    char *module;
    char delim[2] = {' ', 0};
    char first_occurence[50] = "MODULE";
    char addr_first[50] = "IN_ADDR";
    char addr_last[50] = "OUT_ADDR";

    while(fgets(row, sizeof(row), f1))
    {
        if (strlen(row) > 4 && row[1] != ' ' && row[1] != '%'){
            module = get_module_name(row);

            if (strcmp(module, first_occurence)) {
                module_changed(f2, first_occurence, addr_first, addr_last, mode);

                strcpy(first_occurence, module);
                strcpy(addr_first, strtok(0, delim));
                strcpy(addr_last, "---");
            }
            else {
                strcpy(addr_last, strtok(0, delim));
            }
        }
    }
    
    module_changed(f2, first_occurence, addr_first, addr_last, mode);
}

int main(int argc, char* argv[])
{
    printf("\n");

    char* path1 = "./input.txt";
    char* path2 = "./output.txt";
    FILE* f1 = fopen(path1, "r");
    FILE* f2 = fopen(path2, "w");

    if( f1 == NULL ) {
        perror( path1 );  // ??
        printf("ERROR OPENING INPUT FILE\n");
        exit( EXIT_FAILURE );
    }

    if( f2 == NULL ) {
        perror( path2 );  // ??
        printf("ERROR OPENING OUTPUT FILE\n");
        exit( EXIT_FAILURE );
    }

    char mode[20];
    printf("Modules that start with 'NL' and 'ABSOLUT' are not logged!\n");
    printf("To log only modules that contain a certain string enter the string, otherwise enter 'all'.\n");
    printf("The input is case sensitive including for the 'all' keyword!\n\n");
    scanf("%s", mode);

    concise(f1, f2, mode);

    printf("\n");
    printf("MAIN WORK DONE!\n");

    fclose(f1);
    fclose(f2);
    printf("FILES CLOSED!\n");
    printf("\n");

    return 0;
}
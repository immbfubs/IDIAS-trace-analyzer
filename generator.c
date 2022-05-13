//----------------------------------------------------------------------------------------------------------
//   TODO
//----------------------------------------------------------------------------------------------------------
//     x = done  /  ! = important  /  ? = don't remember this problem
//
//  x  1. To handle duplicates  _#Dn
//     2. To refator AREA fields declaration and LIST  _#Cn
//     3. Don't print the names with the suffixes in the final listing.
//        For this use additional field in the Line structure that would hold the string without suffixes.
//     4. Add BIT identifiers' names in the LIST part under B_I_T_S_n
//  ?  5. What about this: AREA(ACT_LENGTH) / CHAR(ACT_LENGTH) --> WARNING IS ISSUED FOR NOW
//  ?  7. Make column type
//     8. HEX_STRING(STRUCT.HEAP_AREA,334)   334 is max fo HEX_STRING  --> make a warning
//  !  9. If the structure is too big (TASK_TAB_MDL) the decimal representation of the offset is missing!
//        Parsing in this case will not be done correctly!
//----------------------------------------------------------------------------------------------------------
//   SUFFIXES
//----------------------------------------------------------------------------------------------------------
//      - #R         Reserved (the suffix is added for the forbidden in PRODAMP identifiers).
//      - #D[1..n]   Duplicate (It is possible to have identifiers with the same name on different levels 
//                   in the structure but having identifiers with the same name in PRODAMP is a problem).
//                   No suffix is added for the first occurrence.
//      - #C[1..n]   Continuation for AREA (NUMERIC can not be bigger than four bytes hence HEX_STRING 
//                   can not print more than four bytes but the whole AREA has to be printed in hex).
//                   One for each 4 bytes (the last one can have LENGTH < 4)
//      - #S         This is a suggestion for the user. It is not necessary that the output is the final 
//                   procedure. It may be easier if some fields are observed as a string than in hex.
//                   In such case, you can add another declaration with the same offset and length and add 
//                   the corresponding LIST line to print the same memory area as a string as well.
//----------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

enum  types {STRUC, CHARS, BIT, FIXED, INTGR, POINT, MODE, AREA, CLASS, BIT_AREA};

struct Line {
   char  level;
   int   level_int;
   char* name;
   int   name_len;
   enum  types type;
   char* type_str;
   char* offset_dec_str;
   char* offset_hex_str_3;
   int   offset_int;
   int   length;
   char  bit_offset;
};

//ready
void test_printf(int n)
{
    printf("checkpoint %d\n", n);
}

//ready
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

//ready
void copy_file(FILE* f2, char* source_file_name, int new_line)
{
    FILE* f3 = fopen(source_file_name, "r");
    char row[256] = "";
    
    for (int row_number = 1; fgets(row, sizeof(row), f3); row_number++)
        fprintf(f2, row);

    if (new_line)
        fprintf(f2, "\n");

    fclose(f3);
}

//ready
char** str_split(char* a_str, const char a_delim)
{
    char** result = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    //30 just in case
    result = malloc(sizeof(char*) * 30);
    memset(result,0,sizeof(char*) * 30);
    
    size_t idx  = 0;
    char* token = strtok(a_str, delim);

    while (token)
    {
        // if token != "("
            //duplicate token and save its address in pointer array
            //why am i not using the already existing string token?
        if (strcmp(token,"("))
            *(result + idx++) = strdup(token);
        token = strtok(0, delim);
    }
    *(result + idx) = 0;
    
    return result;
}

//ready
char* substring(char *string, int startIndex, int length)
{
   char *p;
   int c;
 
   p = malloc(length+1);
   
   if (p == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(1);
   }
 
   for (c = 0; c < length; c++)
   {
      *(p+c) = *(string+startIndex);      
      string++;  
   }
 
   *(p+c) = '\0';
 
   return p;
}

//ready
int substring_compare(char* str1, char* str2, int startIndex)
{
    int len = strlen(str2);

    for (int i = 0; i < len ; i++)
    {
        //printf("%c = %c\n", str1[startIndex], str2[i]);
        if (str1[startIndex++] != str2[i])
            return 0;
    }
    
    //printf("\n");

    return 1;
}

//ready
int str_extract_first_number_occurence(char* strin, char** strout)
{
    //get first digit's index
    int idx0;
    for(idx0 = 0; !isdigit(strin[idx0]); idx0++);

    //count the length
    int len = 0;
    for(int i = idx0; isdigit(strin[i]); i++)
        if (isdigit(strin[i]))
            len++;
    
    //printf("%d\n", len);

    //return the result
    *strout = substring(strin, idx0, len);

    //printf("Count of extracted digits: %d\n", strlen(*strout));
    return atoi(*strout);
}

//not needed - but what's wrong here
int str_extract_digits(char* strin, char** strout)
{
    //count the length
    int len = 0;
    for(int i = 0; i < strlen(strin); i++)
        if (isdigit(strin[i]))
            len++;

    len = (len * sizeof(char) + sizeof(char));
    *strout = malloc(len);
    *strout = memset(*strout, 0, len);

    //take only the digits (skip other symbols)
    int j = 0;
    for(int i = 0; i < strlen(strin); i++)
        if (isdigit(strin[i]))
            *strout[j++] = strin[i];

    printf("Count of extracted digits: %d\n", strlen(*strout));
    return 0;
}

//to be improved
int parse_line(struct Line** lines_anchor, struct Line* line, char* row, int row_number)
{
    
    char** tokens = str_split(row, ' ');
    
    char* token;
    //-------------------------------------------------------------------------- COUNT TOKENS
    int tokens_count;
    for (tokens_count = 0; *(tokens + tokens_count); ++tokens_count) { }
    if (tokens_count < 5 || tokens_count > 8)
    {
        printf("Line %d skipped\n", row_number);
        return 1;
    }
    //-------------------------------------------------------------------------- LEVEL
    token = *(tokens + 0);

    if (strlen(token) != 1 || !isdigit(*token) || *token == '0' || *token == '1')
    {
        printf("Line %d skipped\n", row_number);
        return 1;
    }

    line->level = *token;
    line->level_int = atoi(token);
    //-------------------------------------------------------------------------- TYPE

    if (*(tokens + 4)[0] == '+')
    {
        if (*(tokens + 5)[0] == 'B')                     line->type = BIT;
        if (*(tokens + 5)[0] == 'C')                     line->type = CLASS;
    }
    else if (strcmp(*(tokens + 4), "CLASS") == 0)        line->type = CLASS;
    else if (strstr(*(tokens + 4), "STRUCTURE") != NULL) line->type = STRUC;
    else if (strstr(*(tokens + 4), "CHAR") != NULL)      line->type = CHARS;
    else if (strstr(*(tokens + 4), "FIXED") != NULL)     line->type = FIXED;
    else if (strstr(*(tokens + 4), "INTEGER") != NULL)   line->type = INTGR;
    else if (strstr(*(tokens + 4), "POINTER") != NULL)   line->type = POINT;
    else if (substring_compare(*(tokens + 4), "AREA", 0))      line->type = AREA;
    else if (substring_compare(*(tokens + 4), "MODE", 0))      line->type = MODE;
    else if (substring_compare(*(tokens + 4), "STRUCTURE", 0)) line->type = STRUC;
    else if (substring_compare(*(tokens + 4), "BIT", 0))       line->type = BIT_AREA;
    //else if (strcmp(substring(*(tokens + 4),0,4), "MODE") == 0)line->type = MODE;
    else
    {
        printf("Error during determining of the type: %s\n", *(tokens + 4));
        printf("line = %d\n", row_number);
        printf("[*(tokens + 4] = %c\n", *(tokens + 4)[0]);
        printf("[*(tokens + 5] = %c\n", *(tokens + 5)[0]);
        return 2;
    }

    if (line->type == CHARS)
        line->type_str = "STRING";
    else
        line->type_str = "NUMERIC";
    //-------------------------------------------------------------------------- NAME

    token = *(tokens + 1);

    // handle dots in case of structure or model
    if (line->type == STRUC || line->type == MODE)
    {
        char* dot = strstr(token, ".");
        if (dot != NULL)
        {
            dot[0] = '\0';
        }
    }

    // take care of reserved words
    // do these rows leak memory?
    if(!strcmp(token, "TSN"))     token = "TSN_#R";
    if(!strcmp(token, "VERSION")) token = "VERSION_#R";

    // take care of duplicates
    struct Line* line_iter1;
    struct Line* line_iter2;
    char* temp_token;
    short d_flag = 1;
    char buffer [sizeof(int)*8+1];

    for(int i = 0; *(lines_anchor + i) != line; i++)
    {
        line_iter1 = *(lines_anchor + i);
        if(!strcmp(line_iter1->name, token))
        {   
            // memory leak again?
            temp_token = malloc(strlen(token) + 5);
            
            for(int j = 1; d_flag; j++)
            {
                memset(temp_token, '\0', sizeof(temp_token));
                strcpy(temp_token, token);
                strcat(temp_token, "_#D");
                strcat(temp_token, itoa(j,buffer,10));

                d_flag = 0;
                for(int k = i; *(lines_anchor + k) != line; k++)
                {
                    line_iter2 = *(lines_anchor + k);
                    if(!strcmp(line_iter2->name, temp_token))
                    {
                        d_flag = 1;
                    }
                }
                
                if (d_flag == 0)
                {
                    token = temp_token;
                }
            }
        }
    }
    
    // transfer the name
    line->name = malloc(strlen(token) + 1);
    memset(line->name, '\0', sizeof(line->name));
    strcpy(line->name, token);
    
    line->name_len = strlen(token);
    //-------------------------------------------------------------------------- OFFSET

    //prepare offset_hex_str_len for output for print_LIST_part
    token = *(tokens + 2);

    int offset_hex_str_len = strlen(token);
    char temp_offset_hex_str_3[] = "   ";
    for (int i = 0; offset_hex_str_len > 0; i++)
    {
        temp_offset_hex_str_3[2 - i] = token[offset_hex_str_len-- - 1];
    }
    line->offset_hex_str_3 = strdup(temp_offset_hex_str_3);
    //The following two didn't work.
    //1. the lifespan of the string is shorter than needed (this proc only?).
    //2. program freezes
    //1. line->offset_hex_str_3 = temp_offset_hex_str_3;
    //2. strcpy(line->offset_hex_str_3, temp_offset_hex_str_3);

    //take offset_dec_str
    token = *(tokens + 3);

    if (!isdigit(token[0]) && token[0] != '(')
    {
        printf("Problem while parsing offset! A\n");
        return 2;
    }

    if(token[0] == '(')
    {
        str_extract_first_number_occurence(token+1, &line->offset_dec_str);
    }
    else
    {
        str_extract_first_number_occurence(token, &line->offset_dec_str);
    }

    line->offset_int = atoi(line->offset_dec_str);
    //-------------------------------------------------------------------------- BIT OFFSET
    if (line->type == BIT)
    {
        token = *(tokens + 4);
        line->bit_offset = atoi(token+1);
    }
    //-------------------------------------------------------------------------- LENGTH
    char* strout;
    int len_in_bits = -1;

    if      (line->type == BIT)   line->length = 0;
    else if (line->type == MODE)  line->length = 0;
    else if (line->type == STRUC) line->length = 0;
    else if (line->type == CHARS)
    {
        //check if length is runtime determined
        if (!isdigit((*(tokens + 4))[5]))
        {
            printf("---------------------------------------------------------------------------------------\n");
            printf("                                  W A R N I N G !\n");
            printf("---------------------------------------------------------------------------------------\n");
            printf("The length of the field %s on row %d is runtime determinded!\n", line->name, row_number);
            printf("Only four bytes of it will be printed during the execution of the PRODAMP procedure!\n");
            printf("Keep in mind that it is probably bigger than that!\n\n");
            line->length = 4;
        }
        else
        {
            line->length = str_extract_first_number_occurence(*(tokens + 4), &strout);
        }
    }
    else if (line->type == AREA)
    {
        //check if length is runtime determined
        if (!isdigit((*(tokens + 4))[5]))
        {
            printf("---------------------------------------------------------------------------------------\n");
            printf("                                  W A R N I N G !\n");
            printf("---------------------------------------------------------------------------------------\n");
            printf("The length of the field %s on row %d is runtime determinded!\n", line->name, row_number);
            printf("Only four bytes of it will be printed during the execution of the PRODAMP procedure!\n");
            printf("Keep in mind that it is probably bigger than that!\n\n");
            line->length = 4;
        }
        else
        {
            line->length = str_extract_first_number_occurence(*(tokens + 4), &strout);
        }
    }
    else if (line->type == INTGR || line->type == POINT || line->type == FIXED || line->type == BIT_AREA)
    {
        len_in_bits = str_extract_first_number_occurence(*(tokens + 4), &strout);
    }
    else if (line->type == CLASS)
    {
        if (*(tokens + 4)[0] == '+')
            token = *(tokens + 6);
        else
            token = *(tokens + 5);
        len_in_bits = str_extract_first_number_occurence(token, &strout);
    }

    if(len_in_bits > 0)
    {
        // line->length = (len_in_bits + 7) / 8
        if      (len_in_bits <  9) line->length = 1;
        else if (len_in_bits < 17) line->length = 2;
        else if (len_in_bits < 25) line->length = 3;
        else if (len_in_bits < 33) line->length = 4;
        else if (len_in_bits < 41) line->length = 5;
        else if (len_in_bits < 49) line->length = 6;
        else if (len_in_bits < 57) line->length = 7;
        else if (len_in_bits < 65) line->length = 8;
    }
    
    //printf("%d\n", line->length);
    //-------------------------------------------------------------------------- FREE MEMORY & RETURN
    for (int i = 0; *(tokens + i); i++)
    {
        free(*(tokens + i));
    }
    free(tokens);

    return 0;
}

//to be improved ?
void print_declaration_part(struct Line** elements_anchor, FILE* f2)
{
    //.ADMINISTRATION_INFORMATIONS: OFFSET=0, LENGTH=4, TYPE=NUMERIC;

    printf("---------------------------------------------------------------------------------------\n");
    printf("-------------------------------- PRINTING DECLARATIONS --------------------------------\n");
    printf("---------------------------------------------------------------------------------------\n");

    struct Line*  element;
    int bits_sequence = 1;

    for(int i = 0; *(elements_anchor + i); i++)
    {
        element = *(elements_anchor + i);

        char  level      = element->level;
        char* name       = element->name;
        enum  types type = element->type;
        char* type_str   = element->type_str;
        char* offset_dec_str = element->offset_dec_str;
        int   offset_int = element->offset_int;
        int   length     = element->length;
        char  bit_offset = element->bit_offset;
        int   level_int  = element->level_int;
        int   name_len   = element->name_len;

        if (type == STRUC || type == MODE) continue;

        if (type == AREA)
        {
            //for now do the same as for type != BIT
            fprintf(f2, ".");
            fprintf(f2, "%s:", name);
            fprintf(f2, " OFFSET=");
            fprintf(f2, "%s,", offset_dec_str);
            fprintf(f2, " LENGTH=");
            fprintf(f2, "%d,", length);
            fprintf(f2, " TYPE=");
            fprintf(f2, "%s;", type_str);
            fprintf(f2, "\n");
        }
        else if (type != BIT)
        {
            fprintf(f2, ".");
            fprintf(f2, "%s:", name);
            fprintf(f2, " OFFSET=");
            fprintf(f2, "%s,", offset_dec_str);
            fprintf(f2, " LENGTH=");
            fprintf(f2, "%d,", length);
            fprintf(f2, " TYPE=");
            fprintf(f2, "%s;", type_str);
            fprintf(f2, "\n");
        }
        else
        {
            if (bit_offset == 0)
            {
                fprintf(f2, ".");
                fprintf(f2, "B_I_T_S_%d:", bits_sequence++);
                fprintf(f2, " OFFSET=");
                fprintf(f2, "%s,", offset_dec_str);
                fprintf(f2, " LENGTH=");
                fprintf(f2, "%d,", 1);
                fprintf(f2, " TYPE=");
                fprintf(f2, "%s;", type_str);
                fprintf(f2, "\n");
            }
        }
    }

    printf("\n Declaration part printed!\n\n");
}

//to be improved ?
void print_LIST_part(struct Line** elements_anchor, FILE* f2, int names_width)
{
    // LIST ('SYNTAX_INPUT_MODE:    ' + HEX_STRING(ACT_INPUT@.SYNTAX_INPUT_MODE));
    // LIST ('PROGRAM_NAME:         ' + ACT_INPUT@.PROGRAM_NAME);

    printf("---------------------------------------------------------------------------------------\n");
    printf("------------------------------ PRINTING LIST STATEMENTS -------------------------------\n");
    printf("---------------------------------------------------------------------------------------\n");

    struct Line*  element;
    char* spaces_sequence = "                                                                                         ";
    int bits_sequence = 1;
    
    for(int i = 0; *(elements_anchor + i); i++)
    {
        element = *(elements_anchor + i);

        char  level          = element->level;
        char* name           = element->name;
        enum  types type     = element->type;
        char* type_str       = element->type_str;
        char* offset_dec_str = element->offset_dec_str;
        char* offset_hex_str_3 = element->offset_hex_str_3;
        int   offset_int     = element->offset_int;
        int   length         = element->length;
        char  bit_offset     = element->bit_offset;
        int   level_int      = element->level_int;
        int   name_len       = element->name_len;

        char* prefix_spaces = "";
        if (level_int == 3) prefix_spaces = "  ";
        if (level_int == 4) prefix_spaces = "    ";
        if (level_int == 5) prefix_spaces = "      ";
        if (level_int == 6) prefix_spaces = "        ";
        if (level_int == 7) prefix_spaces = "          ";
        if (level_int == 8) prefix_spaces = "            ";
        if (level_int == 9) prefix_spaces = "              ";

        //---------------------------------------------------------------------------------------------------------
        // skip if
        //---------------------------------------------------------------------------------------------------------
        if (type == BIT && bit_offset != 0) continue;

        //---------------------------------------------------------------------------------------------------------
        // prepare DEC offset string for output by paddding with spaces (always 4 characters)
        // HEX offset string is already prepared for output by parse_line
        //---------------------------------------------------------------------------------------------------------
        int offset_dec_str_len = strlen(offset_dec_str);
        char offset_dec_str_4[] = "    ";
        for (int i = 0; offset_dec_str_len > 0; i++)
            offset_dec_str_4[3 - i] = offset_dec_str[offset_dec_str_len-- - 1];

        //---------------------------------------------------------------------------------------------------------
        // prepare DEC length string for output by paddding with spaces (always 4 characters)
        //---------------------------------------------------------------------------------------------------------
        char length_str[sizeof(int)*8+1];
        itoa(length,length_str,10);
        int length_str_len = strlen(length_str);
        char length_str_4[] = "    ";
        for (int i = 0; length_str_len > 0; i++)
            length_str_4[3 - i] = length_str[length_str_len-- - 1];

        //---------------------------------------------------------------------------------------------------------
        // print offset_hex (offset_dec) | length_dec
        //---------------------------------------------------------------------------------------------------------
        fprintf(f2, "LIST('%s (%s) | %s | ", offset_hex_str_3, offset_dec_str_4, length_str_4);

        //---------------------------------------------------------------------------------------------------------
        // print [offset spaces]name:[spaces after]
        //---------------------------------------------------------------------------------------------------------
        if (type == BIT)
        {
            fprintf(f2, "%sB_I_T_S_%d:", prefix_spaces, bits_sequence);
            fprintf(f2, "%s", substring(spaces_sequence, 0, (names_width - 11 - strlen(prefix_spaces) + 1)));
        }
        else
        {
            fprintf(f2, "%s%s:", prefix_spaces, name);
            fprintf(f2, "%s", substring(spaces_sequence, 0, (names_width - (strlen(prefix_spaces) + name_len) + 1)));
        }

        //---------------------------------------------------------------------------------------------------------
        // print the rest
        //---------------------------------------------------------------------------------------------------------
        if (type == STRUC || type == MODE)
        {
            fprintf(f2, "' );\n");
        }
        else if (type == BIT)
        {
            if (bits_sequence < 10) fprintf(f2, " ");
            fprintf(f2, " ' + HEX_STRING(STRUCT.B_I_T_S_%d,2));\n", bits_sequence++);
        }
        // else if (type == AREA)
        // {
        //     div_t div_result;
        //     div_result = div(length,4);
        //     //div_result.quot
        //     //div_result.rem
        // }
        else if (type_str == "NUMERIC")
        {
            fprintf(f2, "' + HEX_STRING(STRUCT.%s,%d));\n", name, (2*length));
        }
        else if (type_str == "STRING")
        {
            fprintf(f2, "' + STRUCT.%s);\n", name);
        }
    }

    printf("\n LIST part printed!\n\n");
}

int main(int argc, char* argv[])
{
    printf("\n");
    FILE* f1 = fopen("./input.txt",  "r");
    int nr_rows_f1 = count_file_rows(f1);
    char row[256] = "";
    int rc;

    struct Line** elements_anchor;
    elements_anchor = malloc(sizeof(struct Line*) * (nr_rows_f1 + 1));
    memset(elements_anchor, 0, (sizeof(struct Line*) * (nr_rows_f1 + 1)));
    struct Line** elements_pointer = elements_anchor;
    struct Line*  element;

    //-------------------------------------------------------------------------- Parse lines and save the info to a 'Line' structures array
    printf("---------------------------------------------------------------------------------------\n");
    printf("--------------------------------------- PARSING ---------------------------------------\n");
    printf("---------------------------------------------------------------------------------------\n");

    for (int row_number = 1; fgets(row, sizeof(row), f1); row_number++)
    {
        *elements_pointer = malloc(sizeof(struct Line));
        rc = parse_line(elements_anchor, *elements_pointer, row, row_number);
        if (rc == 0) elements_pointer = (elements_pointer + 1);
    }

    fclose(f1);

    printf("\n Parsing done!\n\n");

    //-------------------------------------------------------------------------- Calculate length of characters between the quotes

    int names_width;
    int max_name_len = 1;
    int max_level = 2;
    for(int i = 0; *(elements_anchor + i); i++)
    {
        element = *(elements_anchor + i);

        if (element->name_len > max_name_len)
        {
            max_name_len = element->name_len;
        }

        if (element->level_int > max_level)
        {
            max_level = element->level_int;
        }
    }
    
    names_width = max_name_len + (max_level * 2) - 4;

    //-------------------------------------------------------------------------- Output to file

    FILE* f2 = fopen("./output.txt", "w");

    copy_file(f2, "./text/FIXED_PART_BEFORE.txt", 1);
    print_declaration_part(elements_anchor, f2);
    copy_file(f2, "./text/FIXED_PART_BETWEEN.txt", 1);
    print_LIST_part(elements_anchor, f2, names_width);
    copy_file(f2, "./text/FIXED_PART_AFTER.txt", 0);

    fclose(f2);

    //-------------------------------------------------------------------------- Process has finished. Print some info for the user

    f2 = fopen("./text/info.txt", "r");

    for (int row_number = 1; fgets(row, sizeof(row), f2); row_number++)
    {
        printf("%s", row);
    }

    fclose(f2);

    return 0;
}
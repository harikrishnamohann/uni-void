#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/strings.h"

// Reads given character sequence file int *read_content.
// Returns 0 on success and -1 on failures.
int8_t read_file_content(const char* filename, String* read_content) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
      return -1;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
      fclose(fp);
      return -1;
    }
    long file_size = ftell(fp);
    if (file_size == -1) {
        fclose(fp);
        return -1;
    }

    rewind(fp);

    read_content->length = 0;
    read_content->str = (char*)malloc(file_size + 1);
    if (read_content->str == NULL) {
        fclose(fp);
        return -1;
    }

    size_t read_size = fread(read_content->str, 1, file_size, fp);
    if (read_size != (size_t)file_size) {
        fclose(fp);
        return -1;
    }

    read_content->str[read_size] = '\0';
    read_content->length = read_size;

    fclose(fp);
    return 0;
}

// Writes given content to filename.
// returns 0 on success and -1 on failure.
int8_t write_to_file(const char* filename, const String content) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        return -1;    
    }
    
    size_t written = fwrite(content.str, sizeof(char), content.length, fp);
    fclose(fp);
    
    return (written == str_len(content)) ? 0 : -1;   
}

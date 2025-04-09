#include <stdio.h>
#include "plm_format.h"

int plm_load(const char *filename, plm_program_t *program) {
    FILE *file = fopen(filename, "rb");
    if (!file) return -1;

    plm_header_t header;
    fread(&header, sizeof(plm_header_t), 1, file);

    if (header.magic != PLM_MAGIC) {
        fclose(file);
        return -1;
    }

    program->entry_point = header.entry_point;
    program->code = malloc(header.code_size);
    fread(program->code, header.code_size, 1, file);

    fclose(file);
    return 0;
}

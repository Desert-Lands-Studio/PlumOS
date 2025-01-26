#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plm_format.h"

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <output.plm> <entry_point> <code.bin> <data.bin> [resources...]\n", argv[0]);
        return 1;
    }

    const char *output_file = argv[1];
    uint64_t entry_point = strtoull(argv[2], NULL, 0);
    const char *code_file = argv[3];
    const char *data_file = argv[4];

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        perror("Failed to create PLM file");
        return 1;
    }

    plm_header_t header = {
        .magic = PLM_MAGIC,
        .version = 2,
        .entry_point = entry_point,
        .code_size = 0,
        .data_size = 0,
        .resources_offset = 0,
        .resource_count = argc - 5
    };

    fwrite(&header, sizeof(header), 1, out);

    FILE *code = fopen(code_file, "rb");
    fseek(code, 0, SEEK_END);
    header.code_size = ftell(code);
    fseek(code, 0, SEEK_SET);

    void *code_buffer = malloc(header.code_size);
    fread(code_buffer, header.code_size, 1, code);
    fwrite(code_buffer, header.code_size, 1, out);
    fclose(code);
    free(code_buffer);

    FILE *data = fopen(data_file, "rb");
    fseek(data, 0, SEEK_END);
    header.data_size = ftell(data);
    fseek(data, 0, SEEK_SET);

    void *data_buffer = malloc(header.data_size);
    fread(data_buffer, header.data_size, 1, data);
    fwrite(data_buffer, header.data_size, 1, out);
    fclose(data);
    free(data_buffer);

    header.resources_offset = ftell(out);
    plm_resource_t *resources = malloc(sizeof(plm_resource_t) * header.resource_count);

    for (int i = 0; i < header.resource_count; i++) {
        const char *resource_file = argv[5 + i];
        FILE *res = fopen(resource_file, "rb");
        fseek(res, 0, SEEK_END);
        resources[i].size = ftell(res);
        resources[i].offset = ftell(out);
        resources[i].type = strstr(resource_file, ".png") ? 0x01 : 0x02;
        resources[i].flags = 0;

        fseek(res, 0, SEEK_SET);
        void *res_buffer = malloc(resources[i].size);
        fread(res_buffer, resources[i].size, 1, res);
        fwrite(res_buffer, resources[i].size, 1, out);
        fclose(res);
        free(res_buffer);
    }

    fseek(out, sizeof(header), SEEK_SET);
    fwrite(resources, sizeof(plm_resource_t), header.resource_count, out);
    fclose(out);

    printf("PLM file '%s' created successfully.\n", output_file);
    return 0;
}

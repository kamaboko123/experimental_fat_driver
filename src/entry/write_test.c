#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fat12.h"

void print_file(DE *entry){
    FileName filename;
    set_filename(&filename, entry->filename, entry->ext);

    char type = 'f';
    if (entry->attribute & FAT12_ATTR_DIRECTORY) {
        type = 'd';
    }

    printf("[%c, %04d] %10d %s\n", type, entry->first_cluster_low, entry->file_size, filename.display);
}

void ls_fat12(DE *entry){
    while (entry->filename[0] != 0x00){
        if (entry->filename[0] == 0xE5){
            entry++;
            continue;
        }
        if (entry->attribute == 0x0F){
            printf("[Err] unsupported LFN\n");
        }
        print_file(entry);
        entry++;
    }
}

int main(void) {
    uint32_t *buf = malloc(1440 * 1024);
    if (buf == NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    memset(buf, 0, 1440 * 1024);

    // Read FAT12 image
    FILE *f = fopen("img/sample_fat12.img", "r");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fread(buf, 1, 1440 * 1024, f);
    fclose(f);

    BPB *bpb = (BPB *)buf;
    RDE *rde = get_rde(bpb);

    printf("=== Before creating new file ===\n");
    printf("<dir />\n");
    ls_fat12(rde);
    printf("\n");

    // Create a new file
    printf("=== Creating new file 'test.txt' ===\n");
    FileName filename;
    set_filename(&filename, "test", "txt");
    
    // create_file creates an empty file entry; file size is set when write_file is called
    uint32_t result = create_file(bpb, rde, &filename);
    if (result == 0) {
        printf("Failed to create file!\n");
        exit(1);
    }
    printf("File created successfully!\n");

    // Find the new file
    DE *entry = find_entry(rde, &filename);
    if (entry == NULL) {
        printf("File not found after creation!\n");
        exit(1);
    }

    // Write data to the file
    printf("\n=== Writing data to 'test.txt' ===\n");
    char *data = "Hello, FAT12 World! This is a test file.\n";
    uint32_t bytes_written = write_file(bpb, entry, (uint8_t *)data, strlen(data));
    printf("Wrote %d bytes\n", bytes_written);

    // Verify by reading back
    printf("\n=== Reading back the file content ===\n");
    uint8_t *read_buf = (uint8_t *)malloc(entry->file_size);
    uint32_t bytes_read = read_file(bpb, entry, read_buf, 0, entry->file_size);
    printf("Read %d bytes: ", bytes_read);
    for (uint32_t i = 0; i < bytes_read; i++) {
        printf("%c", read_buf[i]);
    }
    printf("\n");
    free(read_buf);

    // Show updated directory listing
    printf("\n=== After creating new file ===\n");
    printf("<dir />\n");
    ls_fat12(rde);
    printf("\n");

    // Write the modified image back
    printf("=== Saving modified image ===\n");
    f = fopen("img/sample_fat12_modified.img", "w");
    if (f == NULL) {
        printf("Error opening file for writing!\n");
        exit(1);
    }
    fwrite(buf, 1, 1440 * 1024, f);
    fclose(f);
    printf("Modified image saved to img/sample_fat12_modified.img\n");

    free(buf);
    return 0;
}

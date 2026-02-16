#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fat12.h"

void ls_fat12(DE *entry){
    while (entry->filename[0] != 0x00){
        if (entry->filename[0] == 0xE5){
            entry++;
            continue;
        }
        if (entry->attribute == 0x0F){
            entry++;
            continue;
        }
        
        FileName filename;
        set_filename(&filename, entry->filename, entry->ext);
        char type = (entry->attribute & FAT12_ATTR_DIRECTORY) ? 'd' : 'f';
        printf("[%c, %04d] %10d %s\n", type, entry->first_cluster_low, entry->file_size, filename.display);
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

    printf("=== Multi-cluster write test ===\n");
    printf("Sector size: %d bytes\n", bpb->bytes_per_sector);
    printf("\n");

    // Create a new file
    printf("Creating new file 'large.txt'...\n");
    FileName filename;
    set_filename(&filename, "large", "txt");
    
    uint32_t result = create_file(bpb, rde, &filename, 0);
    if (result == 0) {
        printf("Failed to create file!\n");
        exit(1);
    }

    // Find the new file
    DE *entry = find_entry(rde, &filename);
    if (entry == NULL) {
        printf("File not found after creation!\n");
        exit(1);
    }

    // Write multi-sector data (2048 bytes = 4 sectors)
    printf("Writing 2048 bytes (4 sectors) to file...\n");
    uint8_t *data = (uint8_t *)malloc(2048);
    for (int i = 0; i < 2048; i++) {
        data[i] = 'A' + (i % 26);  // Fill with alphabet pattern
    }
    
    uint32_t bytes_written = write_file(bpb, entry, data, 2048);
    printf("Wrote %d bytes\n", bytes_written);
    printf("File size: %d bytes\n", entry->file_size);
    printf("First cluster: %d\n", entry->first_cluster_low);

    // Verify cluster chain
    printf("\nCluster chain: ");
    uint16_t cluster = entry->first_cluster_low;
    int cluster_count = 0;
    while (cluster < FAT12_EOC && cluster > FAT12_RESERVED) {
        printf("%d -> ", cluster);
        cluster = get_fat12_entry(bpb, cluster);
        cluster_count++;
    }
    printf("EOC\n");
    printf("Total clusters used: %d\n", cluster_count);

    // Verify by reading back
    printf("\nVerifying data...\n");
    uint8_t *read_buf = (uint8_t *)malloc(entry->file_size);
    uint32_t bytes_read = read_file(bpb, entry, read_buf, 0, entry->file_size);
    printf("Read %d bytes\n", bytes_read);
    
    // Check if data matches
    int errors = 0;
    for (uint32_t i = 0; i < bytes_read; i++) {
        if (read_buf[i] != data[i]) {
            printf("Mismatch at byte %d: expected %02x, got %02x\n", i, data[i], read_buf[i]);
            errors++;
            if (errors > 10) {
                printf("... (too many errors, stopping)\n");
                break;
            }
        }
    }
    
    if (errors == 0) {
        printf("✓ All data verified successfully!\n");
    } else {
        printf("✗ Data verification failed with %d errors\n", errors);
    }

    free(data);
    free(read_buf);

    // Show directory listing
    printf("\n=== Directory listing ===\n");
    ls_fat12(rde);

    // Write the modified image back
    printf("\n=== Saving modified image ===\n");
    f = fopen("img/sample_fat12_large.img", "w");
    if (f == NULL) {
        printf("Error opening file for writing!\n");
        exit(1);
    }
    fwrite(buf, 1, 1440 * 1024, f);
    fclose(f);
    printf("Modified image saved to img/sample_fat12_large.img\n");

    free(buf);
    return 0;
}

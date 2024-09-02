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

    printf("[%c, %d] %10d %s\n", type, entry->first_cluster_low, entry->file_size, filename.display);
}

void print_cluster_link(BPB *bpb, uint32_t cluster_number){
    printf("cluster link: ");
    uint16_t next_cluster = cluster_number;
    do{
        printf("%d -> ", next_cluster);
        next_cluster = get_fat12_entry(bpb, next_cluster);
    }while(next_cluster < 0xFF8 && next_cluster > 0x001); // 0xFF8: EOC, 0x001: BAD
    printf("[END]\n");
}

void dump_fat12(BPB *bpb, uint16_t cluster_number, uint16_t n){
    printf("[FAT12 dumping from %d to %d]\n", cluster_number, cluster_number + n);
    for(int i = cluster_number; i < cluster_number + n; i++){
        uint16_t fat_entry = get_fat12_entry(bpb, i);
        printf("fat[%d] = %d(0x%03x)\n", i, fat_entry, fat_entry);
    }
}

void ls_fat12(DE *entry){
    while (entry->filename[0] != 0x00){
        if (entry->filename[0] == 0xE5){
            // ファイル名の先頭が0xE5の場合は削除済みのエントリ
            entry++;
            continue;
        }
        if (entry->attribute == 0x0F){
            // ファイル名の先頭が0x0Fの場合はLFN(Long File Name)エントリ
            // TODO: LFNの処理
            printf("[Err] unsupported LFN\n");
        }
        print_file(entry);
        entry++;
    }
}

void type_fat12(BPB *bpb, DE *entry, bool print_file_info, bool print_ascii){
    if(print_file_info){
        printf("[File info]\n");
        printf("filename: %s\n", entry->filename);
        printf("file size: %d\n", entry->file_size);
        print_cluster_link(bpb, entry->first_cluster_low);
    }

    uint8_t *fbuf = (uint8_t *)malloc(entry->file_size);
    uint32_t n = read_file(bpb, entry, fbuf, 0, entry->file_size);
    if(print_file_info){
        printf("Readed: %d bytes\n", n);
    }

    printf("\n[File content]\n");
    for(int i = 0; i < entry->file_size; i++){
        if(print_ascii){
            printf("%c", fbuf[i]);
        }
        else{
            printf("%02x ", fbuf[i]);
            if((i + 1) % 16 == 0) printf("\n");
        }
    }
    printf("[End of content]\n");

    free(fbuf);
}


int main(void) {
    uint32_t *buf = malloc(1440 * 1024);
    if (buf == NULL) {
        printf("Error allocating memory!\n");
        exit(1);
    }
    memset(buf, 0, 1440 * 1024);

    //read file into buf
    FILE *f = fopen("img/sample_fat12.img", "r");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fread(buf, 1, 1440 * 1024, f);
    fclose(f);

    RDE *rde = get_rde((BPB *)buf);

    printf("<dir />\n");
    ls_fat12(rde);
    printf("\n====\n");


    FileName filename;
    set_filename(&filename, "bigfile", "txt");

    DE *entry = find_entry(rde, &filename);
    if (entry == NULL) {
        printf("File not found!\n");
        return 0;
    }
    printf("<type bigfile.txt>\n");
    type_fat12((BPB *)buf, entry, true, true);
    
    //printf("\n====\n");
    //dump_fat12((BPB *)buf, 0, 51);

    free(buf);
}

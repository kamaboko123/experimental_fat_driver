# FAT Driver Exam
This is a FAT driver exam for the My hobby Operating Systems.
The goal of this exam is to implement a FAT driver in C language and built in the My hobby Operating Systems.

## Features
- Read FAT12 filesystem structure
- List directory entries
- Read file contents
- **Write files to FAT12 filesystem** ✨
- Create new file entries
- Multi-cluster file support

## Make first FAT image
```
./mkfat12_image/mkfat12_image.sh
```

## Build
```
make
```

## Run
### List files
```
./bin/test_fat
```

### Test file writing
Write a small file:
```
./bin/write_test
```

Write a multi-cluster file:
```
./bin/multi_cluster_test
```

## API Usage

### Create a new file
```c
FileName filename;
set_filename(&filename, "test", "txt");
create_file(bpb, rde, &filename);
```

### Write data to file
```c
DE *entry = find_entry(rde, &filename);
char *data = "Hello, World!";
write_file(bpb, entry, (uint8_t *)data, strlen(data));
```

### Read data from file
```c
uint8_t *buffer = malloc(entry->file_size);
read_file(bpb, entry, buffer, 0, entry->file_size);
```


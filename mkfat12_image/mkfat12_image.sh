#/bin/bash
CDIR=$(pwd)
WDIR=$(cd $(dirname $0); pwd)

cd $WDIR

rm -rf root

# Create director and files for FAT12 image
mkdir -p root
# simle dir
mkdir -p root/dir1
# nested dir
mkdir -p root/dir2
mkdir -p root/dir2/subdir1
# nested many dirs
mkdir -p root/dir3
for i in {1..30} ; do
    mkdir -p root/dir3/subdir$i
done
# create multi cluster file
# > 5KB
cp files/bigfile.txt root/bigfile.txt

# Create files in the directories
echo "Hello, World!" > root/hello.txt
echo "This is '/root/dir/test.txt'" > root/dir1/test.txt
echo "This is '/root/dir2/subdir1/test.txt'" > root/dir2/subdir1/test.txt
echo "This is '/root/dir3/subdir30/test.txt'" > root/dir3/subdir30/test.txt

cd $CDIR

# Create out directory
mkdir -p img

# Create a FAT12 image
# Create a 1.44MB floppy image
mformat -C -f 1440 -i img/sample_fat12.img ::

# Copy the files to the image
mcopy -i img/sample_fat12.img -s ${WDIR}/root/* ::

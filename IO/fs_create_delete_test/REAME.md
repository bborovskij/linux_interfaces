# Task
Write a program that measures the time required to create and then remove a
large number of 1-byte files from a single directory. The program should create
files with names of the form xNNNNNN, where NNNNNN is replaced by a random six-digit
number. The files should be created in the random order in which their names are
generated, and then deleted in increasing numerical order (i.e., an order that is
different from that in which they were created). The number of files (NF) and the
directory in which they are to be created should be specifiable on the command
line. Measure the times required for different values of NF (e.g., in the range from
1000 to 20,000) and for different file systems (e.g., ext2, ext3, and XFS). What
patterns do you observe on each file system as NF increases? How do the various
file systems compare? Do the results change if the files are created in increasing
numerical order (x000000, x000001, x0000002, and so on) and then deleted in the same
order? If so, what do you think the reason(s) might be? Again, do the results vary
across file-system types?

# Solution
## Resuts
The results show 5 iterations for EXT2 and EXT4 FS. 
5 iterations were started in a loop. We consider 5 iterations as one test.
Caches were cleared between every test.
Incrementally, EXT4 performs way better than EXT2.
When we create files incrementally, the directory’s HTree ends up being more “in order”. 
This leads to faster lookups and less overhead when we later delete the files in order. 
But when files are created in random order, the entries are scattered within the directory’s index. 
As a result, deleting the files sequentially forces EXT4 to search and update a more fragmented index, 
which increases the time it takes to remove each file.


Let's look at each test closely:

### EXT2 5 iterations incrementally:
```bash
# Create an image
$ dd if=/dev/zero of=ext2.img bs=4M count=100
# Format img
$ mkfs.ext2 -b 1024 ext2.img
# Mount
$ mkdir /tmp/test_fs
$ sudo mount -o loop ext2.img /tmp/test_fs
# Compile and run test
$ make
$ for i in {1..5}; do echo $i; sudo ./build/test_fs -n 50000 -i /tmp/test_fs/; done
1
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 2 s 732 ms
Deleted 50000 files in 1 s 732 ms
2
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 19 s 903 ms
Deleted 50000 files in 1 s 712 ms
3
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 20 s 227 ms
Deleted 50000 files in 1 s 697 ms
4
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 20 s 430 ms
Deleted 50000 files in 1 s 696 ms
5
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 20 s 48 ms
Deleted 50000 files in 1 s 699 ms
```

### EXT2 5 iterations in random order (without -i option):
```bash
# sync and clear cache
$ sync
$ sudo -i
$ echo 3 > /proc/sys/vm/drop_caches
$ exit
# Run test
$ for i in {1..5}; do echo $i; sudo ./build/test_fs -n 50000 /tmp/test_fs/; done
1
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 2 s 790 ms
Deleted 50000 files in 1 s 928 ms
2
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 19 s 873 ms
Deleted 50000 files in 1 s 950 ms
3
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 19 s 998 ms
Deleted 50000 files in 1 s 939 ms
4
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 19 s 891 ms
Deleted 50000 files in 1 s 971 ms
5
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 19 s 914 ms
Deleted 50000 files in 1 s 957 ms
```

### EXT4 5 iterations incrementally:
```bash
# Clean up after EXT2 
$ sudo umount /tmp/test_fs
$ rm ext2.img
# Create an image
$ dd if=/dev/zero of=ext4.img bs=4M count=100
# Format img
$ mkfs.ext4 -b 1024 ext4.img
# Mount
$ mkdir /tmp/test_fs
$ sudo mount -o loop ext4.img /tmp/test_fs
# Run test
$ for i in {1..5}; do echo $i; sudo ./build/test_fs -n 50000 -i /tmp/test_fs/; done
1
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 4 s 29 ms
Deleted 50000 files in 6 s 515 ms
2
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 7 s 842 ms
Deleted 50000 files in 5 s 99 ms
3
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 8 s 928 ms
Deleted 50000 files in 8 s 380 ms
4
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 7 s 625 ms
Deleted 50000 files in 9 s 219 ms
5
Creating files incrementally
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 9 s 81 ms
Deleted 50000 files in 8 s 143 ms
```


### EXT4 5 iterations in random order (without -i option):
```bash
# sync and clear cache
$ sync
$ sudo -i
$ echo 3 > /proc/sys/vm/drop_caches
$ exit
# Run test
$ for i in {1..5}; do echo $i; sudo ./build/test_fs -n 50000 /tmp/test_fs/; done
1
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 10 s 137 ms
Deleted 50000 files in 64 s 636 ms
2
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 10 s 666 ms
Deleted 50000 files in 81 s 249 ms
3
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 11 s 480 ms
Deleted 50000 files in 56 s 554 ms
4
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 9 s 42 ms
Deleted 50000 files in 62 s 613 ms
5
Creating files in random order
Creating 50000 files in /tmp/test_fs_mount
Created 50000 files in 9 s 2 ms
Deleted 50000 files in 62 s 40 ms
```

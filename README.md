# Un_CourseProject_OS
FS for Linux based on VFS

- To mount run
```console
foo@bar:~? sudo make
foo@bar:~? sudo insmod gfs.ko
foo@bar:~? sudo mount -o loop -t gfs ./image ./gfs_mount
```

- To fully unmount run
```console
foo@bar:~? sudo umount ./gfs_mount
foo@bar:~? sudo rmmod.ko
```

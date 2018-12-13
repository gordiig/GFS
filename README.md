# Un_CourseProject_OS
FS for Linux based on VFS

- To mount run
```console
foo@bar:~? sudo make
foo@bar:~? sudo insmod gfs.ko
foo@bar:~? sudo mount -t gfs gfs ./gfs_mount
```

- Only root has permissions, so run the code below for operate with filesystem
```console
foo@bar:~? sudo -i
```

- To fully unmount run
```console
foo@bar:~? sudo umount ./gfs_mount
foo@bar:~? sudo rmmod.ko
```

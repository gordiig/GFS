#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/romfs_fs.h>
#include <linux/pagemap.h> 

#define GFS_MAGIC_NUMBER 0x13420228

// Superblock
static struct dentry* gfsMount(struct file_system_type *fst, int flags, const char *devname, void *data);
static int gfsFillSuper(struct super_block *sb, void *data, int silent);
static void gfsPutSuper(struct super_block *sb);

static struct super_operations gfsSbOp = {
    .statfs = simple_statfs,
    .drop_inode = generic_drop_inode,
    .put_super = gfsPutSuper,
};

static void gfsPutSuper(struct super_block *sb)
{
    printk(KERN_INFO "GFS put_super was called! Bye-bye!\n");
}

static struct dentry* gfsMount(struct file_system_type *fst, int flags, const char *dev_name, void *data)
{
    struct dentry *ret = mount_bdev(fst, flags, dev_name, data, gfsFillSuper);
    if (IS_ERR(ret))
    {
        printk(KERN_ERR "GFS Error in mount_bdev()!\n");
    }
    else
    {
        printk(KERN_INFO "GFS mount_bdev passed sucsessfully!\n");
    }
    return ret;
}

static int gfsFillSuper(struct super_block *sb, void *data, int silent)
{
    sb->s_blocksize = PAGE_CACHE_SIZE;
    sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
    sb->s_op = &gfsSbOp;
    sb->s_magic = GFS_MAGIC_NUMBER;

    struct inode *rootInode = new_inode(sb);
    if (!rootInode)
    {
        printk(KERN_ERR "GFS new_inode() error!\n");
        return -1;
    }

    rootInode->i_ino = 0;
    rootInode->i_sb = sb;
    rootInode->i_atime = rootInode->i_mtime = rootInode->i_ctime = CURRENT_TIME;
    inode_init_owner(rootInode, NULL, S_IFDIR);

    struct dentry *rootDentry = d_make_root(rootInode);
    if (!rootDentry)
    {
        printk(KERN_ERR "GFS d_make_root() error!\n");
        return -1;
    }
    sb->s_root = rootDentry;

    return 0;
}


// INIT
static struct file_system_type gfsType = {
    .owner = THIS_MODULE,
    .name = "gfs",
    .mount = gfsMount,
    .kill_sb = kill_block_super,
};

static int __init initFS(void)
{
    printk(KERN_INFO "GFS Starting to create fs!\n");

    int ret = register_filesystem(&gfsType);

    if (ret != 0)
    {
        printk(KERN_ERR "GFS Could't create fs!\n");
        return ret;
    }

    printk(KERN_INFO "GFS fs was registred!\n");
    return 0;
}

static void __exit exitFS(void)
{
    printk(KERN_INFO "GFS Starting to remove module!\n");

    int ret = unregister_filesystem(&gfsType);
    if (ret != 0)
    {
        printk(KERN_ERR "GFS Can't unregister fs!\n");
        return;
    }
    
    printk(KERN_INFO "Module is removed!\n");
}

module_init(initFS);
module_exit(exitFS);

MODULE_AUTHOR("Gordiig");
MODULE_LICENSE("MIT");

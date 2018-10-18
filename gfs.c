#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/romfs_fs.h>
#include <linux/pagemap.h> 

#define GFS_MAGIC_NUMBER 0x13420228

static struct inode* gfsGetInode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev);

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
    struct dentry *ret = mount_nodev(fst, flags, data, gfsFillSuper);
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
    sb->s_blocksize = PAGE_SIZE; // PAGE_CACHE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT; // PAGE_CACHE_SHIFT
    sb->s_op = &gfsSbOp;
    sb->s_magic = GFS_MAGIC_NUMBER;

    // struct inode *rootInode = new_inode(sb);
    struct inode *rootInode = gfsGetInode(sb, NULL, S_IFDIR, NULL);
    if (!rootInode)
    {
        printk(KERN_ERR "GFS new_inode() error!\n");
        return -1;
    }

    // rootInode->i_ino = 0;
    // rootInode->i_sb = sb;
    // rootInode->i_atime = rootInode->i_mtime = rootInode->i_ctime = CURRENT_TIME;
    // rootInode->i_mode = S_IFDIR;
    // inode_init_owner(rootInode, NULL, S_IFDIR);

    struct dentry *rootDentry = d_make_root(rootInode);
    if (!rootDentry)
    {
        printk(KERN_ERR "GFS d_make_root() error!\n");
        return -1;
    }
    sb->s_root = rootDentry;

    return 0;
}

void gfsKillSb(struct super_block *sb)
{
    kill_litter_super(sb);
}


// File
static unsigned long gfsMMUGetUnmappedArea(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags);

static struct file_operations gfsFileOps = {
    .read_iter = generic_file_read_iter,
    .write_iter = generic_file_write_iter,
    .mmap = generic_file_mmap,
    .fsync = generic_file_fsync,
    .splice_read = generic_file_splice_read,
    .splice_write = iter_file_splice_write,
    .llseek = generic_file_llseek,
    .get_unmapped_area = gfsMMUGetUnmappedArea,
};

static unsigned long gfsMMUGetUnmappedArea(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags)
{
    return current->mm->get_unmapped_area(file, addr, len, pgoff, flags);
}


// Inode
static int gfsInodeMknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev);
static int gfsInodeCreate(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static int gfsInodeMkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
static int gfsInodeSymlink(struct inode *dir, struct dentry *dentry, const char *symname);

// static struct inode_operations gfsInodeOps = {
//     .create = gfsInodeCreate,
//     .mknod	= gfsInodeMknod,
//     .mkdir	= gfsInodeMkdir,
//     .symlink = gfsInodeSymlink,
//     // .lookup = gfsInodeLookup,
//     // .link	= gfsInodeLink,
// 	// .unlink = gfsInodeUnlink,
// 	// .rmdir	= gfsInodeRmdir,
// 	// .rename	= gfsInodeRename,
// }

static struct inode_operations gfsInodeOps = {
    .setattr = simple_setattr,
    .getattr = simple_getattr,
};

static struct inode_operations gfsDirInodeOps = {
    .create = gfsInodeCreate,
    .mknod  = gfsInodeMknod,
    .mkdir  = gfsInodeMkdir,
    .symlink = gfsInodeSymlink,
    .lookup = simple_lookup,
    .link   = simple_link,
    .unlink = simple_unlink,
    .rmdir  = simple_rmdir,
    .rename = simple_rename,
};

static struct inode* gfsGetInode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev)
{
    struct inode *new = new_inode(sb);

    if (new)
    {
        new->i_ino = get_next_ino();
        inode_init_owner(new, dir, mode);
        new->i_atime = new->i_ctime = new->i_mtime = CURRENT_TIME;

        switch (mode & S_IFMT)
        {
            case S_IFREG:
                new->i_op = &gfsInodeOps;
                new->i_fop = &gfsFileOps;
                break;

            case S_IFDIR:
                new->i_op = &gfsDirInodeOps;
                new->i_fop = &simple_dir_operations;
                break;
            default:
                init_special_inode(new, mode, dev);
                break;
        }
    }

    return new;
}

static int gfsInodeMknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
    struct inode *inode = gfsGetInode(dir->i_sb, dir, mode, dev);
    int err = -ENOSPC;

    if (inode)
    {
        d_instantiate(dentry, inode);
        dget(dentry);
        err = 0;
        dir->i_mtime = dir->i_ctime = CURRENT_TIME;
    }

    return err;
}

static int gfsInodeCreate(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
    return gfsInodeMknod(dir, dentry, mode | S_IFREG, 0);
}

static int gfsInodeMkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    int ret = gfsInodeMknod(dir, dentry, mode | S_IFDIR, 0);
	if (!ret)
    {
		inc_nlink(dir);
    }
	return ret;
}

static int gfsInodeSymlink(struct inode *dir, struct dentry *dentry, const char *symname)
{
    struct inode *inode = gfsGetInode(dir->i_sb, dir, S_IFLNK | S_IRWXUGO, 0);
    int err = -ENOSPC;

    if (inode)
    {
        int len = strlen(symname) + 1;
        err = page_symlink(inode, symname, len);
        if (!err)
        {
            d_instantiate(dentry, inode);
            dget(dentry);
            dir->i_mtime = dir->i_atime = dir->i_ctime = CURRENT_TIME;
        }
        else 
        {
            iput(inode);
        }
    }

    return err;
}





// INIT
static struct file_system_type gfsType = {
    .owner = THIS_MODULE,
    .name = "gfs",
    .mount = gfsMount,
    .kill_sb = gfsKillSb,
    .fs_flags = FS_USERNS_MOUNT,
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

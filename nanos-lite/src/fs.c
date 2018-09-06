#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb

	file_table[3].size = _screen.width * _screen.height * sizeof(uint32_t);
	file_table[3].open_offset = 0;

}

int fs_open(const char *pathname, int flags, int mode){
	Log("pathname:%s",pathname);
	for(int i = 0; i < NR_FILES; i++){
		if(strcmp(file_table[i].name,pathname) == 0){
			file_table[i].open_offset = 0;
			Log("OK!!");
			return i;
		}
	}
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len){
	Log("fd = %d,size = %d",fd,file_table[fd].size);
	Finfo *file = &file_table[fd];
	int count = file->open_offset + len;
	len = count > (file->size) ? (file->size - file->open_offset) : len;

	switch(fd){
		case FD_STDIN:
		case FD_STDERR:
			return -1;

		case FD_DISPINFO:
			if(len < 0) len = 0;
			dispinfo_read(buf, file->open_offset,len);
						 break;

		default:
			if(file->open_offset >= file->size) return 0;
			assert(fd >= 6 && fd <= NR_FILES);
			ramdisk_read(buf, file->disk_offset + file->open_offset,len);
			break;
	}
	file->open_offset += len;
	Log("Finish read, len = %d", len);
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
	//Log("fd = %d",fd);
	Finfo *file = &file_table[fd];
	int i = 0;
	int count = file->open_offset + len;
	len = count > file->size ? (file->size - file->open_offset) : len;
	if(file->open_offset >= file->size) return 0;
	
	switch(fd){
		case FD_STDOUT:
		case FD_STDERR:
			for(;i < len;i++)
				_putc(((char *)buf)[i]);
			return len;
		case FD_FB:
			fb_write(buf,file->open_offset,len);
			break;
		default:if(fd < 6 || fd >= NR_FILES)
					return -1;
				Log("fd = %d",fd);
				ramdisk_write(buf,file->disk_offset + file->open_offset,len);
				Log("Finish write,len = %d",len);
				break;
	}
	file->open_offset += len;
	return len;

}

off_t fs_lseek(int fd , off_t offset, int whence){
	Log("fd = %d, offset = %d, whence = %d",fd, offset, whence);
	Finfo *file = &file_table[fd];
	assert(fd < NR_FILES);
	switch(whence){
		case SEEK_SET:file->open_offset = offset;
					  break;
		case SEEK_CUR:file->open_offset += offset;
					  break;
		case SEEK_END:file->open_offset = file->size + offset;
					  break;
		default:return -1;
	}

	return file->open_offset;
}

int fs_close(int fd){
	return 0;
}

size_t fs_filesz(int fd){
	return file_table[fd].size;
}

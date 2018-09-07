#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
	printf("t %d\n",_uptime());
	int key = _read_key();
	if(key < 256){
		printf("kd %s\n",keyname[key]);
	}
	else{
		printf("ku %s\n",keyname[key -0x8000]);
	}
	return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf,dispinfo + offset, len);

}

void fb_write(const void *buf, off_t offset, size_t len) {
	  int row, col;
	  offset /= 4;
	  col = offset % _screen.width;
	  row = offset / _screen.width;
	  _draw_rect(buf, col, row, len/4, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention

  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}

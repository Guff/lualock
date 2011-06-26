#include <Imlib2.h>
#include <stdbool.h>

typedef struct {
	Imlib_Image image;
	
	unsigned char *data;
} image_t;

int image_get_width(image_t *image);
int image_get_height(image_t *image);

bool image_new(const char *filename, image_t *image);

void lualock_lua_image_init(lua_State *L);

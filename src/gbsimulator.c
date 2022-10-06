
#include "sidlib.h"
#include "lcdc.h"
#include "error.h"
#include "gameboy.h"
#include "util.h"
#include <sys/time.h>

#include <stdint.h>
#include <stdlib.h>

// Key press bits
#define MY_KEY_UP_BIT    	0x01
#define MY_KEY_DOWN_BIT  	0x02
#define MY_KEY_RIGHT_BIT 	0x04
#define MY_KEY_LEFT_BIT  	0x08
#define MY_KEY_A_BIT     	0x10
#define MY_KEY_B_BIT 	 	0x20
#define MY_KEY_SELECT_BIT 	0x40
#define MY_KEY_START_BIT 	0x80

#define SCALE 3
#define GREY_SCALE(x) (255 - 85 * x)
#define MILLION 1000000

gameboy_t gb;
struct timeval start; 
struct timeval paused;

// ======================================================================
uint64_t get_time_in_GB_cycles_since(struct timeval* from) {
	struct timeval time;
	struct timeval delta;
		
	gettimeofday(&time, NULL);
	if(!timercmp(&time, from, >)) {
		return 0;
	}
	timersub(&time, from, &delta);

	return delta.tv_sec * GB_CYCLES_PER_S + (delta.tv_usec * GB_CYCLES_PER_S) / MILLION;
}

// ======================================================================
static void set_grey(guchar* pixels, int row, int col, int width, guchar grey)
{
    const size_t i = (size_t) (3 * (row * width + col)); // 3 = RGB
    pixels[i+2] = pixels[i+1] = pixels[i] = grey;
}

// ======================================================================
static void generate_image(guchar* pixels, int height, int width)
{
	uint64_t cycles = get_time_in_GB_cycles_since(&start);	
	gameboy_run_until(&gb, cycles);
	
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			uint8_t output;
			image_get_pixel(&output, &gb.screen.display, x / SCALE, y / SCALE);
			set_grey(pixels, y, x, width, GREY_SCALE(output));
		}
	}	
}

// ======================================================================
#define do_key(X) \
    do { \
        if (! (psd->key_status & MY_KEY_ ## X ##_BIT)) { \
            psd->key_status |= MY_KEY_ ## X ##_BIT; \
            joypad_key_pressed(&gb.pad, X ##_KEY); \
        } \
    } while(0)

static gboolean keypress_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
    case GDK_KEY_Up:
        do_key(UP);
        return TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        return TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        return TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        return TRUE;

    case 'A':
    case 'a':
        do_key(A);
        return TRUE;
        
    case 'Z':
    case 'z':
		do_key(B);
		return TRUE;
		
	case GDK_KEY_Page_Up:
		do_key(SELECT);
		return TRUE;
	
	case GDK_KEY_Page_Down:
		do_key(START);
		return TRUE;
		
	case GDK_KEY_space: {
			struct timeval time;
			gettimeofday(&time, NULL);
			if(psd->timeout_id > 0) {
				paused = time;
			} else {
				timersub(&paused, &time, &paused);
				timeradd(&start, &paused, &start);
				timerclear(&paused);
			}
			return ds_simple_key_handler(keyval, data);
		}
    }

    return ds_simple_key_handler(keyval, data);
}
#undef do_key

// ======================================================================
#define do_key(X) \
    do { \
        if (psd->key_status & MY_KEY_ ## X ##_BIT) { \
          psd->key_status &= (unsigned char) ~MY_KEY_ ## X ##_BIT; \
            joypad_key_released(&gb.pad, X ##_KEY); \
        } \
    } while(0)

static gboolean keyrelease_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
    case GDK_KEY_Up:
        do_key(UP);
        return TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        return TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        return TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        return TRUE;

    case 'A':
    case 'a':
        do_key(A);
        return TRUE;
        
    case 'Z':
    case 'z':
		do_key(B);
		return TRUE;
		
	case GDK_KEY_Page_Up:
		do_key(SELECT);
		return TRUE;
	
	case GDK_KEY_Page_Down:
		do_key(START);
		return TRUE;    
    }

    return FALSE;
}
#undef do_key

// ======================================================================
static void error(const char* pgm, const char* msg)
{
    fputs("ERROR: ", stderr);
    if (msg != NULL) fputs(msg, stderr);
    fprintf(stderr, "\nusage:    %s input_file [iterations]\n", pgm);
    fprintf(stderr, "examples: %s rom.gb 1000\n", pgm);
    fprintf(stderr, "          %s game.gb\n", pgm);
}
// ======================================================================
int main(int argc, char *argv[])
{
	if (argc < 2) {
        error(argv[0], "please provide input_file");
        return 1;
    }

    const char* const filename = argv[1];

    zero_init_var(gb);
    int err = gameboy_create(&gb, filename);
    if (err != ERR_NONE) {
        gameboy_free(&gb);
        return err;
    }
    timerclear(&paused);
    gettimeofday(&start, NULL);
    
	sd_launch(&argc, &argv,
			  sd_init("GameBoy Simulator", LCD_WIDTH * SCALE, LCD_HEIGHT * SCALE, 40,
					  generate_image, keypress_handler, keyrelease_handler));				  
	gameboy_free(&gb);

    return 0;
}

#include "v4l2.h"
#include <SDL2/SDL.h>
#include <pthread.h>

/* Global declaration*/
pthread_t thread_stream;
SDL_Window *sdlScreen;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Rect sdlRect;

/* miscellanous */
int thread_exit_sig = 0;

/* Function declaration */
void frame_handler(void *pframe, int length);
void *v4l2_streaming();
void mainstreamloop(void);

/* Extern variable declaration */
extern int fd;
extern char *dev_path, *outfile, *pix_format_str;
extern enum io_method io;
extern struct buffer *buffers;
extern unsigned int n_buffers;
extern unsigned int width , height, capture, frame_count, type, pix_format;
extern struct timeval start_time, end_time;
extern double elapsed_time;

/* Extern function declaration */
extern int read_frame(void);

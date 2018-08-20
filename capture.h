/* Global variable declaration */
int file;

/* Extern variable declaration */
extern int fd;
extern char *dev_path, *outfile, *pix_format_str;
extern enum io_method io;
extern struct buffer *buffers;
extern unsigned int n_buffers;
extern unsigned int width , height, capture, frame_count, type, pix_format, streaming;
extern struct timeval start_time, end_time;
extern double elapsed_time;

/* Function declaration */
void errno_exit(const char *s);
void process_image(const void *buffer_start, int size);
int read_frame(void);
void mainloop(void);
void stop_capturing(void);
void start_capturing(void);
void uninit_device(void);
void init_read(unsigned int buffer_size);
void init_mmap(void);
void init_userp(unsigned int buffer_size);
void init_device(void);
void openDevice(char* dev_path);
void close_device(void);

/* Extern function declaration */
extern void frame_handler(void *pframe, int length);

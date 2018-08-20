/* Extern variable declaration */
extern int fd;
extern char *dev_path, *outfile, *pix_format_str;
extern enum io_method io;
extern struct buffer *buffers;
extern unsigned int n_buffers;
extern unsigned int width , height, capture, frame_count, type, pix_format;
extern struct timeval start_time, end_time;
extern double elapsed_time;

/* Function declaration */
void deviceInfo(void);
void bufferTypeToString(unsigned int ui_type);
void fcc2s(unsigned int ui_pixel_format);
static const char* frmtype2s(unsigned type);
void print_frmsize(const struct v4l2_frmsizeenum frmsize, const char *prefix);
void fract2sec(const struct v4l2_fract f);
void fract2fps(const struct v4l2_fract f);
void print_frmival(const struct v4l2_frmivalenum frmival, const char *prefix);
int listFormats(void);
void enumerateMenu(struct v4l2_queryctrl queryctrl);
void listControls(void);

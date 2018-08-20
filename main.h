
int fd = -1;
char *dev_path = "/dev/video0", *outfile = "default_file", *pix_format_str = "YUYV";
enum io_method io = IO_METHOD_MMAP;
struct buffer *buffers;
unsigned int n_buffers;
unsigned int width = 640, height = 480, capture = 0, frame_count = 1, type = V4L2_CAP_VIDEO_CAPTURE, pix_format = v4l2_fourcc('Y', 'U', 'Y', 'V'), streaming = 1;
struct timeval start_time, end_time;
double elapsed_time;

void usage( FILE *fp, char * name );
int pixStr2pixU32(char* pix_format_str);

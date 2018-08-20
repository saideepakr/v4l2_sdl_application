#include "v4l2.h"
#include "capture.h"

/**
Function Name : errno_exit
Function Description : Displays error message
Parameter : Error name charater pointer
Return : void
**/
void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

/**
Function Name : process_image
Function Description : Writes the image in the output file
Parameter : Start address and size of the image
Return : void
**/
void process_image(const void *buffer_start, int size)
{
		write(file, buffer_start, size);
}

/**
Function Name : read_frame
Function Description : Read the frame and save or stream based on the command line inputs
Parameter : void
Return : int
**/
int read_frame(void)
{
        struct v4l2_buffer buf;
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("read");
                        }
                }

                process_image(buffers[0].start, buffers[0].length);
                break;

        case IO_METHOD_MMAP:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }

                assert(buf.index < n_buffers);
                
                if(buf.flags & 0x00040)
                {
                	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                    //printf("\nFrame crashed");
                    return read_frame(); 
                }
				
				if(streaming == 1)	//Streaming
					frame_handler(buffers[buf.index].start, buf.bytesused);
				else //Capturing	
                	process_image(buffers[buf.index].start, buf.bytesused);

                if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;

        case IO_METHOD_USERPTR:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }

                for (i = 0; i < n_buffers; ++i)
                        if (buf.m.userptr == (unsigned long)buffers[i].start
                            && buf.length == buffers[i].length)
                                break;

                assert(i < n_buffers);
                
                if(buf.flags & 0x00040)
                {
                	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                    //printf("\nFrame crashed");
                    return read_frame(); 
                }

                if(streaming == 1)	//Streaming
					frame_handler(buffers[buf.index].start, buf.bytesused);
				else //Capturing	
                	process_image(buffers[buf.index].start, buf.bytesused);

                if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;
        }

        return 1;
}

/**
Function Name : mainloop
Function Description : Main loop for capturing images by calling read_frame function and printing frames per second (fps)
Parameter : void
Return : void
**/
void mainloop(void)
{
    time_t rawtime;
	struct tm *info;
	
	time( &rawtime );
	info = localtime( &rawtime );
	
    char name_buf[100], suffix[10] = ".", width_height_time_str[50];
    sprintf(width_height_time_str, "_%uX%u_%d_%d_%d_%d_%d_%d", width, height, 1900 + info->tm_year, info->tm_yday, info->tm_hour, info->tm_min, (int)info->tm_sec, (int)start_time.tv_usec/1000); 
    
    strcpy(name_buf, outfile);
    strcat(name_buf, width_height_time_str);
    if(strcmp(pix_format_str, "MJPG") == 0 && frame_count > 1)
    	strcat(suffix, "mpg");
    else if(strcmp(pix_format_str, "MJPG") == 0 && frame_count == 1)
    	strcat(suffix, "jpg");
   	else
    	strcat(suffix, pix_format_str);
    strcat(name_buf, suffix);
	if((file = open(name_buf, O_WRONLY | O_CREAT, 0660)) < 0)
	{
		perror("open");
		exit(1);
	}
	
    unsigned int count;
    
    count = frame_count;
	
	printf("\nRemaincount Frame interval(ms) - Frames per second(fps)\n");
	gettimeofday(&start_time, NULL);
    while (count-- > 0) 
    {
	
    	read_frame();
    	
    	
        fflush(stdout);
		
		gettimeofday(&end_time, NULL);

		elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;      // sec to ms
		elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;   // us to ms

		printf("\r%d \t\t%.2lf - %.1lf", count, elapsed_time, 1000/elapsed_time);
	
		start_time = end_time;

    }
	printf("\n");
	close(file);
}

/**
Function Name : stop_capturing
Function Description : Function to stream off the camera
Parameter : void
Return : void
**/
void stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
                        errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

/**
Function Name : start_capturing
Function Description : Function to queue the buffers and stream on the camera
Parameter : void
Return : void
**/
void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)buffers[i].start;
                        buf.length = buffers[i].length;

                        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;
        }
}

/**
Function Name : uninit_device
Function Description : Function to uninitialize the memory
Parameter : void
Return : void
**/
void uninit_device(void)
{
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
}

/**
Function Name : init_read
Function Description : Function to initialize the memory for io read method
Parameter : size of the image
Return : void
**/
void init_read(unsigned int buffer_size)
{
        buffers = calloc(1, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start) {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }
}

/**
Function Name : init_read
Function Description : Function to initialize the memory for io mmap method
Parameter : void
Return : void
**/
void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "memory mappingn", dev_path);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\\n",
                         dev_path);
                exit(EXIT_FAILURE);
        }

        buffers = calloc(req.count, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
        }
}

/**
Function Name : init_userp
Function Description : Function to initialize the memory for io user pointer method
Parameter : size of the image
Return : void
**/
void init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "user pointer i/on", dev_path);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        buffers = calloc(4, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = malloc(buffer_size);

                if (!buffers[n_buffers].start) {
                        fprintf(stderr, "Out of memory\\n");
                        exit(EXIT_FAILURE);
                }
        }
}

/**
Function Name : init_device
Function Description : Function to query the video capture amd io methods capabilities
Parameter : size of the image
Return : void
**/
void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_format fmt;

        if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\\n",
                                 dev_path);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf(stderr, "%s is no video capture device\\n",
                         dev_path);
                exit(EXIT_FAILURE);
        }

        switch (io) {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                        fprintf(stderr, "%s does not support read i/o\\n",
                                 dev_path);
                        exit(EXIT_FAILURE);
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                        fprintf(stderr, "%s does not support streaming i/o\\n",
                                 dev_path);
                        exit(EXIT_FAILURE);
                }
                break;
        }
        
        struct v4l2_fmtdesc fmtdesc;
        
        fmtdesc.index = 0;
		fmtdesc.type = type;
        
        while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) >= 0)
        {
        	if(pix_format != fmtdesc.pixelformat)
        		fmtdesc.index++;
        	else
        		break;
        }
        
        if(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != 0)
        {
        	printf("Format not supported\n");
        	errno_exit("VIDIOC_ENUM_FMT");
        }
        
        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = width;
        fmt.fmt.pix.height      = height;
        fmt.fmt.pix.pixelformat = fmtdesc.pixelformat;
        
        if (-1 == ioctl(fd, VIDIOC_S_FMT, &fmt))
                        errno_exit("VIDIOC_S_FMT");
             
        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;	       
        
        switch (io) {
        case IO_METHOD_READ:
                init_read(fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                init_mmap();
                break;

        case IO_METHOD_USERPTR:
                init_userp(fmt.fmt.pix.sizeimage);
                break;
        }
                                      
}    

/**
Function Name : openDevice
Function Description : Function to open the device from the specified path
Parameter : device path character pointer
Return : void
**/        
void openDevice(char* dev_path)
{
	if((fd = open(dev_path, O_RDWR)) < 0){
        perror("open");
        exit(1);
    }
}

/**
Function Name : close_device
Function Description : Function to close the device
Parameter : void
Return : void
**/
void close_device(void)
{
	if(fd == -1)
		return;
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

#include "v4l2.h"
#include "v4l2_ctrl.h"
#include "capture.h"

/**
Function Name : deviceInfo
Function Description : Displays the information about the device
Parameter : void
Return : void
**/
void deviceInfo(void)
{
	struct v4l2_capability cap;
	int index;
	
	 if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) 
	 {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\\n",
                                 dev_path);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
     }
     printf("Driver Info\n");
     printf("\tDriver Name : %s\n", cap.driver);
     printf("\tCard Type : %s\n", cap.card);
     printf("\tBus info : %s\n", cap.bus_info);
     printf("\tDriver Version : %u\n", cap.version);
     printf("\tCapabilities : %#x\n", cap.capabilities);
     
     if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
     	printf("\t\tVideo Capture\n");
     if(cap.capabilities & V4L2_CAP_STREAMING)
     	printf("\t\tStreaming\n");
     if(cap.capabilities & V4L2_CAP_EXT_PIX_FORMAT)
     	printf("\t\tExtended Pix Format\n");
     if(cap.capabilities & V4L2_CAP_DEVICE_CAPS)
     {
     	printf("\tDevice Caps : %#x\n", cap.device_caps);
     	if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
		 	printf("\t\tVideo Capture\n");
		if(cap.capabilities & V4L2_CAP_STREAMING)
		 	printf("\t\tStreaming\n");
		if(cap.capabilities & V4L2_CAP_EXT_PIX_FORMAT)
		 	printf("\t\tExtended Pix Format\n");
     }
}

/**
Function Name : bufferTypeToString
Function Description : Displays the information about the device
Parameter : unsigned integer type of V4L2 Capabilities
Return : void
**/
void bufferTypeToString(unsigned int ui_type)
{
	if(ui_type == V4L2_CAP_VIDEO_CAPTURE)
		printf("Video Capture");
	else if(ui_type == V4L2_CAP_VIDEO_OUTPUT)
		printf("Video Output");
}

/**
Function Name : fcc2s
Function Description : Prints the pixel format
Parameter : unsigned integer pixel format
Return : void
**/
void fcc2s(unsigned int ui_pixel_format)
{
	printf("'%c%c%c%c'",(ui_pixel_format & 0xff), ((ui_pixel_format >> 8) & 0xff), ((ui_pixel_format >> 16) & 0xff), ((ui_pixel_format >> 24) & 0xff));
}

/**
Function Name : frmtype2s
Function Description : Uses unsigned integer type and returns the frame type string
Parameter : unsigned integer type
Return : static constant character pointer
**/
static const char* frmtype2s(unsigned type)
{
	static const char *types[] = {
		"Unknown",
		"Discrete",
		"Continuous",
		"Stepwise"
	};

	if (type > 3)
		type = 0;
	return types[type];
}

/**
Function Name : print_frmsize
Function Description : Prints the frame size
Parameter : v4l2_frmsizeenum structure and prefix
Return : void
**/
void print_frmsize(const struct v4l2_frmsizeenum frmsize, const char *prefix)
{
	printf("%s\tSize: %s ", prefix, frmtype2s(frmsize.type));
	if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) 
	{
		printf("%dx%d", frmsize.discrete.width, frmsize.discrete.height);
	} 
	else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) 
	{
		printf("%dx%d - %dx%d with step %d/%d",
				frmsize.stepwise.min_width,
				frmsize.stepwise.min_height,
				frmsize.stepwise.max_width,
				frmsize.stepwise.max_height,
				frmsize.stepwise.step_width,
				frmsize.stepwise.step_height);
	}
	printf("\n");
}

/**
Function Name : fract2sec
Function Description : Converts fractions to seconds and Prints the seconds
Parameter : v4l2_fract structure
Return : void
**/
void fract2sec(const struct v4l2_fract f)
{
	printf("%.3fs ", (1.0 * f.numerator) / f.denominator);
}

/**
Function Name : fract2fps
Function Description : Converts fractions to frames per second and Prints the frames per second
Parameter : v4l2_fract structure
Return : void
**/
void fract2fps(const struct v4l2_fract f)
{
	printf("(%.3f fps)", (1.0 * f.denominator) / f.numerator);
}

/**
Function Name : print_frmival
Function Description : Prints the Frame intervals
Parameter : v4l2_frmivalenum structure and prefix
Return : void
**/
void print_frmival(const struct v4l2_frmivalenum frmival, const char *prefix)
{
	printf("%s\tInterval: %s ", prefix, frmtype2s(frmival.type));
	if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) 
	{
		fract2sec(frmival.discrete);
		fract2fps(frmival.discrete);
		printf("\n");
	} 
	else if (frmival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) 
	{
		fract2sec(frmival.stepwise.min);	printf("- ");
		fract2sec(frmival.stepwise.max);	printf(" (");
		fract2fps(frmival.stepwise.max);
		fract2fps(frmival.stepwise.min);	printf(")\n");
	}
	else if (frmival.type == V4L2_FRMIVAL_TYPE_STEPWISE) 
	{
		fract2sec(frmival.stepwise.min);	printf("- ");
		fract2sec(frmival.stepwise.max);	printf(" with step ");
		fract2sec(frmival.stepwise.step);	printf(" (");
		fract2fps(frmival.stepwise.max);
		fract2fps(frmival.stepwise.min);	printf(")\n");
	}
}

/**
Function Name : listFormats
Function Description : Prints the available formats
Parameter : void
Return : int
**/
int listFormats(void)
{

	struct v4l2_capability cap;
	
	if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) 
 	{
            if (EINVAL == errno) 
            {
                    fprintf(stderr, "%s is no V4L2 device\\n",
                             dev_path);
                    exit(EXIT_FAILURE);
            } 
            else 
            {
                    errno_exit("VIDIOC_QUERYCAP");
            }
 	}
 
 	if(!cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
 	{
 		printf("\t\tVideo Capture is not supported\n");
 		return -1;
 		
 	}
	struct v4l2_fmtdesc fmt;
	struct v4l2_frmsizeenum frmsize;
	struct v4l2_frmivalenum frmival;
	
	fmt.index = 0;
	fmt.type = type;
	
	while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
		printf("\tIndex       : %d\n", fmt.index);
		printf("\tType        : ");	bufferTypeToString(fmt.type); printf("\n");
		printf("\tPixel Format: "); fcc2s(fmt.pixelformat);
		if (fmt.flags)
		{
			if(fmt.flags == V4L2_FMT_FLAG_COMPRESSED)
				printf(" (Compressed)");
			else
				printf(" (Emulated)");
		}
		printf("\n");
		printf("\tName        : %s\n", fmt.description);
		frmsize.pixel_format = fmt.pixelformat;
		frmsize.index = 0;
		while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
			print_frmsize(frmsize, "\t");
			if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
				frmival.index = 0;
				frmival.pixel_format = fmt.pixelformat;
				frmival.width = frmsize.discrete.width;
				frmival.height = frmsize.discrete.height;
				while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0) {
					print_frmival(frmival, "\t\t");
					frmival.index++;
				}
			}
			frmsize.index++;
		}
		printf("\n");
		fmt.index++;
}
	return 0;
	
}

/**
Function Name : enumerateMenu
Function Description : Enumerates and Prints the available menus in the control
Parameter : v4l2_queryctrl structure
Return : void
**/
void enumerateMenu(struct v4l2_queryctrl queryctrl)
{
	struct v4l2_querymenu querymenu;
	CLEAR(querymenu);
	querymenu.id = queryctrl.id;
	
	printf("\n");
	for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++) 
    {
        if (0 != ioctl(fd, VIDIOC_QUERYMENU, &querymenu)) 
        	continue;
        if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				printf("\t\t\t\t%d: %s\n", querymenu.index, querymenu.name);
		else
				printf("\t\t\t\t%d: %lld (0x%llx)\n", querymenu.index, querymenu.value, querymenu.value);
    }
}

/**
Function Name : listControls
Function Description : Prints the available controls
Parameter : void
Return : void
**/
void listControls(void)
{
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control;
	
	CLEAR(queryctrl);
	
	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) 
	{
    	if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) 
    	{
    		printf("\n");
        	switch (queryctrl.type) 
        	{
        		case V4L2_CTRL_TYPE_INTEGER:
					printf("%25s %#8.8x (int)    : min=%d max=%d step=%d default=%d",
					queryctrl.name, queryctrl.id,
					queryctrl.minimum, queryctrl.maximum,
					queryctrl.step, queryctrl.default_value);
				break;
				
				case V4L2_CTRL_TYPE_INTEGER64:
					printf("%25s %#8.8x (int64)  : min=%d max=%d step=%d default=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.step, queryctrl.default_value);
					break;
				case V4L2_CTRL_TYPE_STRING:
					printf("%25s %#8.8x (str)    : min=%d max=%d step=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.step);
					break;
				case V4L2_CTRL_TYPE_BOOLEAN:
					printf("%25s %#8.8x (bool)   : default=%d",
							queryctrl.name, queryctrl.id, queryctrl.default_value);
					break;
				case V4L2_CTRL_TYPE_MENU:
					printf("%25s %#8.8x (menu)   : min=%d max=%d default=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.default_value);
					enumerateMenu(queryctrl);
					break;
				case V4L2_CTRL_TYPE_INTEGER_MENU:
					printf("%25s %#8.8x (intmenu): min=%d max=%d default=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.default_value);
					enumerateMenu(queryctrl);
					break;
				case V4L2_CTRL_TYPE_BUTTON:
					printf("%25s %#8.8x (button) :", queryctrl.name, queryctrl.id);
					break;
				case V4L2_CTRL_TYPE_BITMASK:
					printf("%25s %#8.8x (bitmask): max=0x%08x default=0x%08x",
							queryctrl.name, queryctrl.id, queryctrl.maximum,
							queryctrl.default_value);
					break;
				case V4L2_CTRL_TYPE_U8:
					printf("%25s %#8.8x (u8)     : min=%d max=%d step=%d default=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.step, queryctrl.default_value);
					break;
				case V4L2_CTRL_TYPE_U16:
					printf("%25s %#8.8x (u16)    : min=%d max=%d step=%d default=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.step, queryctrl.default_value);
					break;
				case V4L2_CTRL_TYPE_U32:
					printf("%25s %#8.8x (u32)    : min=%d max=%d step=%d default=%d",
							queryctrl.name, queryctrl.id,
							queryctrl.minimum, queryctrl.maximum,
							queryctrl.step, queryctrl.default_value);
					break;
				default:
					printf("%25s %#8.8x (unknown): type=%x",
							queryctrl.name, queryctrl.id, queryctrl.type);
					break;

							
        	}
        	
        	CLEAR(control);
			control.id = queryctrl.id;

			if (0 == ioctl(fd, VIDIOC_G_CTRL, &control)) 
			{
				printf(" value=%d", control.value);
			}
			
        }
        
        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
     }
     printf("\n");
}

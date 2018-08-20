#include "v4l2.h"
#include "main.h"
#include "v4l2_ctrl.h"
#include "capture.h"



/**
Function Name : main
Function Description : Get the inputs from command line arguments, do the conversions, write and free the memory
Parameter : argc and argv
Return : 0 for success -1 for failure 1 for bad arguments
**/
int main(int argc, char **argv)
{
	
	char c;
    int optidx = 0;

	 struct option longopt[] = {
	 		{"version"		,0,NULL, 0 },
		    {"device-path"	,1,NULL,'d'},
		    {"device-info"	,0,NULL,'D'},
		    {"list-ctrls"	,0,NULL,'c'},
		    {"list-formats"	,0,NULL,'f'},
		    {"help"			,0,NULL,'h'},
		    {"frame-count"	,1,NULL,'C'},
		    {"mmap"			,0,NULL,'m'},
		    {"user-ptr"		,0,NULL,'u'},
		    {"read"			,0,NULL,'r'},
		    {"pix-format"	,1,NULL,'F'},
		    {"width"		,1,NULL,'w'},
		    {"height"		,1,NULL,'v'},
			{"outfile"		,1,NULL,'o'},
			{"stream"		,0,NULL,'s'},
		    {0,0,0,0}
	};
	
	
	while ((c=getopt_long(argc,argv,"d:C:w:v:F:o:fhDcmurs",longopt,&optidx)) != -1)
    {
        switch ( c )
        {
        	case 0:
		        printf("VERSION %s", VERSION);
		        goto CLOSE_AND_EXIT;
            
            case 'd':
                dev_path = strdup( optarg );
                openDevice(dev_path);
                break;
            case 'D':
				openDevice(dev_path);
                deviceInfo();
                break;
            case 'w':
                width = strtol( optarg, NULL, 10 );
                break;
            case 'v':
                height = strtol( optarg, NULL, 10 );
                break;
            case 'C':
                frame_count = strtol( optarg, NULL, 10 );
                capture = 1;
                streaming = 0;
                break;
            case 'o':
                outfile = strdup( optarg );
                break;
            case 'F':
				pix_format_str = strdup( optarg );
				if(pixStr2pixU32(pix_format_str) != 0)
					goto CLOSE_AND_EXIT;
                break;
			case 'f':
				openDevice(dev_path);
				listFormats();
				goto CLOSE_AND_EXIT;
			case 'c':
				openDevice(dev_path);
				listControls();
				goto CLOSE_AND_EXIT;
			case 'h':
				usage(stdout, argv[0]);
				goto CLOSE_AND_EXIT;
				
			case 'm':
				io = IO_METHOD_MMAP;
				break;
			case 'u':
				io = IO_METHOD_USERPTR;
				break;
			case 'r':
				io = IO_METHOD_READ;
				break;
			case 's':
				streaming = 1;
				capture = 0;
				break;
            default:
                printf("bad arg\n");
				usage(stderr, argv[0]);
				goto CLOSE_AND_EXIT;
                
        }
	}
	
	if(capture)
	{
		openDevice(dev_path);
		init_device();
        start_capturing();
        mainloop();
        stop_capturing();
        uninit_device();
	}
	
	if(streaming)
	{
		openDevice(dev_path);
		init_device();
        start_capturing();
        mainstreamloop();
        stop_capturing();
        uninit_device();
	}
	
CLOSE_AND_EXIT:
	close_device();
	printf("\n");
	return 0;
}

/**
Function Name : usage
Function Description : Displays helpful commands
Parameter : File pointer and name of the file name
Return : void
**/
void usage( FILE *fp, char * name )
{
	fprintf(fp,
                 "\nUsage: %s [options]\n"
                 "Options:\n"
                 "     --version       Displays version of the application\n"
                 "-d | --device-path   Video device path [%s]\n"
                 "-D | --device-info   Displays device info\n"
                 "-c | --list-ctrls    Displays all controls and their values\n"
                 "-f | --list-formats  Display all supported formats\n"
                 "-h | --help          Print this message\n"
                 "-C | --frame-count   Number of frames to be captured [%i] and to denote capture the frame***\n"
                 "-m | --mmap          Use memory mapped buffers [default]\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --user-ptr      Use application allocated buffers\n"
                 "-F | --pix-format    Pixel format to select format[default=YUYV]\n"
                 "-o | --outfile       Output file name\n"
                 "-w | --width         Width of output image[Default=640]\n"
                 "-v | --heigth        Height of output image[Default=480]\n"
                 "-s | --stream        Stream output"
                 "",
                 name, dev_path, frame_count);
}

/**
Function Name : pixStr2pixU32
Function Description : Converts string pixel format to unsigned int format and checks validation
Parameter : Pixel format string
Return : int
**/
int pixStr2pixU32(char* pix_format_str)
{
	if(strlen(pix_format_str) != 4 && strlen(pix_format_str) != 3)
		return -1;
		
	pix_format = (__u32)(*(pix_format_str)) | ((__u32)(*(pix_format_str + 1)) << 8) | ((__u32)(*(pix_format_str + 2)) << 16);
		
	if(strlen(pix_format_str) == 3)
		pix_format |= (__u32)(' ') << 24;
	else
		pix_format |= (__u32)(*(pix_format_str + 3)) << 24;
		
	return 0;
}

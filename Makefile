#declare the variable
cc=gcc
CFLAGS=-c

LDFLAGS = -lSDL2 -lpthread
INC_DIR = $(shell pkg-config --cflags sdl2)



all:main


main: 		v4l2_ctrl.o capture.o stream.o main.o
		$(cc) $^  $(LDFLAGS) $(INC_DIR) -o v4l2_app

v4l2_ctrl.o:	v4l2_ctrl.c
		$(cc) $(CFLAGS) v4l2_ctrl.c

capture.o:	capture.c
		$(cc) $(CFLAGS) capture.c

stream.o:	stream.c
		$(cc) $(CFLAGS) stream.c
		
main.o:		main.c
		$(cc) $(CFLAGS) main.c	
		
clean:	
	rm -rf *o conversion

clean_image:
	rm -rf *YUYV *jpg *mpg

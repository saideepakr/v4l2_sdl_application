#include "stream.h"

/**
Function Name : v4l2_streaming
Function Description : Create surface, renderer, texture and infinitly read the frame till exit
Parameter : void
Return : void
**/
void *v4l2_streaming() {
	// SDL2 begins
	CLEAR(sdlRect);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
	printf("Could not initialize SDL - %s\n", SDL_GetError());
	return NULL;
	}

	sdlScreen = SDL_CreateWindow("Simple YUV Window", SDL_WINDOWPOS_UNDEFINED,
		                       SDL_WINDOWPOS_UNDEFINED, width,
		                       height, SDL_WINDOW_SHOWN);

	if (!sdlScreen) {
	fprintf(stderr, "SDL: could not create window - exiting:%s\n",
		    SDL_GetError());
	return NULL;
	}

	sdlRenderer = SDL_CreateRenderer(
	  sdlScreen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (sdlRenderer == NULL) {
	fprintf(stderr, "SDL_CreateRenderer Error\n");
	return NULL;
	}
	sdlTexture =
	  SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_YUY2,
		                SDL_TEXTUREACCESS_STREAMING, width, height);
	sdlRect.w = width;
	sdlRect.h = height;
	
	gettimeofday(&start_time, NULL);
	while (!thread_exit_sig) 
	{
		read_frame();
		
		gettimeofday(&end_time, NULL);

		elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;      // sec to ms
		elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;   // us to ms

		printf("\r%.2lf - %.1lf", elapsed_time, 1000/elapsed_time);
		fflush(stdout);
		start_time = end_time;
	}
	
	return NULL;
	
}

/**
Function Name : frame_handler
Function Description : Create surface, renderer, texture and infinitly read the frame till exit
Parameter : Start address and length of the frame
Return : void
**/
void frame_handler(void *pframe, int length) 
{
	SDL_UpdateTexture(sdlTexture, &sdlRect, pframe, width * 2);
	//  SDL_UpdateYUVTexture
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
	SDL_RenderPresent(sdlRenderer);
}

/**
Function Name : mainstreamloop
Function Description : Main loop to create pthread and stream the frame
Parameter : void
Return : void
**/
void mainstreamloop()
{
	if (pthread_create(&thread_stream, NULL, v4l2_streaming, NULL))
	{
		fprintf(stderr, "create thread failed\n");
		return;
  	}

	int quit = 0;
	SDL_Event e;
	while (!quit) 
	{
		while (SDL_PollEvent(&e)) 
		{
			if (e.type == SDL_QUIT) { // click close icon then quit
				quit = 1;
		}
		if (e.type == SDL_KEYDOWN) 
		{
			if (e.key.keysym.sym == SDLK_ESCAPE) // press ESC the quit
				quit = 1;
		}
	}
	usleep(25);
	}

	thread_exit_sig = 1;               // exit thread_stream
	pthread_join(thread_stream, NULL); // wait for thread_stream exiting
	SDL_Quit();
}

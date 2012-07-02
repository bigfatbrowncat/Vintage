#include <SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <unistd.h>

#include "SDLConsole.h"
#include "HardwareDevice.h"
//#include "font2.h"

wchar_t* SDLScreen::encoding = L" "
					            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					            "abcdefghijklmnopqrstuvwxyz"
					            "0123456789.,:;!?@#$%^&*()[]{}_-+<>=~\"'`/\\|";

wchar_t* SDLScreen::cursor_encoding = L"0123456789";


bool SDLTerminal::handleSpecialKeyDown(SDL_keysym* keysym)
{
	if ((keysym->mod & (KMOD_LCTRL | KMOD_RCTRL)) && (keysym->sym >= SDLK_F1 && keysym->sym <= SDLK_F12))
	{
		setActiveScreen(keysym->sym - SDLK_F1);
		return true;
	}
	else
	{
		return false;
	}

}

bool SDLTerminal::handleSpecialKeyUp(SDL_keysym* keysym)
{
	if (keysym->sym >= SDLK_F1 && keysym->sym <= SDLK_F12)
	{
		// Do nothing
		return true;
	}
	else
	{
		return false;
	}
}

void SDLTerminal::process_events()
{
	/* Our SDL event placeholder. */
	SDL_Event event;
	KeyModifiers modifiers = KEYMOD_NONE;

	/* Grab all the events off the queue. */
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			if (handleSpecialKeyDown(&event.key.keysym))
			{
				// Do nothing
			}
			else //if (state == TS_CPU)
			{
				if (event.key.keysym.mod & KMOD_LSHIFT) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_LSHIFT);
				if (event.key.keysym.mod & KMOD_RSHIFT) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_RSHIFT);
				if (event.key.keysym.mod & KMOD_LCTRL) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_LCTRL);
				if (event.key.keysym.mod & KMOD_RCTRL) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_RCTRL);
				if (event.key.keysym.mod & KMOD_LALT) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_LALT);
				if (event.key.keysym.mod & KMOD_RALT) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_RALT);
				if (event.key.keysym.mod & KMOD_LMETA) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_LMETA);
				if (event.key.keysym.mod & KMOD_RMETA) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_RMETA);
				if (event.key.keysym.mod & KMOD_NUM) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_NUM);
				if (event.key.keysym.mod & KMOD_CAPS) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_CAPS);
				if (event.key.keysym.mod & KMOD_MODE) modifiers = (KeyModifiers)((int)modifiers | (int) KEYMOD_MODE);
				if (screens[activeScreen]->getKeyboardController() != NULL)
				{
					//printf("key");

					screens[activeScreen]->getKeyboardController()->ChangeKeyState(true, modifiers, event.key.keysym.sym);
				}
			}
			break;
		case SDL_KEYUP:
			if (handleSpecialKeyUp(&event.key.keysym))
			{
				// Do nothing
			}
			else //if (state == TS_CPU)
			{
				if (event.key.keysym.mod & KMOD_LSHIFT) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_LSHIFT));
				if (event.key.keysym.mod & KMOD_RSHIFT) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_RSHIFT));
				if (event.key.keysym.mod & KMOD_LCTRL) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_LCTRL));
				if (event.key.keysym.mod & KMOD_RCTRL) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_RCTRL));
				if (event.key.keysym.mod & KMOD_LALT) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_LALT));
				if (event.key.keysym.mod & KMOD_RALT) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_RALT));
				if (event.key.keysym.mod & KMOD_LMETA) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_LMETA));
				if (event.key.keysym.mod & KMOD_RMETA) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_RMETA));
				if (event.key.keysym.mod & KMOD_NUM) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_NUM));
				if (event.key.keysym.mod & KMOD_CAPS) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_CAPS));
				if (event.key.keysym.mod & KMOD_MODE) modifiers = (KeyModifiers)((int)modifiers & ~((int) KEYMOD_MODE));
				if (screens[activeScreen]->getKeyboardController() != NULL)
				{
					screens[activeScreen]->getKeyboardController()->ChangeKeyState(false, modifiers, event.key.keysym.sym);
				}
			}
			break;
		case SDL_QUIT:
			/* Handle quit requests (like Ctrl-c). */
			quit_pending = true;
			break;
		}
	}
}

char *file2string(const char *path)
{
	FILE *fd;
	long len,
		 r;
	char *str;

	if (!(fd = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);

	printf("File '%s' is %ld long\n", path, len);

	fseek(fd, 0, SEEK_SET);

	if (!(str = (char*)malloc(len * sizeof(char))))
	{
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}

	r = fread(str, sizeof(char), len, fd);

	str[r - 1] = '\0'; /* Shader sources have to term with null */

	fclose(fd);

	return str;
}

void printLog(GLuint obj)
{
    int infologLength = 0;
    char infoLog[1024];

	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);

    if (infologLength > 0)
		printf("%s\n", infoLog);
}

GLuint vs, /* Vertex Shader */
	   fs, /* Fragment Shader */
	   sp; /* Shader Program */

bool InitSDL_GL(int windowWidth, int windowHeight)
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, windowWidth, windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 1, 0);

	glMatrixMode(GL_MODELVIEW);

	printf("Initializing glew\n");
	glewInit();
	if (GLEW_VERSION_2_0)
		fprintf(stderr, "INFO: OpenGL 2.0 supported, proceeding\n");
	else
	{
		fprintf(stderr, "INFO: OpenGL 2.0 not supported. Exit\n");
		return false;
	}
	printf("Success.\n");

	/* The vertex shader */
	GLchar *vsSource = (GLchar*)file2string("wave.vert");
	GLchar *fsSource = (GLchar*)file2string("wave.frag");

	const GLchar *cvsSource = vsSource;
	const GLchar *cfsSource = fsSource;

	/* Compile and load the program */

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &cvsSource, NULL);
	glCompileShader(vs);
	printLog(vs);

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &cfsSource, NULL);
	glCompileShader(fs);
	printLog(fs);

	free(vsSource);
	free(fsSource);

	sp = glCreateProgram();
	glAttachShader(sp, vs);
	glAttachShader(sp, fs);
	glLinkProgram(sp);
	printLog(sp);

	glUseProgram(sp);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_2D);

}

SDLTerminal::SDLTerminal(CachedFont& font, CachedFont& curFont):
		frame_buffer_width(100),
		frame_buffer_height(40),
		font(font),
		curFont(curFont),
		activeScreen(0),
		activeScreenChanged(true)

{
	fps = 100;
	frame = 0;
	quit_pending = false;
	window_frame = 4;
	cursorIsOn = true;
	cursor_symbol = '2';
	cursor_x = 0; cursor_y = 0;
	cursor_blink_rate = 100;


	for (int i = 0; i < SCREENS_COUNT; i++)
	{
		screens[i] = new SDLScreen(frame_buffer_width, frame_buffer_height);
	}
	screens[activeScreen]->setActivity(true);

    printf("Initializing SDL.\n");


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1)
    {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }

	int distance = 0;

    printf("SDL initialized.\n");
}

SDLTerminal::~SDLTerminal()
{
	SDL_FreeSurface(slow_surface);
	SDL_Quit();
}

void setGLTextureFromSurface(SDL_Surface* surface, GLuint sp)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	int texture_format;
	int nOfColors = surface->format->BytesPerPixel;
	if (nOfColors == 4)     // contains an alpha channel
	{
	    if (surface->format->Rmask == 0x000000ff)
	        texture_format = GL_RGBA;
	    else
	                texture_format = GL_BGRA;
	} else if (nOfColors == 3)     // no alpha channel
	{
	    if (surface->format->Rmask == 0x000000ff)
	        texture_format = GL_RGB;
	    else
	        texture_format = GL_BGR;
	} else {
	    printf("warning: the image is not truecolor..  this will probably break\n");
	    // this error should not go unhandled
	}

	// Have OpenGL generate a texture object handle for us
	unsigned int texture;
	glGenTextures( 0, &texture );

	// Bind the texture object
	glActiveTexture(GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_2D, texture );

	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
	             texture_format, GL_UNSIGNED_BYTE, surface->pixels);


	GLint RTScene = glGetUniformLocation(sp, "RTScene");
	GLint phase = glGetUniformLocation(sp, "phase");
//	printf("phase is at %d\n", phase);
	glUniform1i(RTScene, 0);	//use texture bound to GL_TEXTURE0
	glUniform1f(phase, (float)SDL_GetTicks() / 1000);
}

void SDLTerminal::Run()
{
	Uint32 starttime = SDL_GetTicks();

	int distance = 0;
    mainSurface = SDL_SetVideoMode(
    		(font.getLetterWidth() + distance) * frame_buffer_width + 2 * window_frame,
    		(font.getLetterHeight() + distance) * frame_buffer_height + 2 * window_frame, 24, SDL_OPENGL);

    if (mainSurface == NULL)
    {
        fprintf(stderr, "Couldn't set the video mode: %s\n",
                        SDL_GetError());
        throw 1;
    }

    InitSDL_GL((font.getLetterWidth() + distance) * frame_buffer_width + 2 * window_frame,
    		(font.getLetterHeight() + distance) * frame_buffer_height + 2 * window_frame);

    slow_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
    		(font.getLetterWidth() + distance) * frame_buffer_width + 2 * window_frame,
    		(font.getLetterHeight() + distance) * frame_buffer_height + 2 * window_frame, 24, 0, 0, 0, 0);

    if(slow_surface == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        throw 2;
    }

    quit_pending = false;
    printf("Starting UI event loop.\n");
    while( !quit_pending )
    {
    	if ((SDL_GetTicks() - starttime) / cursor_blink_rate > 0)
    	{
    		starttime = SDL_GetTicks();
        	cursorIsOn = !cursorIsOn;
    	}
        /* Process incoming events. */
        process_events();

        if (!handleCustomEvents())
        {
        	quit_pending = true;
        }

		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

    	draw();
		setGLTextureFromSurface(slow_surface, sp);

		glBegin(GL_QUADS);

		glTexCoord2f(0, 0);
		glVertex2f(0, 0);

		glTexCoord2f(1, 0);
		glVertex2f(1, 0);

		glTexCoord2f(1, 1);
		glVertex2f(1, 1);

		glTexCoord2f(0, 1);
		glVertex2f(0, 1);

		glEnd();

		SDL_GL_SwapBuffers();

        /* Draw the screen. */
    }
	SDL_FreeSurface(slow_surface);

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(sp);
}

void SDLTerminal::draw()
{
	if ( SDL_MUSTLOCK(slow_surface) ) {
		if ( SDL_LockSurface(slow_surface) < 0 )
		{
			fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
			throw 1;
		}
	}

	//if (screens[activeScreen]->getFrameBufferModified() || activeScreenChanged)
	{
		screens[activeScreen]->draw_framebuffer(font, window_frame, window_frame, slow_surface);
	}

	if (activeScreenChanged)
	{
		updateCaption();
		activeScreenChanged = false;
	}

	//SDL_BlitSurface(slow_surface, NULL, mainSurface, NULL);

	if (cursorIsOn)
	{
		screens[activeScreen]->drawCursor(curFont, window_frame, window_frame, slow_surface);
	}

	float p = (float)(rand()) / RAND_MAX;
	float sliding = 0;
	if (p > 0.97)
	{
		sliding = 0.7 * (float)(rand()) / RAND_MAX;
	}

	float blur = 0.7 + 0.15 * sin(0.123 * frame);
	//cinescope_sim(mainSurface, 0.15, sliding, blur);

	if ( SDL_MUSTLOCK(slow_surface) ) {
		SDL_UnlockSurface(slow_surface);
	}

	//SDL_UpdateRect(mainSurface, 0, 0, mainSurface->w, mainSurface->h);
	frame++;
}


void SDLTerminal::cinescope_sim(SDL_Surface *surface, float interlacing, float sliding, float blur)
{
	float a = sliding;

	Uint8* spx = (Uint8 *)surface->pixels;
	Uint16 spt = surface->pitch;

	float inter_phase = (float)frame / 30;
	float inter_cos = cos(inter_phase)*cos(inter_phase);
	float inter_sin = sin(inter_phase)*sin(inter_phase);

	for (int i = surface->w - 2; i >= 1; i--)
	for (int j = surface->h - 2; j >= 1; j--)
	{
		float deint_line = j % 2 == 0 ? 1 - inter_cos * interlacing : 1 - inter_sin * interlacing;

		Uint8* pij = spx + j * spt + i * 3;
		Uint8* pimj = spx + j * spt + (i - 1) * 3;
		Uint8* pijm = spx + (j - 1) * spt + i * 3;
		Uint8* pipj = spx + j * spt + (i + 1) * 3;
		Uint8* pijp = spx + (j + 1) * spt + i * 3;

		pij[0] = (Uint8)((float)pij[0] * (1 - a) * deint_line + pipj[0] * a * deint_line);
		pij[1] = (Uint8)((float)pij[1] * (1 - a) * deint_line + pipj[1] * a * deint_line);
		pij[2] = (Uint8)((float)pij[2] * (1 - a) * deint_line + pipj[2] * a * deint_line);

		/*Uint8 pij0n = (Uint8)(((float)pij[0] + blur * (float)(pimj[0] + pipj[0] + pijm[0] + pijp[0])) / (1 + 4 * blur));
		Uint8 pij1n = (Uint8)(((float)pij[1] + blur * (float)(pimj[1] + pipj[1] + pijm[1] + pijp[1])) / (1 + 4 * blur));
		Uint8 pij2n = (Uint8)(((float)pij[2] + blur * (float)(pimj[2] + pipj[2] + pijm[2] + pijp[2])) / (1 + 4 * blur));

		if (pij0n + pij1n + pij2n > pij[0] + pij[1] + pij[2])
		{
			pij[0] = pij0n;
			pij[1] = pij1n;
			pij[2] = pij2n;
		}*/
	}
	frame++;
}


#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <unistd.h>

#include "SDLTerminal.h"
#include "HardwareDevice.h"
#include "KeyModifiers.h"

wchar_t* SDLScreen::fontEncoding = L" "
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789.,:;!?@#$%^&*()[]{}_-+<>=~\"'`/\\|";

wchar_t* SDLScreen::cursorEncoding = L"0123456789";

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

void SDLTerminal::processEvents()
{
	/* Our SDL event placeholder. */
	SDL_Event event;
	//KeyModifiers modifiers = KEYMOD_NONE;

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
			else
			{
				if (event.key.keysym.sym != 0)
				{
//					printf("-> %d %d\n", event.key.keysym.sym, true);
					if (screens[activeScreen]->getKeyboardController() != NULL)
					{
						screens[activeScreen]->getKeyboardController()->ChangeKeyState(true, event.key.keysym.sym);
					}
				}
			}
			break;
		case SDL_KEYUP:
			if (handleSpecialKeyUp(&event.key.keysym))
			{
				// Do nothing
			}
			else
			{
				if (event.key.keysym.sym != 0)
				{
//					printf("-> %d %d\n", event.key.keysym.sym, false);
					if (screens[activeScreen]->getKeyboardController() != NULL)
					{
						screens[activeScreen]->getKeyboardController()->ChangeKeyState(false, event.key.keysym.sym);
					}
				}
			}
			break;
		case SDL_QUIT:
			/* Handle quit requests (like Ctrl-c). */
			quitPending = true;
			break;
		}
	}
}

SDLTerminal::SDLTerminal(CachedFont& font, CachedFont& cursorFont):
		frameBufferWidth(128),
		frameBufferHeight(56),
		font(font),
		cursorFont(cursorFont),
		activeScreen(0),
		activeScreenChanged(true)

{
	frame = 0;
	quitPending = false;
	windowFrame = 4;
	cursorIsOn = true;
	cursorSymbol = '2';
	cursorX = 0; cursorY = 0;
	cursorBlinkRate = 100;
	frameRate = 20;

	for (int i = 0; i < SCREENS_COUNT; i++)
	{
		screens[i] = new SDLScreen(frameBufferWidth, frameBufferHeight);
	}
	screens[activeScreen]->setActivity(true);
}

SDLTerminal::~SDLTerminal()
{
	SDL_Quit();
}

bool SDLTerminal::setGLTextureFromSurface(SDL_Surface* surface, GLuint shaderProgramId)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	int textureFormat;
	int nOfColors = surface->format->BytesPerPixel;
	if (nOfColors == 4)		// contains an alpha channel
	{
	    if (surface->format->Rmask == 0x000000ff)
	    {
	    	textureFormat = GL_RGBA;
	    }
	    else
	    {
	    	textureFormat = GL_BGRA;
	    }
	}
	else if (nOfColors == 3)     // no alpha channel
	{
	    if (surface->format->Rmask == 0x000000ff)
	        textureFormat = GL_RGB;
	    else
	        textureFormat = GL_BGR;
	}
	else
	{
	    fprintf(stderr, "The image isn't in true color\n");
	    return false;
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
	             textureFormat, GL_UNSIGNED_BYTE, surface->pixels);


	glUniform1i(texture, 0);	//use texture bound to GL_TEXTURE0
	glUniform1f(phase, (float)(SDL_GetTicks()) / 1000 - phase_start_value);
	return true;
}

bool SDLTerminal::Run()
{
    printf("Initializing SDL.\n");


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1)
    {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }

    printf("SDL initialized.\n");

	int distance = 0;

	SDL_Surface* mainSurface = SDL_SetVideoMode(
    		(font.getLetterWidth() + distance) * frameBufferWidth + 2 * windowFrame,
    		(font.getLetterHeight() + distance) * frameBufferHeight + 2 * windowFrame, 24, SDL_OPENGL);

    if (mainSurface == NULL)
    {
        fprintf(stderr, "Couldn't set the video mode: %s\n",
                        SDL_GetError());
        throw 1;
    }

    printf("Initializing GLEW\n");
	glewInit();
	if (GLEW_VERSION_2_0)
	{
		printf("INFO: OpenGL 2.0 supported, proceeding\n");
	}
	else
	{
		fprintf(stderr, "INFO: OpenGL 2.0 not supported. Exit\n");
		return false;
	}
	printf("GLEW initialized.\n");


	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, (font.getLetterWidth() + distance) * frameBufferWidth + 2 * windowFrame,
    		(font.getLetterHeight() + distance) * frameBufferHeight + 2 * windowFrame);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 1, 0);

	glMatrixMode(GL_MODELVIEW);

	// Reading shaders from files
	GLchar *vsSource = (GLchar*)file2string("res/term.vert");
	GLchar *fsSource = (GLchar*)file2string("res/term.frag");

	const GLchar *cvsSource = vsSource;
	const GLchar *cfsSource = fsSource;

	// Compiling them
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &cvsSource, NULL);
	glCompileShader(vertexShader);
	printLog(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &cfsSource, NULL);
	glCompileShader(fragmentShader);
	printLog(fragmentShader);

	free(vsSource);
	free(fsSource);

	// Attaching shaders and loading them into the videocard
	shaderProgramId = glCreateProgram();
	glAttachShader(shaderProgramId, vertexShader);
	glAttachShader(shaderProgramId, fragmentShader);
	glLinkProgram(shaderProgramId);
	printLog(shaderProgramId);

	glUseProgram(shaderProgramId);

	texture = glGetUniformLocation(shaderProgramId, "texture");
	phase = glGetUniformLocation(shaderProgramId, "phase");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	SDL_Surface* frameSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
    		(font.getLetterWidth() + distance) * frameBufferWidth + 2 * windowFrame,
    		(font.getLetterHeight() + distance) * frameBufferHeight + 2 * windowFrame, 24, 0, 0, 0, 0);

    if (frameSurface == NULL)
    {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        throw 2;
    }

    quitPending = false;
	Uint32 previousCursorBlinkTime = SDL_GetTicks();
	Uint32 previousFrameTime = SDL_GetTicks();
    printf("Starting UI event loop.\n");
    while( !quitPending )
    {
    	if ((SDL_GetTicks() - previousCursorBlinkTime) / cursorBlinkRate > 0)
    	{
    		previousCursorBlinkTime = SDL_GetTicks();
        	cursorIsOn = !cursorIsOn;
    	}

        /* Process incoming events. */
        processEvents();

        if (!handleCustomEvents())
        {
        	quitPending = true;
        }

    	if ((SDL_GetTicks() - previousFrameTime) / frameRate > 0)
    	{
    		previousFrameTime = SDL_GetTicks();
        	draw(frameSurface);
    	}

		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		setGLTextureFromSurface(frameSurface, shaderProgramId);

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
    }
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(shaderProgramId);

	SDL_FreeSurface(frameSurface);
	return true;
}

void SDLTerminal::draw(SDL_Surface* frameSurface)
{
	if ( SDL_MUSTLOCK(frameSurface) ) {
		if ( SDL_LockSurface(frameSurface) < 0 )
		{
			fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
			throw 1;
		}
	}

	//if (screens[activeScreen]->getFrameBufferModified() || activeScreenChanged)
	{
		screens[activeScreen]->draw_framebuffer(font, windowFrame, windowFrame, frameSurface);
	}

	if (activeScreenChanged)
	{
		updateCaption();
		phase_start_value = (float)(SDL_GetTicks()) / 1000;
		activeScreenChanged = false;
	}

	//SDL_BlitSurface(slow_surface, NULL, mainSurface, NULL);

	if (cursorIsOn)
	{
		screens[activeScreen]->drawCursor(cursorFont, windowFrame, windowFrame, frameSurface);
	}

	if (SDL_MUSTLOCK(frameSurface))
	{
		SDL_UnlockSurface(frameSurface);
	}

	//SDL_UpdateRect(mainSurface, 0, 0, mainSurface->w, mainSurface->h);
	frame++;
}

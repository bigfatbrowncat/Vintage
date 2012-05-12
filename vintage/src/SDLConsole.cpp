#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <unistd.h>

#include "SDLConsole.h"
#include "HardwareDevice.h"
//#include "font2.h"

/*void addPixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a)
{
	assert(surface->format->BytesPerPixel == 3);
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 3;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int p0 = p[0] + (int)(r * a);
	int p1 = p[1] + (int)(g * a);
	int p2 = p[2] + (int)(b * a);
	p[0] = p0 < 255 ? p0 : 255;
	p[1] = p1 < 255 ? p1 : 255;
	p[2] = p2 < 255 ? p2 : 255;
#else
	int p2 = p[2] + (int)(r * a);
	int p1 = p[1] + (int)(g * a);
	int p0 = p[0] + (int)(b * a);
	p[2] = p2 < 255 ? p2 : 255;
	p[1] = p1 < 255 ? p1 : 255;
	p[0] = p0 < 255 ? p0 : 255;
#endif
}*/

wchar_t* SDLScreen::encoding = L" "
					            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					            "abcdefghijklmnopqrstuvwxyz"
					            "0123456789.,:;!?@#$%^&*()[]{}_-+<>=~\"'`/\\|";

wchar_t* SDLScreen::cursor_encoding = L"0123456789";


bool SDLTerminal::handleSpecialKeyDown(SDL_keysym* keysym)
{
	if (keysym->sym >= SDLK_F1 && keysym->sym <= SDLK_F12)
	{
		activeScreen = (keysym->sym - SDLK_F1);
		activeScreenChanged = true;
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
				if (screens[activeScreen]->getKeyboard() != NULL)
				{
					screens[activeScreen]->getKeyboard()->ChangeKeyState(true, modifiers, event.key.keysym.sym);
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
				if (screens[activeScreen]->getKeyboard() != NULL)
				{
					screens[activeScreen]->getKeyboard()->ChangeKeyState(false, modifiers, event.key.keysym.sym);
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

SDLTerminal::SDLTerminal(Keyboard& kbd, Font& font, Font& curFont):
		frame_buffer_width(96),
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

    printf("Initializing SDL.\n");

    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)==-1)) {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }

    printf("SDL initialized.\n");

    /* Clean up on exit */
//    atexit(SDL_Quit);


}

SDLTerminal::~SDLTerminal()
{
	SDL_FreeSurface(slow_surface);
	SDL_Quit();
}

void SDLTerminal::Run()
{
	Uint32 starttime = SDL_GetTicks();

	int distance = 0;
    mainSurface = SDL_SetVideoMode(
    		(font.getLetterWidth() + distance) * frame_buffer_width + 2 * window_frame,
    		(font.getLetterHeight() + distance) * frame_buffer_height + 2 * window_frame, 24, SDL_SWSURFACE);

    if ( mainSurface == NULL ) {
        fprintf(stderr, "Couldn't set the video mode: %s\n",
                        SDL_GetError());
        throw 1;
    }

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
        process_events( );
        /* Draw the screen. */
    	draw(mainSurface);
    }
}

void SDLTerminal::draw(SDL_Surface* surface)
{
	if ( SDL_MUSTLOCK(surface) ) {
		if ( SDL_LockSurface(surface) < 0 ) {
			fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
			throw 1;
		}
	}

	if (screens[activeScreen]->getFrameBufferModified() || activeScreenChanged)
	{
		screens[activeScreen]->draw_framebuffer(font, window_frame, window_frame, slow_surface);
	}

	if (activeScreenChanged)
	{
		updateCaption();
		activeScreenChanged = false;
	}

	SDL_BlitSurface(slow_surface, NULL, mainSurface, NULL);

	if (cursorIsOn)
	{
		screens[activeScreen]->drawCursor(curFont, window_frame, window_frame, mainSurface);
	}

	float p = (float)(rand()) / RAND_MAX;
	float sliding = 0;
	if (p > 0.97)
	{
		sliding = 0.7 * (float)(rand()) / RAND_MAX;
	}

	float blur = 0.7 + 0.15 * sin(0.123 * frame);
	cinescope_sim(mainSurface, 0.15, sliding, blur);

	if ( SDL_MUSTLOCK(surface) ) {
		SDL_UnlockSurface(surface);
	}

	SDL_UpdateRect(mainSurface, 0, 0, mainSurface->w, mainSurface->h);
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

		Uint8 pij0n = (Uint8)(((float)pij[0] + blur * (float)(pimj[0] + pipj[0] + pijm[0] + pijp[0])) / (1 + 4 * blur));
		Uint8 pij1n = (Uint8)(((float)pij[1] + blur * (float)(pimj[1] + pipj[1] + pijm[1] + pijp[1])) / (1 + 4 * blur));
		Uint8 pij2n = (Uint8)(((float)pij[2] + blur * (float)(pimj[2] + pipj[2] + pijm[2] + pijp[2])) / (1 + 4 * blur));

		if (pij0n + pij1n + pij2n > pij[0] + pij[1] + pij[2])
		{
			pij[0] = pij0n;
			pij[1] = pij1n;
			pij[2] = pij2n;
		}
	}
	frame++;
}


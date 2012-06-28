class SDLTerminal;

#ifndef SDLCONSOLE_H_
#define SDLCONSOLE_H_

#include <SDL.h>
#include <pthread.h>
#include <assert.h>
#include "SDLScreen.h"
#include "CPU.h"
#include "HardwareDevice.h"
#include "../../FontEditor/include/Font.h"

#define SCREENS_COUNT	12		// F1..F12

typedef bool CustomEventsHandler(void* data);

class SDLTerminal
{
private:
	int frame_buffer_width, frame_buffer_height;

    Font &font, &curFont;

	bool quit_pending;
	int fps;
	int frame;
    SDL_Surface* mainSurface;
    SDL_Surface* slow_surface;

    SDLScreen* screens[SCREENS_COUNT];
    int activeScreen;
    bool activeScreenChanged;

    int window_frame;
    int cursor_blink_rate;
    bool cursorIsOn;
    wchar_t cursor_symbol;
    int cursor_x, cursor_y;

    CustomEventsHandler* customEventsHandler;
    void* customEventsHandlerData;

	void process_events();
	bool handleSpecialKeyDown(SDL_keysym* keysym);
	bool handleSpecialKeyUp(SDL_keysym* keysym);

	void addpixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a);
	void putSymbol(SDL_Surface *surface, vector<bool*>::const_iterator iter, int sx, int sy, int x_left, int y_top, Uint8 r, Uint8 g, Uint8 b);
	void putChar(SDL_Surface *surface, Font& font, wchar_t ch, int sx, int sy, int x_left, int y_top, wchar_t* encoding,  Uint8 r, Uint8 g, Uint8 b);
	void draw_framebuffer(SDL_Surface* surface);
	void draw(SDL_Surface* surface);
	void cinescope_sim(SDL_Surface *surface, float interlacing, float sliding, float blur);
protected:
	void setActiveScreen(int index)
	{
		if (activeScreen >= 0 && activeScreen < SCREENS_COUNT)
		{
			screens[activeScreen]->setActivity(false);
		}
		activeScreen = index;
		if (activeScreen >= 0 && activeScreen < SCREENS_COUNT)
		{
			screens[activeScreen]->setActivity(true);
		}
		activeScreenChanged = true;

	}

	void updateCaption()
	{
	    char captionLong[100];
	    sprintf(captionLong, "Vintage Terminal - Screen %d", activeScreen + 1);
	    char captionShort[100];
	    sprintf(captionShort, "Vintage Terminal (%d)", activeScreen + 1);


	    SDL_WM_SetCaption(captionLong, captionShort);
	}

	// Should return true to continue working
	bool handleCustomEvents()
	{
		if (customEventsHandler != NULL)
		{
			return customEventsHandler(customEventsHandlerData);
		}
		return true;
	}

public:
	SDLTerminal(Font& font, Font& curFont);
	void setCustomEventsHandler(CustomEventsHandler* handler, void* data)
	{
		customEventsHandler = handler;
		customEventsHandlerData = data;
	}
	void Run();

	SDLScreen& getScreen(int index)
	{
		return *(screens[index]);
	}
	virtual ~SDLTerminal();

};


#endif /* SDLCONSOLE_H_ */

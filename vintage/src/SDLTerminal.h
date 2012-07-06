class SDLTerminal;

#ifndef SDLCONSOLE_H_
#define SDLCONSOLE_H_

#include <SDL.h>
#include <GL/glew.h>

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
	int frameBufferWidth, frameBufferHeight;

	CachedFont &font, &cursorFont;

	bool quitPending;
	int frame;

    SDLScreen* screens[SCREENS_COUNT];
    int activeScreen;
    bool activeScreenChanged;

    int windowFrame;
    int cursorBlinkRate;
    int frameRate;
    bool cursorIsOn;
    wchar_t cursorSymbol;
    int cursorX, cursorY;

    GLuint vertexShader, /* Vertex Shader */
    	   fragmentShader, /* Fragment Shader */
    	   shaderProgramId; /* Shader Program */
    volatile GLint texture, phase;
    float phase_start_value;

    CustomEventsHandler* customEventsHandler;
    void* customEventsHandlerData;

	void processEvents();
	bool handleSpecialKeyDown(SDL_keysym* keysym);
	bool handleSpecialKeyUp(SDL_keysym* keysym);

	void draw(SDL_Surface* frameSurface);
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
	bool setGLTextureFromSurface(SDL_Surface* surface, GLuint sp);

public:
	SDLTerminal(CachedFont& font, CachedFont& curFont);
	void setCustomEventsHandler(CustomEventsHandler* handler, void* data)
	{
		customEventsHandler = handler;
		customEventsHandlerData = data;
	}
	bool Run();

	SDLScreen& getScreen(int index)
	{
		return *(screens[index]);
	}
	virtual ~SDLTerminal();

};


#endif /* SDLCONSOLE_H_ */

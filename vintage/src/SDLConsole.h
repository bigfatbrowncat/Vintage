#ifndef SDLCONSOLE_H_
#define SDLCONSOLE_H_

#include <SDL.h>
#include <pthread.h>
#include <assert.h>

#include "CPU.h"
#include "HardwareDevice.h"
#include "../../FontEditor/include/Font.h"

#define SCREENS_COUNT	12		// F1..F12

class Keyboard;

struct SymbolPlace
{
	wchar_t code;
	Uint8 fore_r, fore_g, fore_b;
	Uint8 back_r, back_g, back_b;
};

class SDLBuffer
{
private:
	pthread_mutex_t drawingMutex;
	SymbolPlace* frameBuffer;
	SymbolPlace* frameBufferEditing;
	int frameBufferWidth, frameBufferHeight;
    bool frameBufferModified;
public:
    int getFrameBufferWidth() { return frameBufferWidth; }
    int getFrameBufferHeight() { return frameBufferHeight; }

    const SymbolPlace* lockFrameBufferReadOnly()
    {
    	pthread_mutex_lock(&drawingMutex);
    	return frameBuffer;
    }

    void unlockFrameBufferReadOnly()
    {
    	pthread_mutex_unlock(&drawingMutex);
    }

    SDLBuffer(int frameBufferWidth, int frameBufferHeight) :
    	frameBufferWidth(frameBufferWidth),
    	frameBufferHeight(frameBufferHeight),
    	frameBufferModified(true)
    {
    	frameBuffer = new SymbolPlace[frameBufferWidth * frameBufferHeight];
    	frameBufferEditing = new SymbolPlace[frameBufferWidth * frameBufferHeight];
    	pthread_mutex_init(&drawingMutex, NULL);
    }
    virtual ~SDLBuffer()
    {
    	pthread_mutex_destroy(&drawingMutex);
    	delete [] frameBuffer;
    	delete [] frameBufferEditing;
    }

    SymbolPlace* LockFrameBuffer()
    {
    	//pthread_mutex_lock(&printing_mutex);
    	pthread_mutex_lock(&drawingMutex);
    	memcpy(frameBufferEditing, frameBuffer, frameBufferWidth * frameBufferHeight * sizeof(SymbolPlace));
    	return frameBufferEditing;
    }

    void UnlockFrameBuffer()
    {
    	memcpy(frameBuffer, frameBufferEditing, frameBufferWidth * frameBufferHeight * sizeof(SymbolPlace));
    	frameBufferModified = true;
    	pthread_mutex_unlock(&drawingMutex);


    	//pthread_mutex_unlock(&printing_mutex);
    }

    bool getFrameBufferModified()
    {
    	return frameBufferModified;
    }

    void clearFrameBufferModified()
    {
    	frameBufferModified = false;
    }
};

class SDLScreen : public SDLBuffer
{
private:
	pthread_mutex_t printing_mutex;
    Uint32 selected_fore_color, selected_back_color;
    int cursor_x, cursor_y;

protected:
    static wchar_t* encoding;
    static wchar_t* cursor_encoding;


    void addPixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a)
	{
	//	if(x >= 0 && y >= 0 && x < surface->w && y < surface->h)
		{
			assert(surface->format->BytesPerPixel == 3);
			assert(x >= 0 && y >= 0 && x < surface->w && y < surface->h);

			Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * 3;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			p[0] = (int)(p[0] * (1 - a) + r * a);
			p[1] = (int)(p[1] * (1 - a) + g * a);
			p[2] = (int)(p[2] * (1 - a) + b * a);
	#else
			p[2] = (int)(p[2] * (1 - a) + r * a);
			p[1] = (int)(p[1] * (1 - a) + g * a);
			p[0] = (int)(p[0] * (1 - a) + b * a);
	#endif
		}
	}

	void putSymbol(SDL_Surface *surface, Font& font, vector<bool*>::const_iterator iter, int sx, int sy, int x_left, int y_top, Uint8 r, Uint8 g, Uint8 b)
	{
		// Drawing it
		for (int j = 0; j < font.getLetterHeight(); j++)
		for (int i = 0; i < font.getLetterWidth(); i++)
		{
			float v = 0;
			for (int p = 0; p < font.getOverSize(); p++)
			for (int q = 0; q < font.getOverSize(); q++)
			{
				int ii = i * font.getOverSize() + p;
				int jj = j * font.getOverSize() + q;

				if ((*iter)[jj * font.getLetterWidth() * font.getOverSize() + ii])
				{
					v += 1;
				}
			}
			addPixel24(surface, x_left + sx * font.getLetterWidth() + i,
								y_top + sy * font.getLetterHeight() + j, r, g, b, v / font.getOverSize() / font.getOverSize());
		}
	}
	void putChar(SDL_Surface *surface, Font& font, wchar_t ch, int sx, int sy, int x_left, int y_top, wchar_t* encoding,  Uint8 r, Uint8 g, Uint8 b)
	{
		wchar_t* pcur_ch = encoding;
		for (vector<bool*>::const_iterator iter = font.getLettersBegin(); iter != font.getLettersEnd(); iter++)
		{
			if (*pcur_ch == ch)
			{
				putSymbol(surface, font, iter, sx, sy, x_left, y_top, r, g, b);
			}
			pcur_ch ++;
		}
	}

public:
	void Clear()
	{
		pthread_mutex_lock(&printing_mutex);
		SymbolPlace* sp = LockFrameBuffer();
		for (int i = 0; i < getFrameBufferWidth() * getFrameBufferHeight(); i++)
		{
			sp[i].fore_b = selected_fore_color & 0xFF;
			sp[i].fore_g = selected_fore_color >> 8 & 0xFF;
			sp[i].fore_r = selected_fore_color >> 16 & 0xFF;

			sp[i].back_b = selected_back_color & 0xFF;
			sp[i].back_g = selected_back_color >> 8 & 0xFF;
			sp[i].back_r = selected_back_color >> 16 & 0xFF;

			sp[i].code = 0;
		}
		UnlockFrameBuffer();
		pthread_mutex_unlock(&printing_mutex);
	}

	void Write(const wchar_t* str)
	{
		pthread_mutex_lock(&printing_mutex);
		SymbolPlace* sp = LockFrameBuffer();
		int len = wcslen(str);
		for (int i = 0; i < len; i++)
		{
			if (str[i] == '\r')
			{
				cursor_x = 0;
			}
			else if (str[i] != '\n')
			{
				int pos = cursor_y * getFrameBufferWidth() + cursor_x;
				sp[pos].fore_b = selected_fore_color & 0xFF;
				sp[pos].fore_g = selected_fore_color >> 8 & 0xFF;
				sp[pos].fore_r = selected_fore_color >> 16 & 0xFF;

				sp[pos].back_b = selected_back_color & 0xFF;
				sp[pos].back_g = selected_back_color >> 8 & 0xFF;
				sp[pos].back_r = selected_back_color >> 16 & 0xFF;

				sp[pos].code = str[i];
				cursor_x ++;
			}
			else
			{
				cursor_x = 0;
				cursor_y ++;
			}
			if (cursor_x >= getFrameBufferWidth())
			{
				cursor_x = 0;
				cursor_y ++;
			}
			if (cursor_y >= getFrameBufferHeight())
			{
				// Move everything up 1 line
				for (int i = 0; i < getFrameBufferWidth(); i++)
				for (int j = 1; j < getFrameBufferHeight(); j++)
				{
					int pos = j * getFrameBufferWidth() + i;
					int pos_lineback = (j - 1) * getFrameBufferWidth() + i;
					sp[pos_lineback] = sp[pos];
				}
				for (int i = 0; i < getFrameBufferWidth(); i++)
				{
					int pos = (getFrameBufferHeight() - 1) * getFrameBufferWidth() + i;
					sp[pos].code = 0;
					sp[pos].fore_b = selected_fore_color & 0xFF;
					sp[pos].fore_g = selected_fore_color >> 8 & 0xFF;
					sp[pos].fore_r = selected_fore_color >> 16 & 0xFF;

					sp[pos].back_b = selected_back_color & 0xFF;
					sp[pos].back_g = selected_back_color >> 8 & 0xFF;
					sp[pos].back_r = selected_back_color >> 16 & 0xFF;
				}
				cursor_x = 0;
				cursor_y --;
			}
		}
		UnlockFrameBuffer();
		pthread_mutex_unlock(&printing_mutex);
	}

	void SelectForeColor(Uint8 r, Uint8 g, Uint8 b)
	{
		pthread_mutex_lock(&printing_mutex);
		selected_fore_color = (r << 16) + (g << 8) + b;
		pthread_mutex_unlock(&printing_mutex);
	}

	void SelectBackColor(Uint8 r, Uint8 g, Uint8 b)
	{
		pthread_mutex_lock(&printing_mutex);
		selected_back_color = (r << 16) + (g << 8) + b;
		pthread_mutex_unlock(&printing_mutex);
	}

	void SetCursorPosition(int x, int y)
	{
		pthread_mutex_lock(&printing_mutex);
		cursor_x = x % getFrameBufferWidth();
		cursor_y = (y + x / getFrameBufferWidth()) % getFrameBufferHeight();
		pthread_mutex_unlock(&printing_mutex);
	}
	void MoveCursor(int dx, int dy)
	{
		SetCursorPosition(cursor_x + dx, cursor_y + dy);
	}

	SDLScreen(int frameBufferWidth, int frameBufferHeight) :
		SDLBuffer(frameBufferWidth, frameBufferHeight),
		cursor_x(0), cursor_y(0)
	{
		selected_fore_color = 0xaaaaaa;
		selected_back_color = 0x000000;
		pthread_mutex_init(&printing_mutex, NULL);

		Clear();
	}

	~SDLScreen()
	{
		pthread_mutex_destroy(&printing_mutex);
	}

	void draw_framebuffer(Font& font, int xLeft, int yTop, SDL_Surface* surface)
	{
		const SymbolPlace* frame_buffer = lockFrameBufferReadOnly();
		{
			SDL_PixelFormat* pfm = surface->format;

			for (int i = 0; i < getFrameBufferWidth() * getFrameBufferHeight(); i++)
			{
				int distance = 0;

				SDL_Rect rct;
				rct.x = (i % getFrameBufferWidth()) * (font.getLetterWidth() + distance) + xLeft;
				rct.y = (i / getFrameBufferWidth()) * (font.getLetterHeight() + distance) + yTop;
				rct.w = font.getLetterWidth() + distance;
				rct.h = font.getLetterHeight() + distance;

				SDL_FillRect(surface, &rct, SDL_MapRGB(pfm,
									frame_buffer[i].back_r,
									frame_buffer[i].back_g,
									frame_buffer[i].back_b));
				putChar(surface, font, frame_buffer[i].code,
									i % getFrameBufferWidth(),
									i / getFrameBufferWidth(),
									xLeft,
									yTop,
									encoding,
									frame_buffer[i].fore_r,
									frame_buffer[i].fore_g,
									frame_buffer[i].fore_b);
			}
		}
		unlockFrameBufferReadOnly();
		clearFrameBufferModified();
	}

	void drawCursor(Font& cursorFont, int xLeft, int yTop, SDL_Surface* surface)
	{
		const SymbolPlace* frame_buffer = lockFrameBufferReadOnly();
		{
			putChar(surface, cursorFont, '2',
								cursor_x,
								cursor_y,
								xLeft,
								yTop, cursor_encoding,
								frame_buffer[cursor_y * getFrameBufferWidth() + cursor_x].fore_r,
								frame_buffer[cursor_y * getFrameBufferWidth() + cursor_x].fore_g,
								frame_buffer[cursor_y * getFrameBufferWidth() + cursor_x].fore_b);
		}
		unlockFrameBufferReadOnly();
	}

};

class SDLTerminal
{
private:
	int frame_buffer_width, frame_buffer_height;

	Keyboard& keyboard;
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
	void updateCaption()
	{
	    char captionLong[100];
	    sprintf(captionLong, "Vintage Terminal - Screen %d", activeScreen + 1);
	    char captionShort[100];
	    sprintf(captionShort, "Vintage Terminal (%d)", activeScreen + 1);


	    SDL_WM_SetCaption(captionLong, captionShort);
	}

public:
	SDLTerminal(Keyboard& kbd, Font& font, Font& curFont);
	void Run();
	SDLScreen& getScreen(int index)
	{
		return *(screens[index]);
	}
	virtual ~SDLTerminal();

};


#endif /* SDLCONSOLE_H_ */

class SDLScreen;

#ifndef SDL_SCREEN_H
#define SDL_SCREEN_H

#include <SDL.h>
#include <pthread.h>
#include <vector>
#include <assert.h>

#include "HardwareDevice.h"
#include "SDLScreen.h"
#include "../../FontEditor/include/Font.h"
#include "KeyboardController.h"

using namespace std;

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
	//SymbolPlace* frameBufferEditing;
	int frameBufferWidth, frameBufferHeight;
    bool frameBufferModified;
public:
    int getFrameBufferWidth() { return frameBufferWidth; }
    int getFrameBufferHeight() { return frameBufferHeight; }

    const SymbolPlace* lockFrameBufferReadOnly();
    void unlockFrameBufferReadOnly();
    SDLBuffer(int frameBufferWidth, int frameBufferHeight);
    virtual ~SDLBuffer();
    SymbolPlace* LockFrameBuffer();
    void UnlockFrameBuffer();
    bool getFrameBufferModified() { return frameBufferModified; }
    void clearFrameBufferModified() { frameBufferModified = false; }
};

class SDLScreen : public SDLBuffer
{
	friend class SDLTerminal;
private:
	pthread_mutex_t printing_mutex;
    Uint32 selected_fore_color, selected_back_color;
    int cursor_x, cursor_y;
    KeyboardController* keyboardController;
    volatile bool activity;

protected:
    static wchar_t* fontEncoding;
    static wchar_t* cursorEncoding;

    void addPixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a);

    void putSymbol(SDL_Surface *surface, CachedFont& font, vector<unsigned char*>::const_iterator iter, int sx, int sy, int x_left, int y_top, Uint8 r, Uint8 g, Uint8 b);
	void putChar(SDL_Surface *surface, CachedFont& font, wchar_t ch, int sx, int sy, int x_left, int y_top, wchar_t* encoding,  Uint8 r, Uint8 g, Uint8 b);

	void setActivity(bool value)
	{
		activity = value;
	}
public:
	bool isActive()
	{
		return activity;
	}

    SymbolPlace createSymbolPlaceWithCurrentColors(wchar_t ch)
    {
    	SymbolPlace res;
		res.fore_b = selected_fore_color & 0xFF;
		res.fore_g = selected_fore_color >> 8 & 0xFF;
		res.fore_r = selected_fore_color >> 16 & 0xFF;

		res.back_b = selected_back_color & 0xFF;
		res.back_g = selected_back_color >> 8 & 0xFF;
		res.back_r = selected_back_color >> 16 & 0xFF;

		res.code = ch;

		return res;
    }
	void Clear();
	void Write(const wchar_t* str);

	void SelectForeColor(Uint8 r, Uint8 g, Uint8 b);
	void SelectBackColor(Uint8 r, Uint8 g, Uint8 b);
	void SetCursorPosition(int x, int y);
	int GetCursorPositionX() { return cursor_x; }
	int GetCursorPositionY() { return cursor_y; }
	void MoveCursor(int dx, int dy);

	void setKeyboardController(KeyboardController* value) { keyboardController = value; }
	KeyboardController* getKeyboardController() { return keyboardController; }

	SDLScreen(int frameBufferWidth, int frameBufferHeight);
	~SDLScreen();
	void draw_framebuffer(CachedFont& font, int xLeft, int yTop, SDL_Surface* surface);
	void drawCursor(CachedFont& cursorFont, int xLeft, int yTop, SDL_Surface* surface);
};

#endif

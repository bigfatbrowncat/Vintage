class SDLScreen;

#ifndef SDL_SCREEN_H
#define SDL_SCREEN_H

#include <SDL.h>
#include <pthread.h>
#include <vector>
#include <assert.h>

#include "HardwareDevice.h"
#include "../../FontEditor/include/Font.h"

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
	SymbolPlace* frameBufferEditing;
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
private:
	pthread_mutex_t printing_mutex;
    Uint32 selected_fore_color, selected_back_color;
    int cursor_x, cursor_y;
    Keyboard* keyboard;

protected:
    static wchar_t* encoding;
    static wchar_t* cursor_encoding;

    void addPixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a);

	void putSymbol(SDL_Surface *surface, Font& font, vector<bool*>::const_iterator iter, int sx, int sy, int x_left, int y_top, Uint8 r, Uint8 g, Uint8 b);
	void putChar(SDL_Surface *surface, Font& font, wchar_t ch, int sx, int sy, int x_left, int y_top, wchar_t* encoding,  Uint8 r, Uint8 g, Uint8 b);

public:
	void Clear();
	void Write(const wchar_t* str);

	void SelectForeColor(Uint8 r, Uint8 g, Uint8 b);
	void SelectBackColor(Uint8 r, Uint8 g, Uint8 b);
	void SetCursorPosition(int x, int y);
	void MoveCursor(int dx, int dy);

	void setKeyboard(Keyboard* value) { keyboard = value; }
	Keyboard* getKeyboard() { return keyboard; }

	SDLScreen(int frameBufferWidth, int frameBufferHeight);
	~SDLScreen();
	void draw_framebuffer(Font& font, int xLeft, int yTop, SDL_Surface* surface);
	void drawCursor(Font& cursorFont, int xLeft, int yTop, SDL_Surface* surface);
};

#endif

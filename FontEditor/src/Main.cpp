#include <SDL.h>
#include <stdio.h>
#include <assert.h>

#include <Font.h>

using namespace std;

int width = 800;
int height = 600;

bool quit_pending = false;

void blendPixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a)
{
	if(x >= 0 && y >= 0 && x < surface->w && y < surface->h)
	{
		assert(surface->format->BytesPerPixel == 3);
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

void addPixel24(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, float a)
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
}

void blendFrameFilled(SDL_Surface *surface, int x1, int x2, int y1, int y2, Uint8 r, Uint8 g, Uint8 b, float a)
{
	for (int x = x1; x <= x2; x++)
	for (int y = y1; y <= y2; y++)
	{
		blendPixel24(surface, x, y, r, g, b, a);
	}
}

void blendFrame(SDL_Surface *surface, int x1, int x2, int y1, int y2, Uint8 r, Uint8 g, Uint8 b, float a)
{
	for (int x = x1; x <= x2; x++)
	{
		blendPixel24(surface, x, y1, r, g, b, a);
		blendPixel24(surface, x, y2, r, g, b, a);
	}
	for (int y = y1; y <= y2; y++)
	{
		blendPixel24(surface, x1, y, r, g, b, a);
		blendPixel24(surface, x2, y, r, g, b, a);
	}
}

int drawing_box_x = 35;
int drawing_box_y = 70;

int char_view_y = 20;
int char_view_margin = 20;

//int symbol_preview_x = 387;
int symbol_preview_y = 70;

int drawing_box_frame_width = 5;
int symbol_preview_frame_width = 10;

/*int symbol_w = 10;
int symbol_h = 16;*/

int cell_size = 9;

int small_cell_x = 0, small_cell_y = 0, big_cell_x = 0, big_cell_y = 0;

wchar_t* encoding = L" "
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                     "abcdefghijklmnopqrstuvwxyz"
                     "0123456789.,:;!?@#$%^&*()[]{}_-+<>=~\"'`/\\|";

enum Mode
{
	mIdle,
	mPaintingSmallCells,
	mPaintingBigCells,
	mSelecting,
	mMovingSelection
};

Mode currentMode = mIdle;
bool ctrlButtonIsDown = false;
bool shiftButtonIsDown = false;

enum color { black, white } painting_color;

int cell_under_cursor_x = -1, cell_under_cursor_y = -1;
int cell_selection_start_x = -1, cell_selection_start_y = -1;

void draw(SDL_Surface* surface, const Font& edited);
void process_events(Font& edited);
void putSymbol(SDL_Surface *surface, const Font& font, vector<bool*>::const_iterator iter, int x, int y, Uint8 r, Uint8 g, Uint8 b);
void putChar(SDL_Surface *surface, const Font& font, const wchar_t* encoding, wchar_t ch, int x, int y, Uint8 r, Uint8 g, Uint8 b);

void putString(SDL_Surface *surface, const Font& font, const wchar_t* encoding, const wchar_t* string, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	const wchar_t* ps = string;
	int xcur = x, ycur = y;
	while (*ps != 0)
	{
		if (*ps != '\n' && *ps != '\t')
		{
			putChar(surface, font, encoding, *ps, xcur, ycur, r, g, b);
		}

		if (*ps == '\n')
		{
			xcur = x;
			ycur += font.getLetterHeight();
		}
		else if (*ps == '\t')
		{
			xcur += 4 * font.getLetterWidth();
		}

		else
		{
			xcur += font.getLetterWidth();
		}
		ps ++;
	}
}

void putChar(SDL_Surface *surface, const Font& font, const wchar_t* encoding, wchar_t ch, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	const wchar_t* pcur_ch = encoding;
	for (vector<bool*>::const_iterator iter = font.getLettersBegin(); iter != font.getLettersEnd(); iter++)
	{
		if (*pcur_ch == ch)
		{
			putSymbol(surface, font, iter, x, y, r, g, b);
		}
		pcur_ch ++;
	}
}

void putSymbol(SDL_Surface *surface, const Font& font, vector<bool*>::const_iterator iter, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	// Searching the symbol code in the current encoding
/*	int code = 0;
	for (unsigned int k = 0; k < wcslen(encoding); k++)
	{
		if (encoding[k] == symbol) { code = k; break; }
	}*/

	// Drawing it
	for (int j = 0; j < font.getLetterHeight() * font.getOverSize(); j++)
	for (int i = 0; i < font.getLetterWidth() * font.getOverSize(); i++)
	{
		if ((*iter)[j * font.getLetterWidth() * font.getOverSize() + i])
		{
			addPixel24(surface, x + i / font.getOverSize(), y + j / font.getOverSize(), r, g, b, 1.0 / font.getOverSize() / font.getOverSize());
		}
	}
}

vector<bool*>::iterator currentLetter;

int main(int argc, char* argv[])
{
	EditableFont& edited = *new EditableFont("font.txt");
	currentLetter = edited.getLettersBegin();

	printf("Starting UI event loop.\n");
    fflush(stdout);
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
    	printf("Could not initialize SDL: %d", SDL_GetError());
    	return 1;
    }

    SDL_WM_SetCaption("Font editor", "Font editor");

    atexit(SDL_Quit);
    printf("Starting UI event loop.\n");

    SDL_Surface* screen;
    screen = SDL_SetVideoMode(width, height, 24, SDL_HWSURFACE | /*SDL_FULLSCREEN |*/ SDL_DOUBLEBUF);

    if(screen == NULL)
    {
    	printf("Could not set video mode: %d", SDL_GetError());
    	return 1;
    }

    //scanSymbols("font.txt");

    quit_pending = false;
    printf("Starting UI event loop.\n");
    bool success = false;
    while( !quit_pending )
    {
        SDL_Rect rct;
        rct.x = 0; rct.y = 0; rct.w = width; rct.h = height;
        SDL_FillRect(screen, &rct, 0x000000);

    	draw(screen, edited);
        process_events(edited);

        SDL_Delay(10);
        SDL_UpdateRect(screen, 0, 0, 0, 0);
    }

    edited.saveToFile("font.txt");
}

void process_events(Font& edited)
{
	/* Our SDL event placeholder. */
	SDL_Event event;
	bool down = false;

	/* Grab all the events off the queue. */
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				// Quiting the program
				quit_pending = true;
			}
			else if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL)
			{
				ctrlButtonIsDown = true;
			}
			else if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
			{
				shiftButtonIsDown = true;
			}
			else if (event.key.keysym.sym == SDLK_PAGEDOWN)
			{
				// Changing to the next letter
				currentLetter++;
				if (currentLetter == edited.getLettersEnd()) currentLetter = edited.getLettersBegin();
			}
			else if (event.key.keysym.sym == SDLK_PAGEUP)
			{
				// Changing to the previous letter
				if (currentLetter == edited.getLettersBegin())
				{
					currentLetter = edited.getLettersEnd() - 1;
				}
				else
				{
					currentLetter--;
				}
			}
			else if (event.key.keysym.sym == SDLK_INSERT)
			{
				// Inserting the new character here
				currentLetter = edited.insert(currentLetter);
			}
			else if (event.key.keysym.sym == SDLK_DELETE)
			{
				// Removing the current character
				edited.remove(currentLetter);
				if (edited.getLettersBegin() == edited.getLettersEnd())
				{
					edited.insert(edited.getLettersBegin());
				}

				if (currentLetter == edited.getLettersEnd()) currentLetter--;
			}
			else if (event.key.keysym.sym == SDLK_c)
			{
				((EditableFont&)edited).copyToClipboard(currentLetter);
			}
			else if (event.key.keysym.sym == SDLK_v)
			{
				((EditableFont&)edited).pasteFromClipboard(currentLetter);
			}
			else if (event.key.keysym.sym == SDLK_m)
			{
				((EditableFont&)edited).mirrorHorizontal(currentLetter, currentSelection);
			}
			else if (event.key.keysym.sym == SDLK_n)
			{
				((EditableFont&)edited).mirrorVertical(currentLetter, currentSelection);
			}
			else if (event.key.keysym.sym == SDLK_LEFT)
			{
				((EditableFont&)edited).kernLeft(currentLetter);
			}
			else if (event.key.keysym.sym == SDLK_RIGHT)
			{
				((EditableFont&)edited).kernRight(currentLetter);
			}
			else if (event.key.keysym.sym == SDLK_DOWN)
			{
				((EditableFont&)edited).kernDown(currentLetter);
			}
			else if (event.key.keysym.sym == SDLK_UP)
			{
				((EditableFont&)edited).kernUp(currentLetter);
			}
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL)
			{
				ctrlButtonIsDown = false;
			}
			else if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
			{
				shiftButtonIsDown = false;
			}
			break;
		case SDL_QUIT:
			/* Handle quit requests (like Ctrl-c). */
			quit_pending = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			small_cell_x = (event.button.x - drawing_box_x - drawing_box_frame_width) / cell_size;
			small_cell_y = (event.button.y - drawing_box_y - drawing_box_frame_width) / cell_size;
			big_cell_x = (event.button.x - drawing_box_x - drawing_box_frame_width) / cell_size / edited.getOverSize();
			big_cell_y = (event.button.y - drawing_box_y - drawing_box_frame_width) / cell_size / edited.getOverSize();

			if (event.button.x > drawing_box_x + drawing_box_frame_width && event.button.x < drawing_box_x + drawing_box_frame_width + edited.getLetterWidth() * edited.getOverSize() * cell_size &&
			    event.button.y > drawing_box_y + drawing_box_frame_width && event.button.y < drawing_box_y + drawing_box_frame_width + edited.getLetterHeight() * edited.getOverSize() * cell_size)
			{
				if (ctrlButtonIsDown && !shiftButtonIsDown)
				{
					currentMode = mPaintingSmallCells;

					if (event.button.button == SDL_BUTTON_LEFT)
					{
						(*currentLetter)[small_cell_y * edited.getLetterWidth() * edited.getOverSize() + small_cell_x] = true;
						painting_color = white;
					}
					else if (event.button.button == SDL_BUTTON_RIGHT)
					{
						(*currentLetter)[small_cell_y * edited.getLetterWidth() * edited.getOverSize() + small_cell_x] = false;
						painting_color = black;
					}

					cell_under_cursor_x = small_cell_x;
					cell_under_cursor_y = small_cell_y;
				}
				else if (!ctrlButtonIsDown && !shiftButtonIsDown)
				{
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						for (int p = 0; p < edited.getOverSize(); p++)
						for (int q = 0; q < edited.getOverSize(); q++)
						{
							(*currentLetter)[(edited.getOverSize() * big_cell_y + q) * edited.getLetterWidth() * edited.getOverSize() + (edited.getOverSize() * big_cell_x + p)] = true;
						}
						painting_color = white;
					}
					else if (event.button.button == SDL_BUTTON_RIGHT)
					{
						for (int p = 0; p < edited.getOverSize(); p++)
						for (int q = 0; q < edited.getOverSize(); q++)
						{
							(*currentLetter)[(edited.getOverSize() * big_cell_y + q) * edited.getLetterWidth() * edited.getOverSize() + (edited.getOverSize() * big_cell_x + p)] = false;
						}
						painting_color = black;
					}

					currentMode = mPaintingBigCells;
					cell_under_cursor_x = big_cell_x;
					cell_under_cursor_y = big_cell_y;
				}
				else if (!ctrlButtonIsDown && shiftButtonIsDown)
				{
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						currentMode = mSelecting;
						cell_selection_start_x = small_cell_x;
						cell_selection_start_y = small_cell_y;
					}
					else if (event.button.button == SDL_BUTTON_RIGHT)
					{
						currentMode = mIdle;
						currentSelection = SELECTION_NONE;
					}

				}
			}
			break;
		case SDL_MOUSEMOTION:
			small_cell_x = (event.motion.x - drawing_box_x - drawing_box_frame_width) / cell_size;
			small_cell_y = (event.motion.y - drawing_box_y - drawing_box_frame_width) / cell_size;
			big_cell_x = (event.button.x - drawing_box_x - drawing_box_frame_width) / cell_size / edited.getOverSize();
			big_cell_y = (event.button.y - drawing_box_y - drawing_box_frame_width) / cell_size / edited.getOverSize();

			if (event.motion.x > drawing_box_x + drawing_box_frame_width &&
				event.motion.x < drawing_box_x + drawing_box_frame_width + edited.getLetterWidth() * edited.getOverSize() * cell_size &&
			    event.motion.y > drawing_box_y + drawing_box_frame_width &&
			    event.motion.y < drawing_box_y + drawing_box_frame_width + edited.getLetterHeight() * edited.getOverSize() * cell_size)
			{

				if (currentMode == mPaintingSmallCells)
				{
					if (cell_under_cursor_x != small_cell_x || cell_under_cursor_y != small_cell_y)
					{

						if (painting_color == white)
						{
							(*currentLetter)[small_cell_y * edited.getLetterWidth() * edited.getOverSize() + small_cell_x] = true;
						}
						else if (painting_color == black)
						{
							(*currentLetter)[small_cell_y * edited.getLetterWidth() * edited.getOverSize() + small_cell_x] = false;
						}

						cell_under_cursor_x = small_cell_x;
						cell_under_cursor_y = small_cell_y;
					}
				}
				else if (currentMode == mPaintingBigCells)
				{
					if (painting_color == white)
					{
						for (int p = 0; p < edited.getOverSize(); p++)
						for (int q = 0; q < edited.getOverSize(); q++)
						{
							(*currentLetter)[(edited.getOverSize() * big_cell_y + q) * edited.getLetterWidth() * edited.getOverSize() + (edited.getOverSize() * big_cell_x + p)] = true;
						}
						painting_color = white;
					}
					else if (painting_color == black)
					{
						for (int p = 0; p < edited.getOverSize(); p++)
						for (int q = 0; q < edited.getOverSize(); q++)
						{
							(*currentLetter)[(edited.getOverSize() * big_cell_y + q) * edited.getLetterWidth() * edited.getOverSize() + (edited.getOverSize() * big_cell_x + p)] = false;
						}
						painting_color = black;
					}

					cell_under_cursor_x = big_cell_x;
					cell_under_cursor_y = big_cell_y;
				}
				else if (currentMode == mSelecting)
				{
					currentSelection = Selection(cell_selection_start_x, cell_selection_start_y, small_cell_x, small_cell_y);
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			currentMode = mIdle;
			break;
		}
	}
}

void draw(SDL_Surface* surface, const Font& edited)
{
	blendFrame(surface, drawing_box_x, drawing_box_x + 2 * drawing_box_frame_width + edited.getLetterWidth() * edited.getOverSize() * cell_size,
			             drawing_box_y, drawing_box_y + 2 * drawing_box_frame_width + edited.getLetterHeight() * edited.getOverSize() * cell_size, 255, 255, 255, 1);

	// Drawing the small cells

	for (int i = 0; i < edited.getLetterWidth() * edited.getOverSize(); i++)
	for (int j = 0; j < edited.getLetterHeight() * edited.getOverSize(); j++)
	{
		if ((*currentLetter)[j * edited.getLetterWidth() * edited.getOverSize() + i])
		{
			// Drawing white pixel
			blendFrameFilled(surface, drawing_box_x + drawing_box_frame_width + cell_size * i,
			                           drawing_box_x + drawing_box_frame_width + cell_size * (i + 1),
			                           drawing_box_y + drawing_box_frame_width + cell_size * j,
			                           drawing_box_y + drawing_box_frame_width + cell_size * (j + 1), 255, 255, 255, 1);

		}
		else if (i == small_cell_x || j == small_cell_y)
		{
			// Drawing cell under mouse and it's cross
			blendFrameFilled(surface, drawing_box_x + drawing_box_frame_width + cell_size * i,
			                           drawing_box_x + drawing_box_frame_width + cell_size * (i + 1),
			                           drawing_box_y + drawing_box_frame_width + cell_size * j,
			                           drawing_box_y + drawing_box_frame_width + cell_size * (j + 1), 64, 64, 64, 1);
		}
		else if (i >= currentSelection.xLeft && i <= currentSelection.xRight && j >= currentSelection.yTop && j <= currentSelection.yBottom)
		{
			// Drawing selection
			blendFrameFilled(surface, drawing_box_x + drawing_box_frame_width + cell_size * i,
			                           drawing_box_x + drawing_box_frame_width + cell_size * (i + 1),
			                           drawing_box_y + drawing_box_frame_width + cell_size * j,
			                           drawing_box_y + drawing_box_frame_width + cell_size * (j + 1), 96, 96, 96, 1);
		}

		blendFrame(surface, drawing_box_x + drawing_box_frame_width + cell_size * i,
							 drawing_box_x + drawing_box_frame_width + cell_size * (i + 1),
							 drawing_box_y + drawing_box_frame_width + cell_size * j,
							 drawing_box_y + drawing_box_frame_width + cell_size * (j + 1), 128, 128, 128, 0.3);
	}

	// Drawing the big (pixel) cells

	for (int i = 0; i < edited.getLetterWidth(); i++)
	for (int j = 0; j < edited.getLetterHeight(); j++)
	{
		blendFrame(surface, drawing_box_x + drawing_box_frame_width + cell_size * edited.getOverSize() * i,
							 drawing_box_x + drawing_box_frame_width + cell_size * edited.getOverSize() * (i + 1),
							 drawing_box_y + drawing_box_frame_width + cell_size * edited.getOverSize() * j,
							 drawing_box_y + drawing_box_frame_width + cell_size * edited.getOverSize() * (j + 1), 128, 128, 128, 0.8);
	}

	// Currently char preview

	int symbol_preview_x = width / 2 - edited.getLetterWidth() / 2 - symbol_preview_frame_width;

	blendFrame(surface, symbol_preview_x, symbol_preview_x + edited.getLetterWidth() + 2 * symbol_preview_frame_width,
	                    symbol_preview_y, symbol_preview_y + edited.getLetterHeight() + 2 * symbol_preview_frame_width, 255, 255, 255, 1);

	putSymbol(surface, edited, currentLetter, symbol_preview_x + symbol_preview_frame_width,
	                                 symbol_preview_y + symbol_preview_frame_width, 255, 255, 255);

	// Drawing characters palette

	int char_view_number = (width - 2 * char_view_margin) / (edited.getLetterWidth() + 5);
	if (edited.getLettersNumber() < char_view_number)
	{
		char_view_number = edited.getLettersNumber();
	}
	if (char_view_number % 2 == 0) char_view_number --;

	int char_view_left = (int)(width / 2 - (double)(edited.getLetterWidth() + 5) * (((float)char_view_number - 1) / 2 + 0.5));

	vector<bool*>::const_iterator iter = currentLetter;
	for (int i = 0; i <= ((float)char_view_number - 1) / 2; i++)
	{
		if (iter == edited.getLettersBegin())
			iter = edited.getLettersEnd() - 1;
		else
			iter --;
	}

	for (int ind = 0; ind < char_view_number; ind++)
	{
		iter++;
		if (iter == edited.getLettersEnd()) iter = edited.getLettersBegin();

		int x1 = char_view_left + (edited.getLetterWidth() + 5) * ind;
		int x2 = char_view_left + (edited.getLetterWidth() + 5) * (ind + 1) - 1;
		int y1 = char_view_y;
		int y2 = char_view_y + edited.getLetterHeight() + 4;

		blendFrame(surface, x1, x2, y1, y2, 128, 128, 128, 1);
		if (iter == currentLetter)
		{
			blendFrame(surface, x1, x2, y1, y2, 255, 255, 255, 1);
		}

		putSymbol(surface, edited, iter, x1 + 3, y1 + 3, 255, 255, 255);
	}

	const wchar_t* test_str = L"The quick brown fox\njumps over the lazy dog.\n\nTHE QUICK BROWN FOX\nJUMPS OVER THE LAZY DOG.";

	putString(surface, edited, encoding, test_str, symbol_preview_x, symbol_preview_y + 50, 255, 255, 255);
}

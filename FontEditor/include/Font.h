#ifndef FONT_H
#define FONT_H

#include <stdlib.h>
#include <vector>
#include <string>

using namespace std;

class Font
{
private:
	vector<bool*> letters;
	int letterWidth, letterHeight, overSize;
protected:
	void initFromOther(const Font& other)
	{
		letterWidth = other.letterWidth;
		letterHeight = other.letterHeight;
		for (vector<bool*>::const_iterator iter = other.getLettersBegin(); iter != other.getLettersEnd(); iter++)
		{
			bool* newChar = *insert(letters.end());
			memcpy(newChar, *iter, letterWidth * letterHeight * sizeof(bool));
		}
	}
public:
	// Getters

	int getOverSize() const { return overSize; }
	int getLetterWidth() const { return letterWidth; }
	int getLetterHeight() const { return letterHeight; }
	int getLettersNumber() const { return letters.size(); }
	const vector<bool*>::const_iterator getLettersBegin() const { return letters.begin(); }
	const vector<bool*>::const_iterator getLettersEnd() const { return letters.end(); }
	const vector<bool*>::iterator getLettersBegin() { return letters.begin(); }
	const vector<bool*>::iterator getLettersEnd() { return letters.end(); }

	// Constructors / destructor

	Font(const Font& other)
	{
		initFromOther(other);
	}

	Font(int letterWidth, int letterHeight) : letterWidth(letterWidth),
	                                      letterHeight(letterHeight)
	{

	}

	Font(const string& fileName)
	{
		if (!loadFromFile(fileName))
		{
			throw exception();
		}
	}

	virtual ~Font()
	{
		clear();
	}

	// Operators

	Font& operator = (const Font& other)
	{
		clear();
		initFromOther(other);
		return *this;
	}

	// Methods

	bool loadFromFile(const string& fileName)
	{
		clear();
		FILE* f = fopen(fileName.c_str(), "r");
		if (f != NULL)
		{
			fscanf(f, "%d %d %d", &letterWidth, &letterHeight, &overSize);
			fgetc(f);	// skip '\n'
		}

		if (f != NULL)
		{
			while (!feof(f))
			{
				bool* newChar = *insert(letters.end());
				for (int j = 0; j < letterHeight * overSize; j++)
				{
					for (int i = 0; i < letterWidth * overSize; i++)
					{
						char c = fgetc(f);
						newChar[j * letterWidth * overSize + i] = (c == '#');
					}
					fgetc(f);	// skip '\n'
				}
				fgetc(f);	// skip '\n'
			}
			fclose(f);
			return true;
		}
		else
			return false;
	}

	bool saveToFile(const string& filename)
	{
		FILE* f = fopen(filename.c_str(), "w");
		fprintf(f, "%d %d %d\n", letterWidth, letterHeight, overSize);

		if (f != NULL)
		{
			for (vector<bool*>::iterator iter = letters.begin(); iter != letters.end(); iter++)
			{
				for (int j = 0; j < letterHeight * overSize; j++)
				{
					for (int i = 0; i < letterWidth * overSize; i++)
					{
						if ((*iter)[j * letterWidth * overSize + i])
							fprintf(f, "#");
						else
							fprintf(f, " ");
					}
					fprintf(f, "\n");
				}
				if (iter != letters.end() - 1)
					fprintf(f, "\n");
			}
			fclose(f);
			return true;
		}
		else
			return false;
	}

	vector<bool*>::iterator insert(vector<bool*>::iterator iter)
	{
		bool* newChar = new bool[letterWidth * letterHeight * overSize * overSize];
		for (int i = 0; i < letterWidth * letterHeight * overSize * overSize; i++) newChar[i] = false;
		return letters.insert(iter, newChar);
	}

	void remove(vector<bool*>::iterator iter)
	{
		delete [] (*iter);
		letters.erase(iter);
	}

	void clear()
	{
		while (letters.size() > 0) remove(letters.begin());
	}
};

class EditableFont : public Font
{
private:
	bool* clipboard;
public:
	EditableFont(const string& fileName) : Font(fileName)
	{
		clipboard = new bool[getLetterWidth() * getLetterHeight() * getOverSize() * getOverSize()];
	}

	void copyToClipboard(vector<bool*>::iterator iter)
	{
		for (int i = 0; i < getLetterWidth() * getLetterHeight() * getOverSize() * getOverSize(); i++)
		{
			clipboard[i] = (*iter)[i];
		}
	}
	void pasteFromClipboard(vector<bool*>::iterator iter)
	{
		for (int i = 0; i < getLetterWidth() * getLetterHeight() * getOverSize() * getOverSize(); i++)
		{
			(*iter)[i] = clipboard[i];
		}
	}
	void mirrorHorizontal(vector<bool*>::iterator iter)
	{
		int letterWidth = getLetterWidth();
		if (letterWidth >= 2)
		{
			int overSize = getOverSize();
			for (int j = 0; j < getLetterHeight() * overSize; j++)
			{
				for (int i = 0; i < letterWidth * overSize / 2; i++)
				{
					int w = letterWidth * overSize;
					bool tmp = (*iter)[j * w + (w - 1 - i)];
					(*iter)[j * w + (w - 1 - i)] = (*iter)[j * w + i];
					(*iter)[j * w + i] = tmp;
				}
			}
		}
	}

	void mirrorVertical(vector<bool*>::iterator iter)
	{
		int letterWidth = getLetterWidth();
		int letterHeight = getLetterHeight();
		if (letterHeight >= 2)
		{
			int overSize = getOverSize();
			for (int i = 0; i < letterWidth * overSize; i++)
			{
				for (int j = 0; j < getLetterHeight() * overSize / 2; j++)
				{
					int w = letterWidth * overSize;
					int h = letterHeight * overSize;
					bool tmp = (*iter)[(h - 1 - j) * w + i];
					(*iter)[(h - 1 - j) * w + i] = (*iter)[j * w + i];
					(*iter)[j * w + i] = tmp;
				}
			}
		}
	}

	void kernLeft(vector<bool*>::iterator iter)
	{
		int overSize = getOverSize();
		int letterWidth = getLetterWidth();
		int letterHeight = getLetterHeight();
		int w = letterWidth * overSize;
		int h = letterHeight * overSize;
		for (int j = 0; j < h; j++)
		{
			bool tmp = (*iter)[j * w];
			for (int i = 1; i < w; i++)
			{
				(*iter)[j * w + (i + w - 1) % w] = (*iter)[j * w + i];
			}
			(*iter)[j * w + (w - 1)] = tmp;
		}
	}

	void kernRight(vector<bool*>::iterator iter)
	{
		int overSize = getOverSize();
		int letterWidth = getLetterWidth();
		int letterHeight = getLetterHeight();
		int w = letterWidth * overSize;
		int h = letterHeight * overSize;
		for (int j = 0; j < h; j++)
		{
			bool tmp = (*iter)[j * w + (w - 1)];
			for (int i = w - 2; i >= 0; i--)
			{
				(*iter)[j * w + (i + 1) % w] = (*iter)[j * w + i];
			}
			(*iter)[j * w] = tmp;
		}
	}

	void kernDown(vector<bool*>::iterator iter)
	{
		int overSize = getOverSize();
		int letterWidth = getLetterWidth();
		int letterHeight = getLetterHeight();
		int w = letterWidth * overSize;
		int h = letterHeight * overSize;
		for (int i = 0; i < w; i++)
		{
			bool tmp = (*iter)[(h - 1) * w + i];
			for (int j = h - 2; j >= 0; j--)
			{
				(*iter)[(j + 1) * w + i] = (*iter)[j * w + i];
			}
			(*iter)[i] = tmp;
		}
	}


	~EditableFont()
	{
		delete [] clipboard;
	}
};

#endif

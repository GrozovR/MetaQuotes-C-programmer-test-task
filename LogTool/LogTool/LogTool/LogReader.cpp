#include "LogReader.h"

#include <new>
#include <iostream>

// TODO: 
// new на маллоки
// GetnextLine - investigate optimal buffer size
// d
// f

CLogReader::CLogReader()
	: fileHandle(INVALID_HANDLE_VALUE), filter(nullptr), buffer(nullptr),
	bufferSize(0), bufferPos(0), bufferEnd(0)
{
}

CLogReader::~CLogReader()
{
	Close();
}

bool CLogReader::Open(const char* _fileName)
{
	Close();

	// TODO: check fielname size по MAX_PATH

	fileHandle = CreateFileA(_fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// инициализируем буфер для чтения, если еще не
	if (buffer == nullptr)
	{
		bufferSize = 64 * 1024; // 64 Кб
		DWORD fileSize = GetFileSize(fileHandle, NULL);
		if (fileSize == 0) {
			return false;
		}
		if (bufferSize > fileSize) {
			bufferSize = fileSize + 1;
		}

		buffer = new (std::nothrow) char[bufferSize];
		if (buffer == nullptr)
		{
			std::cout << "Error new memory WorkWithFile()" << std::endl;
			return false;
		}
		ZeroMemory(buffer, bufferSize);
		bufferPos = 0;
		bufferEnd = 0;
	}

	return true;
}

void CLogReader::Close()
{
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fileHandle);
		fileHandle = INVALID_HANDLE_VALUE;
	}
	if (buffer != nullptr)
	{
		delete[] buffer;
		buffer = nullptr;
	}
	if (filter != nullptr)
	{
		delete[] filter;
		filter = nullptr;
	}
}

bool CLogReader::SetFilter(const char* _filter)
{
	if (_filter == nullptr)
	{
		return false;
	}
	size_t length = strlen(_filter);
	if (length == 0) {
		return false;
	}
	size_t filterSize = length + 1;

	filter = new (std::nothrow) char[filterSize];
	if (filter == nullptr)
	{
		return false;
	}

	strcpy_s(filter, filterSize, _filter);
	filter[filterSize - 1] = NULL;

	// Сокращаем рядом стоящие *
	for (size_t i = 1; i < filterSize; i++)
	{
		if (filter[i] == '*' && filter[i - 1] == '*') {
			memmove_s(&filter[i - 1], filterSize, &filter[i], filterSize - i);
			i--;
			filterSize--;
		}
	}
	return true;
}

bool CLogReader::GetNextLine(char* _buf, const int _bufsize)
{
	if (fileHandle == INVALID_HANDLE_VALUE || filter == nullptr || _buf == nullptr || _bufsize <= 0)
	{
		return false; // Проверка корректности входных данных
	}

	ZeroMemory(_buf, _bufsize);

	while (true) {
		// Текущий буфер обработан, считываем следующую часть файла
		if ( bufferPos == bufferEnd
			&& !ReadFile(fileHandle, buffer, bufferSize, &bufferEnd, NULL))
		{
			std::cout << "Error ReadFile code: " << GetLastError() << std::endl;
			return false;
		}
		// конец файла
		if (bufferEnd == 0) {
			return false;
		}
		// Обрабатываем прочитанную часть
		if (workWithReadedPart(_buf, _bufsize))
		{
			// заполнен входной буффер
			// сохранили позицию в буфере
			// сброшена позиция фильтра
			filterPos = 0;
			return true;
		} else {
			// Дошли до конца буффера, но не нашли подходящей строки
		}
	}
	return false;
}

// Работа с буфером
bool CLogReader::workWithReadedPart(char* _buf, const int _bufsize)
{
	static bool isFilterMatched = false;
	int lineStartPos = bufferPos;

	int asterixPos = -1;
	bool stringNotMatched = false;

	for (; bufferPos < bufferEnd; bufferPos++) {
		char bufferChar = buffer[bufferPos];
		char filterChar = filter[filterPos];
		if (filterChar == '*') {
			asterixPos = filterPos;
			filterPos++;
			filterChar = filter[filterPos];
		}

		// TODO: излишнее ? // Тут filterChar не должен быть звездочкой, только символом
		if (filterChar != '?' && bufferChar != filterChar && asterixPos == -1) {
			stringNotMatched = true; // больше проверок не нужно, просто найдем конец строки
		}

		if (filterChar == NULL && bufferChar == NULL)
			return true; // Конец файла + совпадение по фильтру ( чет пока не поймал

		if (bufferChar == '\n' || ( bufferPos + 1 == bufferEnd ) ) {

			// тут нужно еще отбрасывать \r, для строк у которых нет * на конце

			if ((filterChar == NULL) || (filterChar == '*' && filter[filterPos + 1] == NULL)) {
				fillInputBuffer(lineStartPos,_buf, _bufsize);
				bufferPos++;	
				return true;
			}
			else {
				_buf[0] = NULL;// сбрасываем входной буфер - Обнуляем первый символ, чтобы запись пошла с начала
				lineStartPos = bufferPos + 1;
			}
			filterPos = 0;
			asterixPos = -1;
		}
		else {
			if (stringNotMatched) {
				// строка уже не матчится, просто ищем конец
				continue;
			}
			if (bufferChar == filterChar || filterChar == '?') {
				filterPos++;
			}
			else if (asterixPos == -1) {
				// символ не сходится, предыдущей звездочки нет
				stringNotMatched = true;
			}
			else if (asterixPos != -1) {
				filterPos = asterixPos + 1;
				if (bufferChar == filter[filterPos] || filterChar == '?') {
					filterPos++;
				}
			}
		}
	}
	// прошли прочитанный буффер до конца, прихраним строку, если есть куда
	fillInputBuffer(lineStartPos, _buf, _bufsize);
	return false;
}

void CLogReader::fillInputBuffer(int lineStart, char* _buf, const int _bufsize)
{
	// TODO: проверить
	int inputBufferFreeSize = _bufsize - strnlen_s(_buf, _bufsize);// -1;

	if (inputBufferFreeSize > 1) {
		int inputBufoffset = _bufsize - inputBufferFreeSize;
		int lineLenght = bufferPos - lineStart;

		int toCopy = inputBufferFreeSize < lineLenght
			? inputBufferFreeSize : lineLenght;	
		memcpy_s(&_buf[inputBufoffset], inputBufferFreeSize, &buffer[lineStart], toCopy);
		// окончание строки
		_buf[inputBufoffset + toCopy] = NULL;
	}
}


/*

bool CLogReader::isMatched()
{
	if ((filter[filterPos] == '\0') && (buffer[bufferPos] == '\0'))
		return true;

	if (filter[filterPos] == '\0') // не понимаю - типа конец фильтра, но не конец строки???
		return false;

	if (filter[filterPos] == '*')
	{
		if (filter[filterPos] == '\0')
			return true; // звезда в конце фильтра - нам без разницы, как заканчивается buf

		for (size_t i = 0, size = strlen(buf); i <= size; i++)
			if ((*(buf + i) == *(filter + 1)) || (*(filter + 1) == '?')) {
				// наткнулись на нужный символ, рекурсивно??? проверяем
				if (isMatch(buf + i + 1, filter + 2))
					return true;
			}
	}
	else
	{
		if (buffer[bufferPos] == '\0')
			return false; // как будто нужно проверять 

		if ((filter[filterPos] == '?') || (filter[filterPos] == buffer[bufferPos]))
			if (isMatch(buf + 1, filter + 1))
				return true;
		// как будто супер неоптимально, можно выйти раньше?
	}
	return false;
}

size_t CLogReader::workWithLine(char* buf, const int bufsize, bool isEndLine)
{
	size_t pos(0);

	for (int i = 0; i < bufsize; ++i)
	{
		if (buf[i] == '\n')
		{
			getNextLine(&buf[pos], i - pos);
			pos = i + 1;
		}
	}

	if (isEndLine)
		getNextLine(&buf[pos], bufsize - pos);

	return pos;
}

void CLogReader::getNextLine(char* buf, int bufsize)
{
	//А нужно ли нам подобное условие?
	if (bufsize > 1 && buf[bufsize - 1] == '\r')
		bufsize = bufsize - 1;

	char* StrReg = new(std::nothrow) char[bufsize + 1];
	if (StrReg)
	{
		ZeroMemory(StrReg, bufsize + 1);
		memcpy_s(StrReg, bufsize + 1, buf, bufsize);

		if (isMatch(StrReg, const_cast<char*>(filter)))
			//printStr(StrReg);

		delete[] StrReg;
		StrReg = nullptr;
	}
	else
		std::cout << "Error new memory GetNextLine()" << std::endl;
}

bool CLogReader::isMatch(char* buf, char* filter)
{
	if ((*filter == '\0') && (*buf == '\0'))
		return true;

	if (*filter == '\0') // не понимаю
		return false;

	if (*filter == '*')
	{
		if (*(filter + 1) == '\0')
			return true; // звезда в конце фильтра - нам без разницы, как заканчивается buf

		for (size_t i = 0, size = strlen(buf); i <= size; i++)
			if ((*(buf + i) == *(filter + 1)) || (*(filter + 1) == '?')) {
				// наткнулись на нужный символ, рекурсивно??? проверяем
				if (isMatch(buf + i + 1, filter + 2))
					return true;
			}
	}
	else
	{
		if (*buf == '\0')
			return false;

		if ((*filter == '?') || (*filter == *buf))
			if (isMatch(buf + 1, filter + 1))
				return true;
		// как будто супер неоптимально, можно выйти раньше?
	}
	return false;
}
*/
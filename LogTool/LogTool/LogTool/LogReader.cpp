#include "LogReader.h"
#include <stdio.h>

constexpr size_t NotFound = MAXSIZE_T;

CLogReader::CLogReader()
	: fileHandle(INVALID_HANDLE_VALUE),
	filter(nullptr), filterPos(0), filterAsteriskPos(NotFound), buffer(nullptr),
	bufferSize(0), bufferPos(0), bufferEnd(0)
{
	fileSize.QuadPart = 0;
	fileBytesReaded.QuadPart = 0;
}

CLogReader::~CLogReader()
{
	Close();
	releaseFilter();
}

bool CLogReader::Open(const char* _fileName)
{
	Close();

	fileHandle = CreateFileA(_fileName, GENERIC_READ, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// инициализируем буфер для чтения
	if (buffer == nullptr)
	{
		bufferSize = 64 * 1024; // 64 Кб
		if (!GetFileSizeEx(fileHandle, &fileSize)) {
			return false;
		}
		if (bufferSize > fileSize.QuadPart) {
			bufferSize = fileSize.QuadPart + 1;
		}

		buffer = (char*) malloc(bufferSize * sizeof(char));
		if (buffer == nullptr)
		{
			printf_s("Error new memory for read buffer");
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
		fileSize.QuadPart = 0;
		fileBytesReaded.QuadPart = 0;
	}
	if (buffer != nullptr)
	{
		free(buffer);
		buffer = nullptr;
	}
}

bool CLogReader::SetFilter(const char* _filter)
{
	if (_filter == nullptr)
		return false;

	size_t length = strlen(_filter);
	if (length == 0)
		return false;

	releaseFilter();

	size_t filterSize = length + 1;
	filter = (char*) malloc(filterSize * sizeof(char));
	if (filter == nullptr)
	{
		printf_s("Error new memory for set filter");
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
	if (_buf == nullptr || _bufsize <= 0 ||
		fileHandle == INVALID_HANDLE_VALUE || buffer == nullptr || filter == nullptr )
	{
		return false; // Проверка корректности входных данных
	}

	ZeroMemory(_buf, _bufsize);

	while (true) {
		// Текущий буфер обработан, считываем следующую часть файла
		if (bufferPos == bufferEnd)
		{
			if (!ReadFile(fileHandle, buffer, bufferSize, &bufferEnd, NULL)){
				printf_s("Error ReadFile");
				return false;
			}
			fileBytesReaded.QuadPart += bufferEnd;
			bufferPos = 0;
		}
		// конец файла
		if (bufferEnd == 0) {
			return false;
		}
		// Обрабатываем прочитанную часть
		if (workWithReadedPart(_buf, _bufsize, fileBytesReaded.QuadPart == fileSize.QuadPart))
		{
			// Нашли строку и в этот момент заполнен входной буффер,
			// сохранили позицию в буфере чтения, сброшена позиция фильтра
			filterPos = 0;
			filterAsteriskPos = NotFound;
			return true;
		}
	}
	return false;
}

void CLogReader::releaseFilter()
{
	if (filter != nullptr) {
		free(filter);
		filterPos = 0;
		filterAsteriskPos = NotFound;
	}
}

// Работа с буфером
bool CLogReader::workWithReadedPart(char* _buf, const int _bufsize, bool fileLastPart)
{
	int lineStartPos = bufferPos;
	bool stringNotMatched = false; // TODO: если кончится буфер, то скинется
	DWORD bufPosTmp = bufferPos; // Куда хотим откатиться, но если кончится буффер, тоже шляпа получится

	auto isFilterEnd = [&]() -> bool {
		// Определим, можно ли пройти фильтр до конца, если он не закончен
		char filterChar = filter[filterPos];
		if (filterChar == '*') {
			filterChar = filter[filterPos + 1];
		}
		return filterChar == NULL;
	};

	for (; bufferPos < bufferEnd; bufferPos++) {

		char filterChar = filter[filterPos];
		// Запоминаем позицию звездочки в фильтре
		if (filterChar == '*') {
			filterAsteriskPos = filterPos;
			filterPos++;
			filterChar = filter[filterPos];
			bufPosTmp = bufferPos;
		}

		char bufferChar = buffer[bufferPos];
		// Пропускаем возврат корретки перед переносом строки
		if (bufferChar == '\r' && bufferPos + 1 < bufferEnd && buffer[bufferPos + 1] == '\n') {
			bufferPos++;
			bufferChar = buffer[bufferPos];
		}

		if (bufferChar == '\n' ) { // конец строки
			// Конец строки + конец фильтра - это совпадение
			if (isFilterEnd()) {
				fillInputBuffer(lineStartPos,_buf, _bufsize, fileLastPart);
				bufferPos++;	
				return true;
			}
			else {
				// сбрасываем входной буфер - Обнуляем первый символ, чтобы запись пошла с начала
				_buf[0] = NULL;
				lineStartPos = bufferPos + 1;
			}
			filterPos = 0;
			filterAsteriskPos = NotFound;
			stringNotMatched = false;
		} else if( !stringNotMatched ) {
			if (bufferChar == filterChar || filterChar == '?') {
				// совпадение
				filterPos++;
			}
			else if (filterAsteriskPos == NotFound) {
				// символ не совпадает, предыдущей звездочки нет
				// строка уже не матчится, просто ищем конец строки
				stringNotMatched = true;
			}
			else if (filterAsteriskPos != NotFound) {
				// откатываемся к предыдущей звездочке в фильтре
				filterPos = filterAsteriskPos + 1;
				bufferPos = bufPosTmp++;
				if (bufferChar == filter[filterPos] || filterChar == '?') {
					filterPos++;
				}
			}
		}
	}

	// Обработаем конец файла
	if (fileLastPart && bufferPos == bufferEnd && !stringNotMatched && isFilterEnd()) {
		// Определим, можно ли пройти фильтр до конца, если он не закончен
		// Конец строки + конец фильтра - это совпадение
		fillInputBuffer(lineStartPos, _buf, _bufsize, fileLastPart);
		return true;
	}
	// прошли прочитанный буффер до конца, прихраним кусок строки во входной буффер
	fillInputBuffer(lineStartPos, _buf, _bufsize, fileLastPart);
	return false;
}

void CLogReader::fillInputBuffer(int lineStart, char* _buf, const int _bufsize, bool fileLastPart)
{
	int currentLength = strnlen_s(_buf, _bufsize);
	int freeSpace = _bufsize - currentLength - 1;

	if (freeSpace > 0) {
		int lineLenght = bufferPos - lineStart;
		int toCopy = (freeSpace < lineLenght) ? freeSpace : lineLenght;
		memcpy_s(&_buf[currentLength], freeSpace, &buffer[lineStart], toCopy);
		// окончание строки
		_buf[currentLength + toCopy] = NULL;
	}
}

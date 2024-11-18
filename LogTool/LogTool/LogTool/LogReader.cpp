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

	// �������������� ����� ��� ������
	if (buffer == nullptr)
	{
		bufferSize = 64 * 1024; // 64 ��
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

	// ��������� ����� ������� *
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
		return false; // �������� ������������ ������� ������
	}

	ZeroMemory(_buf, _bufsize);

	while (true) {
		// ������� ����� ���������, ��������� ��������� ����� �����
		if (bufferPos == bufferEnd)
		{
			if (!ReadFile(fileHandle, buffer, bufferSize, &bufferEnd, NULL)){
				printf_s("Error ReadFile");
				return false;
			}
			fileBytesReaded.QuadPart += bufferEnd;
			bufferPos = 0;
		}
		// ����� �����
		if (bufferEnd == 0) {
			return false;
		}
		// ������������ ����������� �����
		if (workWithReadedPart(_buf, _bufsize, fileBytesReaded.QuadPart == fileSize.QuadPart))
		{
			// ����� ������ � � ���� ������ �������� ������� ������,
			// ��������� ������� � ������ ������, �������� ������� �������
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

// ������ � �������
bool CLogReader::workWithReadedPart(char* _buf, const int _bufsize, bool fileLastPart)
{
	static bool isFilterMatched = false;
	int lineStartPos = bufferPos;
	bool stringNotMatched = false;

	for (; bufferPos < bufferEnd; bufferPos++) {

		char filterChar = filter[filterPos];
		// ���������� ������� ��������� � �������
		if (filterChar == '*') {
			filterAsteriskPos = filterPos;
			filterPos++;
			filterChar = filter[filterPos];
		}

		char bufferChar = buffer[bufferPos];
		// ���������� ������� �������� ����� ��������� ������
		if (bufferChar == '\r' && bufferPos + 1 < bufferEnd && buffer[bufferPos + 1] == '\n') {
			bufferPos++;
			bufferChar = buffer[bufferPos];
		}

		if (bufferChar == '\n' ) { // ����� ������
			// ���������, ����� �� ������ ������ �� �����, ���� �� �� ��������
			if (filterChar != NULL) {
				if (filterChar == '*')
					filterChar = filter[filterPos+1];
			}
			// ����� ������ + ����� ������� - ��� ����������
			if (filterChar == NULL) {
				fillInputBuffer(lineStartPos,_buf, _bufsize, fileLastPart);
				bufferPos++;	
				return true;
			}
			else {
				// ���������� ������� ����� - �������� ������ ������, ����� ������ ����� � ������
				_buf[0] = NULL;
				lineStartPos = bufferPos + 1;
			}
			filterPos = 0;
			filterAsteriskPos = NotFound;
			stringNotMatched = false;
		}
		else if( !stringNotMatched ) {
			if (bufferChar == filterChar || filterChar == '?') {
				// ����������
				filterPos++;
			}
			else if (filterAsteriskPos == NotFound) {
				// ������ �� ���������, ���������� ��������� ���
				// ������ ��� �� ��������, ������ ���� ����� ������
				stringNotMatched = true;
			}
			else if (filterAsteriskPos != NotFound) {
				// ������������ � ���������� ��������� � �������
				filterPos = filterAsteriskPos + 1;
				if (bufferChar == filter[filterPos] || filterChar == '?') {
					filterPos++;
				}
			}
		}
	}

	// ���������� ����� �����
	if (fileLastPart && bufferPos == bufferEnd && !stringNotMatched) {
		char filterChar = filter[filterPos];
		// ���������, ����� �� ������ ������ �� �����, ���� �� �� ��������
		if (filterChar != NULL) {
			if (filterChar == '*')
				filterChar = filter[filterPos + 1];
		}
		// ����� ������ + ����� ������� - ��� ����������
		if (filterChar == NULL) {
			fillInputBuffer(lineStartPos, _buf, _bufsize, fileLastPart);
			return true;
		}
	}
	// ������ ����������� ������ �� �����, ��������� ����� ������ �� ������� ������
	fillInputBuffer(lineStartPos, _buf, _bufsize, fileLastPart);
	return false;
}

void CLogReader::fillInputBuffer(int lineStart, char* _buf, const int _bufsize, bool fileLastPart)
{
	int bufferSize = _bufsize - 1;
	int bufferFreeSize = bufferSize - strnlen_s(_buf, _bufsize);

	if (bufferFreeSize > 0) {
		int inputBufoffset = bufferSize - bufferFreeSize;
		int lineLenght = bufferPos - lineStart;

		int toCopy = bufferFreeSize < lineLenght ? bufferFreeSize : lineLenght;
		memcpy_s(&_buf[inputBufoffset], bufferFreeSize, &buffer[lineStart], toCopy);
		// ��������� ������
		_buf[inputBufoffset + toCopy] = NULL;
	}
}

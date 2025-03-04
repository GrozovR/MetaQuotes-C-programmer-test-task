#pragma once

#include <windows.h>

class CLogReader final
{
public:
    CLogReader();
    ~CLogReader();

    bool Open(const char* filename);    // открытие файла, false - ошибка
    void Close();                        // закрытие файла

    bool SetFilter(const char* filter); // установка фильтра строк, false - ошибка
    bool GetNextLine(char* buf, const int bufsize); // запрос очередной найденной строки
    // buf - буфер, bufsize - максимальная длина, false - конец файла или ошибка

private:
    HANDLE fileHandle;             // хэндл к файлу
    LARGE_INTEGER fileSize;        // размер файла
    LARGE_INTEGER fileBytesReaded; // общее кол-во прочитанных байт

    char* filter;       // строка фильтра

    char* buffer;       // буфер для чтения из файла
    DWORD bufferSize;   // размер буфера
    DWORD bufferPos;    // позиция чтения в буфере
    DWORD bufferEnd;    // считано байт в буфере

    void releaseFilter();
    bool workWithReadedPart(char* _buf, const int _bufsize, bool fileLastPart);
    void fillInputBuffer(DWORD lineStart, char* _buf, const int _bufsize);
};
 
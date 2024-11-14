#pragma once

#include <stdio.h>

#include <windows.h>
#include <cstdio>
#include <cstring>


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

    void WorkInFile();

private:
    HANDLE fileHandle;  // хэндл к файлу
    char* filter;       // строка фильтра
    DWORD filterPos;    // позиция фильтра при проходе по строке
    char* buffer;       // буфер для чтения из файла
    DWORD bufferSize;   // размер буфера
    DWORD bufferPos;    // позиция чтения в буфере
    DWORD bufferEnd;    // считано байт в буфере

    bool workWithReadedPart(char* _buf, const int _bufsize);
    void fillInputBuffer(int lineStart, char* _buf, const int _bufsize);
    /*bool isMatched();


    size_t workWithLine(char* buf, const int bufsize, bool isEndLine = false);
    void getNextLine(char* buf, int bufsize);
    bool isMatch(char* buf, char* filter);
    */
};
 
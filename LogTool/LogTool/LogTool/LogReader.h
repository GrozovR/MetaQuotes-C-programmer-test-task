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

    bool Open(const char* filename);    // �������� �����, false - ������
    void Close();                        // �������� �����

    bool SetFilter(const char* filter); // ��������� ������� �����, false - ������
    bool GetNextLine(char* buf, const int bufsize); // ������ ��������� ��������� ������
    // buf - �����, bufsize - ������������ �����, false - ����� ����� ��� ������

    void WorkInFile();

private:
    HANDLE fileHandle;  // ����� � �����
    char* filter;       // ������ �������
    DWORD filterPos;    // ������� ������� ��� ������� �� ������
    char* buffer;       // ����� ��� ������ �� �����
    DWORD bufferSize;   // ������ ������
    DWORD bufferPos;    // ������� ������ � ������
    DWORD bufferEnd;    // ������� ���� � ������

    bool workWithReadedPart(char* _buf, const int _bufsize);
    void fillInputBuffer(int lineStart, char* _buf, const int _bufsize);
    /*bool isMatched();


    size_t workWithLine(char* buf, const int bufsize, bool isEndLine = false);
    void getNextLine(char* buf, int bufsize);
    bool isMatch(char* buf, char* filter);
    */
};
 
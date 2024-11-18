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

private:
    HANDLE fileHandle;  // ����� � �����
    DWORD fileSize;     // ������ �����
    DWORD fileBytesReaded; // ����� ���-�� ����������� ����

    char* filter;       // ������ �������
    int filterPos;      // ������� ������� ��� ������� �� ������
    int filterAsteriskPos; // ������� ��������� � �������

    char* buffer;       // ����� ��� ������ �� �����
    DWORD bufferSize;   // ������ ������
    DWORD bufferPos;    // ������� ������ � ������
    DWORD bufferEnd;    // ������� ���� � ������

    void releaseFilter();
    bool workWithReadedPart(char* _buf, const int _bufsize, bool fileLastPart);
    void fillInputBuffer(int lineStart, char* _buf, const int _bufsize, bool fileLastPart);
};
 
#pragma once

#include <windows.h>

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
    LARGE_INTEGER fileSize;     // ������ �����
    LARGE_INTEGER fileBytesReaded; // ����� ���-�� ����������� ����

    char* filter;       // ������ �������
    size_t filterPos;      // ������� ������� ��� ������� �� ������
    size_t filterAsteriskPos; // ������� ��������� � �������

    char* buffer;       // ����� ��� ������ �� �����
    DWORD bufferSize;   // ������ ������
    DWORD bufferPos;    // ������� ������ � ������
    DWORD bufferEnd;    // ������� ���� � ������
    DWORD questionMark;


    void releaseFilter();
    bool workWithReadedPart(char* _buf, const int _bufsize, bool fileLastPart);
    void fillInputBuffer(int lineStart, char* _buf, const int _bufsize, bool fileLastPart);
};
 
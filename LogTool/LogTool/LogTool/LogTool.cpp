#include "LogReader.h"
#include <stdio.h>

static const int ErrorCode = 1;

int main( int argc, char* argv[] )
{
    if (argc != 3) {
        printf_s("Expected arguments: filePath and mask");
        return ErrorCode;
    }

    CLogReader logReader;

    if (!logReader.Open(argv[1])) {
        printf_s("Incorrect filePath argument");
        return ErrorCode;
    }

    if (!logReader.SetFilter(argv[2])) {
        printf_s("Incorrect mask argument");
        return ErrorCode;
    }

    const int bufsize = 4096;
    char* buf = (char*) malloc(bufsize * sizeof(char));

    while (logReader.GetNextLine(buf, bufsize)) {
        printf_s("%s\n", buf);
    }
    free(buf);
    logReader.Close();
    return 0;
}

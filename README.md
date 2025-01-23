# MetaQuotes C++ programmer test task
Scroll down for russina language.
По-русски - скролльте вниз.
## C++ developer test task
Write a pure C++ class, which is able to read large text log files (hundreds of megabytes, tens of gigabytes) as fast as possible, and output lines that satisfy the conditions set by the simplest regular expressions: (at least operators * and ?, a wider range of features is welcome):

* character '*' - a sequence of any characters of unlimited length
* character '?' - any single character
* the class must correctly process masks: \*Some\*, \*Some, Some\*, \*\*\*\*\*Some\*\*\* - no restrictions on the position of '*' in the mask.
Search should return lines that satisfy the mask.

For example:

* Mask \*abc\* selects all lines that contain 'abc', beginning and ending with any sequence of characters.
* Mask abc\* selects all lines beginning with 'abc' and ending with any sequence of characters.
* Mask abc? selects all lines beginning with 'abc' and ending with any additional character.
* Mask abc selects all lines that are equal to this mask.

The class will not be extended and will not be a base class. The only task is to parse and to search for ANSI (non-Unicode) text in files.

The class should have the following public interface:
``` C++
class CLogReader
  {
public:
           CLogReader(...);
          ~CLogReader(...);

   bool    Open(...);                       // open the file, false - false
   void    Close();                         // close the file

   bool    SetFilter(const char *filter);   // set the filter, false - false
   bool    GetNextLine(char *buf,           // request the next found line
                       const int bufsize);  // buf - buffer, bufsize - max length
                                            // false - file end or error
  };
```

Requirements for implementation:
* Platform: Windows 7 and higher
* Functionality: maximum performance (caching of search results and file caching are not required)
* Resources: memory consumption must be minimal (reasonable)
* Components: the use of third-party libraries (including STL) and components is not allowed Only WinAPI and CRT are allowed for use
* Exceptions: the use of Windows and C++ exceptions is not allowed
* The code should be absolutely foolproof and robust

Requirements for code style:
* The code should be as simple as possible
* The code should be as clean, elegant and understandable as possible
* The result must contain the created class (cpp + h files) and a command-line usage example (open a 1-2 MB file and draw a sample of "order*closed"). The file and selection string should be specified in the example's arguments.
* The command-line example should receive the path to the text file (the first parameter) and the filter (the second parameter) as command line arguments, and output the entire lines found to the console.  
Example: ```tool.exe 20190102.log *bbb*```
* The project should be implemented for building in MS Visual Studio 2015 and higher (all in one archive).


## Тестовое задание C++ программиста
Необходимо написать на чистом С++ класс, умеющий максимально быстро читать текстовые лог-файлы огромных размеров (сотни мегабайт, десятки гигабайт) и выдавать строки, удовлетворяющие условиям простейшего regexp: (как минимум операторы * и ?, более широкий набор возможностей приветствуется):
* cимвол '\*' - последовательность любых символов неограниченной длины;
* cимвол "?" - один любой символ;
* должны корректно отрабатываться маски: \*Some\*, \*Some, Some\*, \*\*\*\*\*Some\*\*\* - нет никаких ограничений на положение \* в маске.
  
Результатом поиска должны быть строки, удовлетворяющие маске.
Например:

* Маска \*abc\*  отбирает все строки содержащие abc и начинающиеся и заканчивающиеся любой последовательностью символов.
* Маска abc\*  отбирает все строки начинающиеся с abc и заканчивающиеся любой последовательностью символов.
* Маска abc?  отбирает все строки начинающиеся с abc и заканчивающиеся любым дополнительным символом.
* Маска abc   отбирает все строки которые равны этой маске.

Класс не будет расширяться и не будет базовым классом. Единственная задача: сканирование и поиск текстовых ANSI (не unicode) строк в файлах.
Класс должен иметь следующий публичный интерфейс:
``` C++
class CLogReader
  {
public:
           CLogReader(...);
          ~CLogReader(...);

   bool    Open(...);                       // открытие файла, false - ошибка
   void    Close();                         // закрытие файла

   bool    SetFilter(const char *filter);   // установка фильтра строк, false - ошибка
   bool    GetNextLine(char *buf,           // запрос очередной найденной строки,
                       const int bufsize);  // buf - буфер, bufsize - максимальная длина
                                            // false - конец файла или ошибка
  };
```
### Требование к реализации:
* Платформа: Windows 7 и выше;
* Функциональность: максимальная производительность (кэширование результатов поиска и файла не требуется);
* Ресурсы: затраты памяти должны быть минимальны (в разумных пределах);
* Компоненты: запрещено использование сторонних библиотек (в том числе STL) и компонентов. Разрешено использовать только WinAPI и CRT;
* Исключения: нельзя использовать Windows и С++ исключения;
* Код должен быть абсолютно «неубиваемый» и защищённым от ошибок.
* Результатом должен быть готовый класс (cpp + h файлы) и консольный пример (открыть текстовый файл в пару мегабайт и сделать выборку "order*closed") его использования. Файл и строка выбора должна указываться в аргументах примера;
* Консольный пример должен получать в качестве параметров командной строки путь к текстовому файлу (первый параметр) и фильтр строк (второй параметр) и выводить в консоль найденные строки целиком.  
Пример строки запуска:```tool.exe 20190102.log *bbb*```

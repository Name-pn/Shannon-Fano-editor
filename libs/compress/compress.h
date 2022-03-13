#ifndef INC_COMPRESS_H
#define INC_COMPRESS_H

#include <stdio.h>
#include <string.h>
#include <windows.h>

#define MAX_CODE 10 // Максимальное число байтов в коде сжатого символа
#define ASCII_SIZE 256 // Размер исходного алфавита
#define COMPRESS_END_CHAR '\0'


// Элемент массива
struct symbol {
    unsigned char c; //Символ
    long double p;
    // Частота встречи n1 / N, n1 число cимволов в тексте, N длина строки
    int n; // Размер кода сжатого символа code в байтах
    char code[MAX_CODE];
};

// Чтобы пропускать struct в заголовке функций
#define symbol struct symbol

// Записывает в a число вхождений i-го символа ASCII в string
void entry_arr(int *a, unsigned char* string, int n);

// Возвращает мощность алфавита по массиву вхождений a
int alphabet(int const* a);

// Конструктор алфавита сжатия
int init_struct_arr(symbol *a, char * s, int n);

// Возвращает число символов записанных в compressive\_string
// сжатой string по массиву a размера alp
int compress(unsigned char* string, int n, unsigned char* compressive_string, symbol* a, int alp, int *bytes);

/*// Сжимает string, записывает результат в compressive_string, возвращает число, записанных байтов
int up_compress(unsigned char* string, unsigned char* compressive_string);*/

// Выводит двоичное представление a размера size байтов
void print_var (void *a, size_t size);

// Выводит символы и их сжатые битовые коды
void write_alp(symbol *a, int alp);

// Получение строки длины n из кода code в start\_string по a размера alp
int unpack(unsigned char* code, unsigned char* start_string, symbol* a, int alp);

#endif //INC_COMPRESS_H

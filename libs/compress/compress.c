#include "compress.h"

// Записывает в a число вхождений i-го символа ASCII в string
void entry_arr(int *a, unsigned char* string, int n) {
    for (int i = 0; i < ASCII_SIZE; ++i) {
        a[i] = 0;
    }
    for (int i = 0; i < n; ++i) {
        ++a[string[i]];
    }
}

// Возвращает мощность алфавита по массиву вхождений a
int alphabet(int const* a) {
    int r = 0;
    for (int i = 0; i < ASCII_SIZE; ++i) {
        if (a[i])
            ++r;
    }
    return r;
}

// Устанавливает вероятности символов string в a
void set_possible(symbol* a, int const* b, int n) {
    int k = 0;
    for (int i = 0; i < ASCII_SIZE; ++i) {
        if (b[i]) {
            a[k].c = i;
            a[k++].p = ((double)b[i]) / n;
        }
    }
}

// Сортирует элементы a размера n по невозрастанию выбором по их вероятностям
void sort_possible(symbol* a, int n) {
    symbol t;
    int maxi;
    for (int i = 0; i < n; ++i) {
        maxi = i;
        for (int j = i + 1; j < n; ++j) {
            if (a[maxi].p < a[j].p)
                maxi = j;
        }
        t = a[maxi];
        a[maxi] = a[i];
        a[i] = t;
    }
}

// Устанавливает биту bite строки code значение с
void set_bite(unsigned char* code, int bite, unsigned char c) {
    int byte = bite / 8;
    unsigned char shift = bite % 8;
    unsigned char l = 0xff << (shift + 1);
    unsigned char r = 0xff >> (7 - shift + 1);
    if (code[byte] & 1 << shift)
        code[byte] = code[byte] & (c << shift | l | r);
    else
        code[byte] = code[byte] | (c << shift);
}

// Возвращает вероятность встречи символа из части алфавита a с i1 по i2 символ
double sum_change(symbol* a, int i1, int i2) {
    double res = 0;
    for (int i = i1; i <= i2; ++i) {
        res += a[i].p;
    }
    return res;
}

// Разбивает массив a с i1 по i2 элементы
void alg_step(symbol* a, int i1, int i2) {
    double k = sum_change(a, i1, i2) / 2;
    double sum = a[i1].p;
    int m = i1 + 1; //
    while (m < i2 && sum < k) {
        sum += a[m].p;
        ++m;
    }
    for (int i = i1; i < m; ++i) {
        set_bite((unsigned char*)a[i].code, a[i].n, 0);
        ++a[i].n;
    }
    for (int i = m; i <= i2; ++i) {
        set_bite((unsigned char*)a[i].code, a[i].n, 1);
        ++a[i].n;
    }
    if (m < i2) {
        alg_step(a, m, i2);
    }
    if (i1 + 1 != m)
        alg_step(a, i1, m - 1);
}

// Получает коды для символов a, которых n
void set_code(symbol* a, int alp) {
    alg_step(a, 0, alp - 1);
}

// Инициализирует поля битовых длин массива a размера alp
void init_symb(symbol *a, int alp) {
    for (int i = 0; i < alp; ++i) {
        a[i].n = 0;
    }
}

// Конструктор алфавита сжатия
int init_struct_arr(symbol *a, char * s, int n) {
    int b[ASCII_SIZE];
    entry_arr(b, (unsigned char *)s, n);
    int alp = alphabet(b);
    init_symb(a, alp);
    set_possible(a, b, n);
    sort_possible(a, alp);
    set_code(a, alp);

    return alp;
}

// Выводит двоичное представление a
void print_byte(unsigned char a) {
    unsigned char m = ~(((unsigned char)(~0u)) >> 1);
    while (m != 0) {
        if (m & a)
            printf("1");
        else
            printf("0");
        m >>= 1;
    }
}

// Выводит двоичное представление a размера size байтов
void print_var (void *a, size_t size) {
    unsigned char *c = (unsigned char *)a;
    for (size_t i = 0; i < size; ++i) {
        print_byte(*c);
        printf(" ");
        ++c;
    }
    printf("\n");
}

// Выводит символы и их сжатые битовые коды
void write_alp(symbol *a, int alp) {
    int lim;
    for (int i = 0; i < alp; ++i) {
        lim = a[i].n / 8;
        if (a[i].c != '\n') {
            printf("Символ '%c', размер кода %i бит, код:\n", a[i].c, a[i].n);
        }
        else {
            printf("Символ '\\n', размер кода %i бит, код:\n", a[i].n);
        }
        print_var(a[i].code, lim + 1);
    }
}

// Возвращает индекс символа c в a размера alp, или -1 в случае его отсутствия
int find_char(symbol* a, int alp, unsigned char c) {
    for (int i = 0; i < alp; ++i) {
        if (c == a[i].c)
            return i;
    }
    return -1;
}

// Перемещает на один бит указатели i\_bytes и i\_bites
void bit_step(int* i_bytes, int* i_bites) {
    ++*i_bites;
    if (*i_bites >= 8) {
        *i_bites = 0;
        ++*i_bytes;
    }
}

// Возвращает бит bite строки code
unsigned char get_bite(const char* code, int bite) {
    int byte = bite / 8;
    char tmp = (char)(code[byte] & (1u << bite % 8));
    return tmp != 0;
}

// Записывает битовый код from в compressive\_string с позиции i\_bytes и i\_bites
void insert_binary_code(unsigned char* compressive_string, symbol* from, int* i_bytes, int* i_bites) {
    for (int j = 0; j < from->n; ++j) {
        set_bite(compressive_string, *i_bytes*8+*i_bites, get_bite(from->code, j));
        bit_step(i_bytes, i_bites);
    }
}

// Возвращает число символов записанных в compressive\_string
// сжатой string по массиву a размера alp
int compress(unsigned char* string, int n, unsigned char* compressive_string, symbol* a, int alp, int *bytes) {
    int i; // Указатель на символ алфавита a
    int i_bytes = 0;
    int i_bites = 0;
    for (int cir_i = 0; cir_i < n; ++cir_i) {
        i = find_char(a, alp, string[cir_i]);
        if (i != -1)
            insert_binary_code(compressive_string, &(a[i]), &i_bytes, &i_bites);
    }

    *bytes = i_bytes + 1;
    return(n);
}

// Определение совпадения кода code c i\_bites бита, байта i\_bytes с кодом a
int compare(symbol a, unsigned char const * code, int i_bytes, int i_bites) {
    for (int i = 0; i < a.n; ++i) {
        // i бит буквы не совпадает с соответствующим битом в строке
        if (get_bite(a.code, i) != (((code[i_bytes]) & (1 << ((i_bites)))) != 0))
            return 0;
        bit_step(&i_bytes, &i_bites);
    }
    return 1;
}

// Читает символы из code с c i\_bites бита, байта i\_bytes по их коду из a размера alp
int read_char(unsigned char* code, int *i_bytes, int *i_bites, symbol* a, int alp) {
    for (int i = 0; i < alp; ++i) {
        if (compare(a[i], code, *i_bytes, *i_bites)) {
            for (int j = 0; j < a[i].n; ++j) {
                bit_step(i_bytes, i_bites);
            }
            return (unsigned char) a[i].c;
        }
    }
    return -1;
}

// Получение строки длины n из кода code в start\_string по a размера alp
int unpack(unsigned char* code, unsigned char* start_string, symbol* a, int alp) {
    int i = 0;
    int c; // Код считанного символа или -1
    int i_bytes, i_bites;
    i_bytes = i_bites = 0;

    do {
        c = read_char(code, &i_bytes, &i_bites, a, alp);
        if (c != -1)
            start_string[i++] = c;
    } while (c != COMPRESS_END_CHAR);
    // Условие остановки достижение '\0'

    start_string[i] = '\0';
}
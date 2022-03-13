#include "compress.h"

// ���������� � a ����� ��������� i-�� ������� ASCII � string
void entry_arr(int *a, unsigned char* string, int n) {
    for (int i = 0; i < ASCII_SIZE; ++i) {
        a[i] = 0;
    }
    for (int i = 0; i < n; ++i) {
        ++a[string[i]];
    }
}

// ���������� �������� �������� �� ������� ��������� a
int alphabet(int const* a) {
    int r = 0;
    for (int i = 0; i < ASCII_SIZE; ++i) {
        if (a[i])
            ++r;
    }
    return r;
}

// ������������� ����������� �������� string � a
void set_possible(symbol* a, int const* b, int n) {
    int k = 0;
    for (int i = 0; i < ASCII_SIZE; ++i) {
        if (b[i]) {
            a[k].c = i;
            a[k++].p = ((long double)b[i]) / n;
        }
    }
}

// ��������� �������� a ������� n �� ������������� ������� �� �� ������������
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

// ������������� ���� bite ������ code �������� �
void set_bite(unsigned char* code, int bite, unsigned char c) {
    int byte = bite / 8;
    unsigned char shift = ((char)bite) % 8;
    unsigned char l = 0xff << (shift + 1);
    unsigned char r = 0xff >> (7 - shift + 1);
    if (code[byte] & 1 << shift)
        code[byte] = code[byte] & (c << shift | l | r);
    else
        code[byte] = code[byte] | (c << shift);
}

/*// ������������� ���� bite ������ code �������� �
void set_bite(char* code, int bite, unsigned char c) {
    int byte = bite / 8;
    unsigned char shift = (char)bite % 8;
    if (code[byte] & (1 << bite % 8))
        code[byte] = (char)((code[byte]) & ~((~c) << shift));
    else
        code[byte] = (char)(code[byte] | (c << shift));
}*/

// ���������� ����������� ������� ������� �� ����� �������� a � i1 �� i2 ������
long double sum_change(symbol* a, int i1, int i2) {
    long double res = 0;
    for (int i = i1; i <= i2; ++i) {
        res += a[i].p;
    }
    return res;
}

// ��������� ������ a � i1 �� i2 �������� ����� ������� ����� ����� ������������
// ������ ����� ���� ������ k
void alg_step(symbol* a, int i1, int i2) {
    long double k = sum_change(a, i1, i2) / 2;
    long double sum = a[i1].p;
    int m = i1 + 1; //
    while (m < i2 && sum < k) {
        sum += a[m].p;
        ++m;
    }
    for (int i = i1; i < m; ++i) {
        set_bite(a[i].code, a[i].n, 0);
        ++a[i].n;
    }
    for (int i = m; i <= i2; ++i) {
        set_bite(a[i].code, a[i].n, 1);
        ++a[i].n;
    }
    if (m < i2) {
        alg_step(a, m, i2);
    }
    if (i1 + 1 != m)
        alg_step(a, i1, m - 1);
}

// �������� ���� ��� �������� a, ������� n
void set_code(symbol* a, int alp) {
    alg_step(a, 0, alp - 1);
}

// �������������� ���� ������� ���� ������� a ������� alp
void init_symb(symbol *a, int alp) {
    for (int i = 0; i < alp; ++i) {
        a[i].n = 0;
        a[i].p = 0;
        for (int j = 0; j < MAX_CODE; ++j) {
            a[i].code[i] = 0;
        }
    }
}

// ����������� �������� ������
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

// ���������� ������ ������� c � a ������� alp
int find_char(symbol* a, int alp, unsigned char c) {
    for (int i = 0; i < alp; ++i) {
        if (c == a[i].c)
            return i;
    }
    return -1;
}

void bit_step(int* i_bytes, int* i_bites) {
    ++*i_bites;
    if (*i_bites >= 8) {
        *i_bites = 0;
        ++*i_bytes;
    }
}

// ���������� ��� bite ������ code
unsigned char get_bite(const char* code, int bite) {
    int byte = bite / 8;
    char tmp = (char)(code[byte] & (1u << bite % 8));
    return tmp != 0;
}

void insert_binary_code(unsigned char* compressive_string, symbol* from, int* i_bytes, int* i_bites) {
    for (int j = 0; j < from->n; ++j) {
        set_bite(compressive_string, *i_bytes*8+*i_bites, get_bite(from->code, j));
        bit_step(i_bytes, i_bites);
    }
}

// ���������� ����� �������� ���������� � compressive\_string
// ������ string �� ������� a ������� alp
int compress(unsigned char* string, int n, unsigned char* compressive_string, symbol* a, int alp, int *bytes) {
    int i; // ��������� �� ������ �������� a
    int i_bytes = 0;
    int i_bites = 0;
    for (int cir_i = 0; cir_i < n; ++cir_i) {
        i = find_char(a, alp, string[cir_i]);
        if (i != -1)
            insert_binary_code(compressive_string, &(a[i]), &i_bytes, &i_bites);
        else {
            fprintf(stderr, "����������� ������, ������� ������ ����� �� �� ��������!\n");
            exit(-1);
        }
    }

    *bytes = i_bytes + 1;
    return(n);
}

/*// ������� string, ���������� ��������� � compressive_string, ���������� �����, ���������� ������
int up_compress(unsigned char* string, unsigned char* compressive_string) {
    int alp, n, bytes;
    n = (int)strlen((char*)string) + 1;
    int b[ASCII_SIZE];
    entry_arr(b, (unsigned char*)string, n);
    alp = alphabet(b);
    symbol symb[alp];
    init_struct_arr(symb, (char*)string, n);
    compress((unsigned char*)string, n, (unsigned char*)string, symb, alp, &bytes);
    return(n);
}*/

// ������� �������� ������������� a
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

// ������� �������� ������������� a ������� size ������
void print_var (void *a, size_t size) {
    unsigned char *c = (unsigned char *)a;
    for (size_t i = 0; i < size; ++i) {
        print_byte(*c);
        printf(" ");
        ++c;
    }
    printf("\n");
}

// ������� ������� � �� ������ ������� ����
void write_alp(symbol *a, int alp) {
    int lim;
    for (int i = 0; i < alp; ++i) {
        lim = (a[i].n - 1) / 8;
        if (a[i].c != '\n') {
            if (a[i].c != '\0')
                printf("������ '%c', ������ ���� %i ���, ���:\n", a[i].c, a[i].n);
            else
                printf("������ '\\0', ������ ���� %i ���, ���:\n", a[i].n);
        }
        else {
            printf("������ '\\n', ������ ���� %i ���, ���:\n", a[i].n);
        }
        print_var(a[i].code, lim + 1);
    }
}

// �������� ��������� i_bytes � i_bites ����� �������, ����� ��� ���������� �� ��������� ���
void step_bite(int *i_bytes, int *i_bites) {
    if (*i_bites < 7)
        ++*i_bites;
    else {
        ++*i_bytes;
        *i_bites = 0;
    }
}

// ����������� ���������� ���� code c i\_bites ����, ����� i\_bytes � ����� a
int compare(symbol *a, unsigned char const * code, int i_bytes, int i_bites) {
    for (int i = 0; i < a->n; ++i) {
        //(((code[i_bytes]) & (1 << ((i_bites)))) != 0)???
        if (get_bite(a->code, i) != get_bite((char *)code, i_bytes*8+i_bites)) // i ��� ����� ��������� � ��������������� ����� � ������
            return 0;
        step_bite(&i_bytes, &i_bites);
    }
    return 1;
}

// ������ ������ �� code � c i\_bites ����, ����� i\_bytes �� �� ���� �� a ������� alp
int read_char(unsigned char* code, int *i_bytes, int *i_bites, symbol* a, int alp) {
    for (int i = 0; i < alp; ++i) {
        if (compare(a+i, code, *i_bytes, *i_bites)) {
            for (int j = 0; j < a[i].n; ++j) {
                step_bite(i_bytes, i_bites);
            }
            return (unsigned char) a[i].c;
        }
    }
    return -1;
}

// ��������� ������ ����� n �� ���� code � start\_string �� a ������� alp
int unpack(unsigned char* code, unsigned char* start_string, symbol* a, int alp) {
    int i = 0;
    int c; // ��� ���������� ������� ��� -1
    int i_bytes, i_bites;
    i_bytes = i_bites = 0;

    do {
        c = read_char(code, &i_bytes, &i_bites, a, alp);
        if (c != -1)
            start_string[i] = c;
        else {
            fprintf(stderr, "����������� ������ ����������, ���������� ���������� ������ �� ����\n");
            //exit(-1);
        }
        ++i;
    } while (c != COMPRESS_END_CHAR && c != -1);

    start_string[i] = '\0';
    return i_bytes;
}
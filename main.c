#include "libs/compress/compress.h"
#include <string.h>
#include <io.h>

const char* CONFIG_NAME = "compress_config.txt";
const char END_CHAR = '~';

#define NAME_MAX 255
#define MAX_FILES 100
#define FILE_MAX_SIZE 10000
#define FILE_ROW_SIZE 255

// Структура с именами всех созданных файлов
struct config_st {
    char file_names[MAX_FILES][NAME_MAX]; // Имена файлов
    int count; // Число файлов
};

// Загрузить данные fs в файл конфигурации f
void read_files(struct config_st* fs, FILE* f) {
    int start = ftell(f);

    fseek(f, 0, SEEK_SET);
    fread(fs, sizeof(struct config_st), 1, f);
    fseek(f, start, SEEK_SET);
}

// Сбросить данные fs в файл конфигурации f
void push_in_file(struct config_st* fs, FILE* f) {
    int start = ftell(f);

    fseek(f, 0, SEEK_SET);
    fwrite(fs, sizeof(struct config_st), 1, f);
    fseek(f, start, SEEK_SET);
}

// Сохраняет fs в долговременной памяти
void save_config(struct config_st* fs) {
    FILE* f = fopen(CONFIG_NAME, "rb+");
    push_in_file(fs, f);
    fclose(f);
}

// Вывести имена созданных файлов из fs
void write_list(struct config_st* fs) {
    if (!fs->count)
        printf("Файлы не обнаружены\n");
    for (int i = 0; i < fs->count; ++i) {
        printf("Файл №%i: ", i + 1);
        fputs(fs->file_names[i], stdout);
    }
    putc('\n', stdout);
}

// Возвращает значение 1, если в fs есть строка file, иначе возвращает 0
int find_file(struct config_st* fs, char* file) {
    for (int i = 0; i < fs->count; ++i) {
        if (!strcmp(fs->file_names[i], file))
            return 1;
    }
    return 0;
}

// Записывает в f1 и f2 имена файлов, добавляя к s расширение сжатого файла для f1,
// и расширение служебного файла для f2
void get_files(char* s, char f1[NAME_MAX], char f2[NAME_MAX]) {
    sscanf(s, "%s", f1);
    sscanf(s, "%s", f2);
    strcat(f1, ".bin");
    strcat(f2, ".mem");
}

// Перезаписывает файлы записки имени s
void rewrite(char* s) {
    printf("Вводите содержимое файла, окончите ввод файла символом '~' в начале новой строки:\n");

    char str_f[FILE_MAX_SIZE] = {0};
    char new_str[FILE_MAX_SIZE] = {0};

    str_f[0] = '\0';

    //Чтение текста до конца ввода
    char buf[FILE_ROW_SIZE] ;
    char* error = fgets(buf, FILE_ROW_SIZE, stdin);
    while (buf[0] != END_CHAR && error != NULL) {
        strcat(str_f, buf);
        error = fgets(buf, FILE_ROW_SIZE, stdin);
    }
    printf("Прочитана строка:\n");
    puts(str_f);
    // Сжатие
    int alp, n, bytes;
    n = (int)strlen(str_f) + 1;
    symbol symb[ASCII_SIZE];
    alp = init_struct_arr(symb, str_f, n);
    write_alp(symb, alp);
    compress((unsigned char*)str_f, n, (unsigned char*)new_str, symb, alp, &bytes);
    printf("Код зашифрованной строки:\n");
    print_var(new_str, bytes);

    printf("Зашифрована строка:\n");
    puts(new_str);
    char res[FILE_MAX_SIZE];
    unpack(new_str, res, symb, alp);
    printf("Рас. строка:\n");
    puts(res);
    // Запись в файлы
    char f_name[NAME_MAX];
    char f_name2[NAME_MAX];

    get_files(s, f_name, f_name2);

    FILE* new_f = fopen(f_name, "w");
    if (new_f == NULL) {
        fprintf(stderr, "Ошибка открытия файла для записи сжатого кода\n");
        return;
    }
    for (int i = 0; i < bytes; ++i) {
        fputc(new_str[i], new_f);
    }
    fclose(new_f);

    new_f = fopen(f_name2, "wb");
    if (new_f == NULL) {
        fprintf(stderr, "Ошибка открытия файла для записи алфавита\n");
        return;
    }

    fwrite(&alp, sizeof(alp), 1, new_f);
    fwrite(symb, sizeof(symbol), ASCII_SIZE, new_f);
    fclose(new_f);
}

// Ищет s в fs, если находит возвращает номер индекса + 1, иначе 0
int find_fname(struct config_st* fs, char* s) {
    for (int i = 0; i <fs->count; ++i) {
        if (!strcmp(s, fs->file_names[i]))
            return i+1;
    }
    return 0;
}

// Создает новую записку, использует список записок fs и файл конфигурации f
void create_new(struct config_st* fs) {
    if (fs->count + 1 < MAX_FILES) {

        printf("Введите название нового файла\n");
        char f_name[NAME_MAX];
        fgets(f_name, NAME_MAX, stdin);

        int k = find_fname(fs, f_name);
        if (k) {
            fprintf(stderr, "Вызвано создание уже существующего файла\n");
            return;
        }

        strcpy(fs->file_names[fs->count], f_name);
        rewrite(fs->file_names[fs->count]);
        fs->count = fs->count + 1;
        save_config(fs);
    } else
        fprintf("stderr", "Ошибка превышения лимита числа файлов\n");
}

// Выводит содержимое записки s
void write_file(char* s) {
    char file1[NAME_MAX] = {0};
    char file2[NAME_MAX] = {0};

    get_files(s, file1, file2);

    FILE* f1 = fopen(file1, "r");
    FILE* f2 = fopen(file2, "rb");
    int alp;

    fread(&alp, sizeof(int), 1, f2);
    symbol symb[ASCII_SIZE];
    fread(symb, sizeof(symbol), ASCII_SIZE, f2);

    write_alp(symb, alp);

    char code[FILE_MAX_SIZE];
    char res[FILE_MAX_SIZE];
    int i = 0;
    char c;
    fscanf(f1, "%c", &c);
    int kk = 0;
    while (!feof(f1)) {
        code[i++] = c;
        fscanf(f1, "%c", &c);
        kk++;
    }
    print_var(code, kk);
    code[i] = 0;

    printf("Считанная зашифрованная строка:\n");
    puts(code);
    unpack(code, res, symb, alp);

    printf("Считанная расшифрованная строка:\n");
    puts(res);
    int k = 0;
    while (res[k] != '\0')
        putc(res[k++], stdout);
    fclose(f1);
    fclose(f2);
}

// Удаляет записку s из файла с перечислением записок f
void delete_name(char* s, FILE* f) {
    int uk_start = ftell(f);
    rewind(f);
    char row[FILE_ROW_SIZE];
    row[0] = 'a';

    int uk_w = 0;
    int uk_r = 0;
    char* error;
    while (!feof(f) && row[0] != 0) {
        error = fgets(row, FILE_ROW_SIZE, f);
        if (error != NULL) {
            uk_r = ftell(f);
            if (strcmp(row, s)) {
                fseek(f, SEEK_SET, uk_w);
                fputs(row, f);
                uk_w = ftell(f);
                fseek(f, SEEK_SET, uk_r);
            }
        }
    }
    if (chsize(fileno(f), uk_w) == -1)
        fprintf(stderr, "Ошибка удаления имени файла из архива\n");
    fflush(f);
}

// Удаляет записку под номером num в fs из f и удаляет связанные с ней файлы
void delete_from_config(struct config_st* fs, int num) {
    //Удаление физических файлов
    char f1[NAME_MAX], f2[NAME_MAX];
    get_files(fs->file_names[num], f1, f2);

    int error;
    error = remove(f1);
    if (error)
        fprintf(stderr, "Ошибка удаления файла\n");

    error = remove(f2);
    if (error)
        fprintf(stderr, "Ошибка удаления файла\n");

    //Удаление из переменной копированием
    strcpy(fs->file_names[num], fs->file_names[fs->count - 1]);
    fs->file_names[fs->count-1][0] = '\0'; // Подтираем строку

    --fs->count;

    save_config(fs);
}

// Правильные ввод, пока пользователь не введет число меньшее или равное lim
// и большее нуля запрашивает у него число
int right_input(int lim) {
    int x;
    scanf("%i", &x);
    while (x > lim || x <= 0) {
        fprintf(stderr, "Некорректное число\n");
        scanf("%i", &x);
    }
    return x - 1;
}

enum codes {final = 0, write_files, write_it, create, rewrite_it, delete_it};

// Обеспечивает интерфейс для управления пользователя. fs список созданных заметок,
//f файл со списком заметок.
void menu(struct config_st* fs) {
    int flag = 1;
    int error;
    int num;
    while (flag) {

        printf("Команды:\n"
               "Для просмотра списка файлов нажмите 1.\n"
               "Для вывода содержимого файла нажмите 2\n"
               "Для создания нового файла нажмите 3.\n"
               "Для перезаписи нажмите 4.\n"
               "Для удаления файла нажмите 5\n"
               "Для выхода нажмите 0.\n");
        error = scanf("%i", &flag);
        fflush(stdin); // Если уберешь, '\n' запишется куда-нибудь не туда
        switch(flag) {
            case write_files:
                write_list(fs);
                break;
            case write_it:
                if (!fs->count) {
                    fprintf(stderr, "Попытка чтения файлов при их отсутствии\n");
                } else {
                    write_list(fs);
                    printf("Введите номер файла для чтения\n");
                    scanf("%i", &num);
                    write_file(fs->file_names[--num]);
                }
                break;
            case create:
                create_new(fs);
                break;
            case rewrite_it:
                if (!fs->count) {
                    fprintf(stderr, "Попытка перезаписи файлов при их отсутствии\n");
                } else {
                    write_list(fs);
                    printf("Введите номер перезаписываемого файла\n");
                    num = right_input(fs->count);
                    fflush(stdin);
                    rewrite(fs->file_names[num]);
                }
                break;
            case delete_it:
                if (!fs->count) {
                    fprintf(stderr, "Попытка удаления файлов при их отсутствии\n");
                } else {
                    write_list(fs);
                    printf("Введите номер удаляемого файла\n");
                    num = right_input(fs->count);
                    delete_from_config(fs, num);
                    fflush(stdin);
                }
        }
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    printf("Программа редактор сжатых текстовых записок приветствует вас.\n");

    FILE* f;
    f = fopen(CONFIG_NAME, "rb+");
    struct config_st config;
    if (f == NULL)
        f = fopen(CONFIG_NAME, "wb+");
    else
        read_files(&config, f);
    fclose(f);
    menu(&config);
    return 0;
}
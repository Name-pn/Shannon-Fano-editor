// Еще нам пригодится
#include "libs/compress/compress.h"

// Имя для файла конфигурации
const char* CONFIG_NAME = "compress_config.txt";

// Максимальная длина имени файла
#define NAME_MAX 255
// Максимальное число записок
#define MAX_FILES 100
#define FILE_MAX_SIZE 10000 // Максимальное число символов в файле
#define FILE_ROW_SIZE 255 // Максимальное число символов в строке ф.
#define END_CHAR '~' // Конец ввода текста

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

// Записывает в f1 и f2 имена файлов, добавляя к s расширение бин. файла для f1,
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

    char str_f[FILE_MAX_SIZE];
    char new_str[FILE_MAX_SIZE];

    str_f[0] = '\0';

    //Чтение текста до конца ввода
    char buf[FILE_ROW_SIZE];
    char* error = fgets(buf, FILE_ROW_SIZE, stdin);
    while (buf[0] != END_CHAR && error != NULL) {
        strcat(str_f, buf);
        error = fgets(buf, FILE_ROW_SIZE, stdin);
    }

    // Сжатие
    int n, bytes;
    n = (int)strlen(str_f) + 1;
    symbol symb[ASCII_SIZE];
    int alp = init_struct_arr(symb, str_f, n);

    compress((unsigned char*)str_f, n, (unsigned char*)new_str, symb, alp, &bytes);

    // Запись в файлы
    char f_name[NAME_MAX];
    char f_name2[NAME_MAX];

    get_files(s, f_name, f_name2);

    FILE* new_f = fopen(f_name, "wb"); // wb - перезапись в бинарном режиме
    // Записать bytes байтов new_str в new_f
    fwrite(new_str, sizeof(char), bytes, new_f);
    fclose(new_f);

    new_f = fopen(f_name2, "wb");

    fwrite(&alp, sizeof(int), 1, new_f);
    fwrite(symb, sizeof(symbol), alp, new_f);
    fclose(new_f);
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

// Ищет s в fs, если находит возвращает номер индекса + 1, иначе 0
int find_fname(struct config_st* fs, char* s) {
    for (int i = 0; i <fs->count; ++i) {
        if (!strcmp(s, fs->file_names[i]))
            return i+1;
    }
    return 0;
}

// Создает новую записку, добавляет ее в список записок fs
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
        fprintf(stderr, "Ошибка превышения лимита числа файлов\n");
}

// Выводит содержимое записки s
void write_file(char* s) {
    char file1[NAME_MAX] = {0};
    char file2[NAME_MAX] = {0};

    // Получение имен с расширениями
    get_files(s, file1, file2);

    FILE* f1 = fopen(file1, "rb");
    FILE* f2 = fopen(file2, "rb");

    // Чтение размера алфавита, а затем самого алфавита
    int alp;
    fread(&alp, sizeof(int), 1, f2);

    symbol symb[ASCII_SIZE];
    fread(symb, sizeof(symbol), ASCII_SIZE, f2);

    // Чтение сжатой строки и ее распаковка
    char code[FILE_MAX_SIZE];
    char res[FILE_MAX_SIZE];
    int i = 0;
    char c;
    fscanf(f1, "%c", &c);

    while (!feof(f1)) {
        code[i++] = c;
        fscanf(f1, "%c", &c);
    }
    code[i] = 0;

    unpack((unsigned char*)code, (unsigned char*)res, symb, alp);

    // Посимвольный вывод
    int k = 0;
    while (res[k] != '\0')
        putc(res[k++], stdout);

    // Закрытие файлов
    fclose(f1);
    fclose(f2);
}

// Удаляет записку под номером num в fs
void delete_from_config(struct config_st* fs, int num) {
    //Удаление физических файлов
    char f1[NAME_MAX], f2[NAME_MAX];
    get_files(fs->file_names[num], f1, f2);

    int error;
    error = remove(f1); // remove в случае ошибки возвращает не ноль
    if (error)
        fprintf(stderr, "Ошибка удаления файла\n");

    error = remove(f2);
    if (error)
        fprintf(stderr, "Ошибка удаления файла\n");

    //Удаление из переменной копированием
    strcpy(fs->file_names[num], fs->file_names[fs->count - 1]);
    fs->file_names[fs->count-1][0] = '\0'; // Подтираем строку

    --fs->count;
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

//Управление будет осуществлятся через коды:
enum codes {final = 0, write_files, write_it, create, rewrite_it, delete_it};

// Обеспечивает интерфейс для управления пользователя. fs список созданных заметок.
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

        fflush(stdin); // Для удаления '$\backslash$n' оставшегося после flag
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
                    printf("Введите номер 	удаляемого файла\n");
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

    menu(&config); // Раскроется далее

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_SYMBOLS 256
#define MAX_CODE_LENGTH 256
#define MAX_FILENAME 256

typedef struct {
    double probability;
    char code[MAX_CODE_LENGTH];
    int code_length;
} SymbolData;

// Функция Up - поиск и вставка суммы вероятностей
int Up(int n, double q, double P[]) {
    int j = n;  // по умолчанию вставляем в конец
    
    int i;
    for (i = n - 1; i >= 2; i--) {
        if (P[i - 1] <= q) {
            P[i] = P[i - 1];
        } else {
            j = i;
            break;
        }
    }
    
    // Если дошли до начала
    if (i == 1) {
        j = 1;
    }
    
    P[j] = q;
    return j;
}

// Функция Down - достраивание кодов
void Down(int n, int j, SymbolData C[], int L[]) {
    // Сохраняем j-ю строку матрицы кодов
    char S[MAX_CODE_LENGTH];
    strcpy(S, C[j].code);
    int L_temp = L[j];
    
    // Сдвигаем вверх строки матрицы C и массива L
    for (int i = j; i <= n - 2; i++) {
        strcpy(C[i].code, C[i + 1].code);
        L[i] = L[i + 1];
    }
    
    // Восстанавливаем префикс для двух последних символов
    strcpy(C[n - 1].code, S);
    strcpy(C[n].code, S);
    
    // Добавляем 0 и 1 к кодам
    C[n - 1].code[L_temp] = '0';
    C[n - 1].code[L_temp + 1] = '\0';
    
    C[n].code[L_temp] = '1';
    C[n].code[L_temp + 1] = '\0';
    
    // Обновляем длины кодов
    L[n - 1] = L_temp + 1;
    L[n] = L_temp + 1;
}

// Рекурсивная функция Хаффмана
void Huffman(int n, double P[], SymbolData C[], int L[]) {
    if (n == 2) {
        // Базовый случай для двух символов
        C[1].code[0] = '0';
        C[1].code[1] = '\0';
        L[1] = 1;
        
        C[2].code[0] = '1';
        C[2].code[1] = '\0';
        L[2] = 1;
    } else {
        // Рекурсивный случай
        double q = P[n - 1] + P[n];
        int j = Up(n, q, P);
        Huffman(n - 1, P, C, L);
        Down(n, j, C, L);
    }
}

// Функция для вычисления энтропии
double calculate_entropy(SymbolData symbols[], int count) {
    double entropy = 0.0;
    for (int i = 1; i <= count; i++) {
        if (symbols[i].probability > 0) {
            entropy -= symbols[i].probability * log2(symbols[i].probability);
        }
    }
    return entropy;
}

// Функция для вычисления средней длины кодового слова
double calculate_average_length(SymbolData symbols[], int count) {
    double avg_length = 0.0;
    for (int i = 1; i <= count; i++) {
        avg_length += symbols[i].probability * symbols[i].code_length;
    }
    return avg_length;
}

// Функция для анализа файла и вычисления вероятностей символов
int analyze_file(const char* filename, SymbolData symbols[], int* symbol_count, 
                 unsigned char symbol_map[], double P[]) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Ошибка: невозможно открыть файл %s\n", filename);
        return 0;
    }
    
    // Подсчет частот символов
    long freq[MAX_SYMBOLS] = {0};
    unsigned char buffer[1024];
    long total_chars = 0;
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        total_chars += bytes_read;
        for (size_t i = 0; i < bytes_read; i++) {
            freq[buffer[i]]++;
        }
    }
    
    fclose(file);
    
    // Заполнение таблицы символов (индексы 1..n)
    *symbol_count = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            (*symbol_count)++;
            symbol_map[*symbol_count] = (unsigned char)i;
            symbols[*symbol_count].probability = (double)freq[i] / total_chars;
            symbols[*symbol_count].code_length = 0;
            symbols[*symbol_count].code[0] = '\0';
            P[*symbol_count] = symbols[*symbol_count].probability;
        }
    }
    
    // Сортировка по убыванию вероятностей (индексы 1..n)
    for (int i = 1; i <= *symbol_count - 1; i++) {
        for (int j = i + 1; j <= *symbol_count; j++) {
            if (P[i] < P[j]) {
                // Меняем местами вероятности
                double temp_p = P[i];
                P[i] = P[j];
                P[j] = temp_p;
                
                // Меняем местами символы в отображении
                unsigned char temp_sym = symbol_map[i];
                symbol_map[i] = symbol_map[j];
                symbol_map[j] = temp_sym;
                
                // Меняем местами данные символов
                SymbolData temp_sym_data = symbols[i];
                symbols[i] = symbols[j];
                symbols[j] = temp_sym_data;
            }
        }
    }
    
    return 1;
}

// Функция для кодирования файла
int encode_file(const char* input_filename, const char* output_filename, 
                SymbolData symbols[], unsigned char symbol_map[], int symbol_count) {
    FILE* input_file = fopen(input_filename, "rb");
    FILE* output_file = fopen(output_filename, "wb");
    
    if (!input_file || !output_file) {
        printf("Ошибка открытия файлов\n");
        if (input_file) fclose(input_file);
        if (output_file) fclose(output_file);
        return 0;
    }
    
    // Создаем таблицу для быстрого поиска кодов по символам
    char* code_table[MAX_SYMBOLS] = {0};
    int code_length_table[MAX_SYMBOLS] = {0};
    
    for (int i = 1; i <= symbol_count; i++) {
        unsigned char symbol = symbol_map[i];
        code_table[symbol] = symbols[i].code;
        code_length_table[symbol] = symbols[i].code_length;
    }
    
    // Кодирование данных
    unsigned char buffer;
    unsigned char output_byte = 0;
    int bit_count = 0;
    
    while (fread(&buffer, 1, 1, input_file) == 1) {
        char* code = code_table[buffer];
        int length = code_length_table[buffer];
        
        for (int i = 0; i < length; i++) {
            output_byte <<= 1;
            if (code[i] == '1') {
                output_byte |= 1;
            }
            bit_count++;
            
            if (bit_count == 8) {
                fwrite(&output_byte, 1, 1, output_file);
                output_byte = 0;
                bit_count = 0;
            }
        }
    }
    
    // Записываем оставшиеся биты
    if (bit_count > 0) {
        output_byte <<= (8 - bit_count);
        fwrite(&output_byte, 1, 1, output_file);
    }
    
    fclose(input_file);
    fclose(output_file);
    return 1;
}

// Функция для вывода кодов на экран
void print_codes(SymbolData symbols[], unsigned char symbol_map[], int count) {
    printf("Коды Хаффмана:\n");
    printf("Символ\tВероятность\tКод\tДлина\n");
    printf("----------------------------------------\n");
    
    for (int i = 1; i <= count; i++) {
        unsigned char symbol = symbol_map[i];
        if (symbol >= 32 && symbol <= 126) {
            printf("'%c'\t", symbol);
        } else {
            printf("0x%02X\t", symbol);
        }
        printf("%.6f\t%s\t%d\n", 
               symbols[i].probability,
               symbols[i].code,
               symbols[i].code_length);
    }
}

// Функция для проверки уникальности кодов
void verify_codes(SymbolData symbols[], int count) {
    printf("\nПроверка уникальности кодов:\n");
    int errors = 0;
    
    for (int i = 1; i <= count; i++) {
        for (int j = i + 1; j <= count; j++) {
            if (strcmp(symbols[i].code, symbols[j].code) == 0) {
                printf("ОШИБКА: Дублирующиеся коды! %s\n", symbols[i].code);
                errors++;
            }
        }
    }
    
    if (errors == 0) {
        printf("Все коды уникальны ✓\n");
    } else {
        printf("Найдено ошибок: %d\n", errors);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Использование: %s <файл_базы_данных>\n", argv[0]);
        return 1;
    }
    
    const char* input_filename = argv[1];
    char output_filename[MAX_FILENAME];
    snprintf(output_filename, sizeof(output_filename), "%s.huff", input_filename);
    
    // Используем индексы 1..MAX_SYMBOLS для соответствия псевдокоду
    SymbolData symbols[MAX_SYMBOLS + 1];  // Индексы 1..n
    unsigned char symbol_map[MAX_SYMBOLS + 1]; // Индексы 1..n
    double P[MAX_SYMBOLS + 1];  // Индексы 1..n
    int L[MAX_SYMBOLS + 1];     // Индексы 1..n
    
    int symbol_count;
    
    printf("Анализ файла: %s\n", input_filename);
    
    // Инициализация массивов
    for (int i = 0; i <= MAX_SYMBOLS; i++) {
        symbols[i].code[0] = '\0';
        symbols[i].code_length = 0;
        symbols[i].probability = 0.0;
        L[i] = 0;
        P[i] = 0.0;
    }
    
    // Анализ файла и вычисление вероятностей
    if (!analyze_file(input_filename, symbols, &symbol_count, symbol_map, P)) {
        return 1;
    }
    
    printf("Найдено %d уникальных символов\n", symbol_count);
    
    if (symbol_count < 2) {
        printf("Ошибка: нужно как минимум 2 символа\n");
        return 1;
    }
    
    // Создаем копию P для работы алгоритма
    double P_work[MAX_SYMBOLS + 1];
    for (int i = 1; i <= symbol_count; i++) {
        P_work[i] = P[i];
    }
    
    // Построение кодов Хаффмана (точно по псевдокоду)
    Huffman(symbol_count, P_work, symbols, L);
    
    // Обновление длин кодов в структуре symbols
    for (int i = 1; i <= symbol_count; i++) {
        symbols[i].code_length = L[i];
    }
    
    // Проверка корректности кодов
    verify_codes(symbols, symbol_count);
    
    // Вывод результатов
    print_codes(symbols, symbol_map, symbol_count);
    
    // Вычисление энтропии и средней длины
    double entropy = calculate_entropy(symbols, symbol_count);
    double avg_length = calculate_average_length(symbols, symbol_count);
    
    printf("\nРезультаты:\n");
    printf("Энтропия: %.6f бит/символ\n", entropy);
    printf("Средняя длина кодового слова: %.6f бит/символ\n", avg_length);
    printf("Избыточность: %.6f бит/символ\n", avg_length - entropy);
    
    if (avg_length >= entropy) {
        printf("Эффективность кодирования: %.2f%%\n", (entropy / avg_length) * 100);
    } else {
        printf("ОШИБКА: Средняя длина меньше энтропии!\n");
        return 1;
    }
    
    // Кодирование файла
    printf("\nКодирование файла...\n");
    if (encode_file(input_filename, output_filename, symbols, symbol_map, symbol_count)) {
        // Вычисление коэффициента сжатия
        FILE* input_file = fopen(input_filename, "rb");
        FILE* output_file = fopen(output_filename, "rb");
        
        if (input_file && output_file) {
            fseek(input_file, 0, SEEK_END);
            fseek(output_file, 0, SEEK_END);
            
            long input_size = ftell(input_file);
            long output_size = ftell(output_file);
            
            fclose(input_file);
            fclose(output_file);
            
            double compression_ratio = (double)input_size / output_size;
            
            printf("Файл успешно закодирован: %s\n", output_filename);
            printf("Размер исходного файла: %ld байт\n", input_size);
            printf("Размер сжатого файла: %ld байт\n", output_size);
            printf("Коэффициент сжатия: %.2f:1\n", compression_ratio);
        }
    } else {
        printf("Ошибка при кодировании файла\n");
    }
    
    return 0;
}
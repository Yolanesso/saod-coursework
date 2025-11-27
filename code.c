#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_SYMBOLS 256
#define MAX_CODE_LENGTH 256
#define MAX_FILENAME 256

typedef struct {
    unsigned char symbol;
    double probability;
    char code[MAX_CODE_LENGTH];
    int code_length;
} SymbolInfo;

typedef struct Node {
    SymbolInfo data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    SymbolInfo symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

// Функция для создания нового узла
Node* create_node(SymbolInfo data) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = data;
    node->left = node->right = NULL;
    return node;
}

// Рекурсивная функция построения кодов Хаффмана
void build_huffman_codes(Node* root, char* code, int depth, SymbolTable* result) {
    if (root == NULL) return;
    
    // Если это лист, сохраняем код
    if (root->left == NULL && root->right == NULL) {
        for (int i = 0; i < MAX_SYMBOLS; i++) {
            if (result->symbols[i].symbol == root->data.symbol) {
                strcpy(result->symbols[i].code, code);
                result->symbols[i].code_length = depth;
                break;
            }
        }
        return;
    }
    
    // Рекурсивно обходим левое поддерево
    if (root->left != NULL) {
        code[depth] = '0';
        code[depth + 1] = '\0';
        build_huffman_codes(root->left, code, depth + 1, result);
    }
    
    // Рекурсивно обходим правое поддерево
    if (root->right != NULL) {
        code[depth] = '1';
        code[depth + 1] = '\0';
        build_huffman_codes(root->right, code, depth + 1, result);
    }
}

// Основная функция построения кода Хаффмана
void huffman_coding(SymbolTable* table) {
    if (table->count <= 1) return;
    
    // Создаем массив узлов
    Node* nodes[MAX_SYMBOLS];
    for (int i = 0; i < table->count; i++) {
        nodes[i] = create_node(table->symbols[i]);
    }
    
    int remaining_nodes = table->count;
    
    while (remaining_nodes > 1) {
        // Берем два узла с наименьшими вероятностями
        Node* min1 = nodes[remaining_nodes - 1];
        Node* min2 = nodes[remaining_nodes - 2];
        
        // Создаем новый удел с суммой вероятностей
        SymbolInfo new_symbol;
        new_symbol.symbol = 0; // Внутренний узел
        new_symbol.probability = min1->data.probability + min2->data.probability;
        
        Node* new_node = create_node(new_symbol);
        new_node->left = min2;
        new_node->right = min1;
        
        // Вставляем новый узел в массив
        nodes[remaining_nodes - 2] = new_node;
        remaining_nodes--;
        
        // Сортируем массив по убыванию вероятностей
        for (int i = remaining_nodes - 1; i > 0; i--) {
            if (nodes[i]->data.probability > nodes[i - 1]->data.probability) {
                Node* temp = nodes[i];
                nodes[i] = nodes[i - 1];
                nodes[i - 1] = temp;
            }
        }
    }
    
    // Строим коды
    char code[MAX_CODE_LENGTH] = {0};
    build_huffman_codes(nodes[0], code, 0, table);
    
    free(nodes[0]); // Освобождаем корень дерева
}

// Функция для вычисления энтропии
double calculate_entropy(SymbolTable* table) {
    double entropy = 0.0;
    for (int i = 0; i < table->count; i++) {
        if (table->symbols[i].probability > 0) {
            entropy -= table->symbols[i].probability * log2(table->symbols[i].probability);
        }
    }
    return entropy;
}

// Функция для вычисления средней длины кодового слова
double calculate_average_length(SymbolTable* table) {
    double avg_length = 0.0;
    for (int i = 0; i < table->count; i++) {
        avg_length += table->symbols[i].probability * table->symbols[i].code_length;
    }
    return avg_length;
}

// Функция для анализа файла и вычисления вероятностей символов
int analyze_file(const char* filename, SymbolTable* table) {
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
    
    // Заполнение таблицы символов
    table->count = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            table->symbols[table->count].symbol = (unsigned char)i;
            table->symbols[table->count].probability = (double)freq[i] / total_chars;
            table->symbols[table->count].code_length = 0;
            table->symbols[table->count].code[0] = '\0';
            table->count++;
        }
    }
    
    // Сортировка по убыванию вероятностей
    for (int i = 0; i < table->count - 1; i++) {
        for (int j = i + 1; j < table->count; j++) {
            if (table->symbols[i].probability < table->symbols[j].probability) {
                SymbolInfo temp = table->symbols[i];
                table->symbols[i] = table->symbols[j];
                table->symbols[j] = temp;
            }
        }
    }
    
    return 1;
}

// Функция для кодирования файла
int encode_file(const char* input_filename, const char* output_filename, SymbolTable* table) {
    FILE* input_file = fopen(input_filename, "rb");
    FILE* output_file = fopen(output_filename, "wb");
    
    if (!input_file || !output_file) {
        printf("Ошибка открытия файлов\n");
        if (input_file) fclose(input_file);
        if (output_file) fclose(output_file);
        return 0;
    }
    
    // Создаем таблицу для быстрого поиска кодов
    char* code_table[MAX_SYMBOLS] = {0};
    int code_length_table[MAX_SYMBOLS] = {0};
    
    for (int i = 0; i < table->count; i++) {
        unsigned char symbol = table->symbols[i].symbol;
        code_table[symbol] = table->symbols[i].code;
        code_length_table[symbol] = table->symbols[i].code_length;
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
void print_codes(SymbolTable* table) {
    printf("Коды Хаффмана:\n");
    printf("Символ\tВероятность\tКод\tДлина\n");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < table->count; i++) {
        if (table->symbols[i].symbol >= 32 && table->symbols[i].symbol <= 126) {
            printf("'%c'\t", table->symbols[i].symbol);
        } else {
            printf("0x%02X\t", table->symbols[i].symbol);
        }
        printf("%.6f\t%s\t%d\n", 
               table->symbols[i].probability,
               table->symbols[i].code,
               table->symbols[i].code_length);
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
    
    SymbolTable table;
    
    printf("Анализ файла: %s\n", input_filename);
    
    // Анализ файла и вычисление вероятностей
    if (!analyze_file(input_filename, &table)) {
        return 1;
    }
    
    printf("Найдено %d уникальных символов\n", table.count);
    
    // Построение кодов Хаффмана
    huffman_coding(&table);
    
    // Вывод результатов
    print_codes(&table);
    
    // Вычисление энтропии и средней длины
    double entropy = calculate_entropy(&table);
    double avg_length = calculate_average_length(&table);
    
    printf("\nРезультаты:\n");
    printf("Энтропия: %.6f бит/символ\n", entropy);
    printf("Средняя длина кодового слова: %.6f бит/символ\n", avg_length);
    printf("Избыточность: %.6f бит/символ\n", avg_length - entropy);
    printf("Эффективность кодирования: %.2f%%\n", (entropy / avg_length) * 100);
    
    // Кодирование файла
    printf("\nКодирование файла...\n");
    if (encode_file(input_filename, output_filename, &table)) {
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
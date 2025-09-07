#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define PAGE_SIZE 20
#define MAX_RECORDS 4000
#define NAME_LEN 32
#define STREET_LEN 18
#define DATE_LEN 10

typedef struct Record {
    char name[NAME_LEN];
    char street[STREET_LEN];
    short int house;
    short int apartment;
    char date[DATE_LEN];
} Record;

// Таблица преобразования CP866 -> UTF-8 для русских букв
const char* cp866_to_utf8(unsigned char c) {
    switch (c) {
        // Заглавные буквы
        case 0x80: return "А"; case 0x81: return "Б"; case 0x82: return "В"; case 0x83: return "Г";
        case 0x84: return "Д"; case 0x85: return "Е"; case 0x86: return "Ж"; case 0x87: return "З";
        case 0x88: return "И"; case 0x89: return "Й"; case 0x8A: return "К"; case 0x8B: return "Л";
        case 0x8C: return "М"; case 0x8D: return "Н"; case 0x8E: return "О"; case 0x8F: return "П";
        case 0x90: return "Р"; case 0x91: return "С"; case 0x92: return "Т"; case 0x93: return "У";
        case 0x94: return "Ф"; case 0x95: return "Х"; case 0x96: return "Ц"; case 0x97: return "Ч";
        case 0x98: return "Ш"; case 0x99: return "Щ"; case 0x9A: return "Ъ"; case 0x9B: return "Ы";
        case 0x9C: return "Ь"; case 0x9D: return "Э"; case 0x9E: return "Ю"; case 0x9F: return "Я";
        
        // Строчные буквы
        case 0xA0: return "а"; case 0xA1: return "б"; case 0xA2: return "в"; case 0xA3: return "г";
        case 0xA4: return "д"; case 0xA5: return "е"; case 0xA6: return "ж"; case 0xA7: return "з";
        case 0xA8: return "и"; case 0xA9: return "й"; case 0xAA: return "к"; case 0xAB: return "л";
        case 0xAC: return "м"; case 0xAD: return "н"; case 0xAE: return "о"; case 0xAF: return "п";
        case 0xE0: return "р"; case 0xE1: return "с"; case 0xE2: return "т"; case 0xE3: return "у";
        case 0xE4: return "ф"; case 0xE5: return "х"; case 0xE6: return "ц"; case 0xE7: return "ч";
        case 0xE8: return "ш"; case 0xE9: return "щ"; case 0xEA: return "ъ"; case 0xEB: return "ы";
        case 0xEC: return "ь"; case 0xED: return "э"; case 0xEE: return "ю"; case 0xEF: return "я";
        
        // Специальные символы
        case 0xF0: return "№"; case 0xF1: return "ё"; case 0xF2: return "ё";
        
        default: return NULL;
    }
}

void convert_cp866_to_utf8(const char* input, char* output, int max_len) {
    int out_index = 0;
    for (int i = 0; i < max_len && input[i] != '\0' && out_index < max_len * 3; i++) {
        unsigned char c = (unsigned char)input[i];
        const char* utf8_char = cp866_to_utf8(c);
        
        if (utf8_char != NULL) {
            int j = 0;
            while (utf8_char[j] != '\0' && out_index < max_len * 3 - 1) {
                output[out_index++] = utf8_char[j++];
            }
        } else if (c < 0x80) {
            output[out_index++] = c;
        } else {
            output[out_index++] = '?';
        }
    }
    output[out_index] = '\0';
}

int string_compare(const char* s1, const char* s2, int max_len) {
    for (int i = 0; i < max_len; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}

void swap_records(Record* a, Record* b) {
    Record temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(Record arr[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n) {
        int street_compare = string_compare(arr[left].street, arr[largest].street, STREET_LEN);
        if (street_compare > 0 || (street_compare == 0 && arr[left].house > arr[largest].house)) {
            largest = left;
        }
    }

    if (right < n) {
        int street_compare = string_compare(arr[right].street, arr[largest].street, STREET_LEN);
        if (street_compare > 0 || (street_compare == 0 && arr[right].house > arr[largest].house)) {
            largest = right;
        }
    }

    if (largest != i) {
        swap_records(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }
}

void heap_sort(Record arr[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int i = n - 1; i >= 0; i--) {
        swap_records(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
}

Record* load_database(const char* filename, int* count) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Ошибка открытия файла %s\n", filename);
        return NULL;
    }
    
    Record* records = malloc(MAX_RECORDS * sizeof(Record));
    *count = 0;
    
    while (*count < MAX_RECORDS) {
        if (fread(records[*count].name, 1, NAME_LEN, file) != NAME_LEN) break;
        if (fread(records[*count].street, 1, STREET_LEN, file) != STREET_LEN) break;
        if (fread(&records[*count].house, sizeof(short int), 1, file) != 1) break;
        if (fread(&records[*count].apartment, sizeof(short int), 1, file) != 1) break;
        if (fread(records[*count].date, 1, DATE_LEN, file) != DATE_LEN) break;
        
        (*count)++;
    }
    
    fclose(file);
    printf("Загружено записей: %d\n", *count);
    return records;
}

void trim_spaces(char* str, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if (str[i] != ' ' && str[i] != '_') {
            str[i + 1] = '\0';
            break;
        }
        if (i == 0) str[0] = '\0';
    }
}

void display_record(Record* record) {
    char temp_name[NAME_LEN * 3 + 1];
    char temp_street[STREET_LEN * 3 + 1];
    char temp_date[DATE_LEN * 3 + 1];
    
    // Конвертируем из CP866 в UTF-8
    convert_cp866_to_utf8(record->name, temp_name, NAME_LEN);
    convert_cp866_to_utf8(record->street, temp_street, STREET_LEN);
    convert_cp866_to_utf8(record->date, temp_date, DATE_LEN);
    
    // Убираем пробелы
    trim_spaces(temp_name, strlen(temp_name));
    trim_spaces(temp_street, strlen(temp_street));
    trim_spaces(temp_date, strlen(temp_date));
    
    printf("ФИО: %s\n", temp_name);
    printf("Улица: %s\n", temp_street);
    printf("Дом: %d\n", record->house);
    printf("Квартира: %d\n", record->apartment);
    printf("Дата: %s\n", temp_date);
    printf("---\n");
}

void display_records_page(Record* records, int count, int start_index, const char* title) {
    printf("\n=== %s ===\n", title);
    int end_index = start_index + PAGE_SIZE;
    if (end_index > count) end_index = count;
    
    for (int i = start_index; i < end_index; i++) {
        printf("Запись %d:\n", i + 1);
        display_record(&records[i]);
    }
    
    printf("Показано записей: %d-%d из %d\n", 
           start_index + 1, end_index, count);
}

int search_by_street_prefix(Record* records, int count, const char* prefix, int** results) {
    int found_count = 0;
    int* indices = malloc(count * sizeof(int));
    
    // Конвертируем префикс из UTF-8 обратно в CP866 для поиска
    char cp866_prefix[4] = {0};
    for (int i = 0; i < 3 && prefix[i] != '\0'; i++) {
        // Простая обратная конвертация для основных русских букв
        if (prefix[i] == 'А') cp866_prefix[i] = 0x80;
        else if (prefix[i] == 'Б') cp866_prefix[i] = 0x81;
        else if (prefix[i] == 'В') cp866_prefix[i] = 0x82;
        else if (prefix[i] == 'Г') cp866_prefix[i] = 0x83;
        else if (prefix[i] == 'Д') cp866_prefix[i] = 0x84;
        else if (prefix[i] == 'Е') cp866_prefix[i] = 0x85;
        else if (prefix[i] == 'Ж') cp866_prefix[i] = 0x86;
        else if (prefix[i] == 'З') cp866_prefix[i] = 0x87;
        else if (prefix[i] == 'И') cp866_prefix[i] = 0x88;
        else if (prefix[i] == 'Й') cp866_prefix[i] = 0x89;
        else if (prefix[i] == 'К') cp866_prefix[i] = 0x8A;
        else if (prefix[i] == 'Л') cp866_prefix[i] = 0x8B;
        else if (prefix[i] == 'М') cp866_prefix[i] = 0x8C;
        else if (prefix[i] == 'Н') cp866_prefix[i] = 0x8D;
        else if (prefix[i] == 'О') cp866_prefix[i] = 0x8E;
        else if (prefix[i] == 'П') cp866_prefix[i] = 0x8F;
        else if (prefix[i] == 'Р') cp866_prefix[i] = 0x90;
        else if (prefix[i] == 'С') cp866_prefix[i] = 0x91;
        else if (prefix[i] == 'Т') cp866_prefix[i] = 0x92;
        else if (prefix[i] == 'У') cp866_prefix[i] = 0x93;
        else if (prefix[i] == 'Ф') cp866_prefix[i] = 0x94;
        else if (prefix[i] == 'Х') cp866_prefix[i] = 0x95;
        else if (prefix[i] == 'Ц') cp866_prefix[i] = 0x96;
        else if (prefix[i] == 'Ч') cp866_prefix[i] = 0x97;
        else if (prefix[i] == 'Ш') cp866_prefix[i] = 0x98;
        else if (prefix[i] == 'Щ') cp866_prefix[i] = 0x99;
        else if (prefix[i] == 'Ъ') cp866_prefix[i] = 0x9A;
        else if (prefix[i] == 'Ы') cp866_prefix[i] = 0x9B;
        else if (prefix[i] == 'Ь') cp866_prefix[i] = 0x9C;
        else if (prefix[i] == 'Э') cp866_prefix[i] = 0x9D;
        else if (prefix[i] == 'Ю') cp866_prefix[i] = 0x9E;
        else if (prefix[i] == 'Я') cp866_prefix[i] = 0x9F;
        else if (prefix[i] == 'а') cp866_prefix[i] = 0xA0;
        else if (prefix[i] == 'б') cp866_prefix[i] = 0xA1;
        else if (prefix[i] == 'в') cp866_prefix[i] = 0xA2;
        else if (prefix[i] == 'г') cp866_prefix[i] = 0xA3;
        else if (prefix[i] == 'д') cp866_prefix[i] = 0xA4;
        else if (prefix[i] == 'е') cp866_prefix[i] = 0xA5;
        else if (prefix[i] == 'ж') cp866_prefix[i] = 0xA6;
        else if (prefix[i] == 'з') cp866_prefix[i] = 0xA7;
        else if (prefix[i] == 'и') cp866_prefix[i] = 0xA8;
        else if (prefix[i] == 'й') cp866_prefix[i] = 0xA9;
        else if (prefix[i] == 'к') cp866_prefix[i] = 0xAA;
        else if (prefix[i] == 'л') cp866_prefix[i] = 0xAB;
        else if (prefix[i] == 'м') cp866_prefix[i] = 0xAC;
        else if (prefix[i] == 'н') cp866_prefix[i] = 0xAD;
        else if (prefix[i] == 'о') cp866_prefix[i] = 0xAE;
        else if (prefix[i] == 'п') cp866_prefix[i] = 0xAF;
        else if (prefix[i] == 'р') cp866_prefix[i] = 0xE0;
        else if (prefix[i] == 'с') cp866_prefix[i] = 0xE1;
        else if (prefix[i] == 'т') cp866_prefix[i] = 0xE2;
        else if (prefix[i] == 'у') cp866_prefix[i] = 0xE3;
        else if (prefix[i] == 'ф') cp866_prefix[i] = 0xE4;
        else if (prefix[i] == 'х') cp866_prefix[i] = 0xE5;
        else if (prefix[i] == 'ц') cp866_prefix[i] = 0xE6;
        else if (prefix[i] == 'ч') cp866_prefix[i] = 0xE7;
        else if (prefix[i] == 'ш') cp866_prefix[i] = 0xE8;
        else if (prefix[i] == 'щ') cp866_prefix[i] = 0xE9;
        else if (prefix[i] == 'ъ') cp866_prefix[i] = 0xEA;
        else if (prefix[i] == 'ы') cp866_prefix[i] = 0xEB;
        else if (prefix[i] == 'ь') cp866_prefix[i] = 0xEC;
        else if (prefix[i] == 'э') cp866_prefix[i] = 0xED;
        else if (prefix[i] == 'ю') cp866_prefix[i] = 0xEE;
        else if (prefix[i] == 'я') cp866_prefix[i] = 0xEF;
        else cp866_prefix[i] = prefix[i]; // ASCII символы
    }
    
    for (int i = 0; i < count; i++) {
        if (string_compare(records[i].street, cp866_prefix, 3) == 0) {
            indices[found_count++] = i;
        }
    }
    
    *results = malloc(found_count * sizeof(int));
    memcpy(*results, indices, found_count * sizeof(int));
    free(indices);
    
    return found_count;
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    int record_count = 0;
    Record* database = load_database("database.dat", &record_count);
    Record* sorted_database = NULL;
    int is_sorted = 0;
    
    if (!database) {
        printf("Не удалось загрузить базу данных\n");
        return 1;
    }
    
    int choice;
    int current_page = 0;
    int current_sorted_page = 0;
    
    do {
        printf("\n=== МЕНЮ ===\n");
        printf("1. Просмотр исходных записей (постранично)\n");
        printf("2. Отсортировать по улице и дому (heapSort)\n");
        printf("3. Просмотр отсортированных записей\n");
        printf("4. Поиск по первым трем буквам улицы\n");
        printf("0. Выход\n");
        printf("Выбор: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("Неверный ввод. Попробуйте снова.\n");
            continue;
        }
        clear_input_buffer();
        
        switch (choice) {
            case 1: {
                int sub_choice;
                do {
                    display_records_page(database, record_count, current_page, "ИСХОДНЫЕ ДАННЫЕ");
                    printf("\n1. Следующая страница\n");
                    printf("2. Предыдущая страница\n");
                    printf("3. В главное меню\n");
                    printf("Выбор: ");
                    
                    if (scanf("%d", &sub_choice) != 1) {
                        clear_input_buffer();
                        printf("Неверный ввод.\n");
                        continue;
                    }
                    clear_input_buffer();
                    
                    if (sub_choice == 1) {
                        current_page += PAGE_SIZE;
                        if (current_page >= record_count) {
                            current_page = 0;
                        }
                    } else if (sub_choice == 2) {
                        current_page -= PAGE_SIZE;
                        if (current_page < 0) {
                            current_page = record_count - (record_count % PAGE_SIZE);
                            if (current_page == record_count) current_page -= PAGE_SIZE;
                        }
                    }
                } while (sub_choice != 3);
                break;
            }
            
            case 2: {
                printf("Сортировка по улице и номеру дома...\n");
                
                if (sorted_database) free(sorted_database);
                sorted_database = malloc(record_count * sizeof(Record));
                memcpy(sorted_database, database, record_count * sizeof(Record));
                
                heap_sort(sorted_database, record_count);
                is_sorted = 1;
                current_sorted_page = 0;
                printf("Сортировка завершена!\n");
                break;
            }
            
            case 3: {
                if (!is_sorted) {
                    printf("Сначала выполните сортировку (пункт 2)!\n");
                    break;
                }
                
                int sub_choice;
                do {
                    display_records_page(sorted_database, record_count, current_sorted_page, "ОТСОРТИРОВАННЫЕ ДАННЫЕ");
                    printf("\n1. Следующая страница\n");
                    printf("2. Предыдущая страница\n");
                    printf("3. В главное меню\n");
                    printf("Выбор: ");
                    
                    if (scanf("%d", &sub_choice) != 1) {
                        clear_input_buffer();
                        printf("Неверный ввод.\n");
                        continue;
                    }
                    clear_input_buffer();
                    
                    if (sub_choice == 1) {
                        current_sorted_page += PAGE_SIZE;
                        if (current_sorted_page >= record_count) {
                            current_sorted_page = 0;
                        }
                    } else if (sub_choice == 2) {
                        current_sorted_page -= PAGE_SIZE;
                        if (current_sorted_page < 0) {
                            current_sorted_page = record_count - (record_count % PAGE_SIZE);
                            if (current_sorted_page == record_count) current_sorted_page -= PAGE_SIZE;
                        }
                    }
                } while (sub_choice != 3);
                break;
            }
            
            case 4: {
                char search_prefix[4];
                printf("Введите первые 3 буквы улицы: ");
                
                if (scanf("%3s", search_prefix) != 1) {
                    clear_input_buffer();
                    printf("Неверный ввод\n");
                    break;
                }
                clear_input_buffer();
                
                Record* search_base = is_sorted ? sorted_database : database;
                const char* base_type = is_sorted ? "отсортированной" : "исходной";
                
                printf("Поиск в %s базе...\n", base_type);
                
                int* results;
                int found_count = search_by_street_prefix(search_base, record_count, search_prefix, &results);
                
                if (found_count > 0) {
                    printf("\nНайдено записей: %d\n", found_count);
                    for (int i = 0; i < found_count; i++) {
                        printf("Запись %d:\n", results[i] + 1);
                        display_record(&search_base[results[i]]);
                    }
                } else {
                    printf("Записей с улицами, начинающимися на '%s' не найдено\n", search_prefix);
                }
                
                free(results);
                break;
            }
            
            case 0:
                printf("Выход...\n");
                break;
                
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
                break;
        }
    } while (choice != 0);
    
    free(database);
    if (sorted_database) free(sorted_database);
    return 0;
}
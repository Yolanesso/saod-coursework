#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Таблица преобразования CP866 -> Windows-1251
unsigned char cp866_to_1251[256] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
    0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
    0x2D,0xA6,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,
    0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,
    0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,
    0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

void convert_cp866_to_1251(char* str, int len) {
    for (int i = 0; i < len; i++) {
        if ((unsigned char)str[i] >= 0x80) {
            str[i] = cp866_to_1251[(unsigned char)str[i]];
        }
    }
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
        printf("Error opening file %s\n", filename);
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
    printf("Loaded records: %d\n", *count);
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
    char temp_name[NAME_LEN + 1];
    char temp_street[STREET_LEN + 1];
    char temp_date[DATE_LEN + 1];
    
    memcpy(temp_name, record->name, NAME_LEN);
    temp_name[NAME_LEN] = '\0';
    convert_cp866_to_1251(temp_name, NAME_LEN);
    trim_spaces(temp_name, NAME_LEN);
    
    memcpy(temp_street, record->street, STREET_LEN);
    temp_street[STREET_LEN] = '\0';
    convert_cp866_to_1251(temp_street, STREET_LEN);
    trim_spaces(temp_street, STREET_LEN);
    
    memcpy(temp_date, record->date, DATE_LEN);
    temp_date[DATE_LEN] = '\0';
    convert_cp866_to_1251(temp_date, DATE_LEN);
    trim_spaces(temp_date, DATE_LEN);
    
    printf("ФИО: %s\n", temp_name);
    printf("Улица: %s\n", temp_street);
    printf("Дом: %d\n", record->house);
    printf("Квартира: %d\n", record->apartment);
    printf("Дата: %s\n", temp_date);
    printf("---\n");
}

void display_records_page(Record* records, int count, int start_index) {
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
    
    // Конвертируем префикс для поиска (он введен в 1251, а данные в 866)
    char cp866_prefix[4];
    strncpy(cp866_prefix, prefix, 3);
    cp866_prefix[3] = '\0';
    
    // Простая обратная конвертация для основных русских букв
    for (int i = 0; i < 3; i++) {
        if (cp866_prefix[i] >= 0xC0 && cp866_prefix[i] <= 0xFF) {
            cp866_prefix[i] = (cp866_prefix[i] - 0xC0) + 0x80;
        }
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

int main() {
    // Устанавливаем русскую кодировку для консоли

    
    int record_count = 0;
    Record* database = load_database("database.dat", &record_count);
    
    if (!database) {
        printf("Не удалось загрузить базу данных\n");
        return 1;
    }
    
    int choice;
    int current_page = 0;
    
    do {
        printf("\nМеню:\n");
        printf("1. Просмотр исходных записей (постранично)\n");
        printf("2. Отсортировать по улице и дому (heapSort)\n");
        printf("3. Поиск по первым трем буквам улицы\n");
        printf("0. Выход\n");
        printf("Выбор: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                int sub_choice;
                do {
                    display_records_page(database, record_count, current_page);
                    printf("\n1. Следующая страница\n");
                    printf("2. Предыдущая страница\n");
                    printf("3. В главное меню\n");
                    printf("Выбор: ");
                    scanf("%d", &sub_choice);
                    
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
                heap_sort(database, record_count);
                printf("Сортировка завершена!\n");
                current_page = 0;
                break;
            }
            
            case 3: {
                char search_prefix[4];
                printf("Введите первые 3 буквы улицы: ");
                scanf("%3s", search_prefix);
                
                int* results;
                int found_count = search_by_street_prefix(database, record_count, search_prefix, &results);
                
                if (found_count > 0) {
                    printf("\nНайдено записей: %d\n", found_count);
                    for (int i = 0; i < found_count; i++) {
                        printf("Запись %d:\n", results[i] + 1);
                        display_record(&database[results[i]]);
                    }
                } else {
                    printf("Записей с улицами, начинающимися на '%s' не найдено\n", search_prefix);
                }
                
                free(results);
                break;
            }
        }
    } while (choice != 0);
    
    free(database);
    return 0;
}
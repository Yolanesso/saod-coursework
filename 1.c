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
    trim_spaces(temp_name, NAME_LEN);
    
    memcpy(temp_street, record->street, STREET_LEN);
    temp_street[STREET_LEN] = '\0';
    trim_spaces(temp_street, STREET_LEN);
    
    memcpy(temp_date, record->date, DATE_LEN);
    temp_date[DATE_LEN] = '\0';
    trim_spaces(temp_date, DATE_LEN);
    
    printf("FIO: %s\n", temp_name);
    printf("Street: %s\n", temp_street);
    printf("House: %d\n", record->house);
    printf("Apartment: %d\n", record->apartment);
    printf("Date: %s\n", temp_date);
    printf("---\n");
}

void display_records_page(Record* records, int count, int start_index) {
    int end_index = start_index + PAGE_SIZE;
    if (end_index > count) end_index = count;
    
    for (int i = start_index; i < end_index; i++) {
        printf("Record %d:\n", i + 1);
        display_record(&records[i]);
    }
    
    printf("Showing records: %d-%d of %d\n", 
           start_index + 1, end_index, count);
}

int search_by_street_prefix(Record* records, int count, const char* prefix, int** results) {
    int found_count = 0;
    int* indices = malloc(count * sizeof(int));
    
    for (int i = 0; i < count; i++) {
        if (string_compare(records[i].street, prefix, 3) == 0) {
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
    setlocale(LC_ALL, "en_US.UTF-8");
    
    int record_count = 0;
    Record* database = load_database("database.dat", &record_count);
    
    if (!database) {
        printf("Failed to load database\n");
        return 1;
    }
    
    int choice;
    int current_page = 0;
    
    do {
        printf("\nMenu:\n");
        printf("1. View original records (page by page)\n");
        printf("2. Sort by street and house (heapSort)\n");
        printf("3. Search by first 3 letters of street\n");
        printf("0. Exit\n");
        printf("Choice: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        
        switch (choice) {
            case 1: {
                int sub_choice;
                do {
                    display_records_page(database, record_count, current_page);
                    printf("\n1. Next page\n");
                    printf("2. Previous page\n");
                    printf("3. Back to main menu\n");
                    printf("Choice: ");
                    
                    if (scanf("%d", &sub_choice) != 1) {
                        clear_input_buffer();
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
                printf("Sorting by street and house number...\n");
                heap_sort(database, record_count);
                printf("Sorting completed!\n");
                current_page = 0;
                break;
            }
            
            case 3: {
                char search_prefix[4];
                printf("Enter first 3 letters of street: ");
                
                if (scanf("%3s", search_prefix) != 1) {
                    clear_input_buffer();
                    printf("Invalid input\n");
                    break;
                }
                clear_input_buffer();
                
                int* results;
                int found_count = search_by_street_prefix(database, record_count, search_prefix, &results);
                
                if (found_count > 0) {
                    printf("\nFound records: %d\n", found_count);
                    for (int i = 0; i < found_count; i++) {
                        printf("Record %d:\n", results[i] + 1);
                        display_record(&database[results[i]]);
                    }
                } else {
                    printf("No records found with streets starting with '%s'\n", search_prefix);
                }
                
                free(results);
                break;
            }
            
            case 0:
                printf("Exiting...\n");
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 0);
    
    free(database);
    return 0;
}
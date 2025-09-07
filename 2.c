#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define N 4000
#define MAX_STR_SIZE 32
#define STREET_SIZE 18
#define DATE_SIZE 10

typedef struct {
    char fio[MAX_STR_SIZE];
    char street[STREET_SIZE];
    short int home;
    short int appartament;
    char date[DATE_SIZE];
} Record;

typedef struct Node {
    Record record;
    struct Node *next;
} Node;

char* prompt(const char *str) {
    printf("%s\n> ", str);
    static char ans[100];
    scanf("%s", ans);
    return ans;
}

Node* load_to_memory() {
    FILE *file = fopen("database.dat", "rb");
    if (!file) {
        return NULL;
    }

    Node *root = NULL;
    for (int i = 0; i < N; ++i) {
        Record record;
        fread(&record, sizeof(Record), 1, file);
        Node *new_node = (Node*)malloc(sizeof(Node));
        new_node->record = record;
        new_node->next = root;
        root = new_node;
    }
    fclose(file);
    return root;
}

void make_index_array(Record *arr[], Node *root, int n) {
    Node *p = root;
    for (int i = 0; i < n; i++) {
        arr[i] = &(p->record);
        p = p->next;
    }
}

// Функция сравнения для сортировки: сначала по улице, затем по дому
int compare_records(const Record *record1, const Record *record2) {
    // Сравниваем названия улиц
    int street_cmp = strcmp(record1->street, record2->street);
    if (street_cmp != 0) {
        return street_cmp;
    }
    // Если улицы одинаковые, сравниваем номера домов
    return record1->home - record2->home;
}

// Функция сравнения для кучи
int compare_for_heap(const void *a, const void *b) {
    Record *r1 = *(Record**)a;
    Record *r2 = *(Record**)b;
    return compare_records(r1, r2);
}

void MakeHeap(Record *array[], int l, int r) {
    Record *x = array[l];
    int i = l;
    while (1) {
        int j = i * 2 + 1; // левый потомок
        if (j > r) break;
        
        // Выбираем большего потомка
        if (j < r && compare_for_heap(&array[j], &array[j + 1]) < 0) {
            j++;
        }
        
        // Если текущий элемент больше или равен потомку, выходим
        if (compare_for_heap(&x, &array[j]) >= 0) break;
        
        array[i] = array[j];
        i = j;
    }
    array[i] = x;
}

void HeapSort(Record *array[], int n) {
    // Построение кучи
    for (int i = n / 2 - 1; i >= 0; i--) {
        MakeHeap(array, i, n - 1);
    }
    
    // Извлечение элементов из кучи
    for (int i = n - 1; i > 0; i--) {
        Record *temp = array[0];
        array[0] = array[i];
        array[i] = temp;
        MakeHeap(array, 0, i - 1);
    }
}

void print_head() {
    printf("Record Full Name                        Street          Home  Apt  Date\n");
}

void print_record(Record *record, int i) {
    printf("[%4d] %-32s  %-15s  %-4d  %-3d  %s\n", 
           i, record->fio, record->street, record->home, 
           record->appartament, record->date);
}

void show_list(Record *ind_arr[], int n) {
    int ind = 0;
    while (1) {
        system("cls");
        print_head();
        for (int i = 0; i < 20 && (ind + i) < n; i++) {
            print_record(ind_arr[ind + i], ind + i + 1);
        }
        
        printf("\nPage %d/%d\n", (ind / 20) + 1, (n / 20) + 1);
        char *chose = prompt("w: Next page\tq: Last page\te: Skip 10 next pages\n"
                             "s: Prev page\ta: First page\td: Skip 10 prev pages\n"
                             "Any key: Exit");
        
        switch (chose[0]) {
            case 'w': ind += 20; break;
            case 's': ind -= 20; break;
            case 'a': ind = 0; break;
            case 'q': ind = n - 20; break;
            case 'd': ind -= 200; break;
            case 'e': ind += 200; break;
            default: return;
        }
        
        if (ind < 0) ind = 0;
        if (ind > n - 20) ind = n - 20;
    }
}

// Функция сравнения для поиска: первые 3 символа улицы
int compare_search(const char *street, const char *key) {
    return strncmp(street, key, 3);
}

// Бинарный поиск по первым 3 символам названия улицы
int binary_search(Record *arr[], const char *key, int *start_index) {
    int left = 0;
    int right = N - 1;
    int found_index = -1;
    
    // Находим первую запись, удовлетворяющую условию
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = compare_search(arr[mid]->street, key);
        
        if (cmp == 0) {
            found_index = mid;
            right = mid - 1; // Ищем первую подходящую запись
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (found_index == -1) {
        return 0; // Не найдено
    }
    
    // Находим все записи с таким же ключом
    *start_index = found_index;
    int count = 1;
    
    // Ищем вперед
    for (int i = found_index + 1; i < N; i++) {
        if (compare_search(arr[i]->street, key) == 0) {
            count++;
        } else {
            break;
        }
    }
    
    return count;
}

void search_by_street(Record *arr[]) {
    char *key;
    
    do {
        key = prompt("Input first 3 letters of street name");
        if (strlen(key) == 0) {
            printf("Please enter search key\n");
            continue;
        }
        
        // Берем только первые 3 символа
        char search_key[4] = {0};
        strncpy(search_key, key, 3);
        search_key[3] = '\0';
        
        int start_index;
        int count = binary_search(arr, search_key, &start_index);
        
        if (count == 0) {
            printf("No records found for street starting with '%s'\n", search_key);
        } else {
            printf("Found %d records for street starting with '%s'\n", count, search_key);
            
            // Создаем массив для найденных записей
            Record **found_arr = (Record**)malloc(count * sizeof(Record*));
            for (int i = 0; i < count; i++) {
                found_arr[i] = arr[start_index + i];
            }
            
            show_list(found_arr, count);
            free(found_arr);
        }
        
        char *again = prompt("Search again? (y/n)");
        if (again[0] != 'y' && again[0] != 'Y') {
            break;
        }
        
    } while (1);
}

void mainloop(Record *unsorted_ind_array[], Record *sorted_ind_array[]) {
    while (1) {
        printf("\n=== MENU ===\n");
        char *chose = prompt("1: Show unsorted list\n"
                             "2: Show sorted list (by street and house)\n"
                             "3: Search by first 3 letters of street\n"
                             "0: Exit");
        
        switch (chose[0]) {
            case '1':
                printf("\n=== UNSORTED LIST ===\n");
                show_list(unsorted_ind_array, N);
                break;
            case '2':
                printf("\n=== SORTED LIST (by street and house number) ===\n");
                show_list(sorted_ind_array, N);
                break;
            case '3':
                printf("\n=== SEARCH BY STREET ===\n");
                search_by_street(sorted_ind_array);
                break;
            case '0':
                return;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

int main() {
    printf("Loading data...\n");
    Node *root = load_to_memory();
    if (!root) {
        printf("Error: File 'testBase4.dat' not found\n");
        return 1;
    }
    
    Record *unsorted_ind_arr[N];
    Record *sorted_ind_arr[N];
    
    make_index_array(unsorted_ind_arr, root, N);
    make_index_array(sorted_ind_arr, root, N);
    
    printf("Sorting data by street and house number...\n");
    HeapSort(sorted_ind_arr, N);
    
    printf("Data loaded successfully. Total records: %d\n", N);
    mainloop(unsorted_ind_arr, sorted_ind_arr);
    
    // Освобождаем память
    while (root) {
        Node *temp = root;
        root = root->next;
        free(temp);
    }
    
    printf("Program finished.\n");
    return 0;
}
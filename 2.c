#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define N 4000
#define MAX_STR_SIZE 32
#define STREET_SIZE 18
#define DATE_SIZE 10

// Color codes
#define COLOR_RESET 7
#define COLOR_RED 12
#define COLOR_GREEN 10
#define COLOR_YELLOW 14
#define COLOR_BLUE 9
#define COLOR_CYAN 11
#define COLOR_MAGENTA 13
#define COLOR_WHITE 15
#define COLOR_GRAY 8

typedef struct {
    char fio[MAX_STR_SIZE];
    char street[STREET_SIZE];
    short int home;
    short int appartament;
    char date[DATE_SIZE];
} Record;

// Очередь
typedef struct QueueNode {
    Record record;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

// Дерево для поиска по номеру квартиры (Vertex из твоего кода)
typedef struct Vertex {
    Record *data;
    struct Vertex *left;
    struct Vertex *right;
} Vertex;

// Color functions
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_WHITE);
}

// Visual elements with simple ASCII
void print_double_line() {
    setColor(COLOR_CYAN);
    printf("================================================================================\n");
    resetColor();
}

void print_single_line() {
    setColor(COLOR_CYAN);
    printf("--------------------------------------------------------------------------------\n");
    resetColor();
}

void print_header_title(const char *title) {
    print_double_line();
    setColor(COLOR_YELLOW);
    printf("| %-72s |\n", title);
    resetColor();
    print_single_line();
}

void print_section_title(const char *title) {
    setColor(COLOR_GREEN);
    printf("\n");
    print_double_line();
    printf("| %-72s |\n", title);
    print_double_line();
    resetColor();
}

void print_success(const char *message) {
    setColor(COLOR_GREEN);
    printf("[SUCCESS] %s\n", message);
    resetColor();
}

void print_error(const char *message) {
    setColor(COLOR_RED);
    printf("[ERROR] %s\n", message);
    resetColor();
}

void print_warning(const char *message) {
    setColor(COLOR_YELLOW);
    printf("[WARNING] %s\n", message);
    resetColor();
}

void print_info(const char *message) {
    setColor(COLOR_CYAN);
    printf("[INFO] %s\n", message);
    resetColor();
}

char* prompt(const char *str) {
    setColor(COLOR_MAGENTA);
    printf("\n%s\n", str);
    setColor(COLOR_WHITE);
    printf("> ");
    resetColor();
    
    static char ans[100];
    scanf("%s", ans);
    return ans;
}

// Функции для работы с очередью
Queue* create_queue() {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue *q, Record record) {
    QueueNode *new_node = (QueueNode*)malloc(sizeof(QueueNode));
    new_node->record = record;
    new_node->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = new_node;
    } else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
}

int dequeue(Queue *q, Record *record) {
    if (q->front == NULL) {
        return 0; // Очередь пуста
    }
    
    QueueNode *temp = q->front;
    *record = temp->record;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    q->size--;
    return 1;
}

int is_queue_empty(Queue *q) {
    return q->front == NULL;
}

void free_queue(Queue *q) {
    while (!is_queue_empty(q)) {
        Record temp;
        dequeue(q, &temp);
    }
    free(q);
}

Queue* load_to_memory() {
    FILE *file = fopen("database.dat", "rb");
    if (!file) {
        return NULL;
    }

    Queue *q = create_queue();
    for (int i = 0; i < N; ++i) {
        Record record;
        fread(&record, sizeof(Record), 1, file);
        enqueue(q, record);
    }
    fclose(file);
    return q;
}

void make_index_array_from_queue(Record *arr[], Queue *q, int n) {
    QueueNode *current = q->front;
    for (int i = 0; i < n && current != NULL; i++) {
        arr[i] = &(current->record);
        current = current->next;
    }
}

int compare_records(const Record *record1, const Record *record2) {
    int street_cmp = strcmp(record1->street, record2->street);
    if (street_cmp != 0) {
        return street_cmp;
    }
    return record1->home - record2->home;
}

int compare_for_heap(const void *a, const void *b) {
    Record *r1 = *(Record**)a;
    Record *r2 = *(Record**)b;
    return compare_records(r1, r2);
}

void heapify(Record *array[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && compare_for_heap(&array[left], &array[largest]) > 0) {
        largest = left;
    }

    if (right < n && compare_for_heap(&array[right], &array[largest]) > 0) {
        largest = right;
    }

    if (largest != i) {
        Record *temp = array[i];
        array[i] = array[largest];
        array[largest] = temp;

        heapify(array, n, largest);
    }
}

void HeapSort(Record *array[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(array, n, i);
    }
    
    for (int i = n - 1; i > 0; i--) {
        Record *temp = array[0];
        array[0] = array[i];
        array[i] = temp;
        
        heapify(array, i, 0);
    }
}

void print_table_header() {
    setColor(COLOR_CYAN);
    printf("+------+----------------------------------+-----------------+------+-----+------------+\n");
    setColor(COLOR_YELLOW);
    printf("|  ID  | Full Name                        | Street          | Home | Apt | Date       |\n");
    setColor(COLOR_CYAN);
    printf("+------+----------------------------------+-----------------+------+-----+------------+\n");
    resetColor();
}

void print_record(Record *record, int i) {
    if (i % 2 == 0) {
        setColor(COLOR_WHITE);
    } else {
        setColor(COLOR_GRAY);
    }
    
    printf("| ");
    setColor(COLOR_YELLOW);
    printf("%4d", i);
    setColor(i % 2 == 0 ? COLOR_WHITE : COLOR_GRAY);
    printf(" | ");
    setColor(COLOR_GREEN);
    printf("%-32s", record->fio);
    setColor(i % 2 == 0 ? COLOR_WHITE : COLOR_GRAY);
    printf(" | ");
    setColor(COLOR_MAGENTA);
    printf("%-15s", record->street);
    setColor(i % 2 == 0 ? COLOR_WHITE : COLOR_GRAY);
    printf(" | ");
    setColor(COLOR_CYAN);
    printf("%4d", record->home);
    setColor(i % 2 == 0 ? COLOR_WHITE : COLOR_GRAY);
    printf(" | ");
    setColor(COLOR_BLUE);
    printf("%3d", record->appartament);
    setColor(i % 2 == 0 ? COLOR_WHITE : COLOR_GRAY);
    printf(" | ");
    setColor(COLOR_RED);
    printf("%-10s", record->date);
    setColor(i % 2 == 0 ? COLOR_WHITE : COLOR_GRAY);
    printf(" |\n");
}

void print_table_footer() {
    setColor(COLOR_CYAN);
    printf("+------+----------------------------------+-----------------+------+-----+------------+\n");
    resetColor();
}

void show_list(Record *ind_arr[], int n) {
    int ind = 0;
    while (1) {
        system("cls");
        print_header_title("DATABASE RECORDS LIST");
        
        printf("\n");
        print_table_header();
        
        for (int i = 0; i < 20 && (ind + i) < n; i++) {
            print_record(ind_arr[ind + i], ind + i + 1);
        }
        
        print_table_footer();
        
        // Pagination info
        setColor(COLOR_YELLOW);
        printf("\n Page %d of %d | Total records: %d\n", 
               (ind / 20) + 1, (n / 20) + 1, n);
        
        // Navigation help
        setColor(COLOR_GREEN);
        printf("\n Navigation Controls:\n");
        printf(" - W - Next Page          A - First Page        E - +10 Pages\n");
        printf(" - S - Previous Page      Q - Last Page         D - -10 Pages\n");
        printf(" - Any other key - Return to Menu\n");
        resetColor();
        
        char *chose = prompt("Enter your choice:");
        
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

int compare_search(const char *street, const char *key) {
    return strncmp(street, key, 3);
}

int binary_search(Record *arr[], const char *key, int *start_index) {
    int left = 0;
    int right = N - 1;
    int found_index = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = compare_search(arr[mid]->street, key);
        
        if (cmp == 0) {
            found_index = mid;
            right = mid - 1;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (found_index == -1) {
        return 0;
    }
    
    *start_index = found_index;
    int count = 1;
    
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
        system("cls");
        print_header_title("STREET SEARCH");
        
        print_info("Enter first 3 letters of street name to search");
        
        key = prompt("Input first 3 letters of street name");
        if (strlen(key) == 0) {
            print_error("Please enter search key");
            Sleep(2000);
            continue;
        }
        
        char search_key[4] = {0};
        strncpy(search_key, key, 3);
        search_key[3] = '\0';
        
        int start_index;
        int count = binary_search(arr, search_key, &start_index);
        
        if (count == 0) {
            print_error("No records found for street starting with given letters");
        } else {
            print_success("Records found successfully!");
            printf("   Found: %d records | Search: '%s'\n", count, search_key);
            
            Record **found_arr = (Record**)malloc(count * sizeof(Record*));
            for (int i = 0; i < count; i++) {
                found_arr[i] = arr[start_index + i];
            }
            
            show_list(found_arr, count);
            free(found_arr);
        }
        
        print_warning("Do you want to search again?");
        char *again = prompt("Search again? (y/n)");
        if (again[0] != 'y' && again[0] != 'Y') {
            break;
        }
        
    } while (1);
}

void show_record_by_number(Record *arr[]) {
    system("cls");
    print_header_title("RECORD VIEWER BY NUMBER");
    
    print_info("Enter record number between 1 and 4000");
    
    char *input = prompt("Enter record number (1-4000) or 'q' to quit");
    
    if (input[0] == 'q' || input[0] == 'Q') {
        return;
    }
    
    int record_number = atoi(input);
    
    if (record_number < 1 || record_number > N) {
        print_error("Invalid record number! Please enter number between 1 and 4000");
        printf("\nPress any key to continue...");
        getchar();
        getchar();
        return;
    }
    
    system("cls");
    print_header_title("RECORD DETAILS");
    
    printf("\n");
    print_table_header();
    print_record(arr[record_number - 1], record_number);
    print_table_footer();
    
    print_info("Press any key to continue...");
    getchar();
    getchar();
}

// АЛГОРИТМ A2 - ОПТИМАЛЬНОЕ ДЕРЕВО ПОИСКА
void SDPREC(Record *D, Vertex **p) {
    if (!*p) {
        *p = (Vertex*)malloc(sizeof(Vertex));
        (*p)->data = D;
        (*p)->left = NULL;
        (*p)->right = NULL;
    } else if (D->appartament < (*p)->data->appartament) {
        SDPREC(D, &(*p)->left);
    } else if (D->appartament >= (*p)->data->appartament) {
        SDPREC(D, &(*p)->right);
    }
}

void A2(int L, int R, int w[], Record *V[], Vertex **root) {
    int wes = 0, sum = 0;
    int i;
    if (L <= R) {
        // Считаем общий вес
        for (i = L; i <= R; i++)
            wes += w[i];
        
        // Ищем центр тяжести
        for (i = L; i <= R - 1; i++) {
            if ((sum < (wes / 2)) && ((sum + w[i]) > (wes / 2))) 
                break;
            sum += w[i];
        }
        
        // Добавляем вершину в дерево (исправленная индексация)
        SDPREC(V[i-1], root);
        
        // Рекурсивно строим левое и правое поддеревья
        A2(L, i - 1, w, V, root);
        A2(i + 1, R, w, V, root);
    }
}

void Print_tree(Vertex *p, int *i) {
    if (p) {
        Print_tree(p->left, i);
        print_record(p->data, (*i)++);
        Print_tree(p->right, i);
    }
}

void search_in_optimal_tree(Vertex *root, int key) {
    int i = 1;
    while (root) {
        if (key < root->data->appartament) {
            root = root->left;
        } else if (key > root->data->appartament) {
            root = root->right;
        } else if (key == root->data->appartament) {
            print_record(root->data, i++);
            root = root->right; // Для поиска возможных дубликатов
        }
    }
}

void rmtree(Vertex *root) {
    if (root) {
        rmtree(root->right);
        rmtree(root->left);
        free(root);
    }
}

// Функция для работы с оптимальным деревом
void optimal_tree_menu(Record *arr[], int n) {
    Vertex *root = NULL;
    
    // Создаем массив весов (в данном случае используем случайные веса)
    int *w = (int*)malloc((n + 1) * sizeof(int));
    for (int i = 0; i <= n; ++i) {
        w[i] = rand() % 100 + 1; // Веса от 1 до 100
    }
    
    // Строим оптимальное дерево
    A2(1, n, w, arr, &root);
    
    char *input;
    do {
        system("cls");
        print_header_title("OPTIMAL SEARCH TREE (ALGORITHM A2)");
        
        printf("\n");
        print_table_header();
        int count = 1;
        Print_tree(root, &count);
        print_table_footer();
        
        setColor(COLOR_GREEN);
        printf("\n Tree Statistics:\n");
        printf(" - Total nodes: %d\n", n);
        printf(" - Built using Algorithm A2 (optimal search tree)\n");
        resetColor();
        
        print_info("Enter apartment number to search (or 'q' to quit)");
        input = prompt("Input search key (apartment number):");
        
        if (input[0] == 'q' || input[0] == 'Q') {
            break;
        }
        
        int key = atoi(input);
        if (key == 0) {
            print_error("Invalid apartment number!");
            Sleep(2000);
            continue;
        }
        
        system("cls");
        print_header_title("SEARCH RESULTS IN OPTIMAL TREE");
        
        printf("Searching for apartment number: ");
        setColor(COLOR_YELLOW);
        printf("%d\n", key);
        resetColor();
        
        printf("\n");
        print_table_header();
        search_in_optimal_tree(root, key);
        print_table_footer();
        
        print_info("Press any key to continue...");
        getchar();
        getchar();
        
    } while (1);
    
    rmtree(root);
    free(w);
}

void print_main_menu() {
    system("cls");
    print_header_title("📊 DATABASE MANAGEMENT SYSTEM 🗃️");
    
    setColor(COLOR_GREEN);
    printf("|                            🎯 MAIN MENU 🎯                                  |\n");
    print_single_line();
    printf("|  📋 1 - Show Unsorted List                                                |\n");
    printf("|  🔄 2 - Show Sorted List (by street and house)                            |\n");
    printf("|  🔍 3 - Search by Street (first 3 letters)                                |\n");
    printf("|  📖 4 - Show Record by Number                                             |\n");
    printf("|  🌳 5 - Show Optimal Search Tree (Algorithm A2)                           |\n");
    printf("|  🔎 6 - Search in Optimal Tree                                            |\n");
    printf("|  🚪 0 - Exit Program                                                      |\n");
    print_double_line();
    resetColor();
    
    print_info("🎮 Please select an option from the menu above");
}

void mainloop_with_queue(Record *unsorted_ind_array[], Record *sorted_ind_array[], Queue *q) {
    while (1) {
        system("chcp 65001 > nul");
        print_main_menu();
        
        char *chose = prompt("👉 Enter your choice");
        
        system("chcp 866 > nul");
        
        switch (chose[0]) {
            case '1':
                system("chcp 65001 > nul");
                printf("📋 Loading unsorted list from QUEUE...\n");
                system("chcp 866 > nul");
                show_list(unsorted_ind_array, N);
                break;
            case '2':
                system("chcp 65001 > nul");
                printf("🔄 Loading sorted list...\n");
                system("chcp 866 > nul");
                show_list(sorted_ind_array, N);
                break;
            case '3':
                system("chcp 65001 > nul");
                printf("🔍 Starting street search...\n");
                system("chcp 866 > nul");
                search_by_street(sorted_ind_array);
                break;
            case '4':
                system("chcp 65001 > nul");
                printf("📖 Opening record viewer from QUEUE...\n");
                system("chcp 866 > nul");
                show_record_by_number(unsorted_ind_array);
                break;
            case '5':
            case '6':
                system("chcp 65001 > nul");
                printf("🌳 Building optimal search tree with Algorithm A2...\n");
                system("chcp 866 > nul");
                optimal_tree_menu(unsorted_ind_array, N);
                break;
            case '0':
                system("chcp 65001 > nul");
                return;
            default:
                system("chcp 65001 > nul");
                print_error("❌ Invalid choice. Please try again.");
                Sleep(1500);
        }
    }
}

int main() {
    system("chcp 65001 > nul");
    
    system("cls");
    
    print_header_title("📊 DATABASE MANAGEMENT SYSTEM 🗃️");
    print_info("📂 Loading data from file to QUEUE...");
    
    system("chcp 866 > nul");
    
    Queue *queue = load_to_memory();
    if (!queue) {
        system("chcp 65001 > nul");
        print_error("❌ File 'database.dat' not found!");
        return 1;
    }
    
    Record *unsorted_ind_arr[N];
    Record *sorted_ind_arr[N];
    
    make_index_array_from_queue(unsorted_ind_arr, queue, N);
    make_index_array_from_queue(sorted_ind_arr, queue, N);
    
    system("chcp 65001 > nul");
    print_info("🔃 Sorting data by street and house number...");
    
    HeapSort(sorted_ind_arr, N);
    
    print_success("✅ System initialized successfully!");
    printf("   📈 Records loaded in queue: %d\n", queue->size);
    
    Sleep(1000);
    mainloop_with_queue(unsorted_ind_arr, sorted_ind_arr, queue);
    
    free_queue(queue);
    
    system("cls");
    system("chcp 65001 > nul");
    print_header_title("🎉 SYSTEM SHUTDOWN 👋");
    print_success("✅ Program completed successfully!");
    print_info("🙏 Thank you for using Database Management System");
    printf("\n");
    
    return 0;
}
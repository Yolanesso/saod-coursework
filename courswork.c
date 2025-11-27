#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

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

typedef struct QueueNode {
    Record *record;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *head;
    QueueNode *tail;
    int size;
} Queue;

typedef struct TreeNode {
    Record *record;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

typedef struct {
    Record *record;
    int weight;
} WeightedRecord;

Record *index_database[N];
Queue *search_queue = NULL;

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

void heapify(Record *array[], int L, int R) {
    Record *x = array[L];
    int i = L;
    
    while (1) {
        int j = 2 * i + 1;
        
        if (j > R) break;
        
        if (j < R && compare_for_heap(&array[j + 1], &array[j]) > 0) {
            j = j + 1;
        }
        
        if (compare_for_heap(&x, &array[j]) > 0) break;
        
        array[i] = array[j];
        i = j;
    }
    
    array[i] = x;
}

void HeapSort(Record *array[], int n) {
    int L = n / 2 - 1;
    
    while (L >= 0) {
        heapify(array, L, n - 1);
        L = L - 1;
    }
    
    int R = n - 1;
    
    while (R > 0) {
        Record *temp = array[0];
        array[0] = array[R];
        array[R] = temp;
        
        R = R - 1;
        
        if (R > 0) {
            heapify(array, 0, R);
        }
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

int compare_search(const char *street, const char *key) {
    return strncmp(street, key, 3);
}

int binary_search(Record *arr[], const char *key, int *first_index) {
    int left = 0;
    int right = N - 1;
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        int cmp = compare_search(arr[mid]->street, key);
        
        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    if (compare_search(arr[left]->street, key) == 0) {
        *first_index = left;
        return 1;
    } else {
        return 0;
    }
}

Queue* create_queue() {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue *q, Record *record) { 
    QueueNode *new_node = (QueueNode*)malloc(sizeof(QueueNode));
    new_node->record = record;
    new_node->next = NULL;
    
    if (q->tail == NULL) {
        q->head = q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->size++;
}

Record* dequeue(Queue *q) {
    if (q->head == NULL) {
        return NULL;
    }
    
    QueueNode *temp = q->head;
    Record *record = temp->record;
    
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    
    free(temp);
    q->size--;
    return record;
}

int is_queue_empty(Queue *q) {
    return q->head == NULL;
}

void free_queue(Queue *q) {
    while (!is_queue_empty(q)) {
        dequeue(q);
    }
    free(q);
}

void add_to_queue(Record *record) {
    if (search_queue == NULL) {
        search_queue = create_queue();
    }
    enqueue(search_queue, record);
}

void print_queue() {
    if (search_queue == NULL || is_queue_empty(search_queue)) {
        printf("Queue is empty\n");
        return;
    }
    
    printf("\n=== FOUND RECORDS QUEUE ===\n");
    printf("Queue size: %d\n", search_queue->size);
    printf("Head: %s, Tail: %s\n", 
           search_queue->head->record->street, 
           search_queue->tail->record->street);
    print_head();
    
    QueueNode *current = search_queue->head;
    int i = 1;
    while (current != NULL) {
        print_record(current->record, i++);
        current = current->next;
    }
}

void search_database() {
    char search_key[4] = {0};
    int first_index;
    int found_count = 0;
    
    do {
        system("cls");
        printf("\n=== BINARY SEARCH IN DATABASE ===\n");
        printf("Search by first 3 letters of street name\n\n");
        
        char *input = prompt("Enter first 3 letters of street name (or 'q' to quit)");
        
        if (input[0] == 'q' || input[0] == 'Q') {
            return;
        }
        
        if (strlen(input) < 3) {
            printf("Error: Please enter at least 3 characters\n");
            printf("Press any key to continue...");
            getchar();
            continue;
        }
        
        strncpy(search_key, input, 3);
        search_key[3] = '\0';
        
        if (search_queue != NULL) {
            free_queue(search_queue);
            search_queue = NULL;
        }
        
        int search_result = binary_search(index_database, search_key, &first_index);
        
        if (!search_result) {
            printf("No records found for street starting with '%s'\n", search_key);
        } else {
            found_count = 0;
            for (int i = first_index; i < N; i++) {
                if (compare_search(index_database[i]->street, search_key) == 0) {
                    add_to_queue(index_database[i]);
                    found_count++;
                } else {
                    break;
                }
            }
            
            printf("Found %d records for street starting with '%s'\n", found_count, search_key);
            printf("First occurrence at index: %d\n", first_index + 1);
            
            print_queue();
        }
        
        char *again = prompt("\nSearch again? (y/n)");
        if (again[0] != 'y' && again[0] != 'Y') {
            break;
        }
        
    } while (1);
    
    if (search_queue != NULL) {
        free_queue(search_queue);
        search_queue = NULL;
    }
}

void show_record_by_number(Record *arr[]) {
    char *input = prompt("Enter record number (1-4000) or 'q' to quit");
    
    if (input[0] == 'q' || input[0] == 'Q') {
        return;
    }
    
    int record_number = atoi(input);
    
    if (record_number < 1 || record_number > N) {
        printf("Invalid record number! Please enter a number between 1 and %d\n", N);
        return;
    }
    
    system("cls");
    printf("=== RECORD %d ===\n", record_number);
    print_head();
    print_record(arr[record_number - 1], record_number);
    
    printf("\nPress any key to continue...");
    getchar();
    getchar();
}

TreeNode* create_tree_node(Record *record) {
    TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode));
    node->record = record; 
    node->left = node->right = NULL;
    return node;
}

int compare_dates(const char *date1, const char *date2) {
    int day1, month1, year1;
    int day2, month2, year2;
    
    sscanf(date1, "%d-%d-%d", &day1, &month1, &year1);
    sscanf(date2, "%d-%d-%d", &day2, &month2, &year2);
    
    if (year1 < 100) year1 += 1900;
    if (year2 < 100) year2 += 1900;
    
    if (year1 != year2) return (year1 < year2) ? -1 : 1;
    if (month1 != month2) return (month1 < month2) ? -1 : 1;
    if (day1 != day2) return (day1 < day2) ? -1 : 1;
    return 0;
}

int compare_weighted_records_by_date(const void *a, const void *b) {
    WeightedRecord *wr1 = (WeightedRecord*)a;
    WeightedRecord *wr2 = (WeightedRecord*)b;
    return compare_dates(wr1->record->date, wr2->record->date);
}

TreeNode* insert_to_tree_by_date(TreeNode *root, Record *record) {
    if (root == NULL) {
        return create_tree_node(record);
    }
    
    int cmp = compare_dates(record->date, root->record->date);
    if (cmp < 0) {
        root->left = insert_to_tree_by_date(root->left, record);
    } else if (cmp > 0) {
        root->right = insert_to_tree_by_date(root->right, record);
    }
    
    return root;
}

int generate_random_weight() {
    return rand() % 100 + 1;
}

void A2_by_date(int L, int R, int w[], Record *V[], TreeNode **root) {
    int wes = 0, sum = 0;
    int i;
    
    if (L <= R) {
        for (i = L; i <= R; i++)
            wes += w[i];

        for (i = L; i <= R - 1; i++) {
            if ((sum < (wes / 2)) && ((sum + w[i]) > (wes / 2)))
                break;
            sum += w[i];
        }

        *root = insert_to_tree_by_date(*root, V[i]);
        A2_by_date(L, i - 1, w, V, root);
        A2_by_date(i + 1, R, w, V, root);
    }
}

TreeNode* build_optimal_tree_from_queue_by_date(Queue *q) {
    if (is_queue_empty(q)) {
        return NULL;
    }
    
    int count = q->size;
    Record **V = (Record**)malloc(count * sizeof(Record*));
    int *w = (int*)malloc(count * sizeof(int));
    
    QueueNode *current = q->head;
    for (int i = 0; i < count && current != NULL; i++) {
        V[i] = current->record;
        w[i] = generate_random_weight();
        current = current->next;
    }
    
    WeightedRecord *temp = (WeightedRecord*)malloc(count * sizeof(WeightedRecord));
    for (int i = 0; i < count; i++) {
        temp[i].record = V[i];
        temp[i].weight = w[i];
    }
    
    qsort(temp, count, sizeof(WeightedRecord), compare_weighted_records_by_date);
    
    for (int i = 0; i < count; i++) {
        V[i] = temp[i].record;
        w[i] = temp[i].weight;
    }
    
    free(temp);
    
    TreeNode *root = NULL;
    A2_by_date(0, count - 1, w, V, &root);
    
    free(V);
    free(w);
    
    return root;
}

void print_tree_inorder(TreeNode *root, int *count) {
    if (root != NULL) {
        print_tree_inorder(root->left, count);
        print_record(root->record, (*count)++);
        print_tree_inorder(root->right, count);
    }
}

void print_tree(TreeNode *root) {
    printf("\n=== OPTIMAL SEARCH TREE (by DATE) ===\n");
    printf("Tree built using A2 algorithm with RANDOM weights\n");
    print_head();
    int count = 1;
    print_tree_inorder(root, &count);
}

TreeNode* search_tree_by_date(TreeNode *root, const char *input_date) {
    if (root == NULL) return NULL;
    
    char search_date[20];
    if (strchr(input_date, '.')) {
        int day, month, year;
        sscanf(input_date, "%d.%d.%d", &day, &month, &year);
        if (year < 100) year += 1900;
        sprintf(search_date, "%02d-%02d-%02d", day, month, year % 100);
    } else {
        strcpy(search_date, input_date);
    }
    
    int cmp = compare_dates(search_date, root->record->date);
    if (cmp == 0) {
        return root;
    } else if (cmp < 0) {
        return search_tree_by_date(root->left, input_date);
    } else {
        return search_tree_by_date(root->right, input_date);
    }
}

void search_tree_by_date_range(TreeNode *root, const char *input_start_date, const char *input_end_date, int *count) {
    if (root != NULL) {
        char start_date[20], end_date[20];
        
        if (strchr(input_start_date, '.')) {
            int day, month, year;
            sscanf(input_start_date, "%d.%d.%d", &day, &month, &year);
            if (year < 100) year += 1900;
            sprintf(start_date, "%02d-%02d-%02d", day, month, year % 100);
        } else {
            strcpy(start_date, input_start_date);
        }
        
        if (strchr(input_end_date, '.')) {
            int day, month, year;
            sscanf(input_end_date, "%d.%d.%d", &day, &month, &year);
            if (year < 100) year += 1900;
            sprintf(end_date, "%02d-%02d-%02d", day, month, year % 100);
        } else {
            strcpy(end_date, input_end_date);
        }
                
        int cmp_start = compare_dates(root->record->date, start_date);
        int cmp_end = compare_dates(root->record->date, end_date);
        
        if (cmp_start >= 0) {
            search_tree_by_date_range(root->left, input_start_date, input_end_date, count);
        }
        
        if (cmp_start >= 0 && cmp_end <= 0) {
            print_record(root->record, (*count)++);
        }
        
        if (cmp_end <= 0) {
            search_tree_by_date_range(root->right, input_start_date, input_end_date, count);
        }
    }
}

void search_in_tree_by_date(TreeNode *root) {
    printf("\n=== SEARCH IN OPTIMAL TREE BY DATE ===\n");
    printf("Note: Date format in database: DD-MM-YY (e.g., 26-12-96)\n");
    printf("You can search using: DD-MM-YY, DD.MM.YY, or DD.MM.YYYY\n\n");
    
    char *search_type = prompt("1: Exact date search\n2: Date range search\n0: Back to menu");
    
    switch (search_type[0]) {
        case '1': {
            char *input_date = prompt("Enter date to search (e.g., 26-12-96 or 26.12.1996)");
            TreeNode *result = search_tree_by_date(root, input_date);
            if (result == NULL) {
                printf("Record with date '%s' not found in optimal tree\n", input_date);
            } else {
                printf("Record found in optimal tree:\n");
                print_head();
                print_record(result->record, 1);
            }
            break;
        }
            case '2': {
            char start_date_input[20], end_date_input[20];
            
            printf("Enter start date (e.g., 01-01-96 or 01.01.1996)\n> ");
            scanf("%19s", start_date_input);
            
            printf("Enter end date (e.g., 31-12-96 or 31.12.1996)\n> ");
            scanf("%19s", end_date_input);
            
            // Очистка буфера ввода
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            
            printf("DEBUG BEFORE CALL: start='%s', end='%s'\n", start_date_input, end_date_input);
            
            printf("\nRecords in date range %s - %s:\n", start_date_input, end_date_input);
            print_head();
            int count = 1;
            search_tree_by_date_range(root, start_date_input, end_date_input, &count);
            
            if (count == 1) {
                printf("No records found in specified date range\n");
            } else {
                printf("\nTotal records found: %d\n", count - 1);
            }
            break;
        }
        case '0':
            return;
        default:
            printf("Invalid choice\n");
    }
    
    printf("\nPress any key to continue...");
    getchar();
    getchar();
}

void free_tree(TreeNode *root) {
    if (root != NULL) {
        free_tree(root->left);
        free_tree(root->right);
        free(root);
    }
}

void mainloop(Record *unsorted_ind_array[], Record *sorted_ind_array[]) {
    for (int i = 0; i < N; i++) {
        index_database[i] = sorted_ind_array[i];
    }
    
    while (1) {
        system("cls");
        printf("\n=== DATABASE MANAGEMENT SYSTEM ===\n");
        printf("Total records: %d\n\n", N);
        printf("SORT KEY: Street + House number\n");
        printf("SEARCH METHOD: Binary Search (Version 2)\n");
        printf("QUEUE: Classical implementation with head and tail\n\n");
        
        char *chose = prompt("1: Show unsorted list\n"
                             "2: Show sorted list (by street and house)\n"
                             "3: Binary search by street key\n"
                             "4: Show record by number\n"
                             "5: Create queue and build OPTIMAL search tree by DATE\n"
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
                search_database();
                break;
            case '4':
                printf("\n=== SHOW RECORD BY NUMBER ===\n");
                show_record_by_number(unsorted_ind_array);
                break;
            case '5':
                printf("\n=== CREATE QUEUE AND BUILD OPTIMAL SEARCH TREE BY DATE ===\n");
                {
                    char *key = prompt("Enter first 3 letters of street name to create queue");
                    if (strlen(key) < 3) {
                        printf("Please enter at least 3 characters\n");
                        printf("Press any key to continue...");
                        getchar();
                        break;
                    }
                    
                    char search_key[4] = {0};
                    strncpy(search_key, key, 3);
                    
                    int first_index;
                    if (binary_search(index_database, search_key, &first_index)) {
                        Queue *q = create_queue();
                        
                        for (int i = first_index; i < N; i++) {
                            if (compare_search(index_database[i]->street, search_key) == 0) {
                                enqueue(q, index_database[i]);
                            } else {
                                break;
                            }
                        }
                        
                        print_queue(q);
                        
                        printf("\nBuilding optimal search tree with random weights...\n");
                        TreeNode *tree = build_optimal_tree_from_queue_by_date(q);
                        printf("Optimal tree built successfully!\n");
                        
                        print_tree(tree);
                        search_in_tree_by_date(tree);
                        
                        free_queue(q);
                        free_tree(tree);
                    } else {
                        printf("No records found for street starting with '%s'\n", search_key);
                        printf("Press any key to continue...");
                        getchar();
                    }
                }
                break;
            case '0':
                return;
            default:
                printf("Invalid choice. Please try again.\n");
                printf("Press any key to continue...");
                getchar();
        }
    }
}

int main() {
    srand(time(NULL));
    
    printf("Loading data...\n");
    Node *root = load_to_memory();
    if (!root) {
        printf("Error: File 'database.dat' not found\n");
        printf("Make sure database.dat is in the same directory as the program\n");
        printf("Press any key to exit...");
        getchar();
        return 1;
    }
    
    Record *unsorted_ind_arr[N];
    Record *sorted_ind_arr[N];
    
    make_index_array(unsorted_ind_arr, root, N);
    make_index_array(sorted_ind_arr, root, N);
    
    printf("Sorting data by street and house number using Heap Sort...\n");
    HeapSort(sorted_ind_arr, N);
    
    printf("Data loaded successfully. Total records: %d\n", N);
    printf("Press any key to continue...");
    getchar();
    
    mainloop(unsorted_ind_arr, sorted_ind_arr);
    
    while (root) {
        Node *temp = root;
        root = root->next;
        free(temp);
    }
    
    printf("Program finished.\n");
    return 0;
}
// --------------- BIBLIOTECAS / STRUCTS / DEFINES ------------------//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysinfo.h>


#define RECORD_SIZE 100
#define KEY_SIZE 4
#define DATA_SIZE 96

// Estrutura que representa um registro
typedef struct {
    int key;               // Chave de 32 bits (4 bytes)
    unsigned char data[DATA_SIZE]; // Dados (96 bytes)
} Record;

// Estrutura para passar parâmetros para as threads
typedef struct {
    Record *arr;
    int left;
    int right;
    int num_threads;
} ThreadData;

size_t file_size; 

// ------------------ FUNÇÕES DE ORDENAÇÃO PARALELA ------------------//

// Função para mesclar duas metades ordenadas
void merge(Record *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Record *L = (Record*)malloc(n1 * sizeof(Record));
    Record *R = (Record*)malloc(n2 * sizeof(Record));

    if (L == NULL || R == NULL) {
        perror("Erro ao alocar memória para os arrays temporários");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n1; i++) {
        L[i] = arr[left + i];
    }
    for (int i = 0; i < n2; i++) {
        R[i] = arr[mid + 1 + i];
    }

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (L[i].key <= R[j].key) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1) {
        arr[k++] = L[i++];
    }
    while (j < n2) {
        arr[k++] = R[j++];
    }

    free(L);
    free(R);
}

// Função que será executada por cada thread
void *mergeSortParallel(void *args) {
    ThreadData *data = (ThreadData *)args;
    Record *arr = data->arr;
    int left = data->left;
    int right = data->right;
    int num_threads = data->num_threads;

    if (left >= right) {
        return NULL;
    }

    int mid = left + (right - left) / 2;

    if (num_threads > 1) {
        ThreadData *leftData = malloc(sizeof(ThreadData));
        ThreadData *rightData = malloc(sizeof(ThreadData));

        if (leftData == NULL || rightData == NULL) {
            perror("Erro ao alocar memória para dados das threads");
            exit(EXIT_FAILURE);
        }

        *leftData = (ThreadData){arr, left, mid, num_threads / 2};
        *rightData = (ThreadData){arr, mid + 1, right, num_threads / 2};

        pthread_t leftThread, rightThread;

        pthread_create(&leftThread, NULL, mergeSortParallel, (void *)leftData);
        pthread_create(&rightThread, NULL, mergeSortParallel, (void *)rightData);

        pthread_join(leftThread, NULL);
        pthread_join(rightThread, NULL);

        free(leftData);
        free(rightData);
    } else {
        ThreadData leftData = {arr, left, mid, 1};
        ThreadData rightData = {arr, mid + 1, right, 1};
        mergeSortParallel((void *)&leftData);
        mergeSortParallel((void *)&rightData);
    }

    merge(arr, left, mid, right);
    return NULL;
}

// Função principal para iniciar o Merge Sort Paralelo
void mergeSort(Record *arr, int num_records, int num_threads) {
    ThreadData *data = malloc(sizeof(ThreadData));
    if (data == NULL) {
        perror("Erro ao alocar memória para ThreadData");
        exit(EXIT_FAILURE);
    }

    *data = (ThreadData){arr, 0, num_records - 1, num_threads};
    pthread_t mainThread;

    pthread_create(&mainThread, NULL, mergeSortParallel, (void *)data);
    pthread_join(mainThread, NULL);

    free(data);
}

// -------------------------- FUNÇÕES DE I/O --------------------------//

// Função para imprimir o vetor de registros obtidos utilizada para debug
void print_record(Record *arr, int num_records) {
    for (int i = 0; i < num_records; i++) {
        printf("Registro %d:\n", i + 1);
        printf("  Chave: %d\n", arr[i].key);
        printf("  Dados: ");
    for (int j = 0; j < DATA_SIZE; j++) {
        printf("%02x ", arr[i].data[j]);  // Imprime os dados como hexadecimal
    }
        printf("\n\n");
    }
}

// Função para ler os registros do arquivo de entrada
int read_records(const char *input_filename, Record **records) {
    int input_file = open(input_filename, O_RDONLY);
    if (input_file < 0) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    struct stat file_stat;
    if (fstat(input_file, &file_stat) < 0) {
        perror("Erro ao obter informações do arquivo");
        close(input_file);
        return 1;
    }

    file_size = file_stat.st_size;
    size_t num_records = file_size / RECORD_SIZE;

    *records = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, input_file, 0);
    if (*records == MAP_FAILED) {
        perror("Erro ao mapear o arquivo para memória");
        close(input_file);
        return 1;
    }

    close(input_file);

    return num_records;
}

// Função para escrever os registros no arquivo de saída
void write_records(const char *output_filename, Record *records, int num_records) {

    // Abrir o arquivo de saída
    int output_file = open(output_filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (output_file < 0) {
        perror("Erro ao abrir arquivo de saída");
        munmap(records, file_size);
        return;
    }

    if (write(output_file, records, file_size) != file_size) {
        perror("Erro no arquivo de output");
        close(output_file);
        free(records);
        munmap(records, file_size);
        return;
    }

    fsync(output_file);
    close(output_file);
    munmap(records, file_size);

    return;
}

// -------------------------- FUNÇÃO PRINCIPAL --------------------------//

int main(int argc, char *argv[]) {

    // Verificar os argumentos de entrada do programa

    if (argc < 4) {
        fprintf(stderr, "Uso: %s <entrada> <saida> <threads>\n", argv[0]);
        return 1;
    }

    // Nomes dos arquivos de Entrada e Saída : 

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    int threads = atoi(argv[3]);
    int num_threads_sys;

    // Determinar o número de threads, se necessário

    if (threads == 0) {
        threads = get_nprocs(); // Número de núcleos de CPU disponíveis
        num_threads_sys = get_nprocs_conf(); // Número de núcleos de CPU configurados

        if (threads == -1 || num_threads_sys == -1) {
            perror("Erro ao obter o número de processadores");
            exit(EXIT_FAILURE);
        }

        threads = (threads > num_threads_sys) ? threads : num_threads_sys; // Atribui sempre o maior valor entre núcleos disponíveis ou configurados. 
    }

    // Ler os registros do arquivo de entrada
    Record *records;
    int num_records = read_records(input_filename, &records);

    // Se o arquivo de entrada não possui registros
    
    if (num_records < 0) {
        printf("Não existem registros no arquivo de entrada.");
        return 1;
    }

    mergeSort(records, num_records, threads);

    write_records(output_filename, records, num_records);

    return 0;
}
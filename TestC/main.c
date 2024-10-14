/*
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// Estructura de un barco
typedef struct {
    int id;           // ID del barco
    int tiempo;       // Tiempo total que le toma cruzar el canal
    int tiempo_restante; // Tiempo restante para completar el cruce
    int prioridad;    // Prioridad del barco (mayor valor = mayor prioridad)
    int lado;         // 0 = Izquierda (Atlántico), 1 = Derecha (Pacífico)
    int tipo;         // Tipo de barco: 0 = Normal, 1 = Pesquero, 2 = Patrulla
    int arrival_time; // Tiempo de llegada del barco al canal
    pthread_t hilo;   // Hilo asociado al barco
    int en_ejecucion; // 0 = No, 1 = Sí
} Barco;

pthread_mutex_t mutex;  // Mutex para proteger la interrupción
int tiempo_global = 0;  // Tiempo actual del sistema

// Función que simula el cruce del canal por un barco
void* cruzar_canal(void* arg) {
    Barco* barco = (Barco*) arg;  // Convertimos el argumento a un puntero a Barco
    
    while (barco->tiempo_restante > 0) {
        pthread_mutex_lock(&mutex);
        if (barco->en_ejecucion) {
            printf("Barco %d está cruzando. Tiempo restante: %d segundos\n", barco->id, barco->tiempo_restante);
            sleep(1);  // Simulamos un segundo de ejecución
            barco->tiempo_restante--;
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    printf("Barco %d ha completado el cruce.\n", barco->id);
    return NULL;
}

// Función para encontrar el barco con el menor tiempo restante
int seleccionar_barco(Barco* barcos, int num_barcos) {
    int indice = -1;
    int menor_tiempo = 9999;

    for (int i = 0; i < num_barcos; i++) {
        if (barcos[i].tiempo_restante > 0 && barcos[i].arrival_time <= tiempo_global) {
            if (barcos[i].tiempo_restante < menor_tiempo) {
                menor_tiempo = barcos[i].tiempo_restante;
                indice = i;
            }
        }
    }
    return indice;
}

// Función que ejecuta el algoritmo Real Time (SRTF)
void ejecutar_RealTime(Barco* barcos, int num_barcos) {
    int barcos_completados = 0;

    while (barcos_completados < num_barcos) {
        int barco_en_ejecucion = seleccionar_barco(barcos, num_barcos);

        if (barco_en_ejecucion != -1) {
            Barco* barco = &barcos[barco_en_ejecucion];
            if (!barco->en_ejecucion) {
                barco->en_ejecucion = 1;
                pthread_create(&barco->hilo, NULL, cruzar_canal, barco);
            }
        }

        sleep(1);  // Simulamos el paso del tiempo
        tiempo_global++;

        // Verificamos si todos los barcos han terminado
        barcos_completados = 0;
        for (int i = 0; i < num_barcos; i++) {
            if (barcos[i].tiempo_restante == 0) {
                barcos_completados++;
            }
        }
    }

    // Esperamos que todos los hilos terminen
    for (int i = 0; i < num_barcos; i++) {
        pthread_join(barcos[i].hilo, NULL);
    }
}

int main() {
    // Inicializamos el mutex
    pthread_mutex_init(&mutex, NULL);

    // Creamos una lista de barcos
    Barco barcos[] = {
        {1, 6, 6, 1, 0, 0, 0, 0},  // Barco 1, llega en t0
        {2, 2, 2, 2, 1, 0, 3, 0},  // Barco 2, llega en t3
        {3, 4, 4, 2, 1, 0, 1, 0},  // Barco 3, llega en t1
    };
    
    int num_barcos = sizeof(barcos) / sizeof(barcos[0]);

    printf("\n--- Ejecución con Real Time (SRTF) ---\n");
    ejecutar_RealTime(barcos, num_barcos);

    // Destruimos el mutex
    pthread_mutex_destroy(&mutex);
    
    return 0;
}
*/


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  // Para usar sleep
#include <stdlib.h>  // Para usar qsort
#include <time.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>

#define MAX_BARCOS 100

// Estructura de un barco
typedef struct {
    int id;           // ID del barco
    int tiempo;       // Tiempo que le toma cruzar el canal
    int tiempo_restante; // Tiempo restante para completar el cruce
    int prioridad;    // Prioridad del barco (mayor valor = mayor prioridad)
    int lado;         // 0 = Izquierda (Atlántico), 1 = Derecha (Pacífico)
    int tipo;         // Tipo de barco: 0 = Normal, 1 = Pesquero, 2 = Patrulla
    int arrival_time; // Tiempo de llegada del barco al canal
    pthread_t hilo;   // Hilo asociado al barco
    int en_ejecucion; // 0 = No, 1 = Sí
} Barco;
int W;
int Letrero;
bool estadoL = false;  // Variable que se enciende/apaga
char metodo[20];
int largo_canal;
int velocidad_barco;
int cantidad_barcos;

// Función que simula el cruce del canal por un barco
void* cruzar_canal(void* arg) {
    Barco* barco = (Barco*) arg;  // Convertimos el argumento a un puntero a Barco
    char* lado = (barco->lado == 0) ? "izquierda (Atlántico)" : "derecha (Pacífico)";
    char* tipo;
    
    // Definimos el tipo de barco para imprimir
    if (barco->tipo == 0) tipo = "Normal";
    else if (barco->tipo == 1) tipo = "Pesquero";
    else tipo = "Patrulla";
    
    printf("El barco %d (%s) está cruzando desde el lado %s. Tiempo de cruce: %d segundos, Prioridad: %d\n", 
           barco->id, tipo, lado, barco->tiempo, barco->prioridad);
    sleep(barco->tiempo);  // Simula el tiempo que toma cruzar el canal
    printf("El barco %d (%s) ha terminado de cruzar desde el lado %s\n", barco->id, tipo, lado);
    return NULL;
}

// Comparador para SJF (Shortest Job First), basado en el tiempo de cruce
int comparar_por_tiempo(const void* a, const void* b) {
    Barco* barcoA = (Barco*) a;
    Barco* barcoB = (Barco*) b;
    return barcoA->tiempo - barcoB->tiempo;  // Ordena de menor a mayor tiempo
}

// Comparador para Prioridad, basado en el valor de prioridad (mayor primero)
int comparar_por_prioridad(const void* a, const void* b) {
    Barco* barcoA = (Barco*) a;
    Barco* barcoB = (Barco*) b;
    return barcoA->prioridad - barcoB->prioridad;  // Ordena de mayor a menor prioridad
}

// Función para ejecutar el algoritmo FCFS para una lista de barcos
void ejecutar_FCFS(Barco* barcos, int num_barcos) {
    pthread_t hilos[num_barcos];
    
    for (int i = 0; i < num_barcos; i++) {
        pthread_create(&hilos[i], NULL, cruzar_canal, &barcos[i]);
        pthread_join(hilos[i], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
    }
}

// Función para ejecutar el algoritmo FCFS para una lista de barcos
void ejecutar_FCFS_Equidad(Barco* barcos_izquierda, Barco* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    pthread_t hilosI[num_barcos_izquierda];
    pthread_t hilosD[num_barcos_derecha];
    int contI = 0;
    int contD = 0;
    
    while (contI!=num_barcos_izquierda || contD!=num_barcos_derecha){
        //printf("El contI %d\nEl contD %d\n ",contI,contD);
        for (int i = 0; i < W; i++) {
            if (contI!=num_barcos_izquierda){
                pthread_create(&hilosI[contI], NULL, cruzar_canal, &barcos_izquierda[contI]);
                pthread_join(hilosI[contI], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
                contI++;
            }
        }
        for (int i = 0; i < W; i++) {
            if (contD!=num_barcos_derecha){
                pthread_create(&hilosD[contD], NULL, cruzar_canal, &barcos_derecha[contD]);
                pthread_join(hilosD[contD], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
                contD++;
            }
        }
    }   
}

void ejecutar_FCFS_Letrero(Barco* barcos_izquierda, Barco* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    pthread_t hilosI[num_barcos_izquierda];
    pthread_t hilosD[num_barcos_derecha];
    int contI = 0;
    int contD = 0;
    int time = 0;
    while (contI!=num_barcos_izquierda || contD!=num_barcos_derecha){
        printf("El contI %d\nEl contD %d\n ",contI,contD);
        printf("El Letrero %d\n",estadoL);
        if (contI==num_barcos_izquierda && !estadoL){
            estadoL = !estadoL;
            time = 0;
        }
        if (contD==num_barcos_derecha && estadoL){
            estadoL = !estadoL;
            time = 0;
        }
        if (estadoL){
            time+=barcos_derecha[contD].tiempo;
            pthread_create(&hilosD[contD], NULL, cruzar_canal, &barcos_derecha[contD]);
            pthread_join(hilosD[contD], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
            contD++;
        } else {
            time+=barcos_izquierda[contI].tiempo;
            pthread_create(&hilosI[contI], NULL, cruzar_canal, &barcos_izquierda[contI]);
            pthread_join(hilosI[contI], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
            contI++;
        }
        if (time>=Letrero){
            estadoL = !estadoL;
            time = 0;
        }
    }

    
     
}

// Función para ejecutar el algoritmo SJF para una lista de barcos
void ejecutar_SJF(Barco* barcos, int num_barcos) {
    qsort(barcos, num_barcos, sizeof(Barco), comparar_por_tiempo);
    ejecutar_FCFS(barcos, num_barcos);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

// Función para ejecutar el algoritmo SJF para una lista de barcos
void ejecutar_SJF_Equidad(Barco* barcos_izquierda, Barco* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco), comparar_por_tiempo);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco), comparar_por_tiempo);
    ejecutar_FCFS_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

void ejecutar_SJF_Letrero(Barco* barcos_izquierda, Barco* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco), comparar_por_tiempo);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco), comparar_por_tiempo);
    ejecutar_FCFS_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

// Función para ejecutar el algoritmo por Prioridad para una lista de barcos
void ejecutar_Priority(Barco* barcos, int num_barcos) {
    qsort(barcos, num_barcos, sizeof(Barco), comparar_por_prioridad);
    ejecutar_FCFS(barcos, num_barcos);
}

void ejecutar_Priority_Equidad(Barco* barcos_izquierda, Barco* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco), comparar_por_prioridad);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco), comparar_por_prioridad);
    ejecutar_FCFS_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

void ejecutar_Priority_Letrero(Barco* barcos_izquierda, Barco* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco), comparar_por_prioridad);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco), comparar_por_prioridad);
    ejecutar_FCFS_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

int leer_barcos_desde_archivo(Barco* barcos, const char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo ships.txt");
        return -1;
    }

    int i = 0;
    while (fscanf(archivo, "%d %d %d %d %d %d %d %d", 
                  &barcos[i].id, 
                  &barcos[i].tiempo,
                  &barcos[i].tiempo_restante, 
                  &barcos[i].prioridad, 
                  &barcos[i].lado, 
                  &barcos[i].tipo,
                  &barcos[i].arrival_time,
                  &barcos[i].en_ejecucion) != EOF) {
        i++;
        if (i >= MAX_BARCOS) {
            printf("Se alcanzó el número máximo de barcos soportado.\n");
            break;
        }
    }

    fclose(archivo);
    return i;  // Retorna el número de barcos leídos
}



int main() {

     FILE* file = fopen("canal.txt", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo de configuración");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "Metodo: %s\n", metodo);
    fscanf(file, "Largo del canal: %d\n", &largo_canal);
    fscanf(file, "Velocidad del barco: %d\n", &velocidad_barco);
    fscanf(file, "Cantidad de barcos: %d\n", &cantidad_barcos);
    fscanf(file, "Cambio de letrero: %d\n", &Letrero);
    fscanf(file, "Parametro W: %d\n", &W);
    
    fclose(file);

    // Puedes ahora utilizar las variables globales en el programa
    // Ejemplo de impresión para verificar los valores
    printf("Metodo: %s\n", metodo);
    printf("Largo del canal: %d\n", largo_canal);
    printf("Velocidad del barco: %d\n", velocidad_barco);
    printf("Cantidad de barcos: %d\n", cantidad_barcos);
    printf("Cambio de letrero: %d\n", Letrero);
    printf("Parametro W: %d\n", W);
    
    Barco barcos[MAX_BARCOS];
    int num_barcos = leer_barcos_desde_archivo(barcos, "ships.txt");

    if (num_barcos == -1) {
        return 1;  // Error al leer el archivo
    }

    // Listas separadas para barcos en lado izquierdo (Atlántico) y lado derecho (Pacífico)
    Barco barcos_izquierda[num_barcos];  // Lista para los barcos en el lado izquierdo
    Barco barcos_derecha[num_barcos];    // Lista para los barcos en el lado derecho

    int num_barcos_izquierda = 0;  // Contador para barcos en el lado izquierdo
    int num_barcos_derecha = 0;    // Contador para barcos en el lado derecho

    // Clasificamos los barcos en las dos listas separadas
    for (int i = 0; i < num_barcos; i++) {
        if (barcos[i].lado == 0) {  // Lado izquierdo (Atlántico)
            barcos_izquierda[num_barcos_izquierda++] = barcos[i];
        } else {  // Lado derecho (Pacífico)
            barcos_derecha[num_barcos_derecha++] = barcos[i];
        }
    }

    // Ejecutar algoritmos
    //printf("\n--- Ejecución con FCFS ---\n");
    //ejecutar_FCFS_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);

    //ejecutar_FCFS_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);

    //printf("\n--- Ejecución con SJF con Equidad ---\n");
    //ejecutar_SJF_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);

    //ejecutar_SJF_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);

    //printf("\n--- Ejecución con SJF (Shortest Job First) ---\n");
    //ejecutar_SJF(barcos, num_barcos);

    //printf("\n--- Ejecución con Prioridad con Equidad ---\n");
    //ejecutar_Priority_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);

    ejecutar_Priority_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);

    //printf("\n--- Ejecución con Prioridad ---\n");
    //ejecutar_Priority(barcos, num_barcos);

    return 0;
}

#include <stdio.h>
//#include <pthread.h>
#include <unistd.h>  // Para usar sleep
#include <stdlib.h>  // Para usar qsort
#include <time.h>
#include <stdbool.h>
#include "cethreads.h"  // Incluimos la biblioteca personalizada CEThreads

//--------------------------------------------------------------------------------------------------------

// Estructura de un barco
typedef struct {
    int id;           // ID del barco
    int tiempo;       // Tiempo total que le toma cruzar el canal
    int tiempo_restante; // Tiempo restante para completar el cruce
    int prioridad;    // Prioridad del barco (mayor valor = mayor prioridad)
    int lado;         // 0 = Izquierda (Atlántico), 1 = Derecha (Pacífico)
    int tipo;         // Tipo de barco: 0 = Normal, 1 = Pesquero, 2 = Patrulla
    int arrival_time; // Tiempo de llegada del barco al canal
    CEthread hilo;   // Hilo asociado al barco
    int en_ejecucion; // 0 = No, 1 = Sí
} Barco2;
int W = 3;  // Tamaño del lote
int barcos_cruzados_izq = 0;  // Barcos cruzados desde el lado izquierdo
int barcos_cruzados_der = 0;  // Barcos cruzados desde el lado derecho
int lado_actual = 0;  // 0 = Atlántico, 1 = Pacífico
int contador_lado = 0;  // Contador de barcos que han cruzado consecutivamente desde un lado
int letrero = 0;        // 0 = Izquierda, 1 = Derecha
int intervalo_letrero = 5; // Tiempo en segundos antes de cambiar el letrero
int quantum = 2;        // Quantum de tiempo (en segundos) para Round Robin
int Letrero = 3;
bool estadoL = false;  // Variable que se enciende/apaga

//pthread_mutex_t mutex;  // Mutex para proteger la interrupción
CEmutex mutex;  // Mutex usando CEThreads
int tiempo_global = 0;  // Tiempo actual del sistema
#include <pthread.h>
// Función que simula el cruce del canal por un barco create
void* cruzar_canal2(void* arg) {
    Barco2* barco = (Barco2*) arg;  // Convertimos el argumento a un puntero a Barco
    
    while (barco->tiempo_restante > 0) {
        //pthread_mutex_lock(&mutex);
        CEmutex_lock(&mutex);  // Bloqueo del mutex con CEThreads

        if (barco->en_ejecucion) {
            printf("Barco %d está cruzando. Tiempo restante: %d segundos\n", barco->id, barco->tiempo_restante);
            sleep(1);  // Simulamos un segundo de ejecución
            barco->tiempo_restante--;
        }
        //pthread_mutex_unlock(&mutex);
        CEmutex_unlock(&mutex);  // Desbloqueo del mutex con CEThreads

        sleep(1);
    }

    printf("Barco %d ha completado el cruce.\n", barco->id);
    return NULL;
}

// Función para encontrar el barco con el menor tiempo restante
int seleccionar_barco(Barco2* barcos, int num_barcos) {
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
void ejecutar_RealTime(Barco2* barcos, int num_barcos) {
    int barcos_completados = 0;

    while (barcos_completados < num_barcos) {
        int barco_en_ejecucion = seleccionar_barco(barcos, num_barcos);

        if (barco_en_ejecucion != -1) {
            Barco2* barco = &barcos[barco_en_ejecucion];
            if (!barco->en_ejecucion) {
                barco->en_ejecucion = 1;
                //pthread_create(&barco->hilo, NULL, cruzar_canal2, barco);
                CEthread_create(&barco->hilo, cruzar_canal2, barco);  // Crear hilo usando CEThreads

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
        //pthread_join(barcos[i].hilo, NULL);
        CEthread_join(barcos[i].hilo);  // Esperar a que el hilo termine con CEThreads

    }
}

/////////////
// Función para seleccionar el barco con menor tiempo restante del lado actual
int seleccionar_barco2(Barco2* barcos, int num_barcos) {
    int indice = -1;
    int menor_tiempo = 9999;

    for (int i = 0; i < num_barcos; i++) {
        if (barcos[i].tiempo_restante > 0) {
            // Seleccionar barcos solo del lado actual (0 = izquierda, 1 = derecha)
            if (barcos[i].lado == lado_actual) {
                if (barcos[i].tiempo_restante < menor_tiempo) {
                    menor_tiempo = barcos[i].tiempo_restante;
                    indice = i;
                }
            }
        }
    }
    return indice;
}
void ejecutar_RealTime_Equidad(Barco2* barcos, int num_barcos) {
    int barcos_completados = 0;

    while (barcos_completados < num_barcos) {
        int barco_en_ejecucion = seleccionar_barco2(barcos, num_barcos);

        if (barco_en_ejecucion != -1) {
            Barco2* barco = &barcos[barco_en_ejecucion];
            if (!barco->en_ejecucion) {
                barco->en_ejecucion = 1;
                //pthread_create(&barco->hilo, NULL, cruzar_canal2, barco);
                CEthread_create(&barco->hilo, cruzar_canal2, barco);
            }
        }

        sleep(1);  // Simula el paso del tiempo
        tiempo_global++;

        // Actualizamos cuántos barcos han cruzado de cada lado
        for (int i = 0; i < num_barcos; i++) {
            if (barcos[i].tiempo_restante == 0) {
                if (barcos[i].lado == 0 && barcos[i].en_ejecucion) {
                    barcos_cruzados_izq++;
                } else if (barcos[i].lado == 1 && barcos[i].en_ejecucion) {
                    barcos_cruzados_der++;
                }
            }
        }

        // Verificar si se ha completado el lote de tamaño W
        if ((lado_actual == 0 && barcos_cruzados_izq >= W) ||
            (lado_actual == 1 && barcos_cruzados_der >= W)) {
            // Alternar el lado
            lado_actual = (lado_actual == 0) ? 1 : 0;
            // Reiniciar el contador de barcos cruzados del lado actual
            if (lado_actual == 0) {
                barcos_cruzados_izq = 0;
            } else {
                barcos_cruzados_der = 0;
            }
        }

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
        //pthread_join(barcos[i].hilo, NULL);
        CEthread_join(barcos[i].hilo);  // Esperar a que el hilo termine con CEThreads

    }
}

// Función que ejecuta el algoritmo Real Time (SRTF) con Equidad (W)
void ejecutar_RealTime_Equidad2(Barco2* barcos, int num_barcos) {
    int barcos_completados = 0;

    while (barcos_completados < num_barcos) {
        // Verificar si hemos alcanzado el límite de barcos desde el lado actual
        if (contador_lado >= W) {
            lado_actual = (lado_actual == 0) ? 1 : 0;  // Cambiamos de lado
            contador_lado = 0;  // Reiniciamos el contador
            printf("Cambiando al lado %d\n", lado_actual);
        }

        int barco_en_ejecucion = seleccionar_barco(barcos, num_barcos);

        if (barco_en_ejecucion != -1) {
            Barco2* barco = &barcos[barco_en_ejecucion];
            if (!barco->en_ejecucion) {
                barco->en_ejecucion = 1;
                //pthread_create(&barco->hilo, NULL, cruzar_canal2, barco);
                CEthread_create(&barco->hilo, cruzar_canal2, barco);  // Crear hilo usando CEThreads
                contador_lado++;  // Incrementamos el contador de barcos que cruzan desde este lado
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
        //pthread_join(barcos[i].hilo, NULL);
        CEthread_join(barcos[i].hilo);  // Esperar a que el hilo termine con CEThreads

    }
}

////////////

// Función para encontrar el barco con el menor tiempo restante del lado permitido por el letrero
int seleccionar_barco3(Barco2* barcos, int num_barcos, int lado_permitido) {
    int indice = -1;
    int menor_tiempo = 9999;

    for (int i = 0; i < num_barcos; i++) {
        // Solo seleccionar barcos del lado permitido por el letrero
        if (barcos[i].tiempo_restante > 0 && barcos[i].arrival_time <= tiempo_global && barcos[i].lado == lado_permitido) {
            if (barcos[i].tiempo_restante < menor_tiempo) {
                menor_tiempo = barcos[i].tiempo_restante;
                indice = i;
            }
        }
    }
    return indice;
}

// Función que ejecuta el algoritmo Real Time (SRTF) con control del letrero
void ejecutar_RealTime_Letrero(Barco2* barcos, int num_barcos) {

    int barcos_completados = 0;
    int tiempo_ultimo_cambio = 0;

    while (barcos_completados < num_barcos) {
        // Cambiar el letrero después de un intervalo de tiempo
        if (tiempo_global - tiempo_ultimo_cambio >= intervalo_letrero) {
            letrero = (letrero == 0) ? 1 : 0;  // Cambia el letrero de izquierda (0) a derecha (1) y viceversa
            tiempo_ultimo_cambio = tiempo_global;
            printf("El letrero ha cambiado. Ahora pueden cruzar los barcos del lado: %s\n", letrero == 0 ? "Izquierda" : "Derecha");
        }

        // Seleccionar el barco del lado que está permitido por el letrero
        int barco_en_ejecucion = seleccionar_barco3(barcos, num_barcos, letrero);

        if (barco_en_ejecucion != -1) {
            Barco2* barco = &barcos[barco_en_ejecucion];
            if (!barco->en_ejecucion) {
                barco->en_ejecucion = 1;
                //pthread_create(&barco->hilo, NULL, cruzar_canal2, barco);
                CEthread_create(&barco->hilo, cruzar_canal2, barco);  // Crear hilo usando CEThreads

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
        //pthread_join(barcos[i].hilo, NULL);
        CEthread_join(barcos[i].hilo);  // Esperar a que el hilo termine con CEThreads

    }
}




// Función que simula el cruce del canal por un barco durante su quantum
void cruzar_canal_quantum(Barco2* barco) {
    CEmutex_lock(&mutex);
    printf("Barco %d está cruzando. Tiempo restante: %d segundos\n", barco->id, barco->tiempo_restante);
    
    // Simular el tiempo que el barco está cruzando hasta el final de su quantum o hasta que termine
    for (int i = 0; i < quantum && barco->tiempo_restante > 0; i++) {
        sleep(1);  // Simula el paso de un segundo de ejecución
        barco->tiempo_restante--;
    }

    // Si el barco ha terminado de cruzar, imprimimos un mensaje
    if (barco->tiempo_restante == 0) {
        printf("Barco %d ha completado el cruce.\n", barco->id);
    }

    CEmutex_unlock(&mutex);
}

// Función que ejecuta el algoritmo Round Robin
void ejecutar_RoundRobin(Barco2* barcos, int num_barcos) {
    int barcos_completados = 0;
    int indice_actual = 0;

    while (barcos_completados < num_barcos) {
        Barco2* barco = &barcos[indice_actual];

        if (barco->tiempo_restante > 0) {
            // Si el barco no ha terminado, lo ejecutamos en su quantum
            cruzar_canal_quantum(barco);
        }

        // Cambiamos al siguiente barco
        indice_actual = (indice_actual + 1) % num_barcos;

        // Verificamos si todos los barcos han completado el cruce
        barcos_completados = 0;
        for (int i = 0; i < num_barcos; i++) {
            if (barcos[i].tiempo_restante == 0) {
                barcos_completados++;
            }
        }

        tiempo_global++;  // Simula el tiempo global que avanza en cada ciclo
    }

    printf("Todos los barcos han completado el cruce.\n");
}

// Función que simula el cruce del canal por un barco durante su quantum
void cruzar_canal_quantum2(Barco2* barco) {
    CEmutex_lock(&mutex);
    printf("Barco %d (%s) está cruzando. Tiempo restante: %d segundos\n", barco->id, (barco->lado == 0 ? "Izquierda" : "Derecha"), barco->tiempo_restante);
    
    // Simular el tiempo que el barco está cruzando hasta el final de su quantum o hasta que termine
    for (int i = 0; i < quantum && barco->tiempo_restante > 0; i++) {
        sleep(1);  // Simula el paso de un segundo de ejecución
        barco->tiempo_restante--;
    }

    // Si el barco ha terminado de cruzar, imprimimos un mensaje
    if (barco->tiempo_restante == 0) {
        printf("Barco %d ha completado el cruce.\n", barco->id);
    }

    CEmutex_unlock(&mutex);
}

// Función que ejecuta el algoritmo Round Robin con Equidad
void ejecutar_RoundRobin_Equidad(Barco2* barcos, int num_barcos) {
    int barcos_completados = 0;
    int indice_actual = 0;
    int lado_actual = 0;  // Inicia desde el lado izquierdo (0)
    int barcos_por_lado = 0;  // Contador de barcos que han cruzado en el lado actual

    while (barcos_completados < num_barcos) {
        Barco2* barco = &barcos[indice_actual];

        // Verificar si el barco está en el lado actual y aún no ha terminado de cruzar
        if (barco->lado == lado_actual && barco->tiempo_restante > 0) {
            cruzar_canal_quantum2(barco);
            barcos_por_lado++;
        }

        // Cambiamos al siguiente barco en Round Robin
        indice_actual = (indice_actual + 1) % num_barcos;

        // Si se han cruzado W barcos de este lado, cambiamos de lado
        if (barcos_por_lado == W) {
            lado_actual = (lado_actual == 0) ? 1 : 0;  // Cambia de 0 a 1 o de 1 a 0
            barcos_por_lado = 0;
            printf("\n*** Cambiando de lado a %s ***\n\n", (lado_actual == 0 ? "Izquierda" : "Derecha"));
        }

        // Verificamos si todos los barcos han completado el cruce
        barcos_completados = 0;
        for (int i = 0; i < num_barcos; i++) {
            if (barcos[i].tiempo_restante == 0) {
                barcos_completados++;
            }
        }

        tiempo_global++;  // Simula el tiempo global que avanza en cada ciclo
    }

    printf("Todos los barcos han completado el cruce.\n");
}


// Función que ejecuta el algoritmo Round Robin con Letrero
void ejecutar_RoundRobin_Letrero(Barco2* barcos, int num_barcos) {
    int barcos_completados = 0;
    int indice_actual = 0;
    int barcos_por_lado = 0;  // Contador de barcos que han cruzado en el lado actual

    while (barcos_completados < num_barcos) {
        Barco2* barco = &barcos[indice_actual];

        // Verificar si el barco está en el lado indicado por el letrero y aún no ha terminado de cruzar
        if (barco->lado == letrero && barco->tiempo_restante > 0) {
            cruzar_canal_quantum(barco);
            barcos_por_lado++;
        }

        // Cambiamos al siguiente barco en Round Robin
        indice_actual = (indice_actual + 1) % num_barcos;

        // Si se han cruzado W barcos de este lado, cambiamos de lado
        if (barcos_por_lado == W) {
            letrero = (letrero == 0) ? 1 : 0;  // Cambia de 0 a 1 o de 1 a 0
            barcos_por_lado = 0;
            printf("\n*** Cambiando de lado a %s ***\n\n", (letrero == 0 ? "Izquierda" : "Derecha"));
        }

        // Verificamos si todos los barcos han completado el cruce
        barcos_completados = 0;
        for (int i = 0; i < num_barcos; i++) {
            if (barcos[i].tiempo_restante == 0) {
                barcos_completados++;
            }
        }

        tiempo_global++;  // Simula el tiempo global que avanza en cada ciclo
    }

    printf("Todos los barcos han completado el cruce.\n");
}

/*

//--------------------------------------------------------------------------------------------------------
// Estructura de un barco
typedef struct {
    int id;           // ID del barco
    int tiempo;       // Tiempo que le toma cruzar el canal
    int prioridad;    // Prioridad del barco (mayor valor = mayor prioridad)
    int lado;         // 0 = Izquierda (Atlántico), 1 = Derecha (Pacífico)
    int tipo;         // Tipo de barco: 0 = Normal, 1 = Pesquero, 2 = Patrulla
} Barco;
//int W = 3;
int Letrero = 3;
bool estadoL = false;  // Variable que se enciende/apaga
*/
// Función que simula el cruce del canal por un barco
void* cruzar_canal(void* arg) {
    Barco2* barco = (Barco2*) arg;  // Convertimos el argumento a un puntero a Barco
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
    Barco2* barcoA = (Barco2*) a;
    Barco2* barcoB = (Barco2*) b;
    return barcoA->tiempo - barcoB->tiempo;  // Ordena de menor a mayor tiempo
}

// Comparador para Prioridad, basado en el valor de prioridad (mayor primero)
int comparar_por_prioridad(const void* a, const void* b) {
    Barco2* barcoA = (Barco2*) a;
    Barco2* barcoB = (Barco2*) b;
    return barcoA->prioridad - barcoB->prioridad;  // Ordena de mayor a menor prioridad
}

// Función para ejecutar el algoritmo FCFS para una lista de barcos
void ejecutar_FCFS(Barco2* barcos, int num_barcos) {
    pthread_t hilos[num_barcos];

    
    for (int i = 0; i < num_barcos; i++) {
        //CEthread_create(&hilos[i], cruzar_canal, &barcos[i]);  // Crear hilo usando CEThreads

        pthread_create(&hilos[i], NULL, cruzar_canal, &barcos[i]);
        //CEthread_join(hilos[i]);  // Esperar a que el hilo termine con CEThreads

        pthread_join(hilos[i], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
    }
}

// Función para ejecutar el algoritmo FCFS para una lista de barcos
void ejecutar_FCFS_Equidad(Barco2* barcos_izquierda, Barco2* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    pthread_t hilosI[num_barcos_izquierda];
    pthread_t hilosD[num_barcos_derecha];
    int contI = 0;
    int contD = 0;
    
    while (contI!=num_barcos_izquierda || contD!=num_barcos_derecha){
        //printf("El contI %d\nEl contD %d\n ",contI,contD);
        for (int i = 0; i < W; i++) {
            if (contI!=num_barcos_izquierda){
                //CEthread_create(&hilosI[contI], cruzar_canal, &barcos_izquierda[contI]);
                //CEthread_join(hilosI[contI]);  // Esperar a que el hilo termine con CEThreads

                pthread_create(&hilosI[contI], NULL, cruzar_canal, &barcos_izquierda[contI]);
                pthread_join(hilosI[contI], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
                contI++;
            }
        }
        for (int i = 0; i < W; i++) {
            if (contD!=num_barcos_derecha){
                //CEthread_create(&hilosD[contD], cruzar_canal, &barcos_derecha[contD]);
                //CEthread_join(hilosD[contD]);  // Esperar a que el hilo termine con CEThreads

                pthread_create(&hilosD[contD], NULL, cruzar_canal, &barcos_derecha[contD]);
                pthread_join(hilosD[contD], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
                contD++;
            }
        }
    }   
}

void ejecutar_FCFS_Letrero(Barco2* barcos_izquierda, Barco2* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
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
            //CEthread_create(&hilosD[contD], cruzar_canal, &barcos_derecha[contD]);  // Crear hilo usando CEThreads
            //CEthread_join(hilosD[contD]);  // Esperar a que el hilo termine con CEThreads
            pthread_join(hilosD[contD], NULL);  // Esperamos a que el barco termine antes de pasar al siguiente
            contD++;
        } else {
            time+=barcos_izquierda[contI].tiempo;
            pthread_create(&hilosI[contI], NULL, cruzar_canal, &barcos_izquierda[contI]);
            //CEthread_create(&hilosI[contI], cruzar_canal, &barcos_izquierda[contI]);
            //CEthread_join(hilosI[contI]);  // Esperar a que el hilo termine con CEThreads

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
void ejecutar_SJF(Barco2* barcos, int num_barcos) {
    qsort(barcos, num_barcos, sizeof(Barco2), comparar_por_tiempo);
    ejecutar_FCFS(barcos, num_barcos);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

// Función para ejecutar el algoritmo SJF para una lista de barcos
void ejecutar_SJF_Equidad(Barco2* barcos_izquierda, Barco2* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco2), comparar_por_tiempo);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco2), comparar_por_tiempo);
    ejecutar_FCFS_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

void ejecutar_SJF_Letrero(Barco2* barcos_izquierda, Barco2* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco2), comparar_por_tiempo);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco2), comparar_por_tiempo);
    ejecutar_FCFS_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

// Función para ejecutar el algoritmo por Prioridad para una lista de barcos
void ejecutar_Priority(Barco2* barcos, int num_barcos) {
    qsort(barcos, num_barcos, sizeof(Barco2), comparar_por_prioridad);
    ejecutar_FCFS(barcos, num_barcos);
}

void ejecutar_Priority_Equidad(Barco2* barcos_izquierda, Barco2* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco2), comparar_por_prioridad);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco2), comparar_por_prioridad);
    ejecutar_FCFS_Equidad(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

void ejecutar_Priority_Letrero(Barco2* barcos_izquierda, Barco2* barcos_derecha, int num_barcos_izquierda, int num_barcos_derecha) {
    qsort(barcos_izquierda, num_barcos_izquierda, sizeof(Barco2), comparar_por_prioridad);
    qsort(barcos_derecha, num_barcos_derecha, sizeof(Barco2), comparar_por_prioridad);
    ejecutar_FCFS_Letrero(barcos_izquierda, barcos_derecha, num_barcos_izquierda, num_barcos_derecha);  // Reutilizamos la lógica de FCFS pero con la lista ordenada
}

int main() {

    // Inicializamos el mutex
    //pthread_mutex_init(&mutex, NULL);

    // Creamos una lista de barcos
    Barco2 barcos2[] = {
        {1, 7, 7, 1, 0, 0, 0, 0},  // Barco 1, llega en t0
        {2, 2, 2, 2, 1, 1, 3, 0},  // Barco 2, llega en t3
        {3, 1, 1, 5, 1, 1, 1, 0},  // Barco 3, llega en t1
        {4, 6, 6, 5, 0, 1, 0, 0},  // Barco 4, llega en t0
        {5, 4, 4, 3, 1, 1, 3, 0},  // Barco 5
        {6, 3, 3, 2, 1, 0, 1, 0},  // Barco 6
    };
    
    int num_barcos2 = sizeof(barcos2) / sizeof(barcos2[0]);

    // Listas separadas para barcos en lado izquierdo (Atlántico) y lado derecho (Pacífico)
    Barco2 barcos_izquierda2[num_barcos2];  // Lista para los barcos en el lado izquierdo
    Barco2 barcos_derecha2[num_barcos2];    // Lista para los barcos en el lado derecho

    int num_barcos_izquierda2 = 0;  // Contador para barcos en el lado izquierdo
    int num_barcos_derecha2 = 0;    // Contador para barcos en el lado derecho

    // Clasificamos los barcos en las dos listas separadas
    for (int i = 0; i < num_barcos2; i++) {
        if (barcos2[i].lado == 0) {  // Lado izquierdo (Atlántico)
            barcos_izquierda2[num_barcos_izquierda2++] = barcos2[i];
        } else {  // Lado derecho (Pacífico)
            barcos_derecha2[num_barcos_derecha2++] = barcos2[i];
        }
    }

    // Ejecutar algoritmos ----------------------------------------------------------------------------------
    //printf("\n--- Ejecución con FCFS ---\n");
    //ejecutar_FCFS_Equidad(barcos_izquierda2, barcos_derecha2, num_barcos_izquierda2, num_barcos_derecha2);

    //ejecutar_FCFS_Letrero(barcos_izquierda2, barcos_derecha2, num_barcos_izquierda2, num_barcos_derecha2);

    //ejecutar_FCFS(barcos2, num_barcos2);

    //printf("\n--- Ejecución con SJF con Equidad ---\n");
    //ejecutar_SJF_Equidad(barcos_izquierda, barcos_derecha2, num_barcos_izquierda2, num_barcos_derecha2);

    //ejecutar_SJF_Letrero(barcos_izquierda, barcos_derecha2, num_barcos_izquierda2, num_barcos_derecha2);

    //printf("\n--- Ejecución con SJF (Shortest Job First) ---\n");
    //ejecutar_SJF(barcos2, num_barcos2);

    //printf("\n--- Ejecución con Prioridad con Equidad ---\n");
    //ejecutar_Priority_Equidad(barcos_izquierda2, barcos_derecha2, num_barcos_izquierda2, num_barcos_derecha2);

    //ejecutar_Priority_Letrero(barcos_izquierda2, barcos_derecha2, num_barcos_izquierda2, num_barcos_derecha2);

    //printf("\n--- Ejecución con Prioridad ---\n");
    //ejecutar_Priority(barcos2, num_barcos2);



    //printf("\n--- Ejecución con Real Time (SRTF) ---\n");
    //ejecutar_RealTime(barcos2, num_barcos2);
    //ejecutar_RealTime_Equidad2(barcos2, num_barcos2);
    //ejecutar_RealTime_Letrero(barcos2, num_barcos2);

    //printf("\n--- Ejecución con Round Robin ---\n");
    //ejecutar_RoundRobin(barcos2, num_barcos2);
    //ejecutar_RoundRobin_Equidad(barcos2, num_barcos2);
    //ejecutar_RoundRobin_Letrero(barcos2, num_barcos2);



    // Destruimos el mutex
    //pthread_mutex_destroy(&mutex);
    

    return 0;
}

#include "cethreads.h"
#include <stdio.h>
#include <stdlib.h>

void *print_fibonacci(void* arg) {
    int n = *(int *)arg;
    int a = 0, b = 1;
    for (int i = 0; i < n; i++) {
        printf("%d\n", a);
        int tmp = a;
        a = b;
        b = tmp + b;
    }
    return NULL;  // Corregido para retornar un puntero void*
}

int main() {
    CEthread thread;
    int* value = malloc(sizeof(int));

    *value = 10;  // Número de iteraciones para la secuencia de Fibonacci

    // Crear un hilo que ejecute la secuencia de Fibonacci
    if (CEthread_create(&thread, print_fibonacci, value) != 0) {
        printf("Error al crear el hilo\n");
        return 1;
    } else {
        printf("Hilo creado\n");
    }

    // Esperar a que el hilo termine
    CEthread_join(thread);

    printf("Hilo terminado\n");

    // Liberar la memoria asignada
    free(value);

    return 0;
}

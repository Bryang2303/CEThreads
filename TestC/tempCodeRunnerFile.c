
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

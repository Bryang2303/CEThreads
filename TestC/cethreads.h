#ifndef CETHREADS_H
#define CETHREADS_H

#include <sys/types.h>
#include <sys/mman.h>  // Para mmap
#include <sched.h>     // Para clone
#include <unistd.h>    // Para getpid
#include <sys/syscall.h>  // Para hacer syscalls
#include <linux/futex.h>  // Para futex
#include <stdlib.h>

#define STACK_SIZE 8192

// Estructura del hilo
typedef struct {
    pid_t tid;  // Identificador del hilo (ID del thread)
    void *stack;  // Puntero a la pila del hilo
} CEthread;

// Estructura del mutex
typedef struct {
    int lock;  // 0 = desbloqueado, 1 = bloqueado
} CEmutex;

// Funciones de la biblioteca de hilos
int CEthread_create(CEthread *thread, void *(*start_routine)(void *), void *arg);
int CEthread_join(CEthread thread);
void CEthread_exit();

// Funciones para los mutexes
int CEmutex_init(CEmutex *mutex);
int CEmutex_destroy(CEmutex *mutex);
int CEmutex_lock(CEmutex *mutex);
int CEmutex_unlock(CEmutex *mutex);

#endif

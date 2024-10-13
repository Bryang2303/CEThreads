#define _GNU_SOURCE
#include "cethreads.h"
#include <stdio.h>
#include <sys/wait.h>  // Para waitpid
#include <sys/syscall.h>  // Para syscalls
#include <errno.h>

// Syscall futex
int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

static int thread_wrapper(void *arg) {
    // Desempaquetar los argumentos
    void **args = (void **)arg;
    void *(*start_routine)(void *) = args[0];
    void *routine_arg = args[1];

    // Ejecutar la función del hilo con su argumento
    if (start_routine != NULL) {
        start_routine(routine_arg);
    }

    // Asegurarse de que el hilo termine correctamente
    CEthread_exit();
    return 0;
}

int CEthread_create(CEthread *thread, void *(*start_routine)(void *), void *arg) {
    // Reservar memoria para la pila (debe estar alineada correctamente)
    thread->stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    if (thread->stack == MAP_FAILED) {
        perror("Error al asignar la pila");
        return -1;
    }

    // Preparar los argumentos para el hilo
    void **args = (void **)malloc(2 * sizeof(void *));
    args[0] = start_routine;
    args[1] = arg;

    // Crear el hilo usando clone
    thread->tid = clone(thread_wrapper, (char *)thread->stack + STACK_SIZE, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM, args);
    if (thread->tid == -1) {
        perror("Error al crear el hilo");
        munmap(thread->stack, STACK_SIZE);
        free(args);
        return -1;
    }

    return 0;
}

int CEthread_join(CEthread thread) {
    // Esperar a que el hilo termine
    int status;
    if (waitpid(thread.tid, &status, __WCLONE) == -1) {
        perror("Error al esperar el hilo");
        return -1;
    }
    return 0;
}

void CEthread_exit() {
    // Terminar el hilo
    syscall(SYS_exit, 0);
}

// Inicializar el mutex
int CEmutex_init(CEmutex *mutex) {
    mutex->lock = 0;
    return 0;
}

// Destruir el mutex
int CEmutex_destroy(CEmutex *mutex) {
    return 0;
}

// Bloquear el mutex usando futex
int CEmutex_lock(CEmutex *mutex) {
    while (__sync_lock_test_and_set(&mutex->lock, 1)) {
        futex(&mutex->lock, FUTEX_WAIT, 1, NULL, NULL, 0);
    }
    return 0;
}

// Desbloquear el mutex
int CEmutex_unlock(CEmutex *mutex) {
    __sync_lock_release(&mutex->lock);
    futex(&mutex->lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}

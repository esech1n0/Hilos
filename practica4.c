#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    pthread_mutex_t lock; //mutex para sincronizar
    int hilos_finalizados; //contador de hilos finalizados
} MonitorFactorial;

MonitorFactorial monitor; //Variable global que se comparte entre los hilos

typedef struct {
    int num; 
    long long resultado;
    int id_hilo;
} FactorialArgs;

void monitor_registrar_termino(int id_hilo) {
    pthread_mutex_lock(&monitor.lock); //bloquea acceso
    monitor.hilos_finalizados++;
    printf("[Hilo %d] TERMINÓ. Total completados: %d\n",
           id_hilo, monitor.hilos_finalizados);
    pthread_mutex_unlock(&monitor.lock); //libera acceso
}

void* calcular_factorial(void* arg) {
    FactorialArgs* datos = (FactorialArgs*)arg; //Convierte el argumento genérico a estructura
    datos->resultado = 1;   //inicializa el factorial

    for (int i = 2; i <= datos->num; i++) {  //ciclo para calcular el factorial
        datos->resultado *= i; //Multiplica lo acumulado para un calculo factorial

        usleep(50000); //pausa para que se note el paralelismo
    }

    printf("[Hilo %d] Resultado final: %d! = %lld\n",
           datos->id_hilo, datos->num, datos->resultado);

    monitor_registrar_termino(datos->id_hilo); //Notifica que su función termino
    pthread_exit(NULL); //finaliza el hilo
}

// Versión secuencial, esta sirve para poder comparar
long long factorial_secuencial(int n) { //Funcion normal sin hilos
    long long res = 1;
    for (int i = 2; i <= n; i++) {      //hace lo mismo que la función anterior, pero esta es en serie.
        res *= i;
        usleep(50000);  
    }
    return res;
}

// Medición de tiempo
double tiempo_actual() {
    struct timeval t; 
    gettimeofday(&t, NULL); //obtiene el tiempo del sistema, puramente estetica.
    return t.tv_sec + t.tv_usec / 1000000.0; //convierte a segundos con decimales decimales.
}

int main(int argc, char *argv[]) {
    if (argc != 3) {       //se hace cargo de que solamente pasen 2 números.
        printf("Uso: %s <num1> <num2>\n", argv[0]);
        return 1;
    }

    pthread_mutex_init(&monitor.lock, NULL);    //inicializa el mutex
    monitor.hilos_finalizados = 0; //inicializa el contador.

    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]); //convierte los argumentos a intervalos

    printf(">>>> EJECUCION PARALELA <<<<\n");

    double inicio_paralelo = tiempo_actual();       //hace un guardado del tiempo actual para sacar la conversión a segundos

    pthread_t hilo1, hilo2;     //declara cuales seran los hilos.
    FactorialArgs arg1 = {num1, 1, 1};      //prepara los datos para cada hilo
    FactorialArgs arg2 = {num2, 1, 2};

    pthread_create(&hilo1, NULL, calcular_factorial, &arg1);
    pthread_create(&hilo2, NULL, calcular_factorial, &arg2);    //Se crean y mandan los hilos.

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);      //espera a q terminen

    double fin_paralelo = tiempo_actual();      //Guarda el tiempo de nuez jejeje

    printf("\n<<<<RESULTADOS PARALELOS>>>>\n");
    printf("Hilo 1 (%d!) = %lld\n", arg1.num, arg1.resultado);
    printf("Hilo 2 (%d!) = %lld\n", arg2.num, arg2.resultado);
    printf("Tiempo paralelo: %.2f segundos\n", fin_paralelo - inicio_paralelo);

    // EJECUCIÓN SECUENCIAL

    printf("\n<<<<EJECUCIÓN SECUENCIAL>>>>\n");

    double inicio_sec = tiempo_actual();    //Toma tiempo

    long long r1 = factorial_secuencial(num1);  
    long long r2 = factorial_secuencial(num2);

    double fin_sec = tiempo_actual();       //Guarda el tiempo jejeje

    printf("Resultado 1 (%d!) = %lld\n", num1, r1);
    printf("Resultado 2 (%d!) = %lld\n", num2, r2);
    printf("Tiempo secuencial: %.2f segundos\n", fin_sec - inicio_sec);

    pthread_mutex_destroy(&monitor.lock);       //libera el mutex
    return 0;
}


#include "simplegrade.h"
#include "io.h"
#include <stdio.h>

//throughput em x 10^6
#define GB 1000
#define MB 1


#define SEGUNDO 1000000 // supondo freq 1 GHz, 100K ciclos
#define MILISSEGUNDO 1000
#define MICROSSEGUNDO 1

#define PRINT_RESULTS( a )\
    printf("Resultados: \n");\
    printf("Ciclos \t%d\n", a->idle + a->busy);\
    printf("Bus livre: \t%d\n", a->idle);\
    printf("Bus ocupado: \t%d\n", a->busy);\
    printf("CPU em I/O: \t%d\n", a->cpu_usage);\

#define PRINT_REQS( a, n )\
    printf("Requests: \n");\
    for(int i=0; i<n; i++)\
        printf("%d ", a->requests[i]);\
    printf("\n");\

#define CLEANUP( a ) \
    free(a->requests);\
    free(a);\

struct param configs[9] = {
    {10*MILISSEGUNDO, 10*MILISSEGUNDO,100*MB}, // disco A
    {10*MILISSEGUNDO, 10*MILISSEGUNDO,100*MB}, // disco B
    {1*MICROSSEGUNDO, 1*MICROSSEGUNDO,100*MB}, // placa de video
    {100*MICROSSEGUNDO, 100*MICROSSEGUNDO,500*MB}, // rede
    {100*MICROSSEGUNDO, 100*MICROSSEGUNDO,300*MB}, // USB
    {10*MICROSSEGUNDO, 10*MICROSSEGUNDO,10*MB}, // SOM
    {1*MILISSEGUNDO, 1*MILISSEGUNDO,1*MB}, // teclado
    {1*MILISSEGUNDO, 1*MILISSEGUNDO,1*MB}, // serial
    {1*MILISSEGUNDO, 1*MILISSEGUNDO,1*MB}, // paralela
};




void test_um_pedido(){
//9 10 R 5 0  9B + 10B + wait de 1 ms
// 9 B a 1 MB/s = 9*10^6/1  = 9.000.000 ciclos 
// 5 B a 1 MB/s = 5.000.000 ciclos
// wait de 1 ms = 1000 ciclos
// total = 14.001.000 ciclos

    WHEN("Tendo apenas um pedido de polling");
    IF("Executado a partir do tempo zero");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t1.txt"); 
    isEqual(r->busy, 14001000, 1);
    isEqual(r->cpu_usage, 14001000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}
void test_um_pedido_atrasado(){
//9 10 R 5 10  9B + 10B + wait de 1 ms + 10 ciclos
// 9 B a 1 MB/s = 9*10^6/1  = 9.000.000 ciclos 
// 5 B a 1 MB/s = 5.000.000 ciclos
// wait de 1 ms = 1000 ciclos
// total = 14.001.000 ciclos

    WHEN("Tendo apenas um pedido de polling");
    IF("Executado a partir do tempo 10");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t1b.txt"); 
    isEqual(r->busy, 14001000, 1);
    isEqual(r->cpu_usage, 14001000, 1);
    isEqual(r->idle, 10, 1);

    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}
void test_dois_pedidos(){
    WHEN("Tendo dois pedidos de polling");
    IF("Executado a partir do tempo 10");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t2.txt"); 
    isEqual(r->busy, 28002000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 2); //2 requests
    CLEANUP(r);
}

void test_dma(){
    WHEN("Tendo um pedido de DMA");
    IF("Executado a partir do tempo zero");
    struct result_io * r;
    r = sim_io(configs, "t3.txt"); 
    THEN("Barramento fica idle por 10K ciclos esperando o dispositivo");
    isEqual(r->idle, 10000, 1);
    isEqual(r->busy, 140000, 1);    
    isEqual(r->cpu_usage, 90000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}

void test_int(){
    WHEN("Tendo um pedido de int");
    IF("Executado a partir do tempo zero");
    THEN("Espero que use o barramento e a CPU todo o tempo, menos na espera");
    struct result_io * r;
    r = sim_io(configs, "t4.txt"); 
    isEqual(r->busy, 1400000, 1);
    isEqual(r->cpu_usage, 1400000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}


void test_dma_polling(){

    WHEN("Tendo um pedido de polling e um de DMA");
    IF("Executado a partir do tempo zero");
    struct result_io * r;
    r = sim_io(configs, "t5.txt"); 
    THEN("Deve executar na ordem de prioridade");
    isEqual(r->requests[0], 1, 1);
    isEqual(r->requests[1], 0, 1);
    
    isEqual(r->busy, 14141000, 1);
    isEqual(r->cpu_usage, 14091000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 2); //2 requests
    CLEANUP(r);
}

void test_dma_polling_atrasado(){

    WHEN("Tendo um pedido de polling e um de DMA");
    IF("DMA começa no tempo 10");
    struct result_io * r;
    r = sim_io(configs, "t5b.txt"); 
    THEN("Deve executar na ordem de ocorrência");
    isEqual(r->requests[0], 0, 1);
    isEqual(r->requests[1], 1, 1);
    THEN("Deve ter o barramento livre 10K ciclos");
    isEqual(r->idle, 10000, 1);
    THEN("Deve usar o barramento 1004 ciclos");
    isEqual(r->busy, 14141000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 2); //2 requests
    CLEANUP(r);
}

void test_muitos_polling(){
     WHEN("Tendo dez pedidos de polling");
    IF("Criados um por ciclo");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t6.txt"); 
    isEqual(r->busy, 140010000, 1);
    isEqual(r->cpu_usage, 140010000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 10); //10 requests
    CLEANUP(r);
}

void test_sortidos(){

    WHEN("Tendo dez pedidos sortidos");
    IF("Criados no ciclo zero");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t7.txt"); 
    isEqual(r->busy, 99253334, 1);
    isEqual(r->cpu_usage, 49253334, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 9); //9 requests
    CLEANUP(r);
}




int main(){

    test_um_pedido();
    test_um_pedido_atrasado();
    test_dois_pedidos();
    test_dma();
    test_int();
    test_dma_polling();
    test_dma_polling_atrasado();
    test_muitos_polling();
    test_sortidos();
    
    GRADEME();
   	if (grade==maxgrade)
		return 0;
	else return grade;


}



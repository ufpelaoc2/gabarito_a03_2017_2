
#include "simplegrade.h"
#include "io.h"
#include <stdio.h>

//throughput em x 10^6
#define GB 1000
#define MB 1


#define MEGA 1000000
#define KILO 1000

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

// supondo um barramento de 1 GHz, 1 ciclo tem 1 ns

// comecar leitura, escrita e velocidade de transmissao 
struct param configs[9] = {  //tempo em ciclos
    {8*KILO, 8*KILO, 1}, // disco A
    {10*KILO, 10*KILO, 1}, // disco B
    {1, 1, 1}, // placa de video
    {10, 10, 2}, // rede
    {1, 1, 3}, // USB
    {10, 10, 10}, // SOM
    {1*KILO, 1*KILO, 100}, // teclado
    {1*KILO, 1*KILO, 100}, // serial
    {1*KILO, 1*KILO, 100}, // paralela
};


//disp  end  bytes ciclo 
//9     10 R 5        0
// porta paralela
// ocupa CPU + Barramento + Dispositivo tempo todo
// Inicio: 9 bytes do pedido, 100 ciclos por byte
//         900 ciclos
// Espera 1000 ciclos
// Transfere 100*5 ciclos
// Total = 900 + 1000 + 500 = 2400

void test_um_pedido(){
    WHEN("Tendo apenas um pedido de polling");
    IF("Executado a partir do tempo zero");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t1.txt", 0); 
    isEqual(r->busy, 2400, 1);
    isEqual(r->cpu_usage, 2400, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}

// anterior + 10

void test_um_pedido_atrasado(){

    WHEN("Tendo apenas um pedido de polling");
    IF("Executado a partir do tempo 10");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t1b.txt", 0); 
    isEqual(r->busy, 2400, 1);
    isEqual(r->cpu_usage, 2400, 1);
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
    r = sim_io(configs, "t2.txt", 0); 
    isEqual(r->busy, 4800, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 2); //2 requests
    CLEANUP(r);
}

//disp  end  bytes ciclo 
// 1    10 R 4096    0
//{8*KILO, 8*KILO, 1}, // disco A

void test_dma(){
    WHEN("Tendo um pedido de DMA");
    IF("Executado a partir do tempo zero");
    struct result_io * r;
    r = sim_io(configs, "t3.txt", 0); 
    THEN("Barramento fica idle por 8K ciclos esperando o dispositivo");
    isEqual(r->idle, 8000, 1);
    THEN("Barramento fica ocupado por 9 ciclos de pedido + 4096 ciclos de transferencia + 2 int");
    isEqual(r->busy, 4107, 1);    
    THEN("CPU fica ocupada apenas durante pedido e int");
    isEqual(r->cpu_usage, 11, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}

void test_int(){
    WHEN("Tendo um pedido de int");
    IF("Executado a partir do tempo zero");
    THEN("Espero que use o barramento e a CPU todo o tempo, menos na espera");
    struct result_io * r;
    r = sim_io(configs, "t4.txt", 0); 
    isEqual(r->busy, 160, 1);
    isEqual(r->cpu_usage, 160, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 1); //1 request
    CLEANUP(r);
}


void test_dma_polling(){

    WHEN("Tendo um pedido de polling e um de DMA");
    IF("Executado a partir do tempo zero");
    struct result_io * r;
    r = sim_io(configs, "t5.txt", 0); 
    THEN("Deve executar na ordem de prioridade");
    isEqual(r->requests[0], 1, 1);
    isEqual(r->requests[1], 0, 1);
    
    isEqual(r->busy, 2416, 1);
    isEqual(r->cpu_usage, 2411, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 2); //2 requests
    CLEANUP(r);
}

void test_dma_polling_atrasado(){

    WHEN("Tendo um pedido de polling e um de DMA");
    IF("DMA começa no tempo 10");
    struct result_io * r;
    r = sim_io(configs, "t5b.txt", 0); 
    THEN("Deve executar na ordem de ocorrência");
    isEqual(r->requests[0], 0, 1);
    isEqual(r->requests[1], 1, 1);
    THEN("Deve ter o barramento livre 8K ciclos");
    isEqual(r->idle, 8000, 1);
    THEN("Deve usar o barramento 2416 ciclos");
    isEqual(r->busy, 2416, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 2); //2 requests
    CLEANUP(r);
}

void test_muitos_polling(){
     WHEN("Tendo dez pedidos de polling");
    IF("Criados um por ciclo");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t6.txt", 0); 
    isEqual(r->busy, 24000, 1);
    isEqual(r->cpu_usage, 24000, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 10); //10 requests
    CLEANUP(r);
}

void test_sortidos(){

    WHEN("Tendo dez pedidos sortidos");
    IF("Criados no ciclo zero");
    THEN("Espero que use o barramento e a CPU todo o tempo");
    struct result_io * r;
    r = sim_io(configs, "t7.txt", 0); 
    isEqual(r->busy, 30226, 1);
    isEqual(r->cpu_usage, 7226, 1);
    PRINT_RESULTS(r);
    PRINT_REQS(r, 9); //9 requests
    CLEANUP(r);
}




int main(){
    DESCRIBE("test_um_pedido");
    test_um_pedido();
    DESCRIBE("test_um_pedido_atrasado");
    test_um_pedido_atrasado();
    DESCRIBE("test_dois_pedidos");
    test_dois_pedidos();
    DESCRIBE("test_dma");
    test_dma();
    DESCRIBE("test_int");
    test_int();
    DESCRIBE("test_dma_polling");
    test_dma_polling();
    DESCRIBE("test_dma_polling_atrasado");
    test_dma_polling_atrasado();
    DESCRIBE("test_muitos_polling");
    test_muitos_polling();
    DESCRIBE("test_sortidos");
    test_sortidos();
    
    GRADEME();
   	if (grade==maxgrade)
		return 0;
	else return grade;


}



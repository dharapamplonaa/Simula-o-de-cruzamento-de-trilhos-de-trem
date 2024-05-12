/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 *
 * This project is provided as an example of how to create a FreeRTOS project
 * that does not need a heap.  configSUPPORT_STATIC_ALLOCATION is set to 1 to
 * allow RTOS objects to be created using statically allocated RAM, and
 * configSUPPORT_DYNAMIC_ALLOCATION is set to 0 to remove any build dependency
 * on the FreeRTOS heap.  When configSUPPORT_DYNAMIC_ALLOCATION is set to 0
 * pvPortMalloc() just equates to NULL, and calls to vPortFree() have no
 * effect.  See:
 *
 * http://www.freertos.org/a00111.html and
 * http://www.freertos.org/Static_Vs_Dynamic_Memory_Allocation.html
 *
 *******************************************************************************
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard demo includes. */
#include "StaticAllocation.h"

//por via das duvidas
#include "semphr.h"
#include "queue.h"
#include <stdbool.h>

typedef enum dir {
	NORTE,
	SUL,
	LESTE,
	OESTE
} Direcao;


// Declarar buffers de memória estática para as tasks e semáforos

/*-----------------------------------------------------------*/
// Declarar buffers de memória estática para as tasks e semáforos
StaticTask_t xTaskBuffer[6];  // Increased buffer for additional tasks
StackType_t xStack[6][configMINIMAL_STACK_SIZE];
StaticSemaphore_t xSemaphoreBuffer[4];

StaticSemaphore_t xSemaphoreBufferSemaforoSync;
SemaphoreHandle_t semaforoSync;


/*-----------------------------------------------------------*/
// Semáforos e Mutexes
SemaphoreHandle_t semaforoCruzamentoNorteSul;
SemaphoreHandle_t semaforoCruzamentoLesteOeste; // Additional semaphore for East-West cars
SemaphoreHandle_t mutexCancela;


/*-----------------------------------------------------------*/
// Estados
volatile bool cancelaFechada = 0; // Declaração da variável global
volatile bool semaforoVerde = false; // Verdadeiro se o semáforo estiver verde para os carros


/*
 * Prototypes for the standard FreeRTOS stack overflow hook (callback)
 * function.  http://www.freertos.org/Stacks-and-stack-overflow-checking.html
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

/*
 * This demo has configSUPPORT_STATIC_ALLOCATION set to 1 so the following
 * application callback function must be provided to supply the RAM that will
 * get used for the Idle task data structures and stack.
 */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/*
* This demo has configSUPPORT_STATIC_ALLOCATION set to 1 and configUSE_TIMERS
* set to 1 so the following application callback function must be provided to
* supply the RAM that will get used for the Timer task data structures and
* stack.
*/
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* This demo only uses the standard demo tasks that use statically allocated
RAM.  A 'check' task is also created to periodically inspect the demo tasks to
ensure they are still running, and that no errors have been detected. */
static void prvStartCheckTask( void );
static void prvCheckTask( void *pvParameters );

/*-----------------------------------------------------------*/
// Protótipos das funções das tasks
void TaskTrem(void* pvParameters);
void TaskCarro(void* pvParameters);
void TaskCancela(void* pvParameters);
void TaskSemaforo(void* pvParameters);



int main(void) {
	// Criação de um semáforo binário para sincronização e sua liberação inicial
	semaforoSync = xSemaphoreCreateBinaryStatic(&xSemaphoreBufferSemaforoSync);
	xSemaphoreGive(semaforoSync);  // Inicia com o semáforo disponível

	// Criação e liberação inicial dos semáforos de cruzamento
	semaforoCruzamentoNorteSul = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer[0]);
	semaforoCruzamentoLesteOeste = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer[1]); 
	
	// Criação de um mutex para gerenciar o acesso à cancela
	mutexCancela = xSemaphoreCreateMutexStatic(&xSemaphoreBuffer[2]);

	
	xSemaphoreGive(semaforoCruzamentoNorteSul);
	xSemaphoreGive(semaforoCruzamentoLesteOeste); 

	// Criação das tasks com suas respectivas prioridades e parâmetros
	xTaskCreateStatic(TaskTrem, "Trem Norte-Sul", configMINIMAL_STACK_SIZE, (void*)NORTE, 1, xStack[0], &xTaskBuffer[0]);
	xTaskCreateStatic(TaskTrem, "Trem Sul-Norte", configMINIMAL_STACK_SIZE, (void*)SUL, 1, xStack[1], &xTaskBuffer[1]);
	xTaskCreateStatic(TaskCarro, "Carro Leste-Oeste", configMINIMAL_STACK_SIZE, (void*)LESTE, 1, xStack[2], &xTaskBuffer[2]);
	xTaskCreateStatic(TaskCarro, "Carro Oeste-Leste", configMINIMAL_STACK_SIZE, (void*)OESTE, 1, xStack[3], &xTaskBuffer[3]);
	xTaskCreateStatic(TaskCancela, "Cancela", configMINIMAL_STACK_SIZE, NULL, 1, xStack[4], &xTaskBuffer[4]);
	xTaskCreateStatic(TaskSemaforo, "Semaforo", configMINIMAL_STACK_SIZE, NULL, 1, xStack[5], &xTaskBuffer[5]);

	// Inicia o escalonador do FreeRTOS
	vTaskStartScheduler();
	for (;;);
	return 0;
}

/*----------------------------TREM-------------------------------*/
void TaskTrem(void* pvParameters) {
	
	Direcao dir = (Direcao)pvParameters; // Interpreta o parâmetro recebido como a direção do trem
	const char* dirName = (dir == NORTE) ? "Norte" : "Sul";
	SemaphoreHandle_t semaforoCruzamento = (dir == NORTE) ? semaforoCruzamentoNorteSul : semaforoCruzamentoLesteOeste;

	while (1) {
		printf("Trem %s chegando.\n", dirName);// Anuncia a chegada do trem
		xSemaphoreTake(semaforoCruzamento, portMAX_DELAY);// Aguarda permissão para cruzar
		// Aguarda permissão para operar a cancela
		xSemaphoreTake(mutexCancela, portMAX_DELAY);
		cancelaFechada = 1;
		printf("Cancela fechando.\n");
		// Libera o mutex da cancela após fechá-la
		xSemaphoreGive(mutexCancela);
		vTaskDelay(pdMS_TO_TICKS(5000)); // Simulando tempo de cruzamento

		printf("Trem %s passou.\n", dirName); // Anuncia que o trem passou
		xSemaphoreTake(mutexCancela, portMAX_DELAY); // Toma novamente o mutex da cancela para abri-la
		cancelaFechada = 0;
		printf("Cancela abrindo.\n");
		xSemaphoreGive(mutexCancela);
		xSemaphoreGive(semaforoCruzamento);// Libera o semáforo para outros trens ou carros
		vTaskDelay(pdMS_TO_TICKS(10000)); // Intervalo até o próximo trem
	}
}






/*----------------------------CARRO-------------------------------*/
void TaskCarro(void* pvParameters) {
	 // Interpreta o parâmetro passado para determinar a direção do carro
	Direcao dir = (Direcao)pvParameters;
	const char* dirName = (dir == LESTE) ? "Leste" : "Oeste";// Armazena o nome da direção para uso nas mensagens de log

	while (1) {
		xSemaphoreTake(mutexCancela, portMAX_DELAY);
		if (semaforoVerde) {
			printf("Carro do %s passando...\n", dirName);// Se o semáforo está verde, o carro pode passar
			vTaskDelay(pdMS_TO_TICKS(1000)); // Simula o tempo para o carro cruzar
			printf("Carro do %s passou.\n", dirName);
		}
		else {
			printf("Carro do %s esperando, semáforo vermelho.\n", dirName);
		}
		xSemaphoreGive(mutexCancela);
		vTaskDelay(pdMS_TO_TICKS(2000)); // Intervalo até o próximo carro
	}
}




/*----------------------------CANCELA-------------------------------*/

void TaskCancela(void* pvParameters) {
	bool lastState = false;
	while (1) {
		xSemaphoreTake(mutexCancela, portMAX_DELAY);
		if (cancelaFechada != lastState) {
			if (cancelaFechada) {
				printf("Cancela fechando...\n");
				vTaskDelay(pdMS_TO_TICKS(500)); // Tempo para simular fechamento da cancela
				printf("Cancela fechada.\n");
			}
			else {
				printf("Cancela abrindo...\n");
				vTaskDelay(pdMS_TO_TICKS(500)); // Tempo para simular abertura da cancela
				printf("Cancela aberta.\n");
			}
			lastState = cancelaFechada;
			xSemaphoreGive(mutexCancela);
			xSemaphoreGive(semaforoSync);  // Libera o semáforo para atualização
		}
		else {
			xSemaphoreGive(mutexCancela);
		}
		vTaskDelay(pdMS_TO_TICKS(1000));  // Verificação menos frequente para evitar carga desnecessária
	}
}







/*----------------------------SEMAFORO-------------------------------*/

void TaskSemaforo(void* pvParameters) {
	bool lastState = false;
	while (1) {
		xSemaphoreTake(semaforoSync, portMAX_DELAY);  // Espera pelo sinal da TaskCancela
		if (cancelaFechada != lastState) {
			semaforoVerde = !cancelaFechada;  // Semáforo verde quando a cancela não está fechada
			printf("Semáforo para carros: %s.\n", semaforoVerde ? "Verde" : "Vermelho");
			lastState = cancelaFechada;
		}
	}
}





static void prvStartCheckTask( void )
{
/* Allocate the data structure that will hold the task's TCB.  NOTE:  This is
declared static so it still exists after this function has returned. */
static StaticTask_t xCheckTask;

/* Allocate the stack that will be used by the task.  NOTE:  This is declared
static so it still exists after this function has returned. */
static StackType_t ucTaskStack[ configMINIMAL_STACK_SIZE * sizeof( StackType_t ) ];

	/* Create the task, which will use the RAM allocated by the linker to the
	variables declared in this function. */
	xTaskCreateStatic( prvCheckTask, "Check", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, ucTaskStack, &xCheckTask );
}
/*-----------------------------------------------------------*/

static void prvCheckTask( void *pvParameters )
{
const TickType_t xCycleFrequency = pdMS_TO_TICKS( 2500UL );
static char *pcStatusMessage = "No errors";

	/* Just to remove compiler warning. */
	( void ) pvParameters;

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again. */
		vTaskDelay( xCycleFrequency );

		/* Check the tasks that use static allocation are still executing. */
		if( xAreStaticAllocationTasksStillRunning() != pdPASS )
		{
			pcStatusMessage = "Error: Static allocation";
		}

		/* This is the only task that uses stdout so its ok to call printf()
		directly. */
		printf( "%s - tick count %d - number of tasks executing %d\r\n",
													pcStatusMessage,
													xTaskGetTickCount(),
													uxTaskGetNumberOfTasks() );
	}
}

/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  This function is
	provided as an example only as stack overflow checking does not function
	when running the FreeRTOS Windows port. */
	vAssertCalled( __LINE__, __FILE__ );
}
/*-----------------------------------------------------------*/

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

	/* Called if an assertion passed to configASSERT() fails.  See
	http://www.freertos.org/a00110.html#configASSERT for more information. */

	/* Parameters are not used. */
	( void ) ulLine;
	( void ) pcFileName;

	printf( "ASSERT! Line %d, file %s\r\n", ulLine, pcFileName );

 	taskENTER_CRITICAL();
	{
		/* You can step out of this function to debug the assertion by using
		the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
		value. */
		while( ulSetToNonZeroInDebuggerToContinue == 0 )
		{
			__asm{ NOP };
			__asm{ NOP };
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

	/* Pass out a pointer to the StaticTask_t structure in which the Timer
	task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


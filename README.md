# Simula-o-de-cruzamento-de-trilhos-de-trem


Este projeto implementa uma simulação de um cruzamento de uma via ferroviária com uma via de carros utilizando o FreeRTOS. O sistema garante a segurança dos veículos e trens no cruzamento, controlando uma cancela e semáforos com base na aproximação de trens.

## Funcionalidades

- **Detecção de Trens**: O sistema detecta a aproximação de trens tanto do norte quanto do sul.
- **Controle de Cancela**: Automaticamente fecha quando um trem está se aproximando e abre quando o trem passa.
- **Semáforos para Carros**: Controla um semáforo que indica se os carros podem passar ou devem esperar.
- **Prioridade de Trens**: Trens têm prioridade sobre os carros, garantindo que não haja colisões no cruzamento.

## Estrutura do Projeto

O projeto é estruturado em várias tasks do FreeRTOS que gerenciam diferentes partes do sistema:

- **TaskTrem**: Simula a detecção e o tráfego de trens.
- **TaskCarro**: Gerencia o tráfego de carros tentando cruzar a via férrea.
- **TaskCancela**: Controla a abertura e fechamento da cancela baseada na presença de trens.
- **TaskSemaforo**: Atualiza o estado do semáforo para carros baseado no estado da cancela.

## Tecnologias Utilizadas

- **FreeRTOS**: Utilizado para gerenciamento de multitarefas.
- **C**: Linguagem de programação usada para implementação do projeto.

## Pré-requisitos

Para executar este projeto, você precisará de um ambiente capaz de compilar e executar código C com FreeRTOS. Isso pode ser configurado em uma IDE como o Visual Studio Code com extensões para C/C++ e suporte para compilar projetos FreeRTOS.

## Resultados esperados

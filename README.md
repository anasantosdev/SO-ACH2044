# Algoritmo de Ordenação Paralelo (MergeSort) - Sistemas Operacionais

## 💡 Decisões de Projeto

### Algoritmo de Ordenação

Para a elaboração do EP, inicialmente busquei compreender quais algoritmos de ordenação permitiriam a paralelização de seus passos. Após alguns estudos, o **Merge Sort** foi escolhido devido à sua estrutura recursiva, que permite a divisão do problema em subproblemas independentes, ideal para execução em múltiplas threads.

### Número de Threads

Outra decisão importante diz respeito ao número de threads a ser utilizado pelo algoritmo quando o parâmetro de entrada é “0”. Para isso, adaptei o programa para permitir tanto a **especificação manual** (valores de 1 a 8) quanto a **adaptação ao número de núcleos do processador**. Para determinar os recursos disponíveis quando o número de threads é dado como "0", utilizei as funções `get_nprocs` e `get_nprocs_conf` da biblioteca `sys.info`, que fornecem a quantidade de núcleos disponíveis ou configurados no sistema. Nesse caso, o número de threads determinado pelo programa é sempre o maior valor entre os núcleos disponíveis ou configurados.

### Estrutura do Código

Organizei o código em três funções principais, além da função `main`, para melhorar a legibilidade e simplificar a manutenção. As funções são detalhadas a seguir:

1. **Leitura e Mapeamento de Arquivo**
   A primeira função implementada foi a **`read_records`**, responsável por realizar a leitura e mapeamento dos registros do arquivo de entrada `.dat` diretamente na memória utilizando a função **`mmap`**. Isso otimiza o desempenho ao lidar com arquivos grandes, reduzindo os acessos ao disco.

2. **Merge Sort Paralelo**
   A segunda função, **`mergeSort`**, gerencia a ordenação paralela dos registros, distribuindo o trabalho entre as threads disponíveis e realizando a ordenação em partes menores do array. Nesse ponto, o algoritmo invoca a função **`mergeSortParallel`**, que realiza a ordenação recursiva, chamando a função **`merge`** ao finalizar a rotina.

3. **Mesclagem dos Resultados**
   Após a execução das threads, cada thread retorna um resultado parcial da ordenação. Esses resultados são combinados de forma sequencial através da função **`merge`**, que garante que o array final esteja completamente ordenado.

4. **Gravação no Arquivo de Saída**
   Por fim, a função **`write_records`** é responsável por gravar os registros ordenados no arquivo de saída `.out`, utilizando também a função **`mmap`** para otimizar o desempenho durante a gravação.


## 📂 Arquivos

- **Entrada**: O arquivo de entrada é um arquivo `.dat`, contendo os registros a serem ordenados.
- **Saída**: O arquivo de saída gerado pelo programa é um arquivo `.out`, contendo os registros ordenados.


## 🛠 Tecnologias Utilizadas

- **Linguagem:** C
- **Bibliotecas:** 
  - `sys/info` (para detecção de núcleos do processador)
  - `mmap` (para otimização de leitura e gravação de arquivos)
  - `pthread` (para implementação de threads)

## 🧑‍💻 Como Executar

### Pré-requisitos

Para rodar o código deste repositório, você precisará de:

- **Sistema Operacional**: Linux (ou ambiente compatível com threads POSIX como um WSL)
- **Compilador**: GCC ou qualquer compilador C que suporte a biblioteca `pthread`.
- **Bibliotecas**: O código utiliza as bibliotecas padrão de C e a biblioteca `pthread` para manipulação de threads. Além disso, a biblioteca `sys/sysinfo.h` é utilizada para obter o número de núcleos do processador.
- **Memória Virtual**: O código faz uso de **mmap** para otimizar o acesso ao arquivo de entrada e saída, carregando os dados diretamente na memória.

### Como Compilar

1. Abra o terminal no diretório onde o repositório está localizado.
2. Compile o código com o seguinte comando:

```bash
gcc -o merge_sort_parallel merge_sort_parallel.c -lpthread
'''

🌟 Para dúvidas ou sugestões, estou à disposição! ✨

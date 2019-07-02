REQUISITOS:

Nosso simulador foi criado na linguagem C. Portanto é necessário um compilador para a execução. Há vários compliladores disponíveis como o gcc e o minGW.
Escolha o compilador de sua preferência e instale-o.
Lembrando que se você estiver usando Windows é necessário alterar as variáveis de ambiente.

COMPILAÇÃO:

Com um compilador instalado, entre no diretório onde está a pasta com o arquivo do simulador pelo prompt.
Lá dentro digite: "gcc mips.c -o mips.exe", isso fará com que o código seja compilado. Para rodar o programa digite: "mips" ou "mips.exe" e o
código será executado.

EXECUÇÃO:

A utilização do programa é bem simples, a cada passo da execução será exibido uma tabela com o valor dos registradores, sinais de controle e conteúdo
de memória. Também haverá o valor de PC, o estado atual da instrução e o opcode da instrução. Para avançar no código basta apertar enter.

OBSERVAÇÕES:

Há algumas "regras" que precisam ser seguidas para a perfeita execução do programa:

-- O "input" dado precisa se chamar "Programa.mips";

-- O programa precisa estar com as instruções em hexadecimal;

-- Ao contrário do Mars, nosso simulador aloca conteúdo na memória em 1 posição apenas e não em 4.
o que significa que instruções como "addi $sp,$sp,-4" em nosso processador são escritas assim: "addi $sp,$sp,-1";

-- Para o programa saber o que é instrução e o que é variável (.data) é necessário ter uma linha escrita ".data" dividindo as duas áreas;

-- Quando acabar a parte do .data é necessário quebrar uma linha e escrever ".endData".
Caso fique com dúvida, dê uma olhada no programa teste que vem juntamente com o código.
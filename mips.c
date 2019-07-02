#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
**********************************************
*****Declaração de Métodos e Variáveis********
**********************************************
*/

int pc;
int A;
int B;
int UlaSaida;
int regInst;
int regDadoMem;
int regDest(); // Devolve o registrador destino adequado
int ulaFonteA(); // Devolve o valor fonte A da ula adequada.
int ulaFonteB(); // Devolve o valor fonte B da ula adequada.
int atualizaPc();
void escreveMemoria();
void escreveRegistadores();
int registradorInstrucao();
int lerMemoria();
int ula(int func);
void lerArquivo();
int* memoria;
int* data;
int* registradores;


/*
*************************************
*****Estrutura do Bloco de BC********
*************************************
*/
struct Controle{
    
    int opcode;    //Variável que guarda o opcode.

    //Sinais que utilizamos

    int ULAOp;     //Decide Operação da Ula.
    int ULAFonteB; //Decide o primeiro operando (B, 4, Imediato ou 26Bits Jump).
    int ULAFonteA; //Decide o segundo operando (A ou PC).
    int RegDst;    //Decide qual o registrador destino.
    int FontePC;    //Decide o próximo PC (+4, desvio incondicional, desvio condicional).
    int PCEsc;      //Habilita a escrita no PC.
    int Branch;     //Define se é uma instrução de branch. (PCEscCond)
    int IouD;       //Define o endereço de memória trabalhado.
    int LerMem;     //Habilita Leitura da memória.
    int EscMem;     //Habilita escrita em memória.
    int MemParaReg; //Decide o que será escrito no registrador destino.
    int EscReg;     //Habilita escrita nos registradores.
    int IREsc;      //Habilita registrador de instrução.


    //Esses não são necessários por ser uma simulação, apenas abstração


}BC; //Bloco de Controle

enum Estados
{
    Busca = 0,
    Decodifica = 1,
    Execucao = 2,
    Memoria = 3,
    Write = 4,
};

enum Estados estadoAtual = Busca;


/*
*******************************
*****Maquina de Estados********
*******************************
*/

void clearControl(){
    BC.ULAOp = 0;      //Decide Operação da Ula.
    BC.ULAFonteB = 0;  //Decide o primeiro operando (B, 4, Imediato ou 26Bits Jump).
    BC.ULAFonteA = 0;  //Decide o segundo operando (A ou PC).
    BC.RegDst = 0;     //Decide qual o registrador destino.
    BC.FontePC = 0;    //Decide o próximo PC (+4, desvio incondicional, desvio condicional).
    BC.PCEsc = 0;      //Habilita a escrita no PC.
    BC.Branch = 0;     //Define se é uma instrução de branch. (PCEscCond)
    BC.IouD = 0;       //Define o endereço de memória trabalhado.
    BC.LerMem = 0;     //Habilita Leitura da memória.
    BC.EscMem = 0;     //Habilita escrita em memória.
    BC.MemParaReg = 0; //Decide o que será escrito no registrador destino.
    BC.EscReg = 0;
}

//Controla as etapas da execução.
void MaquinaEstados()
{
    clearControl();
    switch (estadoAtual)

    {
    case Busca:
        //Busca da instrução -- 

        //Define sinais de Controle
        BC.LerMem = 1;
        BC.ULAFonteA = 0;
        BC.IouD = 0;
        BC.IREsc = 1;
        BC.ULAFonteB = 1;
        BC.ULAOp = 0;
        BC.PCEsc = 1;
        BC.FontePC = 0;

        //Ações   
        regInst = registradorInstrucao();
        //PC = PC + 1
        pc = atualizaPc();
        
        //Próximo Estado
        estadoAtual = Decodifica;
        break;

    case Decodifica:
        //Decodificação da instrução -- leitura dos registradores Rs e Rt --

        //Define sinais de Controle
        BC.ULAFonteA = 0;
        BC.ULAFonteB = 3;
        BC.ULAOp = 0;
        //Ações
        BC.opcode = (regInst >> 26) & 63;
        A = registradores[(regInst >> 21) & 31];
        B = registradores[(regInst >> 16) & 31];

        UlaSaida = ula(-1);

        //Próximos estados
        estadoAtual = Execucao;
        break;

    case Execucao:

        //Etapa de execução de instruções --
        switch (BC.opcode)
        {

        case 0: // *Tipo R* -- Execução --

            //Define Sinais de Controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 0;
            BC.ULAOp = 2;

            
            //Próximo Estado
            estadoAtual = Memoria;

            break;

        case 9: // *TipoI* -- ADDIU -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 13: // *TipoI* -- ORI -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 3;

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 15: // *TipoI* -- LUI -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;


            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 4: //Branch if Equal -- Execução

            //Sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 0;
            BC.ULAOp = 1;
            BC.PCEsc = 0;
            BC.FontePC = 1;
            BC.Branch = 1;

            if (UlaSaida & pow(2,15) != 0)
            {
                UlaSaida -= pow(2,16);
            }
            

            //Próximo Estado
            estadoAtual = Busca;
            break;

        case 2: //Jump -- Calculo do endereço de salto

            //Sinais de Controle
            BC.PCEsc = 1;
            BC.FontePC = 2;

            
            //Próximo Estado
            estadoAtual = Busca;

            break;

        case 35: //Load Word -- Calculo da posição da memória.

            //Sinais de Controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 43: //Store Word -- Calculo da posição da memória.

            //Sinais de Controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        default : 
             break;
        }

        //Ações (Genérico)
        pc = atualizaPc();
        UlaSaida = ula(regInst & 0b111111);
        if (UlaSaida & pow(2, 15) != 0)
        {
            UlaSaida -= pow(2, 16);
        }

        break;
        
    case Memoria:

        switch (BC.opcode)
        {
        case 0: //Tipo R -- Escrita no Registador Destino

            //Sinais de Controle
            BC.RegDst = 1;
            BC.EscReg = 1;
            BC.MemParaReg = 0;

           
            //Próximo Estado
            estadoAtual = Busca;
            break;

        case 15: //Tipo I -- LUI - Memória -- Acesso à memória

            //Sinais de Controle
            BC.RegDst = 0;
            BC.EscReg = 1;
            BC.MemParaReg = 0;

            UlaSaida = UlaSaida << 16;

            //Próximo Estado
            estadoAtual = Busca;

            break;
        case 35: //Load Word -- Acesso à memória

            //Sinais de Controle
            BC.LerMem = 1;
            BC.IouD = 1;

            //Próximo Estado
            estadoAtual = Write;
            break;

        case 43: //Store Word -- Acesso e escrita em memória

            //Sinais de Controle
            BC.EscMem = 1;
            BC.IouD = 1;

            //Próximo Estado
            estadoAtual = Busca;

            break;

        default: //Tipo I -- ADDIU TESTE - Escrita na parte superior do Registrador Destino

            //Sinais de Controle
            BC.RegDst = 0;
            BC.EscReg = 1;
            BC.MemParaReg = 0;

            //Próximo Estado
            estadoAtual = Busca;
            break;
        }

        //Ações Genérico
        escreveRegistadores();
        escreveMemoria();
        regDadoMem = lerMemoria();

        break;

    case Write:

        switch (BC.opcode)
        {
        case 35:
            //Load Word --- Write Back

            //Sinais de Controle
            BC.RegDst = 0;
            BC.MemParaReg = 1;
            BC.EscReg = 1;

            break;

        default:
            break;
        }

        escreveRegistadores();

        //Próximo Estado
        estadoAtual = Busca;

        break;
   
    default:
        break;
    }
}

/*
*************************************
*****Definição dos métodos***********
*************************************
*/

void printy()
{
    printf("==========================================\n");
    printf("|REGISTRADORES   |               CONTROLE|\n");
    printf("                 |                       |\n");
    printf("|$zero: %d        |             BRANCH: %2d|\n",registradores[0],BC.Branch);
    printf("|$1: %3d         |              PCEsc: %2d|\n",registradores[1],BC.PCEsc);
    printf("|$2: %3d         |               IouD: %2d|\n",registradores[2],BC.IouD);
    printf("|$3: %3d         |             LerMem: %2d|\n",registradores[3],BC.LerMem);
    printf("|$4: %3d         |             EscMem: %2d|\n",registradores[4],BC.EscMem);
    printf("|$5: %3d         |         MemParaReg: %2d|\n",registradores[5],BC.MemParaReg);
    printf("|$6: %3d         |              IREsc: %2d|\n",registradores[6],BC.IREsc);
    printf("|$7: %3d         |            FontePC: %2d|\n",registradores[7],BC.FontePC);
    printf("|$8: %3d         |              ULAOp: %2d|\n",registradores[8],BC.ULAOp);
    printf("|$9: %3d         |           ULAFontB: %2d|\n",registradores[9],BC.ULAFonteB);
    printf("|$10: %3d        |           ULAFontA: %2d|\n",registradores[10],BC.ULAFonteA);
    printf("|$11: %3d        |             EscReg: %2d|\n",registradores[11],BC.EscReg);
    printf("|$12: %3d        |             RegDst: %2d|\n",registradores[12],BC.RegDst);
    printf("|$13: %3d        |_______________________|\n",registradores[13]);
    printf("|$14: %3d        |        MEMORIA        |\n",registradores[14]);
    int j = 15;
    for (int i = 0; i < 21; i++)
    {
        if (memoria[i] != 0)
        {
            if (j <= 31)
            {
                printf("|$%d: %3d        |  mem[%2d]: %11X |\n",j,registradores[j],i,memoria[i]);
            }
            else
            {
                printf("|                |  mem[%2d]: %11X |\n",i,memoria[i]);
            }      
        }
        else
        {
            if (j <= 31)
            {
                printf("|$%d: %3d        |                       |\n",j,registradores[j]);
            }
        }
        j++;
    }
    printf("PC: %d\n",pc);
    printf("OpCode: %d\n",BC.opcode);
    switch (estadoAtual)
    {
    case 0:
        printf("Estado Atual: Busca\n");
        break;
    
    case 1:
        printf("Estado Atual: Decodificacao\n");
        break;

    case 2:
        printf("Estado Atual: Execucao\n");
        break;

    case 3:
        printf("Estado Atual: Memoria\n");
        break;

    case 4:
        printf("Estado Atual: Write-Back\n");
        break;

    default:
        break;
    }
    printf("==========================================\n");
}

void main(){
    memoria = calloc(50 , sizeof(unsigned long long)); 
    data = memoria + 20;
    registradores = calloc(32,sizeof(int));
    registradores[29] = 50;
    lerArquivo();
    pc = 0;

    char i;
    while(memoria[pc] != 0 || (estadoAtual != Busca))
    {
        printy();
        MaquinaEstados();
        printf("Pressione enter para avancar: ");
        scanf("%c",&i);
    }
    printy();
}

int regDest(){
    if(BC.RegDst == 1){
        //15 a 11
        return (regInst >> 11) & 0b11111;
    }else{
        //20 - 16
        return (regInst >> 16) & 0b11111;
    }
}

int registradorInstrucao(){
    if(BC.IREsc == 1){
        return lerMemoria();
    }
}

void escreveRegistadores(){
    if(BC.EscReg == 1){
        if(BC.MemParaReg == 1){
            registradores[regDest()] = regDadoMem;
        }else{
            registradores[regDest()] = UlaSaida;
        }
    }

}

void escreveMemoria(){
    if(BC.EscMem == 1){
        if(BC.IouD == 1){
            memoria[UlaSaida] = B; 
        }
    }
}

int lerMemoria(){
    if(BC.LerMem == 1){
        if(BC.IouD == 1){
            return memoria[UlaSaida];
        }else{
            return memoria[pc];
        }
    }
}

int atualizaPc(){
    int resultadoUla = ula(-1);
    if ((BC.Branch & resultadoUla) == 1 || BC.PCEsc == 1)
    {
      //  printf("Atualizou PC :");
        switch (BC.FontePC)
        {
        case 0:
            return resultadoUla;
            break;
        case 1:
      //      printf("Branch\n");
            return UlaSaida;
            break;
        case 2:
            return (pc & 0b11110000000000000000000000000000) + ((regInst & 0b11111111111111111111111111));
            break;
        default : 
            break;
        }
    }else{
        return pc;
    }
}

int ulaFonteA(){
    if(BC.ULAFonteA == 0){
        //Para a soma do PC
        return pc;
    }else{
        //Para outras operações com a ula
        return A;
    }
}

int ulaFonteB(){
    switch (BC.ULAFonteB)
    {
    case 0:
        //B
        return B;
        break;
    case 1:
        //Para o PC + 1
        return 1;
        break;
    case 2:
        //Imediato
        return regInst & 0b1111111111111111;
        break;
    case 3:
        return (regInst & 0b1111111111111111);
        break;

    default:
        break;
    }
}

int ula(int func)
{
    int a = ulaFonteA();
    int b = ulaFonteB();
   // printf("Operadores: A = %d, B = %d.\n",a,b);

    switch (BC.ULAOp)
    {
    case 0:       // 00: lw, sw, addiu e lui a ula faz um add
        return a + b; // codigo do add
    case 1:       // 01: beq a ula faz um subb
        if(a == b)
            return 1;
        return 0;
    case 2:       // 10: operações do tipo-R
        switch (func)
        {
        case 0:       // 000000 sll
            a = registradores[(regInst >> 16) & 31];
            b = (regInst >> 6) & 0b11111;
            return a << b; // a ula faz sll
        case 2:       // 000010 srl
            a = registradores[(regInst >> 16) & 31];
            b = (regInst >> 6) & 0b11111;
            return a >> b; // a ula faz srl
        case 33:      // 100001 addu                 
            return a + b; // a ula faz um add
        case 36:      // 100100 and
            return a & b; // a ula faz um and
        case 42:      // 101010 slt
            if(a<b)
                return 1;
            return 0;// a ula faz um set on less then
        default:
            //printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
            break;
        }
    case 3:       //11: ori a ula faz um or
        return a | b; // codigo do or
    default:
        break;
    }
}


/*
****************************************
*****Leitura do Arquivo Input***********
****************************************
*/

void lerArquivo(){
    FILE *arq;
    char linha[15];
    arq = fopen("Programa.mips", "rt");
    int* mem = memoria;
    int* dat = data;
    if(arq == NULL){
        printf("Problema ao abrir o arquivo");
        return;
    }
    
    fgets(linha, 15, arq);
    while (linha[0] != '.')
    {
        if(mem < dat){
            *mem = strtoull(linha, NULL, 0);
            //binaryString(*mem);
            //unsigned long long op = *mem >> 26;
            //binaryString(op);
            mem++;
        }else{
            printf("Programa não cabe na memória");
            return;
        }
    
        fgets(linha, 15, arq);
    }

    fgets(linha, 15, arq);
    int teste = 20;
    while(linha[0] != '.'){
        if(teste < 40){
            *dat++ = strtoull(linha, NULL,0);
        }else{
            printf("Dados não cabem na memória");
            return;
        }
        fgets(linha, 15, arq);
        teste++;
    }
    fclose(arq);
}

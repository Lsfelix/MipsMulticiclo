#include <stdio.h>
#include <stdlib.h>

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
int ula(int a, int b, int func);
int charToHex(char c);
int lineHexToInt(char* numero);
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


    //Esses não são necessários por ser uma simulação, apenas abstração

    int PCEsc;      //Habilita a escrita no PC.
    int IouD;       //Define o endereço de memória trabalhado.
    int LerMem;     //Habilita Leitura da memória.
    int IREsc;      //Habilita registrador de instrução.
    int FontePC;    //Decide o próximo PC (+4, desvio incondicional, desvio condicional).
    int Branch;     //Define se é uma instrução de branch.
    int EscMem;     //Habilita escrita em memória.
    int MemParaReg; //Decide o que será escrito no registrador destino.
    int EscReg;     //Habilita escrita nos registradores.

}BC; //Bloco de Controle

enum Estados
{
    Busca,
    Decodifica,
    Execucao,
    Memoria,
    Write,
};

enum Estados estadoAtual = Busca;


/*
*******************************
*****Maquina de Estados********
*******************************
*/

//Controla as etapas da execução.
void MaquinaEstados()
{

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
        regInst = memoria[pc];
        //PC = PC + 1
        pc = ula(pc,1,0);
        
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
        BC.opcode = regInst >> 26;
        A = registradores[(regInst >> 21) & 31];
        B = registradores[(regInst >> 16) & 31];
        UlaSaida = ula(pc , (regInst & 0b1111111111111111), 0);

        //Próximos estados
        estadoAtual = Execucao;
        break;

    case Execucao:
        //Etapa de execução de instruções --


        switch (BC.opcode)
        {
        
        case 0:
            // *Tipo R* -- Execução --
            
            //Define Sinais de Controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 0;
            BC.ULAOp = 2;

            //Ações

            //Se for um sll ou srl, muda a fonte B da ula para o SHAMT
            if (regInst & 63 == 0 || regInst & 63 == 2)
            {
                B = registradores[(regInst >> 6) & 31];
            }

            UlaSaida = ula(A, B, regInst & 0b111111);
            
            //Próximo Estado
            estadoAtual = Memoria;

            break;
        
        case 9:
            // *TipoI* -- ADDIU -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 13:
            // *TipoI* -- ORI -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 3;

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 15:
            // *TipoI* -- LUI -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 4:
            //Branch if Equal -- Execução

            //Sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 0;
            BC.ULAOp = 1;
            BC.PCEsc = 0;
            BC.FontePC = 1;

            //Ação
            if(ula(A,B,0) == 1){
                pc = UlaSaida;
            }

            //Próximo Estado
            estadoAtual = Busca;
            break;

        case 2:
            //Jump -- Calculo do endereço de salto

            //Sinais de Controle
            BC.PCEsc = 1;
            BC.FontePC = 2;

            //Ações
            //***Revisar Calculo***
            pc = (pc & 0b11110000000000000000000000000000) | ((regInst & 0b11111111111111111111111111) << 2);

            //Próximo Estado
            estadoAtual = Busca;

            break;

        case 36:
            //Load Word -- Calculo da posição da memória.

            //Sinais de Controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);
            
            //Próximo Estado
            estadoAtual = Memoria;
            break;

        case 43:
            //Store Word -- Calculo da posição da memória.
        
            //Sinais de Controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;
            BC.ULAOp = 0;

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);

            //Próximo Estado
            estadoAtual = Memoria;
            break;

        default : 
             break;
        }


        break;
        
    case Memoria:

        switch (BC.opcode)
        {
        case 0:
            //Tipo R -- Escrita no Registador Destino

            //Sinais de Controle
            BC.RegDst = 1;
            BC.EscReg = 1;
            BC.MemParaReg = 0;
            
            //Ações
            registradores[regDest()] = UlaSaida;

            //Próximo Estado
            estadoAtual = Busca;
            break;

        case 1:
            //Tipo I -- LUI - Escrita na parte superior do Registrador Destino

            //Sinais de Controle
            BC.RegDst = 0;
            BC.EscReg = 1;
            BC.MemParaReg = 0;

            //Ações
            registradores[regDest()] = UlaSaida << 16;

            //Próximo Estado
            estadoAtual = Busca;
        case 36:
            //Load Word -- Acesso à memória

            //Sinais de Controle
            BC.LerMem = 1;
            BC.IouD = 1;

            //Ações
            regDadoMem = memoria[UlaSaida];

            //Próximo Estado
            estadoAtual = Write;
            break;

        case 43:
            //Store Word -- Acesso e escrita em memória

            //Sinais de Controle
            BC.EscMem = 1;
            BC.IouD = 1;

            //Ações
            memoria[UlaSaida] = B;

            //Próximo Estado
            estadoAtual = Busca;

            break;

        default:
            break;
        }

        break;

    case Write:

        switch (BC.opcode)
        {
        case 36:
            //Load Word --- Write Back

            //Sinais de Controle
            BC.RegDst = 0;

            //Ações
            registradores[regDest()] = regDadoMem;

            break;
        
        default:
            break;
        }

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
    printf("|                |                       |\n");
    printf("|$zero: %d       |             BRANCH: %2d|\n",registradores[0],BC.Branch);
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
    printf("|$14: %3d        |        MEMORIA        |\n",registradores[1]);
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
                printf("|$%d: %3d        |                     |\n",j,registradores[j]);
            }
        }
        j++;
    }
    printf("==========================================\n");
}

void main(){
    memoria = malloc(50*sizeof(int));
    data = memoria + 20;
    registradores = calloc(32,sizeof(int));
    registradores[29] = 50;
    lerArquivo();
    pc = 0;

    int i;
    while(memoria[pc] != 0 || (estadoAtual != Busca))
    {
        MaquinaEstados();
        printy();
        printf("Digite qualquer coisa para avancar: ");
        scanf("%d",&i);
    }
}

int regDest(){
    if(BC.RegDst == 1){
        //15 a 11
        return regInst & 0b1111100000000000;
    }else{
        //20 - 16
        return regInst & 0b111110000000000000000;
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
        //Para o PC + !
        return 1;
        break;
    case 2:
        //Imediato
        return regInst & 0b1111111111111111;
        break;
    case 3:
        return (regInst & 0b1111111111111111) << 2;
        break;

    default:
        break;
    }
}

int ula(int a, int b,int func)
{
    a = ulaFonteA();
    b = ulaFonteB();

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
            return a << b; // a ula faz sll
        case 2:       // 000010 srl 
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
    printf("reskein: %d\n",*memoria);
    while(getc(arq) != '.'){
        fgets(linha, 15, arq);
        if(mem < dat){
            *mem++ = lineHexToInt(linha);
        }else{
            printf("Programa não cabe na memória");
            return;
        }
    }
    fgets(linha, 15, arq);
    int teste = 20;
    while(getc(arq) != '.'){
        fgets(linha, 15, arq);
        if(teste < 40){
            *dat++ = lineHexToInt(linha);
        }else{
            printf("Dados não cabem na memória");
            return;
        }
        teste++;
    }
    fclose(arq);
}
int lineHexToInt(char* num){
    num += 1;
    int resp = 0;
    while (*(num+1) != '\n')
    {
        resp = resp << 4;
        resp += charToHex(*num);
        num++;
    }
    return resp;
}
int charToHex(char c){
    switch (c)
    {
    case '0':
        return 0;
        break;
    case '1':
        return 1;
        break;
    case '2':
        return 2;
        break;
    case '3':
        return 3;
        break;
    case '4':
        return 4;
        break;
    case '5':
        return 5;
        break;
    case '6':
        return 6;
        break;
    case '7':
        return 7;
        break;
    case '8':
        return 8;
        break;
    case '9':
        return 9;
        break;
    case 'A':
        return 10;
        break;
    case 'B':
        return 11;
        break;
    case 'C':
        return 12;
        break;
    case 'D':
        return 13;
        break;
    case 'E':
        return 14;
        break;
    case 'F':
        return 15;
        break;
    default:
        return -1;
        break;
    }
}
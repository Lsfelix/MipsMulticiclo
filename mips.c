#include <stdio.h>
#include <stdlib.h>

/*
*************************************
*****Estrutura do Bloco de BC********
*************************************
*/
struct Controle{
    
    int opcode; //Variável que guarda o opcode.

    int Branch; //Define se é uma instrução de branch.
    int PCEsc; //Habilita a escrita no PC.
    int IouD; //Define o endereço de memória trabalhado.
    int LerMem; //Habilita Leitura da memória.
    int EscMem; //Habilita escrita em memória.
    int MemParaReg;//Decide o que será escrito no registrador destino.
    int IREsc; //Habilita registrador de instrução.
    int FontePC; //Decide o próximo PC (+4, desvio incondicional, desvio condicional).
    int ULAOp; //Decide Operação da Ula.
    int ULAFonteB; //Decide o primeiro operando (B, 4, Imediato ou 26Bits Jump).
    int ULAFonteA; //Decide o segundo operando (A ou PC).
    int EscReg; //Habilita escrita nos registradores.
    int RegDst; //Decide qual o registrador destino.

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
        //Nome Estado -- Instruções que usam estado ---

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
        B = registradores[(regInst >> 17) & 31];
        UlaSaida = ula(pc , (regInst & 0b1111111111111111), 0);

        //Próximos estados
        

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
            UlaSaida = ula(A, B, regInst & 0b111111);

            break;
        
        case 1:
            // *TipoI* -- Execucao --

            //Define sinais de controle
            BC.ULAFonteA = 1;
            BC.ULAFonteB = 2;


            break;
        
        case 4:
            //Branch if Equal -- Execução

            //Sinais de controle

            //Ação
            if(A == B){
                pc = UlaSaida;
            }

            break;

        case 2:
            //Jump

            pc = (pc & 11110000000000000000000000000000) & (regInst & 0b11111111111111111111111111);
            
            break;

        case 36:
            //Load Word -- Calculo da posição da memória.

            //Sinais de Controle
            BC.ULAOp = 0;

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);
            
            break;

        case 43:
            //Store Word -- Calculo da posição da memória.
            BC.ULAOp = 0;

            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);
            break;

        default : 
             break;
        }

        //Próximo Estado
        estadoAtual = Memoria;

        break;
        
    case Memoria:

        switch (BC.opcode)
        {
        case 0:
            //Tipo R -- Escrita no Registador Destino

            //Sinais de Controle

            //Ações
            
            registradores[regInst & 0b1111100000000000] = UlaSaida;

            break;

        case 36:
            //Load Word -- Acesso à memória

            //Sinais de Controle

            //Ações
            regDadoMem = memoria[UlaSaida];

            break;

        case 43:
            //SW

            memoria[UlaSaida] = B;

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

            //Ações
            registradores[regInst & 0b111110000000000000000] = regDadoMem;

            break;
        
        default:
            break;
        }

        break;

        estadoAtual = Busca;
   
    default:
        break;
    }
}
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
int mux2(int a, int b, int controle);
int mux3(int a, int b, int c, int controle);
int ula(int a, int b,int func);
void atualizaPc(int value, int inc);
int escritaPC(int zero);
void leituraRegistradores(int reg1, int reg2);
void escritaRegistradores(int RegEsc, int reg, int dado);
int leMemoria(int endereco, int lerMem);
int charToHex(char c);
int lineHexToInt(char* numero);
void leArquivo();
int* memoria;
int* data;
int registradores[32];


/*
*************************************
*****Definição dos métodos***********
*************************************
*/

void main(){
    memoria = malloc(50*sizeof(int));
    data = memoria + 20;
    leArquivo();
}

// apagado desloca dois, pois ele é usado para pular os endereços de 4 em 4 e nossa memória vai de 1 em 1
int mux2(int a, int b, int controle)
{
    switch (controle)
    {
    case 0:
        return a;
        break;
    case 1:
        return b;
        break;
    default:
        break;
    }
}

int mux3(int a, int b, int c, int controle)
{
    switch (controle)
    {
    case 0:
        return a;
        break;
    case 1:
        return b;
        break;
    case 2:
        return c;
        break;
    default:
        break;
    }
}
//Apagado extensão de sinal, pois não é um prolema em c
//Apagado mux4 pois o mux que vai para ula na vdd só precisa de 3 entradas, desconsiderando a de deslocar 2 bits
//FIXME: Conferir operações de sll e srl

int ula(int a, int b,int func)
{
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


int escritaPC(int zeroUla)
{
    int temp = zeroUla & BC.Branch;
    return temp | BC.PCEsc;
}

void leituraRegistradores(int reg1, int reg2)
{
    A = registradores[reg1];
    B = registradores[reg2];
}

void escritaRegistradores(int RegEsc, int reg, int dado)
{
    if (BC.EscReg == 1)
    {
        registradores[reg] = dado;
    }
}
//FIXME: implementar corretamente o caso em que a memoria não pode ser lida
int memOp(int endereco, int dado, int EscMem, int LerMem)
{
    if (EscMem == 1)
        memoria[endereco] = dado;
    else if (LerMem == 1)
        return memoria[endereco];
}

/*
****************************************
*****Leitura do Arquivo Input***********
****************************************
*/

void leArquivo(){
    FILE *arq;
    char linha[15];
    arq = fopen("Programa.mips", "rt");
    if(arq == NULL){
        printf("Problema ao abrir o arquivo.");
        return;
    }
    while(!feof(arq)){
        fgets(linha, 15, arq);
        if(memoria < data){
            *memoria++ = lineHexToInt(linha);
        }else{
            printf("Programa não cabe na memória.");
            return;
        }
            printf("%d\n", *(memoria - 1));
    }
    fclose(arq);
}
int lineHexToInt(char* num){
    num += 2;
    int i = 0;
    int resp = 0;
    while (i != 8)
    {
        resp = resp << 4;
        resp += charToHex(*num++);
        i++;
      
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
//TODO:
//  Registradores intermediarios
//  "Ligar os fios das funções"
//  Controlador
//  os "ciclos"
//  Leitura de Arquivo
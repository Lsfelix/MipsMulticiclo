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
            UlaSaida = ula(A, B, regInst & 0b111111);
            
            //Próximo Estado
            estadoAtual = Memoria;

            break;
        
        case 9:
            // *TipoI* -- ADDIU -- Execucao --

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

            //Próximo Estado
            estadoAtual = Busca;

            break;

        case 2:
            //Jump

            pc = (pc & 11110000000000000000000000000000) & (regInst & 0b11111111111111111111111111);

            //Próximo Estado
            estadoAtual = Busca;

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

            //Ações
            UlaSaida = ula(A, regInst & 0b1111111111111111, 0);


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
            //Ações
            
            registradores[regDest()] = UlaSaida;

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
            BC.RegDst = 0;

            //Ações
            registradores[regDest()] = regDadoMem;

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
int ula(int a, int b,int func);
int charToHex(char c);
int lineHexToInt(char* numero);
void lerArquivo();
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
    registradores[29] = 50;
    lerArquivo();

    int i;
    while(memoria[pc] != 0)
    {
        MaquinaEstados();
        //metodo print (ta no meu note)
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
    while(getc(arq) != '.'){
        fgets(linha, 15, arq);
        if(mem < data){
            *mem++ = lineHexToInt(linha);
        }else{
            printf("Programa não cabe na memória");
            return;
        }
    }
    fgets(linha, 15, arq);
    while(getc(arq) != '.'){
        fgets(linha, 15, arq);
        if(dat < (memoria + registradores[29])){
            *dat++ = lineHexToInt(linha);
        }else{
            printf("Dados não cabem na memória");
            return;
        }
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
//TODO:
//  Registradores intermediarios
//  "Ligar os fios das funções"
//  Controlador
//  os "ciclos"
//  Leitura de Arquivo
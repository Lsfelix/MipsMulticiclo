#include <stdio.h>
#include <stdlib.h>

//Estrutura do Bloco de BC
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
    Busca0,
    Decodifica1,
    TipoI2,
    AddI3,
    AddiEscrita4,
    Ori5,
    OriEscrita6,
    LoadAcesso7,
    LoadEscrita8,
    SaveAcesso9,
    LuiEscrita10,
    TipoRExec11,
    TipoREscrita12,
    SaltoCond13,
    SaltoIncond14,
    TipoIEscrita15
};

enum Estados estadoAtual = Busca0;

int pc;
int A;
int B;
int saidaUla;
int regInst;
int regDadoMem;
int mux2(int a, int b, int controle);
int mux3(int a, int b, int c, int controle);
int ula(int a, int b,int func, int controle);
void atualizaPc(int value, int inc);
int escritaPC(int zero);
void leituraRegistradores(int reg1, int reg2);
void escritaRegistradores(int RegEsc, int reg, int dado);
int leMemoria(int endereco, int lerMem);
int charToHex(char c);
int lineHexToInt(char* numero);
void leArquivo();
int* memoria;
int* sp;
int* data;
int registradores[32];


//Estabelece os sinais de acordo com o estado atual.
void MaquinaEstados(){


    switch (estadoAtual)
    {
    case Busca0:
        //Busca da instrução -- 0 -> 1

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
        //ula(pc,4); //PC = PC + 4



        estadoAtual = Decodifica1;
        break;

    case Decodifica1:
        //Decodificação da instrução -- leitura dos registradores Rs e Rt -- 1 -> 3 || 6 || 8 || 9
        BC.ULAFonteA = 0;
        BC.ULAFonteB = 3;
        BC.ULAOp = 0;

        switch (BC.opcode)
        {

        case 1:
            estadoAtual = TipoI2;
            break;
        case 2:
            estadoAtual = TipoRExec11; 
            break;
        case 3:
            estadoAtual = SaltoCond13;
            break;
        case 4:
            estadoAtual = SaltoIncond14;
            break;
        default:
            break;
        }

        break;

    case TipoI2:
        // *TipoI* -- Define o tipo I -- 2 -> 3 | 4
        BC.ULAFonteA = 1;
        BC.ULAFonteB = 2;
        BC.RegDst = 0;
        
        switch (BC.opcode)
        {
        case 1:
            estadoAtual = AddI3;
            break;
        
        default:
            estadoAtual = Ori5;
            break;
        }

        break;
    
    case AddI3:
        // *LW* ou *SW* ou *ADDIU* ou *LUI* -- Execução da soma --  3 -> 5 | 3 | 4
        BC.ULAOp = 0;

        switch (BC.opcode)
        {
        case 1://Alterar Case
            estadoAtual = LoadAcesso7;            
            break;
        case 2://Alterar Case
            estadoAtual = SaveAcesso9;
        case 3://Alterar Case
            estadoAtual = AddiEscrita4;
        case 15:
            estadoAtual = LuiEscrita10;
        default:
            break;
        }
    
    case AddiEscrita4:


    case Ori5:
        // *OrI -- Execução ORI -- 5 -> 6 
        BC.ULAOp = 3;

    case OriEscrita6:
        // *OrI -- Escrita ORI -- 6 -> 0
        BC.MemParaReg = 0;
        BC.EscReg = 1;

    case LoadAcesso7:
        // *LW* -- Acesso à memória -- 3 -> 4
        BC.LerMem = 1;
        BC.IouD = 1;
        estadoAtual = LoadEscrita8;
        break;

    case LoadEscrita8:
        // *LW* -- Escrita no registrador Rt -- 4 -> 0
        BC.EscReg= 1;
        BC.MemParaReg = 1;
        estadoAtual = Busca0;
        break;

    case SaveAcesso9:
        // *SW* -- Acesso á memória -- 5 -> 0
        BC.EscMem = 1;
        BC.IouD = 1;
        estadoAtual = Busca0;
        break;

    case LuiEscrita10:
        // *LUI* -- Escrita nos registradores -- 10 -> 0
        BC.EscReg = 1;
        BC.MemParaReg = 0;
        

    case TipoRExec11:
        // *Tipo R* -- Execução -- 6 -> 7
        BC.ULAFonteA = 1;
        BC.ULAFonteB = 0;
        BC.ULAOp = 2;
        estadoAtual = TipoREscrita12;
        break;

    case TipoREscrita12:
        // *Tipo R* -- Escrita em Rd -- 7 -> 0
        BC.RegDst = 1;
        BC.EscReg = 1;
        BC.MemParaReg = 0;
        estadoAtual = Busca0;
        break;

    case SaltoCond13:
        // *Desvio Condicional* -- Término do desvio condicional -- 8 -> 0
        BC.ULAFonteA = 1;
        BC.ULAFonteB = 0;
        BC.ULAOp = 1;
        BC.Branch = 1;
        BC.PCEsc = 0;
        BC.FontePC = 1;
        estadoAtual = Busca0;
        break;

    case SaltoIncond14:
        // *Desvio Incondicional* -- Término do desvio incondicional -- 9 -> 0
        BC.PCEsc = 1;
        BC.FontePC = 2;
        estadoAtual = Busca0;
        break;

    case TipoIEscrita15:
        // *Tipo I* -- Escrita nos registardores -- 10 -> 0
        BC.EscReg = 1;
        BC.RegDst = 0;
        BC.MemParaReg = 0;
        break;
    default:
        break;
    }


}

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

int ula(int a, int b,int func, int controle)
{
    switch (BC.ULAOp)
    {
    case 0:       // 00: lw, sw, addiu e lui a ula faz um add
        return a + b; // codigo do add
    case 1:       // 01: beq a ula faz um subb
        if(a == b)
            return 1;
        return 0; // codigo do subb
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
// conferir inc no controler
void atualizaPc(int value, int inc)
{
    if (inc == 1)
    {
        pc = value;
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
void leArquivo(){
    FILE *arq;
    char linha[15];
    arq = fopen("Programa.mips", "rt");
    if(arq == NULL){
        printf("Problema ao abrir o arquivo");
        return;
    }
    while(!feof(arq)){
        fgets(linha, 15, arq);
        if(memoria < data){
            *memoria++ = lineHexToInt(linha);
        }else{
            printf("Programa não cabe na memória");
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
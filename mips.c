#include <stdio.h>

int pc;
int A;
int B;
int saidaUla;
int zero;
int mux2(int a, int b, int controle);
int mux3(int a, int b, int c, int controle);
int mux4(int a, int b, int c, int d, int controle);
int deslocaDois(int a);
int ula(int a, int b, int operacao);
int operacaoUla(int func, int controle);
void atualizaPc(int value, int inc);
int incPc(int zero, int PCEscControl, int PCEsc);
void leituraRegistradores(int reg1, int reg2);
void escritaRegistradores(int RegEsc, int reg, int dado);
int leMemoria(int endereço, int lerMem);
int extensaoDeSinal(int valor);
int memoria[50];
int registradores[32];

int deslocaDois(int a){
    return a << 2;
}
int mux2(int a, int b, int controle){
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

int mux3(int a, int b, int c, int controle){
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

int extensaoDeSinal(int valor){return valor;}

int mux4(int a, int b, int c, int d, int controle){
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
    case 3:
        return d;
        break;
    default:
        break;
    }
}
//FIXME: Conferir operações de sll e srl
int operacaoUla(int func, int controle){
    switch (controle)
    {
        case 0: // 00: lw, sw, addiu e lui a ula faz um add
            return 2; // codigo do add
        case 1: // 01: beq a ula faz um subb
            return 6; // codigo do subb
        case 2: // 10: operações do tipo-R
            switch (func)
            {
                case 0: // 000000 sll VER QUAL OPERAÇÃO A ULA FAZ
                    return 3; // ?????
                case 2:// 000010 srl VER QUAL OPERAÇÃO A ULA FAZ
                    return 4; // ??????
                case 33: // 100001 addu 
                    return 2; // a ula faz um add
                case 36: // 100100 and
                    return 0; // a ula faz um and
                case 42: // 101010 slt
                    return 7; // a ula faz um set on less then
                default:
                    break;
            }
        case 3: //11: ori a ula faz um or
            return 1; // codigo do or
        default:
            break;
    }
}
//FIXME: Conferir operações de sll e srl
int ula(int a, int b, int operacao){
    int resp;
    switch (operacao)
    {
    case 0:
        resp = a & b;
    case 1:
        resp = a | b;
    case 2:
        resp = a + b;
    case 3:
        resp = -1; //????
    case 4:
        resp =-1;//?????
    case 6:
        resp = a - b;
    case 7:
        resp = -1;// ver como fazer um set on less then
    default:
        break;
    }
    if(resp == 0)
        zero = 1;
    else
        zero = 0;
    return resp;
}

void atualizaPc(int value, int inc){
    if(inc == 1){
        pc = value;
    }
}

int incPc(int zero, int PCEscControl, int PCEsc){
    int temp = zero & PCEscControl;
    return temp | PCEsc;
}

void leituraRegistradores(int reg1, int reg2){
    A = registradores[reg1];
    B = registradores[reg2];
}

void escritaRegistradores(int RegEsc, int reg, int dado){
    if(RegEsc == 1){
        registradores[reg] = dado;
    }
}
//FIXME: implementar corretamente o caso em que a memoria não pode ser lida
int memOp(int endereco, int dado, int EscMem, int LerMem){
    if(EscMem == 1)
        memoria[endereco] = dado;
    else if(LerMem == 1)
        return memoria[endereco];
}

//TODO: 
//  Registradores intermediarios
//  "Ligar os fios das funções"
//  Controlador
//  os "ciclos"
//  Leitura de Arquivo



//Estrutura do Bloco de BC
struct Controle{
    int PCEscCond;
    int PCEsc; //Habilita a escrita no PC
    int IouD; 
    int LerMem; //Habilita Leitura da memória
    int EscMem; //Habilita escrita em memória
    int MemParaReg;//Decide o que será escrito no registrador destino.
    int IREsc; //Habilita registrador de instrução
    int FontePC; //Decide o próximo PC (+4, desvio incondicional, desvio condicional)
    int ULAOp; //Decide Operação da Ula
    int ULAFonteB; //Decide o primeiro operando (B, 4, Imediato ou 26Bits Jump)
    int ULAFonteA; //Decide o segundo operando (A ou PC)
    int EscReg; //Habilita escrita nos registradores
    int RegDst; //Decide qual o registrador destino

}BC; //Bloco de Controle

enum Estados {
    Busca0,
    Decodifica1,
    LoadOuStore2,
    LoadAcesso3,
    LoadEscrita4,
    SaveAcesso5,
    TipoRExec6,
    TipoREscrita7,
    SaltoCond8,
    SaltoIncond9
};


enum Estados estadoAtual = Busca0;

int opcode = 0;

//Estabelece os sinais de acordo com o estado atual.
void DadosBC(){

    switch (estadoAtual)
    {
    case Busca0:
        //Busca da instrução -- 0 -> 1
        BC.LerMem = 1;
        BC.ULAFonteA = 0;
        BC.IouD = 0;
        BC.IREsc = 1;
        BC.ULAFonteB = 1;
        BC.ULAOp = 0;
        BC.PCEsc = 1;
        BC.FontePC = 0;
        estadoAtual = Decodifica1;
        break;

    case Decodifica1:
        //Decodificação da instrução -- leitura dos registradores Rs e Rt -- 1 -> 3 || 6 || 8 || 9
        BC.ULAFonteA = 0;
        BC.ULAFonteB = 3;
        BC.ULAOp = 0;

        switch (opcode)
        {

        case 1:
            estadoAtual = LoadOuStore2;
            break;
        case 2:
            estadoAtual = TipoRExec6; 
            break;
        case 3:
            estadoAtual = SaltoCond8;
            break;
        case 4:
            estadoAtual = SaltoIncond9;
            break;
        default:
            break;
        }

        break;

    case LoadOuStore2:
        // *LW* ou *SW* -- calculo do endereço de acesso à memória -- 2 -> 3
        BC.ULAFonteA = 1;
        BC.ULAFonteB = 2;
        BC.ULAOp = 0;
        estadoAtual = LoadAcesso3;
        break;

    case LoadAcesso3:
        // *LW* -- Acesso à memória -- 3 -> 4
        BC.LerMem = 1;
        BC.IouD = 1;
        estadoAtual = LoadEscrita4;
        break;

    case LoadEscrita4:
        // *LW* -- Escrita no registrador Rt -- 4 -> 0
        BC.EscReg= 1;
        BC.MemParaReg = 1;
        BC.RegDst = 0;
        estadoAtual = Busca0;
        break;

    case SaveAcesso5:
        // *SW* -- Acesso á memória -- 5 -> 0
        BC.EscMem = 1;
        BC.IouD = 1;
        estadoAtual = Busca0;
        break;

    case TipoRExec6:
        // *Tipo R* -- Execução -- 6 -> 7
        BC.ULAFonteA = 1;
        BC.ULAFonteB = 0;
        BC.ULAOp = 2;
        estadoAtual = TipoREscrita7;
        break;

    case TipoREscrita7:
        // *Tipo R* -- Escrita em Rd -- 7 -> 0
        BC.RegDst = 1;
        BC.EscReg = 1;
        BC.MemParaReg = 0;
        estadoAtual = Busca0;
        break;

    case SaltoCond8:
        // *Desvio Condicional* -- Término do desvio condicional -- 8 -> 0
        BC.ULAFonteA = 1;
        BC.ULAFonteB = 0;
        BC.ULAOp = 1;
        BC.PCEscCond = 1;
        BC.PCEsc = 0;
        BC.FontePC = 1;
        estadoAtual = Busca0;
        break;

    case SaltoIncond9:
        // *Desvio Incondicional* -- Término do desvio incondicional -- 9 -> 0
        BC.PCEsc = 1;
        BC.FontePC = 2;
        estadoAtual = Busca0;
        break;

    default:
        break;
    }


}

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
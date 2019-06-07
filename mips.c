#include <stdio.h>

struct{
    int PCEscCond;
    int PCEsc; //Habilita a Escrita no PC
    int IouD; 
    int LerMem; //Habilita Leitura da Memória
    int EscMem; //Habilita escrita em Memória
    int MemParaReg;
    int IREsc; 
    int FontePC; //Decide o próximo PC (+4, desvio incondicional, desvio condicional)
    int ULAOp; //Decide Operação da Ula
    int ULAFonteB; //Decide o primeiro operando (B, 4, Imediato ou 26Bits Jump)
    int ULAFonteA; //Decide o segundo operando (A ou PC)
    int EscReg; //Habilita escrita nos registradores
    int RegDst; //Decide qual o registrador destino

} Controle;

void DadosControle(int estado){

    switch (estado)
    {
    case 0:
        Controle.LerMem = 1;
        Controle.ULAFonteA = 0;
        Controle.IouD = 0;
        Controle.IREsc = 1;
        Controle.ULAFonteB = 1;
        Controle.ULAOp = 0;
        Controle.PCEsc = 1;
        Controle.FontePC = 0;
        break;

    case 1:
        Controle.ULAFonteA = 0;
        Controle.ULAFonteB = 3;
        Controle.ULAOp = 0;
        break;

    case 2:
        Controle.ULAFonteA = 1;
        Controle.ULAFonteB = 2;
        Controle.ULAOp = 0;
        break;

    case 3:
        Controle.LerMem = 1;
        Controle.IouD = 1;
        break;

    case 4:
        Controle.EscReg= 1;
        Controle.MemParaReg = 1;
        Controle.RegDst = 0;
        break;

    case 5:
        Controle.EscMem = 1;
        Controle.IouD = 1;
        break;

    case 6:
        Controle.ULAFonteA = 1;
        Controle.ULAFonteB = 0;
        Controle.ULAOp = 2;
        break;

    case 7:
        Controle.RegDst = 1;
        Controle.EscReg = 1;
        Controle.MemParaReg = 0;
        break;

    case 8:
        Controle.ULAFonteA = 1;
        Controle.ULAFonteB = 0;
        Controle.ULAOp = 1;
        Controle.PCEscCond = 1;
        Controle.PCEsc = 0;
        Controle.FontePC = 1;
        break;

    case 9:
        Controle.PCEsc = 1;
        Controle.FontePC = 2;
        break;

    default:
        break;
    }


}
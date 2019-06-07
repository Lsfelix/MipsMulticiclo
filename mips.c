#include <stdio.h>



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
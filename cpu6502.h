#ifndef __CPU6502__
#define __CPU6502__

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#define CPU6502_FLAG_C 0
#define CPU6502_FLAG_Z 1
#define CPU6502_FLAG_I 2
#define CPU6502_FLAG_D 3
#define CPU6502_FLAG_B 4
#define CPU6502_FLAG_V 6
#define CPU6502_FLAG_N 7

#define MEMORY_SIZE 0xffff

class Cpu_6502{
	protected:
		/* registradores do 6502 */
		int A,
			X, 
			Y,
			PC,
			P,
			S;
			
		int LastOpcode;
		string LastInstruction;
		
		/* ponteiro que vai apontar para memoria do 6502 */
		unsigned char *mem;
		unsigned int mem_size;
		
		/*
			break
		*/
		bool break_status;
		vector<int> *break_address;
		
	public:
		/*
			construtor
		*/
		Cpu_6502(){
			this->allocMem(MEMORY_SIZE);
			
			break_address = new vector<int>();
			this->A = 0;
			this->X = 0;
			this->Y = 0;
			this->PC = 0;
			this->P = 0;
			this->S = 0x1ff;
			this->break_status = false;
		}
	
		/*
			metodo usado para setar valores no registradores
			setReg(str registrador, int dado)
			eg: setReg("A",0x10)
		*/
		void setReg(std::string reg, int dado){
			if(reg.compare(std::string("A")) == 0){
				this->A = dado & 0xff;
			}
			
			else if(reg.compare(std::string("X")) == 0){
				this->X = dado & 0xff;
			}
			
			else if(reg.compare(std::string("Y")) == 0){
				this->Y = dado & 0xff;
			}
			
			else if(reg.compare(std::string("PC")) == 0){
				this->PC = dado & 0xffff;
			}
			
			else if(reg.compare(std::string("P")) == 0){
				this->P = dado & 0xff;
			}
			
			else if(reg.compare(std::string("S")) == 0){
				this->S = dado & 0xff;
			}
		}
		
		/*
			metodo usado para pegar um valor de um registrador
			getReg(string registrador)
			eg: getReg("A")
		*/
		int getReg(std::string reg){
			if(reg.compare(std::string("A")) == 0){
				return this->A;
			}
			
			else if(reg.compare(std::string("X")) == 0){
				return this->X;
			}
			
			else if(reg.compare(std::string("Y")) == 0){
				return this->Y;
			}
			
			else if(reg.compare(std::string("PC")) == 0){
				return this->PC;
			}
			
			else if(reg.compare(std::string("P")) == 0){
				return this->P;
			}
			
			else if(reg.compare(std::string("S")) == 0){
				return this->S;
			}
		}
		
		/*
			seta o registrador da flag (P)
			eg: setFlag(1); //Zero flag = 1
			eg2: setFlag(CPU6502_FLAG_Z)
		*/
		void setFlag(int flag){
			this->P |= (1 << flag);
		}
		
		/*
			Limpa a flag especifica
			eg: cleanFlag(1); //Zero flag = 0
		*/
		void cleanFlag(int flag){
			this->P &= ~(1 << flag);
		}
		
		/*
			retorna ultima instrução que foi executado
		*/
		int getLastOpcode(void){
			return this->LastOpcode;
		}
		
		string getLastInstruction(void){
			return this->LastInstruction;
		}
		
		/*
			aloca espaço para memoria da CPU e atribui a this->mem
			allocMem(tamamnho do barramento)
		*/
		void allocMem(unsigned int size){
			this->mem_size = size;
			this->mem = new unsigned char[size];
		}
		
		//libera a memoria
		void freeMem(void){
			delete this->mem;
		}
		
		/*
			escreve um byte na memoria em um endereço especifico
			writeMemoryByte(endereço,dado)
			eg: writeMemoryByte(0x200,0x61) 
		*/
		void writeMemoryByte(unsigned int address, unsigned char dado){
			this->mem[address] = (unsigned char) (dado & 0xff);
		}
		
		/*
			leitura de um byte na memoria em um endereço especifico
			readMemoryByte(endereço)
			eg: readMemoryByte(0x200) 
		*/
		char readMemoryByte(unsigned int address){
			return (this->mem[address] & 0xff);
		}
		
		/*
			ler um arquivo e joga na memoria
		*/
		void readMemoryFile(std::string arq){
			std::ifstream file(arq,std::ios::binary);
			
			if(!file.is_open()){
				std::cout << "file not opening";
				return ;
			}
			
			file.read(reinterpret_cast<char *>(this->mem),this->mem_size);
		}
		
		/*
			empilha na stack
		*/
		void stackPush(char dado){
			int stackAddr = this->S;
			dado &= 0xff;
			stackAddr &= 0xffff;
			writeMemoryByte(--stackAddr,dado);
			this->S = stackAddr;
		}
		
		/*
			desempilha da stack
		*/
		char stackPop(void){
			int stackAddr = this->S;
			stackAddr &= 0xffff;
			char dado = readMemoryByte(stackAddr);
			stackAddr++;
			this->S = stackAddr;
			return (dado & 0xff);
		}
		
		/*
			pega o status para break da cpu
		*/
		bool getBreakStatus(){
			return this->break_status;
		}
		
		/*
			modifica o status da break
		*/
		bool setBreakStatus(bool valor){
			this->break_status = valor;
		}
		
		/*
			add endereço para dar break
		*/
		void breakAdd(int address){
			this->break_address->push_back(address);
		}
		
		/*
			executa a instrução baseado no registrador PC
			executeInstruction();
		*/
		void executeInstruction(void){
			unsigned int pc = this->PC;
			unsigned char instruction = readMemoryByte(pc);
			unsigned int PC_temp = 0, PC_temp2 = 0;
				
			//BRK
			if(instruction == 0x00){
				this->PC += 2;
	
				//falta empurrar o status e o pc na pilha
	
				setFlag(CPU6502_FLAG_I);
				setFlag(CPU6502_FLAG_B);
				
				PC_temp = readMemoryByte(0xffff);
				PC_temp <<= 8; 
				
				PC_temp += readMemoryByte(0xfffe);
				PC_temp &= 0xffff;
				
				this->LastOpcode = 0x00;
			}
				
			//NOP
			else if(instruction == 0xea){
				this->PC = ++pc;
				this->LastOpcode = 0xea;
				
				this->LastInstruction += "nop";
			}
			
			//LDA immediate
			else if(instruction == 0xa9){
				this->A = readMemoryByte(++pc);
				this->A &= 0xff;
				this->PC = ++pc;
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa9;
			}
			
			//LDY immediate
			else if(instruction == 0xa0){
				this->Y = readMemoryByte(++pc);
				this->Y &= 0xff;
				this->PC = ++pc;
				
				if(this->Y == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->Y & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa0;
			}
			
			//LDX immediate
			else if(instruction == 0xa2){
				this->X = readMemoryByte(++pc);
				this->X &= 0xff;
				this->PC = ++pc;
				
				if(this->X == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->X & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa2;
			}
			
			//LDA absoluto
			else if(instruction == 0xad){
				PC_temp = readMemoryByte(++pc);
				PC_temp = PC_temp + (readMemoryByte(++pc) << 8);
				this->A = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xad;
			}
			
			//LDY absoluto
			else if(instruction == 0xa4){
				PC_temp = readMemoryByte(++pc);
				PC_temp = PC_temp + (readMemoryByte(++pc) << 8);
				this->A = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->Y == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->Y & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa4;
			}
			
			//LDX absoluto
			else if(instruction == 0xa6){
				PC_temp = readMemoryByte(++pc);
				PC_temp = PC_temp + (readMemoryByte(++pc) << 8);
				this->X = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->X == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->X & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa6;
			}
			
			//LDA absoluto,Y
			else if(instruction == 0xb9){
				PC_temp = readMemoryByte(++pc);
				PC_temp = (PC_temp + (readMemoryByte(++pc) << 8)) + (this->Y);
				this->A = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xb9;
			}
			
			//LDA absoluto,X
			else if(instruction == 0xbd){
				PC_temp = readMemoryByte(++pc);
				PC_temp = (PC_temp + (readMemoryByte(++pc) << 8)) + (this->X);
				this->A = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xbd;
			}
			
			//LDY absoluto,X
			else if(instruction == 0xbc){
				PC_temp = readMemoryByte(++pc);
				PC_temp = (PC_temp + (readMemoryByte(++pc) << 8)) + (this->X);
				this->Y = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->Y == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->Y & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xbc;
			}
			
			//LDX absoluto,Y
			else if(instruction == 0xbe){
				PC_temp = readMemoryByte(++pc);
				PC_temp = (PC_temp + (readMemoryByte(++pc) << 8)) + (this->Y);
				this->X = readMemoryByte(PC_temp);
				this->PC = ++pc;	
				
				if(this->X == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->X & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xbe;
			}
			
			//LDA Page0
			else if(instruction == 0xa5){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				
				this->A = (readMemoryByte(PC_temp) & 0xff);
				this->PC = ++pc;	
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa5;
			}
			
			//LDY Page0
			else if(instruction == 0xa4){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				
				this->Y = (readMemoryByte(PC_temp) & 0xff);
				this->PC = ++pc;	
				
				if(this->Y == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa4;
			}
			
			//LDX Page0
			else if(instruction == 0xa6){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				
				this->X = (readMemoryByte(PC_temp) & 0xff);
				this->PC = ++pc;	
				
				if(this->X == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->X & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xa6;
			}
			
			//LDA Page0,Y
			else if(instruction == 0xb6){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				
				this->A = (readMemoryByte(PC_temp) & 0xff) + (this->X);
				this->PC = ++pc;	
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xb6;
			}
				
			//LDA Page0,X
			else if(instruction == 0xb5){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				
				this->A = (readMemoryByte(PC_temp) & 0xff) + (this->X);
				this->PC = ++pc;	
				
				if(this->A == 0){
					setFlag(CPU6502_FLAG_Z);
					cleanFlag(CPU6502_FLAG_N);
				}
				
				else if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->LastOpcode = 0xb5;
			}
			
			//STA absoluto
			else if(instruction == 0x8d){
				PC_temp = readMemoryByte(++pc);
				PC_temp = PC_temp + (readMemoryByte(++pc) << 8);
				PC_temp &= 0xffff;
				
				writeMemoryByte(PC_temp,this->A);
				
				this->PC = ++pc;
				
				this->LastOpcode = 0x8d;
			}
			
			//STA Page0
			else if(instruction == 0x85){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				
				writeMemoryByte(PC_temp,this->A);
				this->PC = ++pc;
				
				this->LastOpcode = 0x85;
			}
			
			//JMP immediate
			else if(instruction == 0x4c){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xffff;
				
				PC_temp = PC_temp + (readMemoryByte(++pc) << 8);
				this->PC = PC_temp;
				this->LastOpcode = 0x4c;
			}
			
			//JMP absoluto
			else if(instruction == 0x6c){
				PC_temp2 = readMemoryByte(++pc);
				PC_temp2 = PC_temp2 + (readMemoryByte(++pc) << 8);
				PC_temp2 &= 0xffff;
				
				PC_temp = readMemoryByte(PC_temp2);
				PC_temp = PC_temp + (readMemoryByte(PC_temp2 + 1) << 8);
				PC_temp &= 0xffff;
				
				this->PC = PC_temp;
				this->LastOpcode = 0x4c;
			}
			
			//BEQ
			else if(instruction == 0xf0){
				PC_temp = readMemoryByte(++pc);
				PC_temp &= 0xff;
				this->PC++;
								
				if((this->P & (1 << CPU6502_FLAG_Z)) != 0){
					
					if((PC_temp & (1 << 7)) != 0){
						this->PC -= ((0xff - PC_temp));
					}
					else{
						this->PC += (PC_temp & 0xffff);
					}
				}
				else{
					//this->PC++;
					cout << endl << "   " << (this->PC++) << endl;
				}

				this->PC &= 0xffff;
				this->LastOpcode = 0xf0;
			}
			
			//CMP imediatte
			else if(instruction == 0xc9){
				PC_temp = (++pc & 0xFFFF);
				if((this->A & 0xff) == (readMemoryByte(PC_temp) & 0xff)){
					cleanFlag(CPU6502_FLAG_N);
					setFlag(CPU6502_FLAG_C);
					setFlag(CPU6502_FLAG_Z);
				}
				else if((this->A & 0xff) > (readMemoryByte(PC_temp) & 0xff)){
					cleanFlag(CPU6502_FLAG_N);
					setFlag(CPU6502_FLAG_C);
					cleanFlag(CPU6502_FLAG_Z);
				}
				else if((this->A & 0xff) < (readMemoryByte(PC_temp) & 0xff)){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_C);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->PC = ++PC_temp;
				this->LastOpcode = 0xc9;
			}
			
			//CMP absolute
			else if(instruction == 0xcd){
				
				PC_temp2 = readMemoryByte(++pc);
				PC_temp2 = PC_temp2 + (readMemoryByte(++pc) << 8);
				PC_temp2 &= 0xffff;
				
				PC_temp = readMemoryByte(PC_temp2);
				PC_temp = PC_temp + (readMemoryByte(PC_temp2 + 1) << 8);
				PC_temp &= 0xffff;
								
				PC_temp2 = (++pc & 0xFFFF);
				if((this->A & 0xff) == (readMemoryByte(PC_temp) & 0xff)){
					cleanFlag(CPU6502_FLAG_N);
					setFlag(CPU6502_FLAG_C);
					setFlag(CPU6502_FLAG_Z);
				}
				else if((this->A & 0xff) > (readMemoryByte(PC_temp) & 0xff)){
					cleanFlag(CPU6502_FLAG_N);
					setFlag(CPU6502_FLAG_C);
					cleanFlag(CPU6502_FLAG_Z);
				}
				else if((this->A & 0xff) < (readMemoryByte(PC_temp) & 0xff)){
					setFlag(CPU6502_FLAG_N);
					cleanFlag(CPU6502_FLAG_C);
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->PC = ++PC_temp;
				
				this->LastOpcode = 0xcd;
			}
			
			//SEC 
			else if(instruction == 0x38){
				setFlag(CPU6502_FLAG_C);
				this->PC = ++pc;
				this->LastOpcode = 0x38;
			}
			
			//CLC 
			else if(instruction == 0x18){
				cleanFlag(CPU6502_FLAG_C);
				this->PC = ++pc;
				this->LastOpcode = 0x18;
			}
			
			//SEI
			else if(instruction == 0x78){
				cleanFlag(CPU6502_FLAG_I);
				this->PC = ++pc;
				this->LastOpcode = 0x78;
			}
			
			//CLI
			else if(instruction == 0x58){
				cleanFlag(CPU6502_FLAG_I);
				this->PC = ++pc;
				this->LastOpcode = 0x58;
			}
			
			//CLV
			else if(instruction == 0xb8){
				cleanFlag(CPU6502_FLAG_V);
				this->PC = ++pc;
				this->LastOpcode = 0xb8;
			}
			
			//SED
			else if(instruction == 0xf8){
				setFlag(CPU6502_FLAG_D);
				this->PC = ++pc;
				this->LastOpcode = 0xf8;
			}
			
			//CLD
			else if(instruction == 0xd8){
				cleanFlag(CPU6502_FLAG_D);
				this->PC = ++pc;
				this->LastOpcode = 0xd8;
			}
			
			//INX
			else if(instruction == 0xE8){
				int msb;
				this->X += 1;
				
				this->X &= 0xff;
				
				if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
				}
				else{
					cleanFlag(CPU6502_FLAG_N);
				}
				
				if(this->A == 0x0){
					setFlag(CPU6502_FLAG_Z);
				}
				else{
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->PC = ++pc;
				this->LastOpcode = 0xE8;
			}
			
			//INY
			else if(instruction == 0xc8){
				int msb;
				this->Y += 1;
				
				this->Y &= 0xff;
				
				if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
				}
				else{
					cleanFlag(CPU6502_FLAG_N);
				}
				
				if(this->A == 0x0){
					setFlag(CPU6502_FLAG_Z);
				}
				else{
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->PC = ++pc;
				this->LastOpcode = 0xc8;
			}
			
			//ADC immediate
			else if(instruction == 0x69){
				int msb = (this->A >> 7) & 1;
				this->A += readMemoryByte(++pc);
				
				if(this->P & (1 << CPU6502_FLAG_C) != 0){
					this->A++;
				}
				
				if(((this->A >> 7) & 1) != msb){
					setFlag(CPU6502_FLAG_C);
				}
				else{
					cleanFlag(CPU6502_FLAG_C);
				}
				
				if(this->A > 0xff){
					setFlag(CPU6502_FLAG_V);
				}
				else{
					cleanFlag(CPU6502_FLAG_V);
				}
				
				this->A &= 0xff;
				
				if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
				}
				else{
					cleanFlag(CPU6502_FLAG_N);
				}
				
				if(this->A == 0x0){
					setFlag(CPU6502_FLAG_Z);
				}
				else{
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->PC = ++pc;
				this->LastOpcode = 0x69;
			}
			
			//ADC absoluto
			else if(instruction == 0x6d){
				int msb;
				PC_temp = readMemoryByte(++pc);
				PC_temp += (readMemoryByte(++pc) << 8);
				PC_temp &= 0xffff;
				this->A += readMemoryByte(PC_temp);
				
				if(this->P & (1 << CPU6502_FLAG_C) != 0){
					this->A++;
				}
				
				if(((this->A >> 7) & 1) != msb){
					setFlag(CPU6502_FLAG_C);
				}
				else{
					cleanFlag(CPU6502_FLAG_C);
				}
				
				if(this->A > 0xff){
					setFlag(CPU6502_FLAG_V);
				}
				else{
					cleanFlag(CPU6502_FLAG_V);
				}
				
				this->A &= 0xff;
				
				if((this->A & (1 << 7)) != 0){
					setFlag(CPU6502_FLAG_N);
				}
				else{
					cleanFlag(CPU6502_FLAG_N);
				}
				
				if(this->A == 0x0){
					setFlag(CPU6502_FLAG_Z);
				}
				else{
					cleanFlag(CPU6502_FLAG_Z);
				}
				
				this->PC = ++pc;
				this->LastOpcode = 0x6d;
			}
			
			//esse opcode nao existe apenas para mim debugar 
			else if(instruction == 0xff){
				this->break_status = true;
				this->LastOpcode = 0xff;
				this->PC = ++pc;
			}
			else{
				this->break_status = true;
				this->LastOpcode = (readMemoryByte(this->PC) & 0xff);
				
			}
			
		}
};

#endif

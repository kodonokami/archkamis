#include <iostream>
#include "cpu6502.h"
#include <ctime>

using namespace std;

int main(int argc, char **argv){
	//endereço 0x2000 para printar
	char buffer_print[255];
	int buffer_pos = 0;
	
	Cpu_6502 *cpu = new Cpu_6502();
		
	cpu->readMemoryFile(argv[1]);
	
	while(1){
		cout << "PC: " << hex << cpu->getReg("PC") << endl;
		cout << "A: " << hex << cpu->getReg("A") << endl;
		cout << "X: " << hex << cpu->getReg("X") << endl;
		cout << "Y: " << hex << cpu->getReg("Y") << endl;
		cout << "S: " << hex << cpu->getReg("S") << endl;
		cout << "P: " << hex << cpu->getReg("P") << 
			" (N=" <<  ((cpu->getReg("P") & 0x80) != 0) << //0000 0100
			" V=" <<  ((cpu->getReg("P") & 0x40) != 0) << 
			" -"  << 
			" B=" <<  ((cpu->getReg("P") & 0x10) != 0) << 
			" D=" <<  ((cpu->getReg("P") & 0x08) != 0) << 
			" I=" <<  ((cpu->getReg("P") & 0x04) != 0) << 
			" Z=" <<  ((cpu->getReg("P") & 0x02) != 0) << 
			" C=" <<  ((cpu->getReg("P") & 0x01) != 0) << 
			")" << endl;
		cpu->executeInstruction();
		cout << "Last Instruction: " << hex << cpu->getLastOpcode() << endl << endl;
		
		//DMA -> position print
		//$2001 = pos, $2002 = 1
		if(cpu->readMemoryByte(0x2001) != 0){
			
			buffer_pos = (char) (cpu->readMemoryByte(0x2002));
			if(buffer_pos != 0){
				buffer_pos = (char) (cpu->readMemoryByte(0x2001));
				cpu->writeMemoryByte(0x2001,0x0);
				cpu->writeMemoryByte(0x2002,0x0);
			}
		}
		
		//DMA -> output serial
		if(cpu->readMemoryByte(0x2000) != 0){
			buffer_print[buffer_pos] = (char) cpu->readMemoryByte(0x2000);
			buffer_print[buffer_pos + 1] = 0x0;
			
			buffer_pos++;
			if(buffer_pos >= 254){
				buffer_pos = 0;
			}
			
			cpu->writeMemoryByte(0x2000,0x0);
		}

		cout << "mem $2000: [ " << buffer_print << " ]" << endl << endl;
		
		_sleep(100);
		
		if(cpu->getBreakStatus()){
			cout << "parado" << endl;
			break;
		}
	}
		
	return EXIT_SUCCESS;
}


#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 
unsigned int slice(bitset<32> number, int start, int end)
{
	unsigned int result = 0;
	for (int i = end; i >= start; i--) {
		result <<= 1;
		result |= number[i];
	}
	return result;
}

int signed_extend(unsigned int number)
{
	if (number & 0x00008000)
		return number | 0xffff0000;
	return number;
}

void reset(stateStruct &state)
{
	state.ID.nop = state.EX.nop = state.MEM.nop = state.WB.nop = true;
	state.EX.is_I_type = false; state.EX.rd_mem = false; state.EX.wrt_mem = false; state.EX.wrt_enable = false; state.EX.alu_op = true;
	state.MEM.rd_mem = false; state.MEM.wrt_mem = false;state.MEM.wrt_enable = false;
	state.WB.wrt_enable = false;
}

#define LW 0x23u
#define SW 0x2Bu
#define BEQ 0x04u
#define ADDU 0x21u
#define SUBU 0x23u

int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
	
	stateStruct state, newState;
	int cycle = 0;

	reset(state);
	reset(newState);
             
    while (1) {

        /* --------------------- WB stage --------------------- */
		if (!state.WB.nop) {
			if (state.WB.wrt_enable) {
				myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
			}
		}


        /* --------------------- MEM stage --------------------- */
		if (!state.MEM.nop) {
			/* Forward: Data Hazard Detection */
			if (!state.WB.nop && state.MEM.wrt_mem && state.MEM.Rt.to_ulong() == state.WB.Wrt_reg_addr.to_ulong()) 
				state.MEM.Store_data = state.WB.Wrt_data;
			
			if (state.MEM.rd_mem) { //lw
				newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
				newState.WB.wrt_enable = true;
			}
			else if (state.MEM.wrt_mem) { //sw
				myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
				newState.WB.Wrt_data = state.MEM.Store_data;
			}
			else if (state.MEM.wrt_enable) { // addu, subu
				newState.WB.Wrt_data = state.MEM.ALUresult;
			}
			newState.WB.wrt_enable = state.MEM.wrt_enable;
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.Rs = state.MEM.Rs;
			newState.WB.Rt = state.MEM.Rt;
			newState.WB.nop = false;
		}
		else 
			newState.WB.nop = true;
		

        /* --------------------- EX stage --------------------- */
		if (!state.EX.nop) {
			/* Forward: Data Hazard Detection */
			if (!state.MEM.nop && state.MEM.wrt_enable) { // MEM
				if (state.MEM.rd_mem) { //lw
					if (state.EX.wrt_enable && (state.MEM.Wrt_reg_addr.to_ulong() == state.EX.Rs.to_ulong()
						|| state.MEM.Wrt_reg_addr.to_ulong() == state.EX.Rt.to_ulong())) { // addu, subu
																						   //cout << "stall the pipeline!\n";
						cout << "impossible!!" << endl;
					}
					else if (state.EX.wrt_mem && state.MEM.Wrt_reg_addr.to_ulong() == state.EX.Rs.to_ulong()) { // sw
																												//cout << "stall the pipeline!\n";
						cout << "impossible!!" << endl;
					}
				}
				else { // addu, subu
					if (state.MEM.Wrt_reg_addr.to_ulong() == state.EX.Rs.to_ulong())
						state.EX.Read_data1 = state.MEM.ALUresult;
					if (!state.EX.wrt_mem && state.MEM.Wrt_reg_addr.to_ulong() == state.EX.Rt.to_ulong())
						state.EX.Read_data2 = state.MEM.ALUresult;
				}
			}
			if (!state.WB.nop && state.WB.wrt_enable) { // WB
				if (state.WB.Wrt_reg_addr.to_ulong() == state.EX.Rs.to_ulong())
					state.EX.Read_data1 = state.WB.Wrt_data;
				if (state.WB.Wrt_reg_addr.to_ulong() == state.EX.Rt.to_ulong())
					state.EX.Read_data2 = state.WB.Wrt_data;
			}
			if (newState.EX.Wrt_reg_addr.to_ulong() == 0)
				newState.MEM.ALUresult = bitset<32>(0);
			else if (state.EX.is_I_type) { // lw, sw, beq
				newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + signed_extend(state.EX.Imm.to_ulong()));
			}
			else { // addu, subu,
				unsigned int alu_result;
				if (state.EX.alu_op)
					alu_result = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
				else
					alu_result = state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong();
				newState.MEM.ALUresult = bitset<32>(alu_result);
			}
			newState.MEM.Store_data = state.EX.Read_data2;
			newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
			newState.MEM.rd_mem = state.EX.rd_mem;
			newState.MEM.wrt_mem = state.EX.wrt_mem;
			newState.MEM.wrt_enable = state.EX.wrt_enable;
			newState.MEM.Rt = state.EX.Rt;
			newState.MEM.Rs = state.EX.Rs;
			newState.MEM.nop = false;
		}
		else
			newState.MEM.nop = true;
          

        /* --------------------- ID stage --------------------- */
		bool stall = false, branch = false;
		if (!state.ID.nop) {
			/* Stall Detection */
			unsigned long rs = slice(state.ID.Instr, 21, 25), rt = slice(state.ID.Instr, 16, 20), imm = slice(state.ID.Instr, 0, 15);
			unsigned int branch_addr = state.IF.PC.to_ulong() + signed_extend(imm << 2);
			unsigned int opcode = slice(state.ID.Instr, 26, 31);
			if (!state.EX.nop && state.EX.rd_mem) {
				if (opcode == LW || opcode == SW) {
					if (rs == state.EX.Wrt_reg_addr.to_ulong()) {
						stall = true;
						//cout << "stall the pipeline" << endl;
					}
				}
				else if (opcode != BEQ) { // addu, subu
					if (rs == state.EX.Wrt_reg_addr.to_ulong() || rt == state.EX.Wrt_reg_addr.to_ulong()) {
						stall = true;
						//cout << "stall the pipeline" << endl;
					}
				}
			}
			newState.EX.Imm = bitset<16>(imm);
			newState.EX.Rs = bitset<5>(rs);
			newState.EX.Rt = bitset<5>(rt);
			newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
			newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
			switch (opcode) {
			case LW:
				newState.EX.is_I_type = true;
				newState.EX.rd_mem = true; newState.EX.wrt_mem = false;
				newState.EX.alu_op = true;
				newState.EX.wrt_enable = true;
				newState.EX.Wrt_reg_addr = newState.EX.Rt;
				//printf("lw\n");
				break;
			case SW:
				newState.EX.is_I_type = true;
				newState.EX.rd_mem = false; newState.EX.wrt_mem = true;
				newState.EX.alu_op = true;
				newState.EX.wrt_enable = false;
				newState.EX.Wrt_reg_addr = newState.EX.Rt;
				//printf("sw\n");
				break;
			case BEQ:
				newState.EX.is_I_type = false; //*
				newState.EX.rd_mem = false; newState.EX.wrt_mem = false;
				newState.EX.alu_op = true;
				newState.EX.wrt_enable = false;
				newState.EX.Wrt_reg_addr = bitset<5>(0);
				if (newState.EX.Read_data1.to_ulong() != newState.EX.Read_data2.to_ulong()) {
					newState.IF.PC = bitset<32>(branch_addr);
					branch = true;
				}
				//cout << "BEQ: " << (newState.EX.Read_data1.to_ulong() != newState.EX.Read_data2.to_ulong()) << endl;
				//cout << "PC: " << branch_addr << endl;
				//printf("beq\n");
				break;
			default: //addu, subu
				newState.EX.is_I_type = false;
				newState.EX.rd_mem = false; newState.EX.wrt_mem = false;
				newState.EX.alu_op = (slice(state.ID.Instr, 0, 5) == ADDU);
				newState.EX.wrt_enable = true;
				newState.EX.Wrt_reg_addr = bitset<5>(slice(state.ID.Instr, 11, 15)); // rd
				//printf("addu, subu\n");
				break;
			}
			newState.EX.nop = stall;
		}
		else
			newState.EX.nop = true;

        
        /* --------------------- IF stage --------------------- */
		if (!stall) {
			/* Stall Detection */
			if (!state.IF.nop && !branch) {
				bitset<32> instruction = myInsMem.readInstr(state.IF.PC);
				newState.ID.Instr = instruction;

				if (instruction.to_ulong() == 0xffffffffu) {
					newState.IF.nop = true;
					newState.ID.nop = true;
				}
				else {
					newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
					newState.IF.nop = false;
					newState.ID.nop = false;
				}
			}
			else
				newState.ID.nop = true;
		}
	

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
		cycle += 1;
    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}

#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "regs.h"
#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "dlite.h"
#include "options.h"
#include "stats.h"
#include "sim.h"
#include "decode.def"

#include "instr.h"

/* PARAMETERS OF THE TOMASULO'S ALGORITHM */

#define INSTR_QUEUE_SIZE         10

#define RESERV_INT_SIZE    4
#define RESERV_FP_SIZE     2
#define FU_INT_SIZE        2
#define FU_FP_SIZE         1

#define FU_INT_LATENCY     4
#define FU_FP_LATENCY      9

/* IDENTIFYING INSTRUCTIONS */

//unconditional branch, jump or call
#define IS_UNCOND_CTRL(op) (MD_OP_FLAGS(op) & F_CALL || \
                         MD_OP_FLAGS(op) & F_UNCOND)

//conditional branch instruction
#define IS_COND_CTRL(op) (MD_OP_FLAGS(op) & F_COND)

//floating-point computation
#define IS_FCOMP(op) (MD_OP_FLAGS(op) & F_FCOMP)

//integer computation
#define IS_ICOMP(op) (MD_OP_FLAGS(op) & F_ICOMP)

//load instruction
#define IS_LOAD(op)  (MD_OP_FLAGS(op) & F_LOAD)

//store instruction
#define IS_STORE(op) (MD_OP_FLAGS(op) & F_STORE)

//trap instruction
#define IS_TRAP(op) (MD_OP_FLAGS(op) & F_TRAP) 

#define USES_INT_FU(op) (IS_ICOMP(op) || IS_LOAD(op) || IS_STORE(op))
#define USES_FP_FU(op) (IS_FCOMP(op))

#define WRITES_CDB(op) (IS_ICOMP(op) || IS_LOAD(op) || IS_FCOMP(op))

/* FOR DEBUGGING */

//prints info about an instruction
#define PRINT_INST(out,instr,str,cycle)	\
  myfprintf(out, "%d: %s", cycle, str);		\
  md_print_insn(instr->inst, instr->pc, out); \
  myfprintf(stdout, "(%d)\n",instr->index);

#define PRINT_REG(out,reg,str,instr) \
  myfprintf(out, "reg#%d %s ", reg, str);	\
  md_print_insn(instr->inst, instr->pc, out); \
  myfprintf(stdout, "(%d)\n",instr->index);

/* VARIABLES */

//instruction queue for tomasulo
static instruction_t* instr_queue[INSTR_QUEUE_SIZE];
//number of instructions in the instruction queue
static int instr_queue_size = 0;

/*ECE552 Assignment 3 - BEGIN CODE */
static int instr_queue_head = 0;
static int instr_queue_tail = 0;
/*ECE552 Assignment 3 - END CODE */

//reservation stations (each reservation station entry contains a pointer to an instruction)
static instruction_t* reservINT[RESERV_INT_SIZE];
static instruction_t* reservFP[RESERV_FP_SIZE];

//functional units
static instruction_t* fuINT[FU_INT_SIZE];
static instruction_t* fuFP[FU_FP_SIZE];

//common data bus
static instruction_t* commonDataBus = NULL;

//The map table keeps track of which instruction produces the value for each register
static instruction_t* map_table[MD_TOTAL_REGS];

//the index of the last instruction fetched
static int fetch_index = 0;

/* RESERVATION STATIONS */

/*ECE552 Assignment 3 - BEGIN CODE */
/* 
 * Description: 
 * 	Checks if simulation is done by finishing the very last instruction
 *      Remember that simulation is done only if the entire pipeline is empty
 * Inputs:
 * 	sim_insn: the total number of instructions simulated
 * Returns:
 * 	True: if simulation is finished
 */
static bool is_simulation_done(counter_t sim_insn) {
	/*check if all the instructions are fetched and instruction queue is empty*/
	if(sim_insn-1>=fetch_index||instr_queue_size!=0)
		return false;
	else
	{
		/*check if the reservation station is empty*/
		for(int i = 0; i<RESERV_INT_SIZE; i++){
			if(reservINT[i])
				return false;
		}

		for(int i = 0; i<RESERV_FP_SIZE; i++){
			if(reservFP[i])
				return false;
		}
	}
	return true;
}

/* 
 * Description: 
 * 	Retires the instruction from writing to the Common Data Bus
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void CDB_To_retire(int current_cycle) {
	if(!commonDataBus)
		return;

	/*check all the instructions in the reservation station and update dependancies*/
	for(int i = 0; i<RESERV_INT_SIZE; i++){
		instruction_t *inst = reservINT[i];
		for(int j = 0; j<3; j++){
			if(inst&&inst->Q[j]==commonDataBus)
				inst->Q[j] =NULL;
		}
	}

	for(int i = 0; i<RESERV_FP_SIZE; i++){
		instruction_t *inst = reservFP[i];
		for(int j = 0; j<3; j++){
			if(inst&&inst->Q[j]==commonDataBus)
				inst->Q[j] =NULL;
		}
	}

	/*check and update the Map Table */
	for(int i=0; i<2; i++){
		if(commonDataBus->r_out[i]>0){
 			/*clean if the instruction in the entry matches the common data bus*/
			if(map_table[commonDataBus->r_out[i]]==commonDataBus)
				map_table[commonDataBus->r_out[i]] = NULL;
		}	
	}
	commonDataBus = NULL;
}


/* 
 * Description: 
 * 	Moves an instruction from the execution stage to common data bus (if possible)
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void execute_To_CDB(int current_cycle) {
  	if(commonDataBus) return;

	/*store condition, clear reservation station and function unit*/	
	for(int i = 0; i<FU_INT_SIZE; i++){
		if(fuINT[i]){
			instruction_t *inst = fuINT[i];
			if(inst&&!inst->tom_cdb_cycle&&(inst->tom_execute_cycle + FU_INT_LATENCY <= current_cycle)&&IS_STORE(inst->op)){
				inst->tom_cdb_cycle = current_cycle;	
				for(int k = 0; k < RESERV_INT_SIZE; k++){
					if(fuINT[i]==reservINT[k]){
						fuINT[i] = NULL;
						reservINT[k] = NULL;		
					}							
				}

				for(int k = 0; k < RESERV_FP_SIZE; k++){
					if(fuINT[i]==reservFP[k]){
						fuINT[i] = NULL;
						reservFP[k] = NULL;		
					}							
				}
			}
		}
	}

	/*check all the other instructions in functional units, we get the oldest finished instruction*/
	instruction_t *oldest_inst =NULL;
	for(int i = 0; i<FU_INT_SIZE; i++){
		if(fuINT[i]){
			instruction_t *inst = fuINT[i];
			if(inst&&!inst->tom_cdb_cycle&&(inst->tom_execute_cycle + FU_INT_LATENCY <= current_cycle)){
				if(!oldest_inst)
					oldest_inst = inst;					
				else	
					oldest_inst = oldest_inst->index > inst->index?inst:oldest_inst;
			}
		}
	}	
	
	for(int i = 0; i<FU_FP_SIZE; i++){
		if(fuFP[i]){
			instruction_t *inst = fuFP[i];
			if(inst&&!inst->tom_cdb_cycle&&(inst->tom_execute_cycle + FU_FP_LATENCY <= current_cycle)){
				if(!oldest_inst)
					oldest_inst = inst;					
				else	
					oldest_inst = oldest_inst->index > inst->index?inst:oldest_inst;
			}
		}
	}

	/*put oldest instruction on common data bus*/
	if(oldest_inst){
		oldest_inst->tom_cdb_cycle = current_cycle;		
		commonDataBus = oldest_inst;

	}

	/*remove the instruction from reservation station*/
	for(int i = 0; i < RESERV_INT_SIZE; i++){
		if(reservINT[i]&&reservINT[i] == oldest_inst){
			//print_tom_instr(oldest_inst);	
			reservINT[i] = NULL;	}						
	}

	for(int i = 0; i < RESERV_FP_SIZE; i++){
		if(reservFP[i]&&reservFP[i]== oldest_inst){
			//print_tom_instr(oldest_inst);	
			reservFP[i] = NULL;}							
	}

	/*remove the instruction from functional unit*/
	for(int i = 0; i<FU_INT_SIZE; i++){
		if(fuINT[i]&&fuINT[i]== oldest_inst)
			fuINT[i] = NULL;	
	}	
	
	for(int i = 0; i<FU_FP_SIZE; i++){
		if(fuFP[i]&&fuFP[i]== oldest_inst)
			fuFP[i] = NULL;	
	}
	
}

/* 
 * Description: 
 * 	Moves instruction(s) from the issue to the execute stage (if possible). We prioritize old instructions
 *      (in program order) over new ones, if they both contend for the same functional unit.
 *      All RAW dependences need to have been resolved with stalls before an instruction enters execute.
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void issue_To_execute(int current_cycle) {
	instruction_t *oldest_int = NULL;
	instruction_t *oldest_fp = NULL;
	/*For int functional unit, we find instruction in corresponding reservation station that is ready to execute*/
	for(int i = 0; i<FU_INT_SIZE; i++){
		if(!fuINT[i]){

			/*get the oldest ready instruction*/
			for(int j = 0; j<RESERV_INT_SIZE; j++){
				instruction_t *inst = reservINT[j];
				if(inst&&inst->tom_issue_cycle&&!inst->tom_execute_cycle&&!inst->Q[0]&&!inst->Q[1]&&!inst->Q[2]){
					if(!oldest_int)
						oldest_int = inst;					
					else	
						oldest_int = oldest_int->index > inst->index?inst:oldest_int;
				}			
			}
	
			fuINT[i] = oldest_int;

			/*start to execute*/
			if(oldest_int!=NULL){
				oldest_int->tom_execute_cycle = current_cycle;
				oldest_int = NULL;
			
			}
		}	
	}

	/*For fp functional unit, we find instruction in corresponding reservation station that is ready to execute*/
	for(int i = 0; i<FU_FP_SIZE; i++){
		if(!fuFP[i]){

			/*get the oldest ready instruction*/
			for(int j = 0; j<RESERV_FP_SIZE; j++){
				instruction_t *inst = reservFP[j];
				if(inst&&inst->tom_issue_cycle&&!inst->tom_execute_cycle&&!inst->Q[0]&&!inst->Q[1]&&!inst->Q[2]){
					if(!oldest_fp)
						oldest_fp = inst;					
					else	
						oldest_fp = oldest_fp->index > inst->index?inst:oldest_fp;
				}			
			}
			fuFP[i] = oldest_fp;

			/*start to execute*/
			if(oldest_fp){
				oldest_fp->tom_execute_cycle = current_cycle;
				oldest_fp = NULL;
			}
		}	
	}
	
}

/* 
 * Description: 
 * 	Moves instruction(s) from the dispatch stage to the issue stage
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void dispatch_To_issue(int current_cycle) {

 	/*peek the instruction in head of instruction queue */
	instruction_t *inst = instr_queue[instr_queue_head];
	if(inst==NULL)
		return;

 	/* put instr into reservation station*/	
	/*If the instruction is conditional or unconditional branch, we simply remove it from instruction queue without putting into reservation station.*/
	if(IS_COND_CTRL(inst->op)||IS_UNCOND_CTRL(inst->op)){
		instr_queue[instr_queue_head] = NULL;
		instr_queue_head = (instr_queue_head + 1)%10;
		instr_queue_size--;
		return;
	}

	if(IS_ICOMP(inst->op)||IS_LOAD(inst->op)||IS_STORE(inst->op)){
		for(int i=0; i<RESERV_INT_SIZE; i++){
			/*find the empty reservation station*/			
			if(reservINT[i]==NULL){
				/*change the issue cycle*/
				inst->tom_issue_cycle = current_cycle;
				/*mark the RAW dependency*/
				for(int j=0;j<3;j++){
					if(inst->r_in[j]>0&&map_table[inst->r_in[j]])
						inst->Q[j] = map_table[inst->r_in[j]];				
				}

				/*update the Map Table*/
				for(int j=0; j<2; j++){
					if(inst->r_out[j]>0){
						map_table[inst->r_out[j]]=inst;		
					}	
				}
				/*pop the intr out of the queue*/
				reservINT[i] = inst;	
				instr_queue[instr_queue_head] = NULL;
				instr_queue_head = (instr_queue_head + 1)%10;
				instr_queue_size--;
				break;
			}
		}	
	}
	if(IS_FCOMP(inst->op)){
		for(int i=0; i<RESERV_FP_SIZE; i++){
			/*find the empty reservation station*/				
			if(reservFP[i]==NULL){

				/*change the issue cycle*/
				inst->tom_issue_cycle = current_cycle;
				/*mark the RAW dependency*/
				for(int j=0;j<3;j++){
					if(inst->r_in[j]>0&&!map_table[inst->r_in[j]])
						inst->Q[j] = map_table[inst->r_in[j]];				
				}

				/*update the Map Table*/
				for(int j=0; j<2; j++){
					if(inst->r_out[j]>0){
						map_table[inst->r_out[j]]=inst;		
					}	
				}
				/*pop the intr out of the queue*/
				reservFP[i] = inst;
				instr_queue[instr_queue_head] = NULL;
				instr_queue_head = (instr_queue_head + 1)%10;
				instr_queue_size--;
				break;
			}
		}	
	}	
}

/* 
 * Description: 
 * 	Grabs an instruction from the instruction trace (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	None
 */
void fetch(instruction_trace_t* trace) {

}

/* 
 * Description: 
 * 	Calls fetch and dispatches an instruction at the same cycle (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void fetch_To_dispatch(instruction_trace_t* trace, int current_cycle) {
	if(sim_num_insn <= fetch_index||instr_queue_size>9)
		return;
	fetch_index++;
	instruction_t *inst = get_instr(trace, fetch_index);

	/*If the instruction fetched is a trap, ignore it and fetch the next instruction until find a non-trap one*/
	while(!inst||IS_TRAP(inst->op)){
		fetch_index++;
		inst = get_instr(trace, fetch_index);
	}

	/*put the instruction into tail of instruction queue*/
	if(inst&&instr_queue_size<10){
		instr_queue[instr_queue_tail] = inst;
		inst->tom_dispatch_cycle = current_cycle;
		instr_queue_size++;
		instr_queue_tail = (instr_queue_tail + 1)%10;
	} 
}
/*ECE552 Assignment 3 - END CODE */

/* 
 * Description: 
 * 	Performs a cycle-by-cycle simulation of the 4-stage pipeline
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	The total number of cycles it takes to execute the instructions.
 * Extra Notes:
 * 	sim_num_insn: the number of instructions in the trace
 */
counter_t runTomasulo(instruction_trace_t* trace)
{
  //initialize instruction queue
  int i;
  for (i = 0; i < INSTR_QUEUE_SIZE; i++) {
    instr_queue[i] = NULL;
  }

  //initialize reservation stations
  for (i = 0; i < RESERV_INT_SIZE; i++) {
      reservINT[i] = NULL;
  }

  for(i = 0; i < RESERV_FP_SIZE; i++) {
      reservFP[i] = NULL;
  }

  //initialize functional units
  for (i = 0; i < FU_INT_SIZE; i++) {
    fuINT[i] = NULL;
  }

  for (i = 0; i < FU_FP_SIZE; i++) {
    fuFP[i] = NULL;
  }

  //initialize map_table to no producers
  int reg;
  for (reg = 0; reg < MD_TOTAL_REGS; reg++) {
    map_table[reg] = NULL;
  }
  
  int cycle = 1;
  while (true) {
/*ECE552 Assignment 3 - BEGIN CODE */
	CDB_To_retire(cycle);
	execute_To_CDB(cycle);
	issue_To_execute(cycle);
	dispatch_To_issue(cycle);
	fetch_To_dispatch(trace,cycle);
/*ECE552 Assignment 3 - END CODE */
    cycle++; 
	if (is_simulation_done(sim_num_insn))
        break;
  }
  
  return cycle;
}

#include "predictor.h"

/////////////////////////////////////////////////////////////
// 2bitsat
/////////////////////////////////////////////////////////////


int pred_table[4096];

void InitPredictor_2bitsat() {
	for(int i =0; i<4096; i++)
		pred_table[i] = 1;
}

bool GetPrediction_2bitsat(UINT32 PC) {
	PC = PC&0xfff;
	if(pred_table[PC] == 0||pred_table[PC] == 1)
		return NOT_TAKEN;
	else
		return TAKEN;		
}

void UpdatePredictor_2bitsat(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
	PC = PC&0xfff;
	if(resolveDir == TAKEN){
		if(pred_table[PC]!=3)
			pred_table[PC]++;
	}
	else{
		if(pred_table[PC]!=0)
			pred_table[PC]--;
	}
}

/////////////////////////////////////////////////////////////
// 2level
/////////////////////////////////////////////////////////////
int bht[512];
int pht[8][64];
void InitPredictor_2level() {
	for(int i = 0; i<512; i++)
		bht[i] = 0;
	for(int i = 0; i<8; i++){
		for(int j = 0; j<64; j++)
			pht[i][j] = 1;
	}
}

bool GetPrediction_2level(UINT32 PC) {
	int low_idx = PC&0x7;
	int mid_idx = (PC&0xff8)>>3;
	int pattern = bht[mid_idx];
	if(pht[low_idx][pattern] == 0||pht[low_idx][pattern] == 1)
		return NOT_TAKEN;
	else
		return TAKEN;	
}

void UpdatePredictor_2level(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
	int low_idx = PC&0x7;
	int mid_idx = (PC&0xff8)>>3;
	int pattern = bht[mid_idx];
	if(resolveDir == TAKEN){
		if(pht[low_idx][pattern]!=3)
			pht[low_idx][pattern] ++;
	}
	else{
		if(pht[low_idx][pattern] !=0)
			pht[low_idx][pattern] --;
	}
	bht[mid_idx] = ((bht[mid_idx])&0x1f)<<1;

	if(resolveDir == TAKEN)
		bht[mid_idx]++;
}

/////////////////////////////////////////////////////////////
// dynamic branch prediction with perceptrons
/////////////////////////////////////////////////////////////

int pht_percep[61];
int weight[256][62];	//make sure use 8 bit space
int output = 0;

void InitPredictor_openend() {
	for(int i=0; i<61; i++)
		pht_percep[i] = -1;
	for(int i=0; i<62; i++){
		for(int j=0; j<256; j++)
			weight[i][j] = 1;	
	}
}

bool GetPrediction_openend(UINT32 PC) {
	int index = PC&0xff; 
	int weight_sum = 0;	
	for(int i = 0; i<61; i++){
		weight_sum += weight[index][i]*pht_percep[i];
	}
	int bias = weight[index][61];
	output = weight_sum + bias;
	if(output>=0)
		return TAKEN;
	else
		return NOT_TAKEN;
}

void UpdatePredictor_openend(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
	int index = PC&0xff;
	int threshold = 50;
	if(predDir!=resolveDir||abs(output)<threshold){
		if(resolveDir == TAKEN){
			if(weight[index][61]<127)		//make sure to use 8 bit space
				weight[index][61]++;
		}
		else{
			if(weight[index][61]>-128)		//make sure to use 8 bit space
				weight[index][61]--;
		}
		 
		for(int i = 0; i<61; i++){
			if(((resolveDir == TAKEN)&&(pht_percep[i]==1))||((resolveDir == NOT_TAKEN)&&(pht_percep[i]==-1))){
				if(weight[index][i]<128)	//make sure to use 8 bit space
					weight[index][i]++;
			}
			else{
				if(weight[index][i]>-127)	//make sure to use 8 bit space
					weight[index][i]--;
			}
		}
	}
	
	for(int i = 0; i<61; i++)
		pht_percep[i] = pht_percep[i+1];
	pht_percep[61] = (resolveDir == TAKEN)?1:-1;


}



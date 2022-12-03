//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// For gshare
uint8_t* gshareGlobalHistoryTable;
uint32_t gshareGolbalHistory;

// For tournament
uint8_t* chooser;

uint8_t* tGlobalHistoryTable;
uint32_t tGlobalHistory;

uint32_t* tLocalRegisterTable;
uint8_t* tLocalHistoryTable;

//------------------------------------//
//      Predictor Util Functions      //
//------------------------------------//

int get_gshare_index(uint32_t pc) {
  int index = (pc ^ gshareGolbalHistory) & ((1 << ghistoryBits) - 1);
  return index;
}

int get_t_global_index() {
  int globalIndex = (tGlobalHistory & ((1 << ghistoryBits) - 1));
  return globalIndex;
}

int get_t_local_index(uint32_t pc) {
  int pcIndex = pc & ((1 << pcIndexBits) - 1);
  int tableIndex = tLocalRegisterTable[pcIndex] & ((1 << lhistoryBits) - 1);
  return tableIndex;
}

uint8_t pred_incre(uint8_t pred) {
  if (pred != ST) {
    pred += 1;
  }
  return pred;
}

uint8_t pred_decre(uint8_t pred) {
  if (pred != SN) {
    pred -= 1;
  }
  return pred;
}

uint8_t pred_to_taken(uint8_t pred) {
  if (pred == ST || pred == WT) {
    return TAKEN;
  } else if (pred == SN || pred == WN) {
    return NOTTAKEN;
  }

  printf("Error: Invalid pred %d\n", pred);
  return NOTTAKEN;
}

uint8_t outcome_to_pred(uint8_t pred, uint8_t outcome) {
  if (outcome == TAKEN) {
    return pred_incre(pred);
  } else if (outcome == NOTTAKEN) {
    return pred_decre(pred);
  } else {
    printf("Error: Invalid outcome %d\n", outcome);
    return pred;
  }
}

void init_global_helper(uint32_t* globalHistory, uint8_t** globalHistoryTable) {
  *globalHistory = 0;

  int size = (1 << ghistoryBits) * sizeof(uint8_t);
  *globalHistoryTable = (uint8_t*)malloc(size);
  memset(*globalHistoryTable, WN, size);

  return;
}

void train_table(int index, uint8_t* table, uint32_t* history, uint8_t outcome) {
  uint8_t old_pred = table[index];
  table[index] = outcome_to_pred(old_pred, outcome);

  *history = ((*history) << 1) | outcome;
  *history &= (1 << ghistoryBits) - 1;

  return;
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

void init_gshare() {
  init_global_helper(&gshareGolbalHistory, &gshareGlobalHistoryTable);
}

void init_tournament() {
  init_global_helper(&tGlobalHistory, &tGlobalHistoryTable);

  int size = (1 << ghistoryBits) * sizeof(uint8_t);
  chooser = (uint8_t*)malloc(size);
  memset(chooser, WN, size);

  size = (1 << pcIndexBits) * sizeof(uint32_t);
  tLocalRegisterTable = (uint32_t*)malloc(size);
  memset(tLocalRegisterTable, 0, size);

  size = (1 << lhistoryBits) * sizeof(uint8_t);
  tLocalHistoryTable = (uint8_t*)malloc(size);
  memset(tLocalHistoryTable, WN, size);
}

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  if (bpType == STATIC) {
    // nothing
  } else if (bpType == GSHARE) {
    init_gshare();
  } else if (bpType == TOURNAMENT) {
    init_tournament();
  } else {
    printf("Error: Invalid bpType: %d \n", bpType);
  }

  return;
}

uint8_t gshare_make_prediction(uint32_t pc) {
  int index = get_gshare_index(pc);
  uint8_t pred = gshareGlobalHistoryTable[index];
  return pred_to_taken(pred);
}

uint8_t tournament_make_prediction(uint32_t pc) {
  int globalIndex = get_t_global_index();
  uint8_t choice = chooser[globalIndex];

  if (choice == SN || choice == WN) {
    // global
    uint8_t pred = tGlobalHistoryTable[globalIndex];
    return pred_to_taken(pred);

  } else if (choice == ST || choice == WT) {
    // local
    int index = get_t_local_index(pc);
    uint8_t pred = tLocalHistoryTable[index];
    return pred_to_taken(pred);

  } else {
    printf("Error: Invalid choice: %d \n", choice);
    return 0;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_make_prediction(pc);
    case TOURNAMENT:
      return tournament_make_prediction(pc);
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

void gshare_train_predictor(uint32_t pc, uint8_t outcome) {
  int index = get_gshare_index(pc);
  // uint8_t old_pred = gshareGlobalHistoryTable[index];
  // gshareGlobalHistoryTable[index] = outcome_to_pred(old_pred, outcome);

  // gshareGolbalHistory = (gshareGolbalHistory << 1) | outcome;
  // gshareGolbalHistory &= (1 << ghistoryBits) - 1;

  train_table(index, gshareGlobalHistoryTable, &gshareGolbalHistory, outcome);

  return;
}

void tournament_train_predictor(uint32_t pc, uint8_t outcome) {
  int globalIndex = get_t_global_index();
  int localIndex = get_t_local_index(pc);

  // chooser
  uint8_t localPred = 0;
  uint8_t globalOutcome = pred_to_taken(tGlobalHistoryTable[globalIndex]);
  uint8_t localOutcome = pred_to_taken(tLocalHistoryTable[localIndex]);

  if (globalOutcome != localOutcome) {
    if (globalOutcome == outcome) {
      // move to global
      chooser[globalIndex] = pred_decre(chooser[globalIndex]);
    } else {
      // move to local
      chooser[globalIndex] = pred_incre(chooser[globalIndex]);
    }
  }

  // global
  train_table(globalIndex, tGlobalHistoryTable, &tGlobalHistory, outcome);

  // local
  int registerIndex = pc & ((1 << pcIndexBits) - 1);
  train_table(localIndex, tLocalHistoryTable, &tLocalRegisterTable[registerIndex], outcome);

  return;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  if (bpType == STATIC) {
    // nothing
  } else if (bpType == GSHARE) {
    gshare_train_predictor(pc, outcome);
  } else if (bpType == TOURNAMENT) {
    tournament_train_predictor(pc, outcome);
  } else {
    printf("Error: Invalid bpType: %d \n", bpType);
  }

  return;
}

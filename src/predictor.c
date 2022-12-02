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

uint8_t* gshareGlobalHistoryTable;
uint32_t gshareGolbalHistory;


//------------------------------------//
//      Predictor Util Functions      //
//------------------------------------//

int get_gshare_index(uint32_t pc) {
  int index = (pc ^ gshareGolbalHistory) & ((1 << ghistoryBits) - 1);
  return index;
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

uint8_t pred_taken(uint8_t pred) {
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

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

void init_gshare() {
  if (verbose) {
    printf("Init gshare\n");
  }

  gshareGolbalHistory = 0;

  int size = (1 << ghistoryBits) * sizeof(uint8_t);
  gshareGlobalHistoryTable = malloc(size);
  memset(gshareGlobalHistoryTable, WN, size);

  return;
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
  } else {
    printf("Error: Invalid bpType: %d \n", bpType);
  }

  return;
}

uint8_t gshare_make_prediction(uint32_t pc) {
  int index = get_gshare_index(pc);
  uint8_t pred = gshareGlobalHistoryTable[index];
  return pred_taken(pred);
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
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

void gshare_train_predictor(uint32_t pc, uint8_t outcome) {
  int index = get_gshare_index(pc);
  uint8_t old_pred = gshareGlobalHistoryTable[index];
  gshareGlobalHistoryTable[index] = outcome_to_pred(old_pred, outcome);

  gshareGolbalHistory = (gshareGolbalHistory << 1) | outcome;

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
  } else {
    printf("Error: Invalid bpType: %d \n", bpType);
  }

  return;
}

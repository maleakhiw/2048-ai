/*
 ============================================================================
 Name        : ai.h
 Author      : Maleakhi Agung Wijaya <maleakhiw>
 StudentID   : 784091
 Description : AI Program to solve 2048 game
 ============================================================================
 */

#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"

/*****************************CONSTANT***************************************/

#define NUMBER_OF_MOVES 4 // number of possible moves
#define INITIAL 0
#define NEXT_LEVEL 1
#define REALLOC_FACTOR 2
#define DECISION_DEPTH 1 // Depth for which we store all of the updated value
#define CHILD_INSERTION 1 

/****************************FUNCTION-DECLARATION****************************/

/** Main AI function */
void initialize_ai();
void freeAI();
move_t 
get_next_move( uint8_t board[SIZE][SIZE], int max_depth, 
	propagation_t propagation, int *numberOfGeneratedNode,
	int *numberOfExpandedNode );

/** Array used for nodes memory purposes */
node_t **createExplored(int size);
node_t **insertIntoExplored(node_t **explored, node_t *node, int *size, 
	int *countExplored);
void freeExplored(node_t **explore, int nexplore);
node_t *createNewNode(uint32_t score, int depth, int num_childs, move_t move, 
	uint8_t board[SIZE][SIZE], node_t *parent);

/** Board function */
bool isEqualBoard(uint8_t board1[SIZE][SIZE], uint8_t board2[SIZE][SIZE]);
void copyBoard(uint8_t duplicate[SIZE][SIZE], uint8_t original[SIZE][SIZE]);
uint8_t maximumTile(uint8_t board[SIZE][SIZE]);

/** Helper function used for main AI function */
void propagateScore(node_t* node, propagation_t propagation);
uint32_t getMaximum(node_t *array[], int n);
int getBestRandomIndex(node_t *decisionMove[], int n, uint32_t max);
void generatePossibility(int *indexDecide, node_t *decisionMove[], 
	node_t *current, propagation_t propagation, int *numberOfGeneratedNode);
move_t bestAction(node_t *decisionMove[], int indexDecide);

#endif

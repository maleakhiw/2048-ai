/*
 ============================================================================
 Name        : ai.c
 Author      : Maleakhi Agung Wijaya <maleakhiw>
 StudentID   : 784091
 Description : AI Program to solve 2048 game
 ============================================================================
 */

#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

/** Frontier heap */
struct heap h;

/***************************MAIN*AI*FUNCTION*********************************/

/** Initialize the ai by initialising the heap */
void initialize_ai(){
	heap_init(&h);
}

/** Free the heap that are used by AI */
void
freeAI() {
	free(h.heaparr);
}

/** Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg 
 * @param board The condition of the current board
 * @param max_depth Maximum depth that will be expanded
 * @param propagation Type of propagation, max or avg
 * @param numberOfGeneratedNode Calculate total number of generated node
 * @param numberOfExpandedNode Calculate total number of expanded node
 * @return best_action Move that lead to highest score
 */
move_t 
get_next_move( uint8_t board[SIZE][SIZE], int max_depth, 
	propagation_t propagation, int *numberOfGeneratedNode,
	int *numberOfExpandedNode ){
	move_t best_action;

	// Handle if it is not given any depth at all, move randomly
	if (max_depth == INITIAL) {
		return (rand() % NUMBER_OF_MOVES);
	}

	// Initial size of explored, sum of 4^0 + 4 ^ 1 +... (closed formula used)
	int maximumNode = (int) ((SIZE * ((pow(SIZE, max_depth) - NEXT_LEVEL) / 
		(SIZE - NEXT_LEVEL))) + NEXT_LEVEL);

	// Used to store depth 1 possible move
	node_t *decisionMove[NUMBER_OF_MOVES];
	int indexDecide = 0;

	// Create the start node, the move for this node can be anything
	node_t *start = createNewNode(INITIAL, INITIAL, INITIAL, left, board, NULL); 

	// Explored array used to free every possible nodes
	node_t **explored = createExplored(maximumNode);
	int countExplored = 0; // number of element on explored

	// Push first element to the heap, heap only contain first element
	heap_push(&h, start);

	/* Iterate until heap is empty, when we finish all possibility  
	 * or when it is game over
	 */
	while(h.count != 0) {

		// Pop the heap and process to check possible decision out of current
		node_t *current = heap_delete(&h);

		// Change value of number of expanded nodes
		(*numberOfExpandedNode) ++;

		// Insert into explored
		explored = insertIntoExplored(explored, current, &maximumNode, 
			&countExplored);

		// Generate possibility graph until specified depth
		if (current -> depth < max_depth) {
			/* Generate every possibility of current node and store valid 
			 * move in heap
			 */
			generatePossibility(&indexDecide, decisionMove, current, 
				propagation, numberOfGeneratedNode);
		}
	}

	// Decide best action best on greatest score, if tie select randomly
	best_action = bestAction(decisionMove, indexDecide);

	// Free all memory on the explored
	freeExplored(explored, countExplored);
	
	return best_action;
}

/***************************EXPLORED*FUNCTION*********************************/

/** Create array for saving explored node
 * @param size Size of the new explored array
 * @return explored Address of the array that are created
 */
node_t
**createExplored(int size) {
	node_t **explored = (node_t **) malloc(sizeof(node_t *) * size);
	assert(explored != NULL);

	return explored;
}

/** Insert node into explored
 * @param explored Expored array to be inserted new node
 * @param node Node that will be inserted
 * @param size Size of explored, pass as pointer as realloc might happen
 * @param countExplored Variable that held n - 1 index of latest node
 */
node_t 
**insertIntoExplored(node_t **explored, node_t *node, int *size, 
	int *countExplored) {
	// Realloc if neccessary
	if (*countExplored == *size) {
		explored = realloc(explored, sizeof(node_t *) * (*size) * 
			REALLOC_FACTOR);
		assert(explored != NULL);
		*size = (*size) * REALLOC_FACTOR; 
	}

	// Insert into explored
	explored[(*countExplored) ++] = node;
	return explored;
}

/** Free array explored 
 * @param explored Array that used to coung explored node
 * @param nexplore Size of explored
 */
void
freeExplored(node_t **explored, int nexplore) {
	int i;

	// Free every nodes on explore
	for (i = 0; i < nexplore; i ++) {
		free(explored[i]);
	}

	// Free the explored itself
	free(explored);
}

/** Create new node and store information
 * @param score Score for that particular state
 * @param depth Depth for that particular state
 * @param num_childs Always 0 as at first does not have child
 * @param move Movement that the state has
 * @param board Current state of board
 * @param parent Node that hold previous state before this move
 * @return node Node which are successfully created
 */
node_t 
*createNewNode(uint32_t score, int depth, int num_childs, move_t move, 
	uint8_t board[SIZE][SIZE], node_t *parent)  {
	// Allocate memory for new node
	node_t *new = (node_t *) malloc(sizeof(node_t));
	assert(new != NULL);

	// Assign information to nodes
	new -> priority = score;
	new -> depth = depth;
	new -> num_childs = num_childs;
	new -> move = move;
	new -> parent = parent;

	// Copy original state of board to the new node before moving
	copyBoard( new -> board, board);
	
	return new;
}

/**************************BOARD*FUNCTION************************************/

/** Compare between board, return true if board is the same, false
 *  otherwise
 * @param board1 Board to be compared
 * @param board2 Board to be compared
 * @return bool True if it is equal, false otherwise
 */
bool
isEqualBoard(uint8_t board1[SIZE][SIZE], uint8_t board2[SIZE][SIZE]) {
	int i, j;

	// Check equality of every elements
	for (i = 0; i < SIZE; i ++) {
		for (j = 0; j < SIZE; j++) {
			if (board1[i][j] != board2[i][j]) {
				return false;
			}
		}
	}

	return true;
}

/** Copy original to duplicate 
 * @param duplicate Destination copy board
 * @param original Source copy board
 */
void
copyBoard(uint8_t duplicate[SIZE][SIZE], uint8_t original[SIZE][SIZE]) {
	int i, j;

	// Copy every elements
	for (i = 0; i < SIZE; i ++) {
		for (j = 0; j < SIZE; j++) {
			duplicate[i][j] = original[i][j];
		}
	}
}

/** Check maximum tiles will be used for output to text file
 * @param board Board that we want to find the maximum tiles value
 * @ return max Maximum tile values on the board
 */
uint8_t
maximumTile(uint8_t board[SIZE][SIZE]) {
	uint8_t max = 0;
	int i, j;

	// Copy every elements
	for (i = 0; i < SIZE; i ++) {
		for (j = 0; j < SIZE; j++) {
			if (board[i][j] > max) {
				max = board[i][j];
			}
		}
	}

	return max;
}

/*******************************HELPER*FUNCTION******************************/

/** Decide the best move depending on final data of expansion 
 * @param decisionMove Array containing depth 1
 * @param indexDecide Index keep track of node on first depth
 * @return move_t Optimal movement based on high score
 */
move_t
bestAction(node_t *decisionMove[], int indexDecide) {
	// Variable keep track the maximum score, choosing between tie
	uint32_t max;
	int bestIndex;

	// Get the maximum value for the decision move
	max = getMaximum(decisionMove, indexDecide);

	// Decide the maximum randomly if it is tie (random index)
	bestIndex = getBestRandomIndex(decisionMove, indexDecide, max);

	return decisionMove[bestIndex] -> move;
}

/** Generate possible expansion for a current condition and store
 * possible movement to the heap
 * @param indexDecide Index that keep track for first depth node
 * @param decisionMove Array that save first depth movement
 * @param current Current node that are directly pop from heap
 * @param propagation Type of propagation
 * @param numberOfGeneratedNode number of nodes after applying action
 */
void
generatePossibility(int *indexDecide, node_t *decisionMove[], node_t *current,
	propagation_t propagation, int *numberOfGeneratedNode) {
	// Possible movement decision choice
	move_t moves[NUMBER_OF_MOVES] = {left, right, up, down};
	int i;

	uint8_t newBoard[SIZE][SIZE]; // newly generated board for copying

	bool success; // used to store validity of movement

	/* Set the score to previous state of board, since node current
	 * with depth 1 might change as when invoking propagation
	 */
	uint32_t parentScore = current -> priority;
	uint32_t score = 0; // keep track of the score for each movement
	
	// Generate every possible decision from current node (only 1 movement)
	for (i = 0; i < NUMBER_OF_MOVES; i ++) {
		// Set the board and score for current condition before move
		score = parentScore;
		copyBoard(newBoard, current -> board);

		// Update the board condition for this movement
		success = execute_move_t(newBoard, &score, moves[i]);

		// Add new random on board if movement is valid
		if (success) {
			addRandom(newBoard);
		}

		/* Change value of number of generated nodes after applying action,
		 * but creation of new nodes is after validity checking for
		 * the purpose of copying node data just once
		 */
		(*numberOfGeneratedNode) ++;

		/* Process the new board if the new nodes movement
		 * is valid (not death or stucked)
		 */
		if (!isEqualBoard(newBoard, current -> board)) {
			/* Create new node with certain movement,
 			 * here we give bonus depending on the number of empty
 			 * space on the board as less element in board
 			 * gives better positioning
 			 */
			node_t *new = createNewNode(score + countEmpty(newBoard),
			 current -> depth + NEXT_LEVEL, INITIAL, moves[i], 
			 newBoard, current);

			/* Append new nodes to array with depth 1 
			 * as it will be used in decision
			 */
			if (new -> depth == DECISION_DEPTH) {
				decisionMove[(*indexDecide) ++] = new;
			}

			// Insert to heap for later process (possible move from new)
			heap_push(&h, new);

			// The parent then have this as child
			current -> num_childs ++;

			// Propagate back score to first action
			propagateScore(new, propagation);
		}
	}
}

/** Get the maximum value of an array 
 * @param array Array to find the maximum
 * @param n Size of the array
 * @return max Maximum balue of the array
 */
uint32_t
getMaximum(node_t *array[], int n) {
	uint32_t max = 0;
	int i;

	// Check for the maximum value
	for (i = 0; i < n; i ++) {
		if (array[i] -> priority > max) {
			max = array[i] -> priority;
		}
	}

	return max;
}

/** Get best action index randomly
 * @param decisionMove First depth array
 * @param n Size of decisionMove
 * @param max Maximum value on the array
 * @return int Random index 
 */
int
getBestRandomIndex(node_t *decisionMove[], int n, uint32_t max) {
	int randomIndex[NUMBER_OF_MOVES];
	int nIndex = 0;
	int i;

	// Initialize seed
	srand(time(NULL));

	// Check for the index of tie element
	for (i = 0; i < n; i ++) {
		// Store every index of that tie maximum
		if (decisionMove[i] -> priority == max) {
			randomIndex[nIndex ++] = i;
		}
	}

	// Get the random index
	return randomIndex[rand() % nIndex]; 
}

/** Used to propagate score back to the first depth node 
 * @param node Current child node
 * @param propagation Type of propagation
 */
void 
propagateScore(node_t* node, propagation_t propagation) {
	// Nothing to propagate as we are at first depth
	if (node -> depth == DECISION_DEPTH) {
		return;
	}

	node_t *parent = node; // store the first depth

	// Get the depth 1 parent for particular node
	while (parent -> depth != DECISION_DEPTH) {
		parent = parent -> parent;
	}

	/* Increment the number of child for move on the first depth, if it is not
	 * have been incremented, used for average of all child
	 */
	if (node -> depth != (DECISION_DEPTH + NEXT_LEVEL)) {
		parent -> num_childs ++;
	}

	// Step for maximum propagation
	if (propagation == max) {
		/* Now we have both the new node and the decision parent node,
		 * thus change the priority of parent if the child is greater 
		 */
		if (node -> priority > parent -> priority) {
			parent -> priority = node -> priority;
		}
	}

	// Step for average propagation  
	else {
		// First insertion of average, just change that first depth priority
		if (parent -> num_childs == CHILD_INSERTION) {
			parent -> priority = node -> priority;
		}
		// Get new average
		else {
			parent -> priority = (parent -> priority * 
				(parent -> num_childs - CHILD_INSERTION) + node -> priority) / 
			(parent -> num_childs);
		}
	}

}



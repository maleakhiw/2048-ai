/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#include "node.h"
#include "ai.h"
#include "utils.h"

/**
 * Setting up terminal to draw the game board
 */
void setBufferedInput(bool enable) {
	static bool enabled = true;
	static struct termios old;
	struct termios new;

	if (enable && !enabled) {
		// restore the former settings
		tcsetattr(STDIN_FILENO,TCSANOW,&old);
		// set the new state
		enabled = true;
	} else if (!enable && enabled) {
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO,&new);
		// we want to keep the old setting to restore them at the end
		old = new;
		// disable canonical mode (buffered i/o) and local echo
		new.c_lflag &=(~ICANON & ~ECHO);
		// set the new settings immediately
		tcsetattr(STDIN_FILENO,TCSANOW,&new);
		// set the new state
		enabled = false;
	}
}


void signal_callback_handler(int signum) {
	printf("         TERMINATED         \n");
	setBufferedInput(true);
	printf("\033[?25h\033[m");
	exit(signum);
}

/**
 * Valid keys for playing
 */

bool execute_keyboard( uint8_t board[SIZE][SIZE], uint32_t* score, char c){
	bool success = false;

	switch(c)
		{
		case 97:	// 'a' key
		case 104:	// 'h' key
		case 68:	// left arrow
		    success = moveLeft(board, score);  break;
		case 100:	// 'd' key
		case 108:	// 'l' key
		case 67:	// right arrow
			success = moveRight(board, score); break;
		case 119:	// 'w' key
		case 107:	// 'k' key
		case 65:	// up arrow
			success = moveUp(board, score);    break;
		case 115:	// 's' key
		case 106:	// 'j' key
		case 66:	// down arrow
			success = moveDown(board, score);  break;
		default: success = false;
		}
	return success;
	
}

void print_usage(){
    printf("To run the AI solver: \n");
    printf("USAGE: ./2048 ai <max/avg> <max_depth> slow\n");
    printf("or, to play with the keyboard: \n");
    printf("USAGE: ./2048\n");
}

/** Main program that handle execution of the game
 */
int main(int argc, char *argv[]) {
    // Variable used for control of the game 
    uint32_t score=0;	
	uint8_t board[SIZE][SIZE];

	int max_depth=0;
	
	char c;
	bool success;
	bool ai_run = false;
	propagation_t propagation=max;
	bool slow = false;

	// Variable used for output to file
	int numberOfGeneratedNode = 0;
	int numberOfExpandedNode = 0;
	uint8_t maximumValueInBoard = 0;
	double seconds = 0; // time for each search
	double searchTime = 0; // compounding time for total search

	/**
	 * Parsing command line options
	 */
	if( argc > 1 && argc < 4 ){
	    print_usage();
	    return 0;
	}
	
	if (argc > 1 ) {	    
		ai_run = true;
		initialize_ai();
		if( strcmp(argv[2],"avg")==0 ){
		    propagation = avg;
		}
		else if( strcmp(argv[2],"max")==0 ){
		    propagation = max;
		}
		else{
		    print_usage();
		    return 0;
		}

		sscanf (argv[3],"%d",&max_depth);
		
		if(argc == 5 && strcmp(argv[4],"slow")==0 )
		    slow = true;
	}


	printf("\033[?25l\033[2J");

	/**
	 *  Register signal handler for when ctrl-c is pressed
	 */
	
	signal(SIGINT, signal_callback_handler);	
	setBufferedInput(false);

	/**
	 * Create initial state
	 */
	initBoard(board, &score);
	
	while (true) {
	    /**
	     * AI execution mode
	     */
	    if(ai_run){
			/**
		 	* ****** Start timing the clock and process selected move
		 	*/
	    	clock_t start = clock();
			move_t selected_move = get_next_move(board, max_depth, propagation,
				&numberOfGeneratedNode, &numberOfExpandedNode);
			clock_t end = clock();
			
			// Seconds store number of seconds of searching the graph
			seconds = (double) (end - start) / CLOCKS_PER_SEC;
			searchTime += seconds; // calculate compound for total search
		
			/**
		 	* Execute the selected action
		 	*/
			success = execute_move_t( board, &score, selected_move);		    		    
	    }
	    else{
			/**
		 	 * Keyboard execution mode
		 	 */
			c=getchar();
			success = execute_keyboard(board, &score, c);
			if (c=='q') {
		    	printf("        QUIT? (y/n)         \n");
		    	c=getchar();
		    	if (c=='y') {
				break;
		    	}
		    	drawBoard(board,score);
			}
			if (c=='r') {
		    	printf("       RESTART? (y/n)       \n");
		    	c=getchar();
		    	if (c=='y') {
				initBoard(board, &score);
		    	}
		    	drawBoard(board,score);
			}
	    }		

	    /**
	     * If selected action merges tiles,
	     * then, add a random tile and redraw the board
	     */
	    if (success) {
		
			drawBoard(board,score);

			if(slow) usleep(150000); //0.15 seconds

			addRandom(board);
			drawBoard(board,score);
		
			if (gameEnded(board)) {
		    	printf("         GAME OVER          \n");
		    	break;
			}
	    }
	}

	// Print ai result to output.txt if running the ai mode
	if (ai_run) {

		// Check the maximum tiles in board after the game ended
		maximumValueInBoard = maximumTile(board);

		printOutput(max_depth, numberOfGeneratedNode, numberOfExpandedNode, 
		    searchTime, maximumValueInBoard, score);

		// Free heap array used by AI 
		freeAI();
	}
	
	setBufferedInput(true);

	printf("\033[?25h\033[m");
	
	return EXIT_SUCCESS;
}

/** Print to output.txt
 * @param max_depth Maximum depth of expansion
 * @param numberOfGeneratedNode Number of nodes that are applied action
 * @param numberOfExpandedNode Number of nodes out of heap
 * @param searchTime Search time until game over
 * @param maxTile Maximum value in board
 * @param score Score when game over
 */
void
printOutput(int max_depth, int numberOfGeneratedNode, int numberOfExpandedNode, 
	double searchTime, uint8_t maxTile, uint32_t score) {
	/** File open */
	FILE *fp = fopen("output.txt", "w");
	assert(fp != NULL);

	// Output every information needed for experimentation to output.txt
	fprintf(fp, "MaxDepth = %d \n", max_depth);
	fprintf(fp, "Generated = %d \n", numberOfGeneratedNode);
	fprintf(fp, "Expanded = %d \n", numberOfExpandedNode);
	fprintf(fp, "Time = %.2f seconds\n", searchTime);

	// Be careful with the expanded / second as second might be 0
	if (abs(searchTime - 0) < EPSILON) {
		fprintf(fp, "Expanded/Second = %.2f \n", numberOfExpandedNode / 
			EPSILON);
	}
	else {
		fprintf(fp, "Expanded/Second = %.2f \n", numberOfExpandedNode / 
			searchTime);
	}

	fprintf(fp, "max_tile = %d \n", (int) pow(BASE, maxTile));
	fprintf(fp, "Score = %d\n", score);
	
	fclose(fp);
}


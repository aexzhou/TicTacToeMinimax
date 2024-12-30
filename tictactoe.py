import random
import sys

class Tictactoe:
    """
    Game class of TicTacToe
    """
    def __init__(self) -> None:
        self.board = [' '] * 9
        # A list of 9 strings, one for each cell, 
        # will contain ' ' or 'X' or 'O'
        self.played = set()    # A set to keep track of the played cells 
        #`win_sets` contains a list of sets in which,
        # all the 3 values within each set must be a subset of 
        # the indices occupied by `who`. 
        # The way this code is laid out ensures that X and O will
        # be able to use this same function.
        self.win_sets = [
            {0,1,2}, # row configuration subsets to win
            {3,4,5},
            {6,7,8},
            {0,3,6}, # cols
            {1,4,7},
            {2,5,8},
            {0,4,8}, # diagonals
            {2,4,6}
        ]
        self.corner_cells = [0,2,6,8]

        self.gui = Screen()
        self.gui.displayWelcomeScreen()
        # Deciding first move 
        if(bool(random.randint(0,1))): # Chooses a random number between 1 and 0 (Heads or Tails)
            self.computer_first_turn = True
            self.gui.displayMessage("Heads. I go first!")
            self.computerNextMove()
        else:
            self.computer_first_turn = False
            self.gui.displayMessage("Tails. you go first!")
        # Show the board for the first time
        self.gui.displayBoard(self.board)    
    
        
    def playerNextMove(self) -> None:
        """ Method that prompts the player for a valid cell number, 
            and prints the info and the updated board;
            error checks that the input is a valid cell number 
        """
        while True: # Keep trying until user enters an integer type
            try:
                raw = self.gui.getPlayerInput()
                cellnum = int(raw)
            except ValueError:  
                self.gui.displayInputError(raw, 'type')
            else:
                if(cellnum < 0 or cellnum > 8):
                    self.gui.displayInputError(cellnum, 'range')

                elif(cellnum in self.played):
                    self.gui.displayInputError(cellnum, 'occupied')

                else:
                    self.gui.displayMessage(f"You chose cell {cellnum}")
                    break
        
        self.played.add(cellnum)
        self.board[cellnum] = 'X'
        self.gui.displayBoard(self.board)


    def hasWon(self, who: str) -> bool:
        """ returns True if who (being passed 'X' or 'O') has won, False otherwise """
        win_indices = [i for i, c in enumerate(self.board) if c == who] 

        if any(oneset.issubset(set(win_indices)) for oneset in self.win_sets):
            return True
        else:
            return False
    

    def empty_cells(self) -> list:
        # Find playable positions based on current unoccupied cells:
        return [i for i, cell in enumerate(self.board) if cell == " "]
    

    def minimax(self, position : int, maximizingPlayer : bool) -> tuple[int, int]:
        """
        This is a minimax algorithm implementation
        that recursively calls itself.
        The computer is the maximizing player, and the algorithm 
        assumes the player to play the best they can

        The computer is the maximizing player, returns the best position to play
        therefore, O is the maximizing player
        
        Args: position (int): The playable position to evaluate a score out of
            maximizingPlayer (bool): Set to false if the computer tries to predict a player move

        Returns:
            max/min_score (int): The highest score witnessed out of this move and all its descendant moves
            best_pos (int) : The best position
        """
        # First, check if last recursive call ended in a win/tie
        if self.hasWon('O'): 
            return 1*len(self.empty_cells()) + 1, position
        elif self.hasWon('X'):
            return -(1*len(self.empty_cells()) + 1), position
        elif len(self.empty_cells()) == 0:
            return 0, position
                
        if maximizingPlayer:
            max_score = -float('inf')
            best_pos = None
            for cell in self.empty_cells():
                self.board[cell] = 'O' # Test a move
                score, _ = self.minimax(cell, False)
                self.board[cell] = ' ' 
                if(score > max_score):
                    max_score = score
                    best_pos = cell # equate to current cell being eval'd if it's the best cell
            return max_score, best_pos
        else:
            min_score = float('inf')
            for cell in self.empty_cells():
                self.board[cell] = 'X' # Fake a player move
                score, _ = self.minimax(cell, True)
                self.board[cell] = ' '
                if(score < min_score):
                    min_score = score # cell corresponding to min_score if therefore what a 'perfect player' would play
                    best_pos = cell
            return min_score, best_pos # technically 'worst pos', but we're in player's POV rn
      

    def computerNextMove(self) -> None:
        """ Computer randomly chooses a valid cell, 
            and prints the info and the updated board 
        """
        cell : int
        temp_cell : int
        temp_score : int
        best_cell = 0
        best_score = -10000
        
        if(len(self.empty_cells()) == 9): # This function was called when the board was empty
            # This means that it's the computer's first move!
            best_cell = random.choice(self.corner_cells) 
        else:
            for cell in self.empty_cells():
                temp_score, temp_cell = self.minimax(cell, True)
                if(temp_score > best_score):
                    best_cell = temp_cell
        
        self.played.add(best_cell) # Adds this chosen cell into the set of occupied cells
        self.board[best_cell] = 'O'
        self.gui.displayBoard(self.board)
    
    def terminate(self, who: str) -> bool:
        """ returns True if who (being passed 'X' or 'O') has won or if it's a draw, False otherwise;
            it also prints the final messages:
                    "You won! Thanks for playing." or 
                    "You lost! Thanks for playing." or 
                    "A draw! Thanks for playing."  
        """
        if(self.hasWon(who) and who == 'X' ):
            self.gui.displayMessage("You won! Thanks for playing.")
            return True
        elif(self.hasWon(who) and who == 'O' ):
            self.gui.displayMessage("You lost! Thanks for playing.")
            return True
        elif(len(self.played) == 9):
            self.gui.displayMessage("A draw! Thanks for playing.")
            return True
        else:
            return False
    

class Screen():
    """
    Fake GUI
    Some screen formatting to make the game look a lot neater.
    ANSI codes are used to set cursor position and to clear the screen or row.
    """
    def __init__(self) -> None:
        self.clearScreen()
        # self.displayWelcomeScreen()
        self.info_message_row = 22
        self.state_messsage_row = 24
        self.board_start_row = 14
        self.player_input_row = 20


    def displayWelcomeScreen(self) -> None:
        self.clearScreen()
        """ prints the banner messages 
            and prints the intial board on the screen
        """
        print("Welcome to Tic-Tac-Toe!\n")
        print("You play X and the computer plays O.\n")
        print("\033[0;96mI've become sentient...\nI, as the computer, will now play strategically!\033[0m\n")
        print("Let's determine who goes first! I call heads.\n")
        input("Press [Enter] to flip a coin ...")


    def displayMessage(self, s: str) -> None:
        self.eraseRow(self.info_message_row) 
        self.setCursor(self.info_message_row,0)
        print(s)


    def displayGameStatus(self, s: str) -> None:
        self.eraseRow(self.state_messsage_row) 
        self.setCursor(self.state_messsage_row,0)
        print(s)


    def displayBoard(self, board: list) -> None:
        self.setCursor(self.board_start_row,0)
        board_print = (f'   {board[0]} | {board[1]} | {board[2]}    0 | 1 | 2\n'
                        f'   --+---+--    --+---+--\n'
                        f'   {board[3]} | {board[4]} | {board[5]}    3 | 4 | 5\n'
                        f'   --+---+--    --+---+--\n'
                        f'   {board[6]} | {board[7]} | {board[8]}    6 | 7 | 8'   
        )
        print(board_print)


    def clearScreen(self) -> None:
        print("\033[2J")
        print("\033[H")
    

    def setCursor(self, row = 0, col = 0) -> None:
        print(f"\033[{row};{col}H")
    

    def eraseRow(self, row = 0) -> None:
        print(f"\033[{row};0H")
        print(f"\033[2K")


    def getPlayerInput(self):
        self.eraseRow(self.player_input_row) # Refreshes the player input line so the last input clears
        self.setCursor(self.player_input_row,0)
        raw = input("> Next move for X (state a valid cell num): ")
        return raw


    def displayInputError(self, input, mode: str) -> None:

        if mode == 'range':
            self.setCursor(self.info_message_row,0)
            print(f"\033[91m '{input}' is occupied! Try another valid cell number\033[0m")

        elif mode == 'type':
            self.setCursor(self.info_message_row,0) 
            print(f"\033[91m '{input}' is not an Integer! Please try again...\033[0m")

        elif mode == 'occupied':
            self.setCursor(self.info_message_row,0) 
            print(f"\033[91m '{input}' is occupied! Try another valid cell number\033[0m")



if __name__ == "__main__":
    
    # Use as is. 
    game = Tictactoe()
    while True:
        while True:
            game.playerNextMove()            # X starts first
            if(game.terminate('X')): break   # if X won or a draw, print message and terminate
            game.computerNextMove()          # computer plays O
            if(game.terminate('O')): break   # if O won or a draw, print message and terminate
        
        game.gui.displayGameStatus("Press [1] to continue game. Press anything else to end.")
        option = input("> ")
        if option == '1':
            game = Tictactoe()
            continue
        else:
            break

    sys.exit()
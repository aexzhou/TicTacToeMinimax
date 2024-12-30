#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <string>
#include <random>
#include <limits>
#include <algorithm>


class Screen 
{
public:
    Screen() {
        clearScreen();
        info_message_row = 22;
        state_message_row = 24;
        board_start_row   = 14;
        player_input_row  = 20;
    }

    void displayWelcomeScreen() {
        clearScreen();
        std::cout << "Welcome to Tic-Tac-Toe!\n\n";
        std::cout << "You play X and the computer plays O.\n\n";
        std::cout << "\033[0;96mI've become sentient...\nI, as the computer, will now play strategically!\033[0m\n\n";
        std::cout << "Let's determine who goes first! I call heads.\n\n";
        std::cout << "Press [Enter] to flip a coin ...";
        std::string option;
        std::getline(std::cin, option);
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    void displayMessage(const std::string &s) {
        eraseRow(info_message_row);
        setCursor(info_message_row, 0);
        std::cout << s << "\n";
    }

    void displayGameStatus(const std::string &s) {
        eraseRow(state_message_row);
        setCursor(state_message_row, 0);
        std::cout << s << "\n";
    }

    void displayBoard(const std::vector<char> &board) {
        setCursor(board_start_row, 0);
        std::cout << "   " << board[0] << " | " << board[1] << " | " << board[2] << "    0 | 1 | 2\n"
                  << "   --+---+--    --+---+--\n"
                  << "   " << board[3] << " | " << board[4] << " | " << board[5] << "    3 | 4 | 5\n"
                  << "   --+---+--    --+---+--\n"
                  << "   " << board[6] << " | " << board[7] << " | " << board[8] << "    6 | 7 | 8\n";
    }

    std::string getPlayerInput() {
        eraseRow(player_input_row);
        setCursor(player_input_row, 0);
        std::cout << "> Next move for X (state a valid cell num): ";
        std::cout.flush();

        std::string raw;
        std::getline(std::cin, raw);
        return raw;
    }

    void displayInputError(const std::string &input, const std::string &mode) {
        setCursor(info_message_row,0);
        if (mode == "range") {
            std::cout << "\033[91m '" << input << "' is not in range 0-8! Please try again...\033[0m\n";
        } else if (mode == "type") {
            std::cout << "\033[91m '" << input << "' is not an Integer! Please try again...\033[0m\n";
        } else if (mode == "occupied") {
            std::cout << "\033[91m '" << input << "' is occupied! Try another valid cell number\033[0m\n";
        }
    }

    void clearScreen() {
        std::cout << "\033[2J" << "\033[H";
    }

    void setCursor(int row = 0, int col = 0) {
        std::cout << "\033[" << row << ";" << col << "H";
    }

    void eraseRow(int row = 0) {
        std::cout << "\033[" << row << ";0H";
        std::cout << "\033[2K";
    }

private:
    int info_message_row;
    int state_message_row;
    int board_start_row;
    int player_input_row;
};


class TicTactoe 
{
public:
    TicTactoe() { // init

        board = std::vector<char>(9, ' ');

        win_sets = {
            std::array<int,3>{0,1,2}, // rows
            std::array<int,3>{3,4,5},
            std::array<int,3>{6,7,8},
            std::array<int,3>{0,3,6}, // cols
            std::array<int,3>{1,4,7},
            std::array<int,3>{2,5,8},
            std::array<int,3>{0,4,8}, // diagonals
            std::array<int,3>{2,4,6}
        };
        num_win_sets = win_sets.size();

        corner_cells = {0,2,6,8};

        bool computer_goes_first = randomBool();

        if (computer_goes_first) {
            computer_first_turn = true;
            gui.displayMessage("Heads. I go first!");
            computerNextMove();
        } else {
            computer_first_turn = false;
            gui.displayMessage("Tails. you go first!");
        }

        gui.displayBoard(board);
    }

    void playerNextMove() {
        while (true) {
            std::string raw = gui.getPlayerInput();
            int cellnum;
            try {
                cellnum = std::stoi(raw);
            } catch (...) {
                gui.displayInputError(raw, "type");
                continue;
            }

            if (cellnum < 0 || cellnum > 8) {
                gui.displayInputError(std::to_string(cellnum), "range");
            } 
            else if (played.find(cellnum) != played.end()) {
                gui.displayInputError(std::to_string(cellnum), "occupied");
            }
            else {
                gui.displayMessage("You chose cell " + std::to_string(cellnum));
                played.insert(cellnum);
                board[cellnum] = 'X';
                gui.displayBoard(board);
                break;
            }
        }
    }
    
    bool hasWon(char who) {
        // scan through the board and get current indices
        std::vector<int> current_indices;
        for (int i = 0; i < 9; i++) {
            if (board[i] == who) { 
                current_indices.push_back(i); // push_back = append
            }
        }

        // see if any set in win_sets is a subset of current_indices
        // convert current_indices to a set, and check for subsets
        std::set<int> current_set(current_indices.begin(), current_indices.end());

        for (auto &wset : win_sets) {
            std::set<int> check_set(wset.begin(), wset.end());
            bool is_subset = true; // "nested" flag
            // this can be improved to quit as soon as a winning set is found
            for (int set : check_set) {
                if (current_set.find(set) == current_set.end()) {
                    is_subset = false;
                    break;
                }
            }
            if (is_subset) return true;
        }
        return false;
    }

    /*
    Returns vector of current empty cells
    */
    std::vector<int> empty_cells() {
      
        std::vector<int> empties;
        for (int i = 0; i < 9; i++) {
            if (board[i] == ' ') {
                empties.push_back(i);
            }
        }
        return empties;
    }

    std::pair<int, int> minimax(int position, bool maximizingPlayer) {

        // check if last recursive call ended in a win/tie
        int highest_score;
        // conditions to break out of current recursive call
        if (hasWon('O')) {
            highest_score = 1* (int)empty_cells().size() + 1;
            return std::make_pair(highest_score, position);
        } 
        else if(hasWon('X')) {
            highest_score = -1 * ((int)empty_cells().size() + 1);
            return std::make_pair(highest_score, position);
        }
        else if(empty_cells().empty()) {
            return std::make_pair(0, position);
        }

        if (maximizingPlayer) {
            int max_score = -9999999;
            int best_pos = -1;
            for (int cell : empty_cells()) {
                board[cell] = 'O';
                auto result = minimax(cell, false); // make recursive call
                board[cell] = ' ';
                int score = result.first;
                if (score > max_score) { 
                    max_score = score;
                    best_pos = cell;
                }
            }
            return std::make_pair(max_score, best_pos);
        } 
        else {
            int min_score = 9999999;
            int best_pos = -1;
            for (int cell : empty_cells()) {
                board[cell] = 'X';
                auto result = minimax(cell, true);
                board[cell] = ' ';
                int score = result.first;
                if(score < min_score) {
                    min_score = score;
                    best_pos = cell;
                }
            }
            return std::make_pair(min_score, best_pos);
        }
    }

    void computerNextMove() {
        int cell;
        int temp_cell;
        int temp_score;
        int best_cell = 0;
        int best_score = -9999999;
        
        if(empty_cells().size() == 9) {
            best_cell = corner_cells[randomInt(0, corner_cells.size() - 1)];
        } 
        else {
            for (int cell : empty_cells()) {
                auto result    = minimax(cell, true);
                int temp_score = result.first;
                int temp_cell  = result.second;
                if (temp_score > best_score) {
                    best_score = temp_score;
                    best_cell  = temp_cell;
                }
            }

            played.insert(best_cell);
            board[best_cell] = 'O';
            gui.displayBoard(board);
        }
    }

    bool terminate(char who) {
        if (hasWon(who) && who == 'X') {
            gui.displayMessage("You won! Thanks for playing.");
            return true;
        } else if (hasWon(who) && who == 'O') {
            gui.displayMessage("You lost! Thanks for playing.");
            return true;
        } else if (played.size() == 9) {
            gui.displayMessage("A draw! Thanks for playing.");
            return true;
        } else {
            return false;
        }
    }

    void restart() {
        for (int i = 0; i < 9; i++) {
            board[i] = ' ';
        }
        played.clear();

        bool computer_goes_first = randomBool();

        if (computer_goes_first) {
            computer_first_turn = true;
            gui.displayMessage("Heads. I go first!");
            computerNextMove();
        } else {
            computer_first_turn = false;
            gui.displayMessage("Tails. you go first!");
        }

        gui.displayBoard(board);
    }

private:
    std::vector<char> board;
    std::set<int> played;
    std::vector<std::array<int,3>> win_sets;
    std::vector<int> corner_cells;
    bool computer_first_turn = false;
    int num_win_sets;

    Screen gui;
    
    // random gen
    bool randomBool() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0,1);
        return dist(gen) == 1;
    }

    int randomInt(int min_val, int max_val) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(min_val, max_val);
        return dist(gen);
    }
};



int main() {
    TicTactoe game;
    Screen gui;
    while (1) {
        while (1) {
            game.playerNextMove();
            if(game.terminate('X')) break;
            game.computerNextMove();
            if(game.terminate('O')) break;
        }
        gui.displayGameStatus("Press [1] to continue game. Press anything else to end.");
        std::cout << "> ";
        std::string option;
        std::getline(std::cin, option);

        if (option == "1") {
            // Reinitialize the game
            // TicTactoe game;
            game.restart();
            continue;
        } else {
            break;
        }
    }
    return 0;
}
/*
 * File: Boggle.cpp
 * ----------------
 * Name: [TODO: enter name here]
 * Section: [TODO: enter section leader here]
 * This file is the main starter file for Assignment #4, Boggle.
 * [TODO: extend the documentation]
 */

#include <iostream>
#include "gboggle.h"
#include "grid.h"
#include "gwindow.h"
#include "lexicon.h"
#include "random.h"
#include "simpio.h"
using namespace std;

/* Constants */

const int BOGGLE_WINDOW_WIDTH = 650;
const int BOGGLE_WINDOW_HEIGHT = 350;

const string SMALL_CUBES[9] = {
	"AAEEGN", "ABBJOO", "ACHOPS",
	"AOOTTW", "CIMOTU", "DEILRX",
	"DISTTY", "EEGHNW", "EEINSU",

};

const string STANDARD_CUBES[16] = {
	"AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS",
	"AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
	"DISTTY", "EEGHNW", "EEINSU", "EHRTVW",
	"EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"
};

const string BIG_BOGGLE_CUBES[25] = {
	"AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM",
	"AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW",
	"CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT",
	"DHHLOR", "DHLNOR", "EIIITT", "EMOTTT", "ENSSSU",
	"FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};


const int DICE_FACE_COUNT = 6;
const int BOARD_LENGTH = 4;
const int DELAY = 300;
/* Function prototypes */

void welcome();
void giveInstructions();
void playGame();
void getGameData();
void shuffleVector(Vector<string>&);
void drawBoard(Vector<string>);
void stringToUpper(string&);
void getUserInput(Vector<string>&, int&);
int usersTurn(Vector<string>&);
int computersTurn(Vector<string>&);
bool tryWord(Vector<string>&, string, Vector<pair<int, int>>&);
void findAllWords(Vector<string>&, int&);
bool dfs(Vector<string>& board, string& target, int index, int row, int col, Vector<pair<int, int>>& path);
void dfsForBot(Vector<string>&, int, int, string&, Vector<Vector<bool>>&, Set<string>&);
void searchAllWordFromCell(Vector<string>&, int, int, Set<string>&);
void findAllWords(Vector<string>&, int&);
string getUserData();
Vector<string> initGame();
Vector<string> generateRandomBoard();
Vector<string> generateCustomBoard(string);
void highlightPath(Vector<pair<int, int>>&);
void decideWinner(int, int);
bool getUsersDecision();
/* Main program */
const string FILENAME = "EnglishWords.dat";
Lexicon WORDS(FILENAME);

int main() {
	GWindow gw(BOGGLE_WINDOW_WIDTH, BOGGLE_WINDOW_HEIGHT);
	welcome();
	giveInstructions();
	initGBoggle(gw);
	playGame();
	return 0;
}

void playGame() {
	while (true) {
		Vector<string> board = initGame();
		int userScore = usersTurn(board);
		int computerScore = computersTurn(board);
		decideWinner(userScore, computerScore);
		if (!getUsersDecision()) {
			break;
		};
	}
}

// It returns user's score
int usersTurn(Vector<string>& board) {
	int score = 0;
	getUserInput(board, score);
	return score;
}

// Returns computer's score
int computersTurn(Vector<string>& board) {
	int score = 0;
	findAllWords(board, score);
	return score;
}

bool getUsersDecision() {
	string dec = getLine("Do you want to play again? (yes or no) ");
	return dec == "yes";
}

void decideWinner(int user, int computer) {
	if (user < computer) {
		cout << "Computer destroyed you :(( (As always)" << endl;
	}
	else {
		cout << "DAMN GOOD JOB" << endl;
	}
}

void findAllWords(Vector<string>& board, int& score) {
	Set<string> words;
	for (int i = 0; i < board.size(); i++) {
		for (int j = 0; j < board[i].size(); j++) {
			searchAllWordFromCell(board, i, j, words);
		}
	}
	for (const string& word : words) {
		score += word.size() - 3;
		recordWordForPlayer(word, Player::COMPUTER);
	}
}

void searchAllWordFromCell(Vector<string>& board, int row, int col, Set<string>& words) {
	string current = "";
	Vector<Vector<bool>> visited(board.size(), Vector<bool>(board[0].size(), false));
	dfsForBot(board, row, col, current, visited, words);
}

void dfsForBot(Vector<string>& board, int row, int col, string& current,
	Vector<Vector<bool>>& visited, Set<string>& words) {
	if (row < 0 || row >= board.size() || col < 0 || col >= board[0].size() || visited[row][col]) {
		return;
	}
	current += board[row][col];
	if (WORDS.contains(current) && current.size() >= 4) {
		words.add(current);
	}
	if (!WORDS.containsPrefix(current)) {
		current.pop_back();
		return;
	}
	visited[row][col] = true;
	int directions[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1},
						   {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
	for (const auto& dir : directions) {
		int newRow = row + dir[0];
		int newCol = col + dir[1];
		dfsForBot(board, newRow, newCol, current, visited, words);
	}
	visited[row][col] = false;
	current.pop_back();
}

void getUserInput(Vector<string>& board, int& score) {
	Set<string> guessedWords;
	while (true) {
		string inp = getLine("Enter a 4+ characters long word (press Enter to finish): ");
		stringToUpper(inp);
		if (inp.empty()) {
			return;
		}
		else if (inp.size() < 4) {
			cout << "It must be at least 4 characters long" << endl;
		}
		else if (!WORDS.contains(inp)) {
			cout << "Not a valid word" << endl;
		}
		else if (guessedWords.contains(inp)) {
			cout << "Word is already used" << endl;
		}
		else {
			Vector<pair<int, int>> path;
			if (tryWord(board, inp, path)) {
				guessedWords.add(inp);
				recordWordForPlayer(inp, Player::HUMAN);
				highlightPath(path);

				score += inp.size() - 3;
			}
			else {
				cout << "That word can not be formed on this board" << endl;
			}
		}
	}
}

// Simple bfs, searching for target word inside board
bool dfs(Vector<string>& board, string& target, int index, int row, int col, Vector<pair<int, int>>& path) {
	if (index == target.size()) return true;
	if (row < 0 || row >= board.size() || col < 0 || col >= board[0].size()) return false;
	if (board[row][col] == '#' || board[row][col] != target[index]) return false;

	char temp = board[row][col];
	board[row][col] = '#';
	path.push_back({ row, col });
	int directions[8][2] = {
		{-1, -1}, {-1, 0}, {-1, 1},
		{ 0, -1},          { 0, 1},
		{ 1, -1}, { 1, 0}, { 1, 1}
	};
	for (auto& dir : directions) {
		int newRow = row + dir[0];
		int newCol = col + dir[1];
		if (dfs(board, target, index + 1, newRow, newCol, path)) {
			board[row][col] = temp;
			return true;
		}
	}
	board[row][col] = temp;
	path.remove(path.size() - 1);
	return false;
}

bool tryWord(Vector<string>& board, string word, Vector<pair<int, int>>& result) {
	for (int row = 0; row < board.size(); row++) {
		for (int col = 0; col < board[row].size(); col++) {
			char c = board[row][col];
			if (c == word[0]) {
				Vector<pair<int, int>> path;
				if (dfs(board, word, 0, row, col, path)) {
					result = path;
					return true;
				}
			}
		}
	}
	return false;
}


void stringToUpper(string& str) {
	for (int i = 0; i < str.size(); i++) {
		str[i] = toupper(str[i]);
	}
}

// This function returns the board state
Vector<string> initGame() {
	string userData = getUserData();
	Vector<string> board = userData.empty() ? generateRandomBoard() : generateCustomBoard(userData);
	drawBoard(4, 4);
	drawBoard(board);
	return board;
}

// Visually draws the board based on state
void drawBoard(Vector<string> board) {
	for (int row = 0; row < board.size(); row++) {
		for (int col = 0; col < board[row].size(); col++) {
			labelCube(row, col, board[row][col]);
		}
	}
}

// Generates the board state based on user input (16 characters long string)
Vector<string> generateCustomBoard(string data) {
	Vector<string> board;
	string curr = "";
	for (int i = 0; i < data.size(); i++) {
		curr += toupper(data[i]);
		if (curr.size() == 4) {
			board += curr;
			curr = "";
		}
	}
	return board;
}

// First shuffles the vector, then chooses random top face of each dice and adds it to the board state
Vector<string> generateRandomBoard() {
	Vector<string> dice;
	for (int i = 0; i < 16; i++) {
		dice += STANDARD_CUBES[i];
	}
	shuffleVector(dice);

	Vector<string>board;
	string currentRow = "";
	for (int i = 0; i < dice.size(); i++) {
		currentRow += dice[i][randomInteger(0, dice[i].size() - 1)];
		if (currentRow.size() == 4) {
			board.add(currentRow);
			currentRow = "";
		}
	}
	return board;
}

// Returns 16 symbol string if user chose custom game, returns "" if user chose random game
string getUserData() {
	string customGameChoice = getLine("Do you want to generate a random board? (yes or no) ");
	if (customGameChoice == "yes") {
		return "";
	}

	string diceSequence = getLine("Type the 16 letters to appear on the board: ");
	while (diceSequence.size() != 16) {
		cout << "It must be 16 characters long" << endl;
		diceSequence = getLine("Type the 16 letters to appear on the board: ");
	}
	return diceSequence;
}

void shuffleVector(Vector<string>& v) {
	for (int i = 0; i < v.size(); i++) {
		int x = randomInteger(i, v.size() - 1);
		swap(v[i], v[x]);
	}
}

void highlightPath(Vector<pair<int, int>>& paths) {
	for (pair<int, int> path : paths) {
		highlightCube(path.first, path.second, true);
		pause(DELAY);
		highlightCube(path.first, path.second, false);
	}
}

/*
 * Function: welcome
 * Usage: welcome();
 * -----------------
 * Print out a cheery welcome message.
 */

void welcome() {
	cout << "Welcome!  You're about to play an intense game ";
	cout << "of mind-numbing Boggle.  The good news is that ";
	cout << "you might improve your vocabulary a bit.  The ";
	cout << "bad news is that you're probably going to lose ";
	cout << "miserably to this little dictionary-toting hunk ";
	cout << "of silicon.  If only YOU had a gig of RAM..." << endl << endl;
}

/*
 * Function: giveInstructions
 * Usage: giveInstructions();
 * --------------------------
 * Print out the instructions for the user.
 */

void giveInstructions() {
	cout << endl;
	cout << "The boggle board is a grid onto which I ";
	cout << "I will randomly distribute cubes. These ";
	cout << "6-sided cubes have letters rather than ";
	cout << "numbers on the faces, creating a grid of ";
	cout << "letters on which you try to form words. ";
	cout << "You go first, entering all the words you can ";
	cout << "find that are formed by tracing adjoining ";
	cout << "letters. Two letters adjoin if they are next ";
	cout << "to each other horizontally, vertically, or ";
	cout << "diagonally. A letter can only be used once ";
	cout << "in each word. Words must be at least four ";
	cout << "letters long and can be counted only once. ";
	cout << "You score points based on word length: a ";
	cout << "4-letter word is worth 1 point, 5-letters ";
	cout << "earn 2 points, and so on. After your puny ";
	cout << "brain is exhausted, I, the supercomputer, ";
	cout << "will find all the remaining words and double ";
	cout << "or triple your paltry score." << endl << endl;
	cout << "Hit return when you're ready...";
	getLine();
}

// [TODO: Fill in the rest of the code]

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

/**
*
* 1 2 3
* 4 5 6
* 7 8 9
*
**/

const char X = 'X';
const char O = 'O';
const char DRAW = '#';
const char IN_PROGRESS = '.';
const char NOT_CALCULATED = '?';
const char ANY_MINIBOARD = 9;
const char NO_MOVE = -1;

struct MiniBoard {
	array<char, 9> state;
	array<char, 9> moves;
	char moves_count;
	char side = NOT_CALCULATED;
	char num_x, num_o;
};

const unsigned short int power3[9] = {
	1, 3, 9, 27 * 1, 27 * 3, 27 * 9, 27 * 27 * 1, 27 * 27 * 3, 27 * 27 * 9 };

MiniBoard miniBoards[20000];

/*
012
345
678
*/
const int win_rows[8][3] = {
	// horysontal
	{ 0, 1, 2 },
	{ 3, 4, 5 },
	{ 6, 7, 8 },
	// vertical
	{ 0, 3, 6 },
	{ 1, 4, 7 },
	{ 2, 5, 8 },
	// diagonal
	{ 0, 4, 8 },
	{ 2, 4, 6 },
};

struct Move {
	char board;
	char pos;
};

short make_mini_board(short base, char pos, char side) {
	auto pow = power3[pos];
	short id = base + pow * (side == X ? 1 : 2);
	auto &b = miniBoards[id];

	if (b.side == NOT_CALCULATED) {
		b = miniBoards[base];
		// update board state
		b.state[pos] = side;

		// update count of X/O
		if (side == X)
			b.num_x++;
		else
			b.num_o++;

		// calculate available moves
		b.moves_count = 0;
		for (int i = 0; i < 9; ++i) {
			if (b.state[i] == '.')
				b.moves[b.moves_count++] = i;
		}

		// reset winning side for this position
		b.side = b.moves_count ? IN_PROGRESS : DRAW;
		for (auto &row : win_rows) {
			int sum = 0;
			for (auto &i : row)
				sum += b.state[i];

			if (sum == side * 3) {
				// winning pattern detected
				b.side = side;
				b.moves_count = 0;
				break;
			}
		}
	}
	return id;
}

struct Board {
	array<short, 9> boards;
	short superBoard;
	char nextMoveSide;
	char availableMiniBoard;

	unsigned int moves_count() const {
		if (availableMiniBoard == ANY_MINIBOARD)
			return accumulate(boards.begin(), boards.end(), 0, [](int sum, short b) {
			return sum + miniBoards[b].moves_count;
		});
		return miniBoards[boards[availableMiniBoard]].moves_count;
	}

	void legal_moves(vector<Move> &moves) const {
		int n = moves_count();
		moves.clear();
		auto add_moves = [&moves, this](char board) {
			auto &b = miniBoards[boards[board]];
			for (auto m : b.moves) {
				if (m == NO_MOVE)
					break;
				moves.push_back({ board, m });
			}
		};
		if (availableMiniBoard != ANY_MINIBOARD)
			add_moves(availableMiniBoard);
		else
			for (auto mini : miniBoards[superBoard].moves)
				add_moves(mini);
	}

	Move random_move(int rand) const {
		int n = moves_count();
		if (n == 0)
			return{};
		rand %= n;
		auto board = availableMiniBoard;
		if (board == ANY_MINIBOARD) {
			auto &super = miniBoards[superBoard];
			for (auto &mini_id : super.moves) {
				auto &mini = miniBoards[boards[mini_id]];
				if (mini.moves_count > rand) {
					board = mini_id;
					break;
				}
				rand -= mini.moves_count;
			}
		}
		Move m = { board, miniBoards[boards[board]].moves[rand] };
		auto new_id = make_mini_board(boards[m.board], m.pos, O);
		if (new_id > 20000)
		{
			int f = 5;
		}
		return{ board, miniBoards[boards[board]].moves[rand] };
	}

	char apply_move(Move m, char side) {
		auto new_id = make_mini_board(boards[m.board], m.pos, side);
		boards[m.board] = new_id;

		auto b = miniBoards[new_id];
		if (b.side == side)
		{
			superBoard = make_mini_board(superBoard, m.board, side);

			auto& super = miniBoards[superBoard];
			if (super.side == side) return side;
		}
		if (b.moves_count == 0)
		{
			int has_any_move = false;
			for (auto i : boards)
				if (has_any_move = miniBoards[i].moves_count > 0)
					break;
			if (!has_any_move)
			{
				auto& super = miniBoards[superBoard];
				if (super.num_x > super.num_o) return X;
				if (super.num_x < super.num_o) return O;
				return DRAW;
			}
		}

		// update next available miniboard
		auto nextBoardIndex = m.pos;
		auto& nextBoard = miniBoards[boards[nextBoardIndex]];
		if (nextBoard.side == IN_PROGRESS)
			this->availableMiniBoard = nextBoardIndex;
		else this->availableMiniBoard = ANY_MINIBOARD;

		return IN_PROGRESS;
	}
};

void swap(char &side) { side ^= O ^ X; }

int single_playout(Board b, char side) {
	auto res = IN_PROGRESS;
	while (res == IN_PROGRESS) {
		int r = 7; // rand();
		res = b.apply_move(b.random_move(r), side);
		swap(side);
	}
	return res;
}

int playout(const Board& b, int n, char side)
{
	int res = 0;
	while (n-- > 0)
		res += single_playout(b, side) == side;
	return res;
}

void initEmptyMiniBoard() {
	MiniBoard &b = miniBoards[0];
	b.state = { { '.', '.', '.', '.', '.', '.', '.', '.', '.' } };
	b.side = IN_PROGRESS;
	b.moves = { { 0, 1, 2, 3, 4, 5, 6, 7, 8 } };
	b.moves_count = 9;
}

void initialBoard(Board &b) {
	initEmptyMiniBoard();
	b.boards = { { 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
	b.superBoard = 0;
	b.nextMoveSide = X;
	b.availableMiniBoard = ANY_MINIBOARD;
}

Move make_move(int row, int col) {
	return Move{ (char)(row / 3 * 3 + col / 3), (char)(row % 3 * 3 + col % 3) };
}
pair<int, int> get_row_and_col(Move m) {
	m = make_move(m.board, m.pos);
	return{ m.board, m.pos };
}

void skip_valid_actions() {
	int validActionCount;
	cin >> validActionCount;
	int row = 1;
	int col = 1;
	for (int i = 0; i < validActionCount; i++) {
		cin >> row >> col;
	}
}

Move chooseMove(const Board& b)
{
	vector<Move> legal_moves;
	b.legal_moves(legal_moves);

	Move best;
	int best_score = -1;
	for (auto m : legal_moves)
	{
		auto board_copy = b;
		auto res = board_copy.apply_move(m, X);
		int score = 0;
		const int N = 100;
		if (res == X)
			score = N * 2;
		else if (res == O)
			score = 0;
		else if (res == DRAW)
			score = N / 2;
		else score = -playout(board_copy, N, O);

		if (score > best_score)
		{
			best_score = score;
			best = m;
			if (score > N) break;
		}
	}

	return best;
}

int main() {
	vector<vector<int>> fields(10, vector<int>(10, 0));

	Board b;

	initialBoard(b);

	// game loop
	while (1) {
		int opponentRow;
		int opponentCol;
		cin >> opponentRow >> opponentCol;

		if (opponentCol != -1 && opponentRow != -1) {
			auto opponentMove = make_move(opponentRow, opponentCol);
			b.apply_move(opponentMove, O);
		}

		skip_valid_actions();

		Move my_move = chooseMove(b);

		b.apply_move(my_move, X);
		auto res = get_row_and_col(my_move);
		cout << res.first << " " << res.second << endl;
	}
}
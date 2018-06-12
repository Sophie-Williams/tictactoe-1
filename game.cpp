#include <algorithm>
#include <functional>
#include <memory>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>
#include <chrono>

#include "tictactoe.h"

using namespace std;

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

namespace game_impl
{

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
	bool operator==(Move m)const { return board == m.board&&pos == m.pos; }
	friend ostream& operator<<(ostream& out, Move m);
};

void swap(char &side) { side ^= O ^ X; }

short make_mini_board(short base, char pos, char side) {
	auto pow = power3[pos];
	short id = base + pow * (side == X ? 1 : 2);
	auto &b = miniBoards[id];

	if (b.side == NOT_CALCULATED) {
		b = miniBoards[base];
		// update board state
		assert(b.state[pos] == '.');
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

			if (sum == X * 3) {
				// winning pattern detected
				b.side = X;
				b.moves_count = 0;
				break;
			}
			if (sum == O * 3) {
				// winning pattern detected
				b.side = O;
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
		{
			return accumulate(boards.begin(), boards.end(), 0, [](int sum, short b) {
				return sum + miniBoards[b].moves_count;
			});
		}
		return miniBoards[boards[availableMiniBoard]].moves_count;
	}

	void legal_moves(vector<Move> &moves) const {
		int n = moves_count();
		moves.clear();
		auto add_moves = [&moves, this](char board) {
			auto &b = miniBoards[boards[board]];
			for (int i = 0; i < b.moves_count; ++i)
				moves.push_back({ board, b.moves[i] });
		};
		if (availableMiniBoard != ANY_MINIBOARD)
			add_moves(availableMiniBoard);
		else
		{
			auto& super = miniBoards[superBoard];
			for (int i = 0; i < super.moves_count; ++i)
				add_moves(super.moves[i]);
		}
	}

	vector<Move> legal_moves() const
	{
		vector<Move> moves;
		legal_moves(moves);
		return moves;
	}

	Move random_move(int rand) const {
		int n = moves_count();
		if (n == 0)
			return{};
		rand %= n;
		auto board = availableMiniBoard;
		if (board == ANY_MINIBOARD) {
			auto &super = miniBoards[superBoard];
			for (int i = 0; i < super.moves_count; ++i) {
				auto mini_id = super.moves[i];
				auto &mini = miniBoards[boards[mini_id]];
				if (mini.moves_count > rand) {
					board = mini_id;
					break;
				}
				rand -= mini.moves_count;
			}
		}

		assert(board != ANY_MINIBOARD);
		return{ board, miniBoards[boards[board]].moves[rand] };
	}

	char apply_move(Move m) {
		char side = nextMoveSide;
		swap(nextMoveSide);
		auto new_id = make_mini_board(boards[m.board], m.pos, side);
		boards[m.board] = new_id;

		const auto& b = miniBoards[new_id];
		if (b.side == side)
		{
			superBoard = make_mini_board(superBoard, m.board, side);

			const auto& super = miniBoards[superBoard];
			if (super.side == side)
			{
				availableMiniBoard = side;
				return side;
			}
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
				if (super.num_x > super.num_o)
				{
					availableMiniBoard = X;
					return X;
				}
				if (super.num_x < super.num_o)
				{
					availableMiniBoard = O;
					return O;
				}
				availableMiniBoard = DRAW;
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

bool has_moves(const Board& b)
{
	return b.availableMiniBoard <= ANY_MINIBOARD;
}

class GameImpl : public ::Game{
public:
	GameImpl(){
		initialBoard(board);
	}

	bool is_valid_move(int a, int b) const override{
		if (a < 0 || a>8 || b < 0 || b>8) return false;
		if (board.availableMiniBoard != ANY_MINIBOARD && board.availableMiniBoard != a) return false;
		return miniBoards[board.boards[a]].state[b] == '.';
	}
	void apply_move(int a, int b) override{
		board.apply_move({(char)a, (char)b });
	}
	bool is_game_over() const override{
		return !has_moves(board);
	}
	char get_winner() const override{
		return board.availableMiniBoard;
	}
	void print_board() const override{

		auto row = [](short id, int r) {
			return string{ &miniBoards[id].state[r * 3], 3 };
		};

		auto& b = board;
		for (int i : {0, 1, 2})
		{
			for (int j : {0, 1, 2})
			{
				cout
					<< row(b.boards[i * 3], j) << "|"
					<< row(b.boards[i * 3 + 1], j) << "|"
					<< row(b.boards[i * 3 + 2], j) << endl;
			}
			if (i < 2) cerr << "---+---+---" << endl;
		}
	}

	Board board;
};

}

shared_ptr<Game> make_game(){
	return make_shared<game_impl::GameImpl>();
}

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>

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
	bool operator==(Move m)const { return board == m.board&&pos == m.pos; }
};

void swap(char &side) { side ^= O ^ X; }

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
		Move m = { board, miniBoards[boards[board]].moves[rand] };
		auto new_id = make_mini_board(boards[m.board], m.pos, O);

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

struct GameStat
{
	int X_wins = 0, O_wins = 0;
};

struct Node
{
	Node* parent = nullptr;
	Node* first = nullptr;
	GameStat stat;
	Board board;
	short children_size = 0;
	Move move;
};

vector<Node> nodes_pool(10 * 1000 * 1000);
Node* nodes_pool_head = &nodes_pool.front();

Node* allocate_nodes(short count)
{
	auto res = nodes_pool_head;
	nodes_pool_head += count;
	assert(nodes_pool_head <= &nodes_pool.back());
	return res;
}

int single_playout(Board b) {
	auto res = IN_PROGRESS;
	while (res == IN_PROGRESS) {
		int r = rand();
		res = b.apply_move(b.random_move(r));
	}
	return res;
}

GameStat playout(const Board& b, int n)
{
	GameStat res;
	while (n-- > 0)
		if (single_playout(b) == X)
			res.X_wins += 1;
		else res.O_wins += 1;
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
		auto res = board_copy.apply_move(m);
		int score = 0;
		const int N = 100;
		if (res == X)
			score = N * 2;
		else if (res == O)
			score = 0;
		else if (res == DRAW)
			score = N / 2;
		else
		{
			auto stat = playout(board_copy, N);
			if (b.nextMoveSide == X) score = stat.X_wins;
			else score = stat.O_wins;
		}

		cerr << (int)m.board << "-" << (int)m.pos << " score " << score << endl;

		if (score > best_score)
		{
			best_score = score;
			best = m;
			if (score > N) break;
		}
	}

	return best;
}

string get_row(short id, int r)
{
	auto b = miniBoards[id];
	return{ &b.state[r * 3], 3 };
}

void print(const Board& b)
{
	for (int i : {0, 1, 2})
	{
		for (int j : {0, 1, 2})
		{
			cerr
				<< get_row(b.boards[i * 3], j) << "|"
				<< get_row(b.boards[i * 3 + 1], j) << "|"
				<< get_row(b.boards[i * 3 + 2], j) << endl;
		}
		if (i < 3) cerr << "---+---+---" << endl;
	}
}

Node* make_root_node(const Board& b)
{
	auto node = allocate_nodes(1);
	node->board = b;
	return node;
}

bool has_moves(const Board& b)
{
	return b.availableMiniBoard <= ANY_MINIBOARD;
}

double childPotential(const Node* child, char side, double log_t)
{
	auto& s = child->stat;
	double n = s.X_wins + s.O_wins;
	double w = side == X ? s.X_wins : s.O_wins;

	assert(n > 0);
	return w / n + 1.4 * sqrt(log_t / n);
}

double childValue(const Node* child, char side)
{
	auto& s = child->stat;
	double n = s.X_wins + s.O_wins;
	double w = side == X ? s.X_wins : s.O_wins;

	assert(n > 0);
	return w / n;
}

Node* find_leaf_node(Node* root)
{
	while (root->children_size > 0)
	{
		auto& s = root->stat;
		double t = s.X_wins + s.O_wins;
		assert(t > 0);
		const auto log_t = log(t);
		auto side = root->board.nextMoveSide;

		double best = -999;
		Node* best_node = nullptr;
		for (int i = 0; i < root->children_size; ++i)
		{
			auto node = root->first + i;
			auto val = childPotential(node, side, log_t);
			if (val > best || !best_node)
			{
				best = val;
				best_node = node;
			}
		}

		root = best_node;
	}

	return root;
}


Node* chose_best_node(Node* root)
{
	auto side = root->board.nextMoveSide;

	double best = -999;
	Node* best_node = nullptr;
	for (int i = 0; i < root->children_size; ++i)
	{
		auto node = root->first + i;
		auto val = childValue(node, side);
		if (val > best || !best_node)
		{
			best = val;
			best_node = node;
		}
	}

	return best_node;
}

void playout_leaf(Node* node, int n)
{
	if (!has_moves(node->board))
	{
		const int BIG_NUM = 100;
		if (node->board.availableMiniBoard == X)
		{
			node->stat.X_wins += BIG_NUM * n;
		}
		else
		{
			// count draw as a win for O
			node->stat.O_wins += BIG_NUM * n;
		}
	}
	else
	{
		node->stat = playout(node->board, n);
	}
}

void backpropagate(Node* node, GameStat stat)
{
	while (node)
	{
		node->stat.X_wins += stat.X_wins;
		node->stat.O_wins += stat.O_wins;
		node = node->parent;
	}
}

GameStat generate_and_playout_children_for_empty_board(Node* node, int playouts)
{
	GameStat result;

	vector<Move> moves = { 
		{ 0, 0 }, { 0, 1 }, { 0, 4 },
		{ 1, 0 }, { 1, 1 }, { 1, 4 },
		{ 4, 0 }, { 4, 1 }, { 4, 4 } };
	auto next_node = allocate_nodes(moves.size());
	node->first = next_node;
	node->children_size = moves.size();
	const auto side = node->board.nextMoveSide;
	for (auto m : moves)
	{
		auto n = next_node++;

		n->parent = node;
		n->move = m;
		n->board = node->board;
		auto& b = n->board;
		b.apply_move(m);
		playout_leaf(n, playouts);
		result.X_wins += n->stat.X_wins;
		result.O_wins += n->stat.O_wins;
	}
	return result;
}

GameStat generate_and_playout_children(Node* node, int playouts)
{
	GameStat result;

	auto moves = node->board.legal_moves();
	auto next_node = allocate_nodes(moves.size());
	node->first = next_node;
	node->children_size = moves.size();
	const auto side = node->board.nextMoveSide;
	for (auto m : moves)
	{
		auto n = next_node++;

		n->parent = node;
		n->move = m;
		n->board = node->board;
		auto& b = n->board;
		b.apply_move(m);
		playout_leaf(n, playouts);
		result.X_wins += n->stat.X_wins;
		result.O_wins += n->stat.O_wins;
	}
	return result;
}

void travel_tree_and_playout(Node* root)
{
	auto leaf = find_leaf_node(root);
	auto stat = generate_and_playout_children(leaf, 10);
	backpropagate(leaf, stat);
}

void playout_empty_board(Node* root)
{
	auto stat = generate_and_playout_children_for_empty_board(root, 10);
	backpropagate(root, stat);
}

Node* find_node_by_move(Node* root, Move m)
{
	if (root->children_size == 0)
	{
		generate_and_playout_children(root, 1);
	}
	for (int i = 0; i < root->children_size; ++i)
	{
		auto node = root->first + i;
		if (node->move == m)
			return node;
	}

	auto board = root->board;
	board.apply_move(m);
	root = make_root_node(board);

	cerr << "Start withfresh node" << endl;

	return root;
}


int main() {
	vector<vector<int>> fields(10, vector<int>(10, 0));

	Node* root = nullptr;

	// game loop
	while (1) {
		int opponentRow;
		int opponentCol;
		cin >> opponentRow >> opponentCol;
		skip_valid_actions();

		if (root)
		{
			auto opponentMove = make_move(opponentRow, opponentCol);
			root = find_node_by_move(root, opponentMove);
			root->parent = nullptr;
		}
		else
		{
			Board b;
			initialBoard(b);

			if (opponentCol != -1 && opponentRow != -1) {
				auto opponentMove = make_move(opponentRow, opponentCol);
				b.apply_move(opponentMove);
			}

			root = make_root_node(b);

			if (opponentCol != -1 && opponentRow != -1) {
				playout_empty_board(root);
			}
		}

		cerr << "node has " << (root->stat.O_wins + root->stat.X_wins) << " plays\n";

		for (int i = 0; i < 10; ++i)
		{
			travel_tree_and_playout(root);
		}

		root = chose_best_node(root);
		root->parent = nullptr;

		auto res = get_row_and_col(root->move);
		cout << res.first << " " << res.second << endl;
	}
}
#pragma once

#include "all_headers.h"

struct TicTacToe{
	virtual bool init(int seed, int time_limit_ms){ return false; }
	virtual bool set(int col, int row, char symbol){ return false; }
	virtual std::pair<int, int> move(int col, int row) = 0;
};

struct TicTacToe2{
	virtual bool init(int seed, int time_limit_ms){ return false; };
	virtual bool set(int big, int small, char symbol){ return false; }
	virtual std::pair<int, int> move(int big, int small) = 0;
};

using TicTacToeFactory = std::function < std::shared_ptr<TicTacToe>() >;
using TicTacToeFactory2 = std::function < std::shared_ptr<TicTacToe2>() >;

extern TicTacToeFactory2 make_player1;
extern TicTacToeFactory2 make_player2;

template<typename From, typename To>
struct Converter : To
{
	Converter(std::shared_ptr<From> base) :base(std::move(base)){}

	std::pair<int, int> convert(std::pair<int, int> v){
		return{ v.first / 3 * 3 + v.second / 3, v.first % 3 * 3 + v.second % 3 };
	}

	void convert(int& a, int& b) { std::tie(a, b) = convert({ a, b }); }

	bool init(int seed, int time_limit_ms) override { return base->init(seed, time_limit_ms); };
	bool set(int a, int b, char symbol) override
	{
		return convert(a, b), base->set(a, b, symbol);
	}
	std::pair<int, int> move(int a, int b) override {
		if (a != -1 && b != -1)
			convert(a, b);
		return convert(base->move(a, b));
	};

	std::shared_ptr<From> base;
};

//inline TicTacToeFactory2 convert(TicTacToeFactory f){
//	return[f](){
//		return std::make_shared<Converter<TicTacToe, TicTacToe2>>(f());
//	};
//}

inline TicTacToeFactory2 convert(TicTacToeFactory2 f){ return f; }

class Game
{
public:
	void set_players(std::shared_ptr<TicTacToe2> x, std::shared_ptr<TicTacToe2> o){
		player_x = x;
		player_o = o;
	}

	char play_game()
	{
		using namespace std;
		int a = -1, b = -1;
		char side1 = 'X', side2 = 'O';
		auto player1 = player_x, player2 = player_o;
		while (!is_game_over())
		{
			tie(a, b) = player1->move(a, b);
			cout << side1 << ": " << a << " " << b << endl;
			if (!is_valid_move(a, b)){
				cout << "Invalid move " << a << b << endl;
				return side2;
			}
			apply_move(a, b);
			print_board();
			swap(player1, player2);
			swap(side1, side2);
		}
		return get_winner();
	}

private:
	virtual bool is_valid_move(int a, int b) const = 0;
	virtual void apply_move(int a, int b) = 0;
	virtual bool is_game_over() const = 0;
	virtual char get_winner() const = 0;
	virtual void print_board() const = 0;

	std::shared_ptr<TicTacToe2> player_x;
	std::shared_ptr<TicTacToe2> player_o;
};

std::shared_ptr<Game> make_game();

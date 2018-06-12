#include "tictactoe.h"

int main()
{
	auto game = make_game();
	auto x = make_player1();
	auto o = make_player2();
	game->set_players(x, o);
	auto winner = game->play_game();
	std::cout << "Winner " << winner << std::endl;
}
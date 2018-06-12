#include "tictactoe.h"

char play(int seed_x, int time_x, int seed_o, int time_o)
{
    auto game = make_game();
    auto x = make_player1();
    x->init(seed_x, time_x);
    auto o = make_player2();
    o->init(seed_o, time_o);

    game->set_players(x, o);
    return game->play_game();
}

std::pair<int, int> play_n(int n, int time_x, int time_o)
{
    int x_wins = 0, o_wins = 0;
    for (int i = 0; i < n; ++i)
    {
        auto winner = play(1000 + i, time_x, 2000 + i, time_o);
        if (winner == 'X') x_wins++;
        else if (winner == 'O') o_wins++;
    }
    return { x_wins, o_wins };
}

int main()
{
    auto res = play_n(100, 50, 50);
    std::cout << "X: " << res.first << "\nO: " << res.second << "\n:DRAW: " << (100 - res.first - res.second) << "\n";
}
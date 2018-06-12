#include "tictactoe.h"

char play(int seed_x, int time_x, int seed_o, int time_o, bool swap_x_o_players)
{
    auto game = make_game();
    auto x = make_latest_player();
    x->init(seed_x, time_x);
    auto o = make_player_v_1_0();
    o->init(seed_o, time_o);

    if (swap_x_o_players) game->set_players(o, x);
    else game->set_players(x, o);

    return game->play_game();
}

std::pair<int, int> play_n(int n, int time_x, int time_o, bool swap)
{
    int x_wins = 0, o_wins = 0;
    for (int i = 0; i < n; ++i)
    {
        auto winner = play(1000 + i, time_x, 2000 + i, time_o, swap);
        if (winner == 'X') x_wins++;
        else if (winner == 'O') o_wins++;
    }
    return { x_wins, o_wins };
}

void play_direct_and_swapped_games(int n, int time_current, int time_v_1_0)
{
    auto res = play_n(n / 2, time_current, time_v_1_0, false);
    auto res2 = play_n(n / 2, time_current, time_v_1_0, true);
    std::cout << "X - latest, O - v.1.0\n";
    std::cout << "X: " << res.first << "\nO: " << res.second << "\n:DRAW: " << (n - res.first - res.second) << "\n";
    std::cout << "O - latest, X - v.1.0\n";
    std::cout << "X: " << res2.first << "\nO: " << res2.second << "\n:DRAW: " << (n - res2.first - res2.second) << "\n";
}

int main()
{
    play_direct_and_swapped_games(100, 50, 50);
}
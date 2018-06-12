#include "all_headers.h"

#include "tictactoe.h"

namespace app_01{
#include "players/player.cpp"
}

namespace app_02{
#include "players/player.cpp"
}

::app_01::Player pl;

TicTacToeFactory2 make_player1 = convert([](){ return std::make_shared<::app_01::Player>(); });
TicTacToeFactory2 make_player2 = convert([](){ return std::make_shared<::app_02::Player>(); });



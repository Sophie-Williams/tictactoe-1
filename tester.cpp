#include "all_headers.h"

#include "tictactoe.h"

namespace app_latest{
#include "players/player.cpp"
}

namespace app_v_1_0{
#include "players/player_v.1.0.cpp"
}

TicTacToeFactory2 make_latest_player = convert([](){ return std::make_shared<::app_latest::Player>(); });
TicTacToeFactory2 make_player_v_1_0 = convert([](){ return std::make_shared<::app_v_1_0::Player>(); });



#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
*
* 1 2 3
* 4 5 6
* 7 8 9
*
**/

const vector<int> goals = { 123, 456, 789, 147, 258, 369, 159, 357 };


const vector<int> coords = { -1, 00, 01, 02, 10, 11, 12, 20, 21, 22 };

int get_index(int x, int y)
{
	return (std::find(coords.begin(), coords.end(), (x % 3) * 10 + (y % 3)) - coords.begin()) % coords.size();
}

int get_field_index(int x, int y)
{
	return get_index(x / 3, y / 3);
}

int row_of(int r, int mod) { return (r / mod) % 10; }

vector<int> get_row(int r) {
	vector<int> res;
	res.push_back(row_of(r, 1));
	res.push_back(row_of(r, 10));
	res.push_back(row_of(r, 100));
	return res;
}

int get_my_move(const vector<int>& field)
{
	auto is_done = [&field](int side)
	{
		for (auto row : goals)
		{
			int sum = 0;
			auto r = get_row(row);
			for (auto room : r)
				sum += field[room] == side;
			if (sum == 3) return true;
		}
		return false;
	};

	if (is_done(1) || is_done(2)) return 0;


	if (field[5] == 0) return 5;

	auto finish_three = [&field](int side)
	{
		for (auto row : goals)
		{
			int sum = 0;
			auto r = get_row(row);
			for (auto room : r)
				sum += field[room] == side;
			if (sum == 2)
				for (int room : r)
					if (field[room] == 0) return room;
		}
		return 0;
	};

	int val = finish_three(1);
	if (val != 0) return val;

	val = finish_three(2);
	if (val != 0) return val;

	for (int corner : {1, 3, 7, 9})
		if (field[corner] == 0) return corner;

	for (int side : {2, 4, 6, 8})
		if (field[side] == 0) return side;

	return 0;
}

int main()
{
	vector<vector<int>> fields(10, vector<int>(10, 0));

	// game loop
	while (1) {
		int opponentRow;
		int opponentCol;
		cin >> opponentRow >> opponentCol; cin.ignore();

		int validActionCount;
		cin >> validActionCount; cin.ignore();
		int row = 1;
		int col = 1;
		for (int i = 0; i < validActionCount; i++) {
			cin >> row >> col; cin.ignore();
		}

		int played_field = get_field_index(opponentRow, opponentCol);
		fields[played_field][get_index(opponentRow, opponentCol)] = 2;

		int field_to_play = get_index(opponentRow, opponentCol);
		if (field_to_play == 0)
			field_to_play = 5;

		auto field = &fields[field_to_play];

		cerr << "Field: " << field_to_play << std::endl;

		//        field[get_index(opponentRow, opponentCol)] = 2;
		int my_move = get_my_move(*field);

		if (my_move == 0)
		{
			for (int f : {5, 1, 3, 7, 9, 2, 4, 6, 8})
			{
				field_to_play = f;
				field = &fields[f];
				my_move = get_my_move(*field);
				if (my_move != 0) break;
			}
			cerr << "change field to " << field_to_play << std::endl;
		}

		for (int row : {0, 1, 2})
		{
			for (int col : {0, 1, 2})
				cerr << (*field)[row * 3 + col][".Xo"];
			cerr << std::endl;
		}

		(*field)[my_move] = 1;

		cerr << "my move(field->" << field_to_play << " cell->" << my_move << std::endl;

		int f = coords[field_to_play];
		int c = coords[my_move];
		// Write an action using cout. DON'T FORGET THE "<< endl"
		// To debug: cerr << "Debug messages..." << endl;

		cout << ((f / 10) * 3 + (c / 10)) << " " << ((f % 10) * 3 + (c % 10)) << endl;
	}
}
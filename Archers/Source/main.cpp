#include "Game.h"

ArchersGame* game_instance = nullptr;

int main(int argc, char* argv[])
{
    game_instance = new ArchersGame();
    if (!game_instance->Prepare())
        return -1;

    game_instance->SetupEvents();
    game_instance->GameCycle();
    delete game_instance;

	return 0;
}
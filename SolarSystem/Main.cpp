#include "Game.h"

int main()
{
    try
    {
        srand(time(0));
        mc::Game game;
        game.Run();
    }
    catch (std::exception& e)
    {
        // TODO: show the message in a message box
        std::cout << "Error: " << e.what() << "\n";
    }
    return 0;
}
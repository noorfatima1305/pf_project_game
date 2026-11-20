#include <SFML/Graphics.hpp>
#include <optional>
#include<fstream>
using namespace std;
const int ROWS = 21, COLS = 19; const float TILE = 30.f, HUD_HEIGHT = 50.f;
char maze[21][19];
int  playerrow, playercol, ghoststartrow[4], ghoststartcol[4], pelletsleft = 0, ghostcount = 0;
sf::Font font;

bool loadmaze()
{
    ifstream m("maze.txt");
    if (!m) { return false; }
    int r, c;
    m >> r >> c;
    for (int i = 0;i < ROWS;i++)
    {
        for (int j = 0;j < COLS;j++)
        {
            m >> maze[i][j];
            if (maze[i][j] == 'o' || maze[i][j] == '.') pelletsleft++;
            if (maze[i][j] == 'P') { playerrow = i; playercol = j;  maze[i][j] = '-'; }
            if (maze[i][j] == 'G') { ghoststartrow[ghostcount++] = i; ghoststartcol[ghostcount++] = j; ghostcount++;  maze[i][j] = '-'; }
        }
    }
    m.close();
    return true;
}
void handleInput(sf::RenderWindow& window)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
}

void updateGame() {}   // empty for now

void drawMaze(sf::RenderWindow& window)
{
    sf::RectangleShape wall({ TILE, TILE });
    wall.setFillColor(sf::Color(33, 33, 222));
    sf::CircleShape pellet(3.f);
    pellet.setFillColor(sf::Color(255, 220, 180));
    sf::CircleShape power(8.f);
    power.setFillColor(sf::Color(255, 220, 180));

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            float x = c * TILE;
            float y = r * TILE + HUD_HEIGHT;
            if (maze[r][c] == '#')
            {
                wall.setPosition({ x, y });
                window.draw(wall);
            }
            else if (maze[r][c] == '.')
            {
                pellet.setPosition({ x + TILE / 2 - 3.f, y + TILE / 2 - 3.f });
                window.draw(pellet);
            }
            else if (maze[r][c] == 'o')
            {
                power.setPosition({ x + TILE / 2 - 8.f, y + TILE / 2 - 8.f });
                window.draw(power);
            }
        }
    }
}

void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(10, 10, 30));
    drawMaze(window);
    window.display();
}

void drawError(sf::RenderWindow& window, sf::Font& font)
{
    sf::Text text(font, "Error: maze.txt not found", 24);
    text.setFillColor(sf::Color::Red);
    text.setPosition({ 60.f, 320.f });
    window.clear(sf::Color::Black);
    window.draw(text);
    window.display();
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);


    if (!font.openFromFile("arial.ttf"))
        return 1;
    bool mazeOk = loadmaze();

    while (window.isOpen())
    {
        handleInput(window);
        if (mazeOk)
        {
            updateGame();
            drawGame(window);
        }
        else
            drawError(window, font);
    }
    return 0;
}






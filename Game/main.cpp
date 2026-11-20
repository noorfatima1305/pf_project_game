#include <SFML/Graphics.hpp>
#include <optional>
#include<fstream>
using namespace std;
const int ROWS = 21, COLS = 19; const float TILE = 30.f, HUD_HEIGHT = 50.f;const int PLAYER_SPEED = 8;
char maze[21][19];
int  playerrow, playercol, ghoststartrow[4], ghoststartcol[4], pelletsleft = 0, ghostcount = 0, keydir = 0, mov_timer = 0, scores = 0;
int playerdir = 0; int lives = 3; 
sf::Font font;
void playermovement()
{
            mov_timer++;
            if (mov_timer < PLAYER_SPEED) return;
            else mov_timer = 0;
            if (keydir != 0) 
            {
                if(keydir==1 && maze[playerrow-1][playercol]!='#')  playerdir = keydir;
                if (keydir == 2 && maze[playerrow + 1][playercol] != '#')  playerdir = keydir;
                if (keydir == 3 && maze[playerrow ][playercol-1] != '#')  playerdir = keydir;
                if (keydir == 4 && maze[playerrow ][playercol+1] != '#')  playerdir = keydir;

            }
            int newrow = playerrow; int newcol = playercol;
            if (playerdir == 1) { newrow -= 1; }
            else if (playerdir == 2) { newrow += 1; }
            else if (playerdir == 3) { newcol -= 1; }
            else if (playerdir == 4) { newcol += 1; }
            if (maze[newrow][newcol] != '#')
            {
                playerrow = newrow;
                playercol = newcol;
                if (maze[playerrow][playercol] == 'o') { scores += 50; pelletsleft--; maze[playerrow][playercol] = '-'; }
                else if (maze[playerrow][playercol] == '.') { scores += 10; pelletsleft--; maze[playerrow][playercol] = '-'; }
            }
}
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
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
                keydir = 1;
            else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
                keydir = 2;
            else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A)
                keydir = 3;
            else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D)
                keydir = 4;
        }
    }
}
void drawPlayer(sf::RenderWindow& window)
{
    sf::CircleShape p(12.f);
    p.setFillColor(sf::Color(255, 220, 0));
    p.setPosition({ playercol * TILE + 3.f, playerrow * TILE + HUD_HEIGHT + 3.f });
    window.draw(p);
}

void drawHUD(sf::RenderWindow& window)
{
    sf::Text s(font, "SCORE " + std::to_string(scores), 22);
    s.setPosition({ 15.f, 12.f });
    window.draw(s);

    sf::Text l(font, "LIVES " + std::to_string(lives), 22);
    l.setPosition({ 420.f, 12.f });
    window.draw(l);
}

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
    drawPlayer(window);
    drawHUD(window);
    window.display();
}

void updateGame() 
{
    playermovement();
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






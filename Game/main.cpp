#include <SFML/Graphics.hpp>
#include <optional>
#include<fstream>
#include <cstdlib>
#include <ctime>
using namespace std;
const int ROWS = 21, COLS = 19; const float TILE = 30.f, HUD_HEIGHT = 50.f;const int PLAYER_SPEED = 8;
char maze[21][19];
int  playerrow, playercol, ghoststartrow[4], ghoststartcol[4], pelletsleft = 0, ghostcount = 0, keydir = 0, mov_timer = 0, scores = 0;
int playerdir = 0; int lives = 3; 
const int GHOSTSPEED = 10; 
int ghostdir[4], ghostrow[4], ghostcol[4];
//for blue ghost
bool blueghost[4]; int playerstartrow, playerstartcol; int bluetimer, ghosttimer[4] = { 0 };

int oldplayerrow, oldplayercol, oldghostrow[4], oldghostcol[4]; // [3, 4]
int gamestate = 1; // [4]

const int BLUETIME = 300, BLUESPEED = 16;
sf::Font font;


void startghosts()
{
    for (int i = 0; i < 4; i++)
    {
        ghostrow[i] = ghoststartrow[i];
        ghostcol[i] = ghoststartcol[i];
        ghostdir[i] = 0;
    }
}

void ghostmovement()
{

    for (int i = 0; i < 4; i++)
    {
        //ghost speed
        ghosttimer[i]++;
        if (blueghost[i])
        {
            if (ghosttimer[i] < BLUESPEED) continue; else ghosttimer[i] = 0;
        }
        else
        {
            if (ghosttimer[i] < GHOSTSPEED) continue; else ghosttimer[i] = 0;
        }
        int countdir = 0, options[4];
        if (maze[ghostrow[i] - 1][ghostcol[i]] != '#' && ghostdir[i] != 2) options[countdir++] = 1;
        if (maze[ghostrow[i]][ghostcol[i]-1] != '#' && ghostdir[i] != 1) options[countdir++] = 3;
        if (maze[ghostrow[i]+1][ghostcol[i]] != '#' && ghostdir[i] != 4) options[countdir++] = 2;
        if (maze[ghostrow[i]][ghostcol[i] + 1] != '#' && ghostdir[i] != 3) options[countdir++] = 4;
        int bestdistance = 3200, bestdir = 0;

        if (countdir == 0)
        {
        //deadend
            if (ghostdir[i] == 1) ghostdir[i] = 2;
            else if (ghostdir[i] == 2) ghostdir[i] = 1;
            else if (ghostdir[i] == 3) ghostdir[i] = 4;
            else if (ghostdir[i] == 4) ghostdir[i] = 3;
        }
        //chasers
        else if ((i == 0 || i == 2) && !blueghost[i]) {
            for (int j = 0; j < countdir;j++) {
                int r = ghostrow[i], c = ghostcol[i];
                if (options[j] == 1) r -= 1;
                else if (options[j] == 2) r += 1;
                else if (options[j] == 3) c -= 1;
                else if (options[j] == 4) c += 1;

                int d = abs(r - playerrow) + abs(c - playercol);
                if (d < bestdistance) { bestdistance = d; bestdir = options[j]; }

            }
            ghostdir[i] = bestdir;
        }
        else ghostdir[i] = options[rand() % countdir];

        if (ghostdir[i] == 1) ghostrow[i] -= 1;
        else if (ghostdir[i] == 2) ghostrow[i] += 1;
        else if (ghostdir[i] == 3) ghostcol[i] -= 1;
        else if (ghostdir[i] == 4) ghostcol[i] += 1;
    }
}

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
                if (maze[playerrow][playercol] == 'o') { scores += 50; pelletsleft--; maze[playerrow][playercol] = '-';
                for (int i = 0;i < 4;i++) blueghost[i] = 1; bluetimer = BLUETIME;
                }
                else if (maze[playerrow][playercol] == '.') { scores += 10; pelletsleft--; maze[playerrow][playercol] = '-'; }
            }
}
bool loadmaze()
{
    pelletsleft = 0;
    ghostcount = 0;
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
            if (maze[i][j] == 'P') {
                playerrow = i;
                playercol = j;
                playerstartrow = i;
                playerstartcol = j;
                maze[i][j] = '-';
            }
            
            if (maze[i][j] == 'G') { ghoststartrow[ghostcount] = i; ghoststartcol[ghostcount] = j; ghostcount++; maze[i][j] = '-'; }
            
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


void drawGhosts(sf::RenderWindow& window)
{
    sf::Color colors[4] = { sf::Color(255, 60, 80), sf::Color(255, 150, 220),
                            sf::Color(60, 220, 255), sf::Color(255, 160, 30) };
    for (int i = 0; i < 4; i++)
    {
        float x = ghostcol[i] * TILE + 3.f;
        float y = ghostrow[i] * TILE + HUD_HEIGHT + 3.f;
        sf::Color c = colors[i];
        if (blueghost[i]) c = sf::Color(40, 40, 255);
        sf::CircleShape head(12.f);
        head.setFillColor(c);
        head.setPosition({ x, y });
        window.draw(head);

        sf::RectangleShape feet({ 24.f, 12.f });
        feet.setFillColor(c);
        feet.setPosition({ x, y + 12.f });
        window.draw(feet);

    
        for (int e = 0; e < 2; e++)
        {
            float ex = x + 4.f + e * 9.f;
            float ey = y + 6.f;

            sf::CircleShape eye(4.f);
            eye.setFillColor(sf::Color::White);
            eye.setPosition({ ex, ey });
            window.draw(eye);

            sf::CircleShape pupil(2.f);
            pupil.setFillColor(sf::Color(30, 30, 200));
            pupil.setPosition({ ex + 2.f, ey + 2.f });
            window.draw(pupil);
        }
    }
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
    drawGhosts(window);
    drawHUD(window);
    window.display();
}
void checkcollision() 
{
    for (int i = 0; i < 4; i++) 
    {
        if ((ghostrow[i] == playerrow && ghostcol[i] == playercol) ||
            (oldghostrow[i] == playerrow && oldghostcol[i] == playercol &&
                oldplayerrow == ghostrow[i] && oldplayercol == ghostcol[i])) 
        {
            
            if (blueghost[i]) 
            {
                scores += 200; 
                ghostrow[i] = ghoststartrow[i]; 
                ghostcol[i] = ghoststartcol[i];
                ghostdir[i] = 0; 
                blueghost[i] = 0; 
            }
          
            else
            {
                lives--; 
                playerrow = playerstartrow; 
                playercol = playerstartcol; 
                oldplayerrow = playerrow; 
                oldplayercol = playercol; 
                for (int j = 0; j < 4; j++) 
                {
                    ghostrow[j] = ghoststartrow[j]; 
                    ghostcol[j] = ghoststartcol[j]; 
                    blueghost[j] = 0; 
                    ghosttimer[j] = 0; 
                    oldghostrow[j] = ghostrow[j];
                    oldghostcol[j] = ghostcol[j]; 
                    ghostdir[j] = 0; 
                    bluetimer = 0; 
                }
                return; 
            }
        }
    }
}

void updateGame() 
{
    oldplayerrow = playerrow;
    oldplayercol = playercol;
    for (int i = 0; i < 4; i++) {
        oldghostrow[i] = ghostrow[i];
        oldghostcol[i] = ghostcol[i];
    }
   
    playermovement();
    if (bluetimer > 0)
    {
        bluetimer--;
        if (bluetimer == 0) { for (int i = 0;i < 4;i++) blueghost[i] = 0; }
    }
    ghostmovement();
    checkcollision();
    
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
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);


    if (!font.openFromFile("arial.ttf"))
        return 1;
    bool mazeOk = loadmaze();
    startghosts();

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






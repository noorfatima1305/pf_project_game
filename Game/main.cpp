#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include<fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>
using namespace std;
const int ROWS = 21, COLS = 19; const float TILE = 30.f, HUD_HEIGHT = 50.f;const int PLAYER_SPEED = 8;
char maze[ROWS][COLS];
int  playerrow, playercol, ghoststartrow[4], ghoststartcol[4], pelletsleft = 0, ghostcount = 0, keydir = 0, mov_timer = 0, scores = 0;
int playerdir = 0; int lives = 3; 
const int GHOSTSPEED = 10; 
int ghostdir[4], ghostrow[4], ghostcol[4];
//for blue ghost
bool blueghost[4]; int playerstartrow, playerstartcol; int bluetimer, ghosttimer[4] = { 0 };
//for collisions
int oldplayerrow, oldplayercol, oldghostrow[4], oldghostcol[4]; 
//for game state
const int MENU = 0, PLAYING = 1, GAME_OVER = 2;
int gamestate = MENU;
int menuchoice = 0;        
bool playerwon = false;

// for saved game
bool nosave = false;


//for bonus task
int dist[ROWS][COLS];
bool showdist = 0;
int lastdistrow = -1, lastdistcol = -1;


const int BLUETIME = 300, BLUESPEED = 16;
sf::Font font;

// sound effects 
sf::SoundBuffer chompBuf, powerBuf, eatGhostBuf, deathBuf, winBuf, gameOverBuf, startBuf, ghostWinBuf;
std::optional<sf::Sound> chompSound, powerSound, eatGhostSound, deathSound, winSound, gameOverSound, startSound, ghostWinSound;

void loadSounds()
{
    if (chompBuf.loadFromFile("chomp.wav"))         chompSound.emplace(chompBuf);
    if (powerBuf.loadFromFile("power.wav"))         powerSound.emplace(powerBuf);
    if (eatGhostBuf.loadFromFile("eatghost.wav"))   eatGhostSound.emplace(eatGhostBuf);
    if (deathBuf.loadFromFile("death.wav"))         deathSound.emplace(deathBuf);
    if (winBuf.loadFromFile("win.wav"))             winSound.emplace(winBuf);
    if (gameOverBuf.loadFromFile("gameover.wav"))   gameOverSound.emplace(gameOverBuf);
    if (startBuf.loadFromFile("start.wav"))         startSound.emplace(startBuf);
    if (ghostWinBuf.loadFromFile("ghostwin.wav"))   ghostWinSound.emplace(ghostWinBuf);
}

void playSound(std::optional<sf::Sound>& sfx, bool avoidOverlap = false)
{
    if (!sfx) return;
    if (avoidOverlap && sfx->getStatus() == sf::Sound::Status::Playing) return;
    sfx->play();
}

//for saved game
bool savegame()
{
    ofstream s("savegame.txt");
    if (!s) return 0;
    s << scores << "  " << lives << endl;
    s << playerrow << "  " << playercol << endl;
    for (int i = 0;i < 4;i++)
    {
        s << ghostrow[i] << "  " << ghostcol[i] << " ";
    }
    s << endl;
    for (int i = 0;i < ROWS;i++)
    {
        for (int j = 0;j < COLS;j++)
            s << maze[i][j] ;
        s << endl;
    }
    s.close();
    return 1;


}
bool loadgame()
{
    ifstream s("savegame.txt");
    if (!s) return 0;
    if (!(s >> scores)) return false;
    s >> lives;
    s >> playerrow >> playercol;
    for (int i = 0;i < 4;i++)
    {
        s >> ghostrow[i];
        s >> ghostcol[i];
    }
    pelletsleft = 0;
    for (int i = 0;i < ROWS;i++)
    {
        for (int j = 0;j < COLS;j++)
        {
            s >> maze[i][j];
            if (maze[i][j] == 'o' || maze[i][j] == '.') pelletsleft++;
          
        }
    }
    playerdir = 0, keydir = 0, mov_timer = 0, bluetimer = 0;
    for (int i = 0;i < 4;i++) {
        ghostdir[i] = 0, ghosttimer[i] = 0, blueghost[i] = 0;
    }
    lastdistrow = -1, lastdistcol = -1;
    return  1;
}
void clearsave()
{
    ofstream s("savegame.txt");
    s.close();
}

//for bonus
void distancemap()
{
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            dist[i][j] = 999;

    dist[playerrow][playercol] = 0;
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int r = 0; r < ROWS; r++)
        {
            for (int c = 0; c < COLS; c++)
            {
                if (maze[r][c] == '#') continue;
                else 
                {
                    if (maze[r - 1][c] != '#' && dist[r - 1][c] + 1 < dist[r][c]) { dist[r][c] = dist[r - 1][c] + 1; changed = true; }
                    if (maze[r + 1][c] != '#' && dist[r + 1][c] + 1 < dist[r][c]) { dist[r][c] = dist[r + 1][c] + 1; changed = true; }
                    if (maze[r][c - 1] != '#' && dist[r][c - 1] + 1 < dist[r][c]) { dist[r][c] = dist[r][c - 1] + 1; changed = true; }
                    if (maze[r][c + 1] != '#' && dist[r][c + 1] + 1 < dist[r][c]) { dist[r][c] = dist[r][c + 1] + 1; changed = true; }
                }
            }
        }
    }
   
}


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
        if (maze[ghostrow[i]][ghostcol[i]-1] != '#' && ghostdir[i] != 4) options[countdir++] = 3;
        if (maze[ghostrow[i]+1][ghostcol[i]] != '#' && ghostdir[i] != 1) options[countdir++] = 2;
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

                int d;
                if (i == 0) d = dist[r][c];
                else d = abs(r - playerrow) + abs(c - playercol);
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
//for players
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
                keydir = 0;
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
                for (int i = 0;i < 4;i++) blueghost[i] = 1; bluetimer = BLUETIME; playSound(powerSound);
                }
                else if (maze[playerrow][playercol] == '.') { scores += 10; pelletsleft--; maze[playerrow][playercol] = '-'; 
                playSound(chompSound, true);
                }
            }
}
//for loading maze
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
//new game
void newgame()
{
    loadmaze();
    startghosts();
    lastdistrow = -1;
    gamestate = PLAYING;
    keydir = 0, mov_timer = 0, scores = 0;
    playerdir = 0; lives = 3;
    bluetimer = 0;
    for(int i=0;i<4;i++)
    { blueghost[i] = 0;
    ghosttimer[i] = 0;
    
}
    nosave = false;
    playSound(startSound);
}
//checking collision
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
                playSound(eatGhostSound);
            }

            else
            {
                lives--;
                playSound(deathSound);
                lastdistrow = -1, lastdistcol = -1;
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
                    playerdir = 0;
                    keydir = 0;
                }
                return;
            }
        }
    }
}
//input handling
void handleInput(sf::RenderWindow& window)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            switch (gamestate)
            {
            case MENU:
                if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
                { 
                    menuchoice = (menuchoice + 2) % 3;
                    nosave = false;

                }
                else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
                {
                    menuchoice = (menuchoice + 1 )% 3;
                    nosave = false;
                }
                else if (key->code == sf::Keyboard::Key::Enter)
                { 
                    if (menuchoice == 0)
                        newgame();
                    else if (menuchoice == 1)
                    {
                      bool b=  loadgame();
                      if (b)
                      {
                          nosave = 0;
                          gamestate = PLAYING;
                          playSound(startSound);
                      }
                      else
                      {
                          nosave = 1;
                      }
                    }
                    else if(menuchoice==2)
                    window.close();
                }
                break;
            case PLAYING:
                if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
                    keydir = 1;
                else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
                    keydir = 2;
                else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A)
                    keydir = 3;
                else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D)
                    keydir = 4;

                else if (key->code == sf::Keyboard::Key::Escape)
                {
                    savegame();
                    gamestate = MENU;
                    menuchoice = 0;
                    nosave = 0;
                }

                else if (key->code == sf::Keyboard::Key::M)
                    showdist = !showdist;

                break;
            case GAME_OVER:
                if (key->code == sf::Keyboard::Key::Enter)
                {
                    gamestate = MENU;
                    menuchoice = 0;
                }
                break;
            }
        }
    }
}

//drawing codes



void drawDistances(sf::RenderWindow& window)
{
    sf::Text t(font, "", 12);
    t.setFillColor(sf::Color(120, 255, 160));
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (maze[r][c] != '#' && dist[r][c] < 999)
            {
                t.setString(to_string(dist[r][c]));
                t.setPosition({ c * TILE + 3.f, r * TILE + HUD_HEIGHT + 6.f });
                window.draw(t);
            }
        }
    }
}



void drawPlayer(sf::RenderWindow& window)
{
    const float cx = playercol * TILE + TILE / 2.f;
    const float cy = playerrow * TILE + HUD_HEIGHT + TILE / 2.f;
    const float radius = 12.f;

    // Mouth opens and closes while moving; stays gently open when idle
    float mouthPhase = (playerdir != 0) ? (0.5f + 0.5f * std::sin(static_cast<float>(mov_timer) * 0.85f)) : 0.35f;
    float mouthHalfAngle = mouthPhase * 34.f;

    // 0 = default right, 1 = up, 2 = down, 3 = left, 4 = right
    float facingDeg = 0.f;
    if (playerdir == 1)      facingDeg = -90.f;
    else if (playerdir == 2) facingDeg = 90.f;
    else if (playerdir == 3) facingDeg = 180.f;
    else if (playerdir == 4) facingDeg = 0.f;

    const int segments = 28;
    sf::ConvexShape pac;
    pac.setPointCount(static_cast<std::size_t>(segments + 2));
    pac.setFillColor(sf::Color(255, 220, 0));
    pac.setPoint(0, { cx, cy });

    const float pi = 3.14159265f;
    float startDeg = facingDeg + mouthHalfAngle;
    float endDeg = facingDeg + 360.f - mouthHalfAngle;

    for (int i = 0; i <= segments; i++)
    {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        float deg = startDeg + t * (endDeg - startDeg);
        float rad = deg * pi / 180.f;
        pac.setPoint(static_cast<std::size_t>(i + 1), {
            cx + radius * std::cos(rad),
            cy + radius * std::sin(rad)
            });
    }
    window.draw(pac);

    // Subtle dark eye positioned cleanly above the mouth
    float eyeX = (playerdir == 3) ? cx - 2.f : cx - 1.f;
    float eyeY = cy - 6.5f;
    sf::CircleShape eye(2.f);
    eye.setFillColor(sf::Color(25, 22, 10));
    eye.setPosition({ eyeX, eyeY });
    window.draw(eye);
}

void drawGhosts(sf::RenderWindow& window)
{
    sf::Color colors[4] = {
        sf::Color(255, 65, 85),
        sf::Color(255, 155, 220),
        sf::Color(70, 225, 255),
        sf::Color(255, 165, 40)
    };

    for (int i = 0; i < 4; i++)
    {
        float x = ghostcol[i] * TILE + 3.f;
        float y = ghostrow[i] * TILE + HUD_HEIGHT + 3.f;

        sf::Color c = colors[i];
        if (blueghost[i])
        {
            bool flashWhite = (bluetimer > 0 && bluetimer < 90 && ((bluetimer / 10) % 2 == 0));
            c = flashWhite ? sf::Color(235, 240, 255) : sf::Color(40, 60, 255);
        }

        sf::CircleShape head(12.f);
        head.setFillColor(c);
        head.setPosition({ x, y });
        window.draw(head);

        sf::RectangleShape body({ 24.f, 9.f });
        body.setFillColor(c);
        body.setPosition({ x, y + 10.f });
        window.draw(body);

        for (int b = 0; b < 3; b++)
        {
            sf::CircleShape bump(4.f);
            bump.setFillColor(c);
            bump.setPosition({ x + b * 8.f, y + 16.f });
            window.draw(bump);
        }

        float pxOff = 0.f, pyOff = 0.f;
        if (ghostdir[i] == 1)      pyOff = -1.5f;
        else if (ghostdir[i] == 2) pyOff = 1.5f;
        else if (ghostdir[i] == 3) pxOff = -1.5f;
        else if (ghostdir[i] == 4) pxOff = 1.5f;

        for (int e = 0; e < 2; e++)
        {
            float ex = x + 3.5f + e * 9.f + pxOff * 0.5f;
            float ey = y + 5.5f + pyOff * 0.5f;

            sf::CircleShape eye(4.f);
            eye.setFillColor(sf::Color::White);
            eye.setPosition({ ex, ey });
            window.draw(eye);

            sf::CircleShape pupil(2.f);
            pupil.setFillColor(sf::Color(25, 35, 180));
            pupil.setPosition({ ex + 2.f + pxOff, ey + 2.f + pyOff });
            window.draw(pupil);
        }
    }
}


void drawHUD(sf::RenderWindow& window)
{
    sf::RectangleShape hudBar({ 570.f, HUD_HEIGHT });
    hudBar.setFillColor(sf::Color(14, 18, 46));
    window.draw(hudBar);

    sf::RectangleShape hudLine({ 570.f, 2.f });
    hudLine.setFillColor(sf::Color(35, 95, 235));
    hudLine.setPosition({ 0.f, HUD_HEIGHT - 2.f });
    window.draw(hudLine);

    sf::Text s(font, "SCORE " + std::to_string(scores), 22);
    s.setFillColor(sf::Color::White);
    s.setPosition({ 16.f, 13.f });
    window.draw(s);

    sf::Text l(font, "LIVES", 22);
    l.setFillColor(sf::Color(255, 220, 0));
    l.setPosition({ 375.f, 13.f });
    window.draw(l);

    int shownLives = lives > 0 ? lives : 0;
    for (int i = 0; i < shownLives; i++)
    {
        sf::CircleShape lifeIcon(8.f);
        lifeIcon.setFillColor(sf::Color(255, 220, 0));
        lifeIcon.setPosition({ 455.f + i * 25.f, 17.f });
        window.draw(lifeIcon);
    }
}

void drawMaze(sf::RenderWindow& window)
{
    sf::RectangleShape wallOuter({ TILE, TILE });
    wallOuter.setFillColor(sf::Color(25, 95, 255));

    const float inset = 4.f;
    sf::RectangleShape wallInner({ TILE - inset * 2.f, TILE - inset * 2.f });
    wallInner.setFillColor(sf::Color(10, 14, 38));

    sf::CircleShape pellet(3.f);
    pellet.setFillColor(sf::Color(255, 225, 185));

    float pulse = 7.f + 1.5f * std::sin(static_cast<float>(mov_timer) * 0.9f);
    sf::CircleShape power(pulse);
    power.setFillColor(sf::Color(255, 215, 110));
    power.setOutlineThickness(1.5f);
    power.setOutlineColor(sf::Color(255, 150, 45));

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            float x = c * TILE;
            float y = r * TILE + HUD_HEIGHT;

            if (maze[r][c] == '#')
            {
                wallOuter.setPosition({ x, y });
                window.draw(wallOuter);

                wallInner.setPosition({ x + inset, y + inset });
                window.draw(wallInner);
            }
            else if (maze[r][c] == '.')
            {
                pellet.setPosition({ x + TILE / 2.f - 3.f, y + TILE / 2.f - 3.f });
                window.draw(pellet);
            }
            else if (maze[r][c] == 'o')
            {
                power.setPosition({ x + TILE / 2.f - pulse, y + TILE / 2.f - pulse });
                window.draw(power);
            }
        }
    }
}

void drawCentered(sf::RenderWindow& window, const string& str, unsigned int size, float y, sf::Color color)
{
    sf::Text t(font, str, size);
    t.setFillColor(color);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition({ (570.f - b.size.x) / 2.f, y });
    window.draw(t);
}

void drawMenu(sf::RenderWindow& window)
{
    // Outer neon border frame
    sf::RectangleShape frame({ 490.f, 580.f });
    frame.setFillColor(sf::Color(15, 20, 48));
    frame.setOutlineThickness(3.f);
    frame.setOutlineColor(sf::Color(40, 110, 255));
    frame.setPosition({ 40.f, 50.f });
    window.draw(frame);

    // Decorative title banner underline
    sf::RectangleShape accent({ 260.f, 4.f });
    accent.setFillColor(sf::Color(255, 190, 20));
    accent.setPosition({ (570.f - 260.f) / 2.f, 184.f });
    window.draw(accent);

    drawCentered(window, "PAC-MAN", 56, 112.f, sf::Color(255, 220, 0));

    string items[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
    {
        bool selected = (i == menuchoice);
        float boxW = 280.f;
        float boxH = 48.f;
        float boxX = (570.f - boxW) / 2.f;
        float boxY = 255.f + i * 72.f;

        sf::RectangleShape optionBox({ boxW, boxH });
        optionBox.setFillColor(selected ? sf::Color(35, 58, 125) : sf::Color(22, 28, 62));
        optionBox.setOutlineThickness(2.f);
        optionBox.setOutlineColor(selected ? sf::Color(255, 220, 0) : sf::Color(55, 75, 145));
        optionBox.setPosition({ boxX, boxY });
        window.draw(optionBox);

        sf::Color c = selected ? sf::Color(255, 220, 0) : sf::Color(215, 222, 240);
        string label = selected ? ("> " + items[i] + " <") : items[i];
        drawCentered(window, label, 26, boxY + 9.f, c);
    }

    if (nosave)
        drawCentered(window, "No saved game found", 20, 480.f, sf::Color(255, 80, 100));

    drawCentered(window, "Up/Down or W/S: Select   Enter: Confirm", 15, 552.f, sf::Color(150, 168, 210));
    drawCentered(window, "In Game: Arrows/WASD Move  |  Esc Save  |  M Map", 14, 582.f, sf::Color(115, 135, 180));
}

void drawEndScreen(sf::RenderWindow& window)
{
    sf::Color themeColor = playerwon ? sf::Color(65, 225, 130) : sf::Color(255, 70, 90);

    sf::RectangleShape card({ 450.f, 380.f });
    card.setFillColor(sf::Color(16, 21, 50));
    card.setOutlineThickness(3.f);
    card.setOutlineColor(themeColor);
    card.setPosition({ 60.f, 150.f });
    window.draw(card);

    string title = playerwon ? "YOU WIN!" : "GAME OVER";
    string subtitle = playerwon ? "All pellets cleared!" : "The ghosts caught you!";

    drawCentered(window, title, 50, 185.f, themeColor);
    drawCentered(window, subtitle, 18, 248.f, sf::Color(195, 205, 230));

    sf::RectangleShape scoreBox({ 270.f, 64.f });
    scoreBox.setFillColor(sf::Color(24, 31, 72));
    scoreBox.setOutlineThickness(2.f);
    scoreBox.setOutlineColor(sf::Color(70, 95, 175));
    scoreBox.setPosition({ (570.f - 270.f) / 2.f, 295.f });
    window.draw(scoreBox);

    drawCentered(window, "FINAL SCORE", 14, 304.f, sf::Color(155, 175, 220));
    drawCentered(window, to_string(scores), 28, 322.f, sf::Color(255, 220, 0));

    drawCentered(window, "Press Enter to return to Menu", 20, 435.f, sf::Color(225, 230, 245));
}

void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(10, 10, 30));
    switch (gamestate)
    {
    case MENU:
        drawMenu(window);
        break;
    case PLAYING:
        drawMaze(window);
        drawPlayer(window);
        drawGhosts(window);
        if (showdist) drawDistances(window);
        drawHUD(window);
        break;
    case GAME_OVER:
        drawEndScreen(window);
        break;
    }
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

//updating game
void updateGame()
{
    if (gamestate == PLAYING)
    {
        oldplayerrow = playerrow;
        oldplayercol = playercol;
        for (int i = 0; i < 4; i++) {
            oldghostrow[i] = ghostrow[i];
            oldghostcol[i] = ghostcol[i];
        }

        playermovement();
        if (lastdistrow != playerrow || lastdistcol != playercol) {
            distancemap();  lastdistrow = playerrow;
            lastdistcol = playercol;
        }
        if (bluetimer > 0)
        {
            bluetimer--;
            if (bluetimer == 0) { for (int i = 0; i < 4; i++) blueghost[i] = 0; }
        }
        ghostmovement();
        checkcollision();

        if (lives <= 0)
        {
            gamestate = GAME_OVER;
            playerwon = 0;
            clearsave();
            playSound(gameOverSound);
        }
        else if (pelletsleft == 0)
        {
           
            gamestate = GAME_OVER;
            playerwon = 1;
            clearsave();
            playSound(winSound);
        }
    }
}




int main()
{
    srand((unsigned)time(0));
    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);


    if (!font.openFromFile("arial.ttf"))
        return 1;
    loadSounds();
    bool mazeOk = loadmaze();
    distancemap();
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






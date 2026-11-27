# pf_project_game
This repo is for pf project game Pacman.

I am Noor Fatima .My roll no is 26L-0012 . me and my team member Tayba Nadim with roll number 26l-005 made this game.
i have added 5 features: i loaded maze from file, i managed the player movements ,W/S/A/D and Arrow keys manage player movement
i added blue ghost feature . when player eat power pellet ghosts turn blue for 5 seconds. Eating a blue ghost gives 200 scores to player and ghost become normal and go back to its position .
i also added menu . you can either start a new game, continue a saved game or exit the game program.
i added continue save game feature by writing maze pattern in savegame.txt and by pressing esc key you can save your game and go back to menu.

I am Tayba Nadim (Roll No. 26L-0005, GitHub: kitty-python) and I added 5 main features across my commits: 
first, I added four wandering ghosts that pick random open directions without turning back unless at a dead end; 
second, I made the red and cyan ghosts chase the player at junctions using Manhattan distance; 
third, I wrote checkcollision() to handle same-cell and cell-swap collisions, deducting a life and resetting positions on normal ghost hits or awarding 200 points and resetting the ghost when blue; 
fourth, I added win and lose conditions in updateGame() to trigger the end screen with the final score when all pellets are eaten or lives reach 0; 
fifth, I added distancemap() so the red ghost follows true walking distances around walls rather than getting stuck, along with a debug view (drawDistances()) toggled by pressing the M key.


Controls: W/S/A/D and arrow keys control player movement. ESC key saves a game that you can continue any time from menu. M key shows distance map in game.
RUN THE GAME: you can play the game by running its program and select new game option from menu to start a new game. 


This is image of our game.S
<img width="577" height="723" alt="image" src="https://github.com/user-attachments/assets/26203366-6cad-4684-b62d-122fadb28fd4" />


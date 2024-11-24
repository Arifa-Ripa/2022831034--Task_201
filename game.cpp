#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int CELL_SIZE = 20;
const int FONT_SIZE = 28;
const int FPS = 7;

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Point {
    int x, y;
};

class SnakeGame {
public:
    SnakeGame();
    ~SnakeGame();
    bool init();
    void menu();
    void run();

private:
    void handleEvents();
    void update();
    void render();
    void render2();
    void close();
    void reset();
    void spawnFood();
    bool checkCollision();
    void resume();
    void help();
    void level();
    void renderText(const char* text, int x, int y, SDL_Color color);
    SDL_Texture* loadTexture(const std::string& path);

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Texture* foodTexture;
    SDL_Texture* fieldTexture;
    SDL_Texture* levelTexture;
    TTF_Font* font;
    Mix_Music* backgroundMusic;
    bool running;
    bool level2;
    bool level3;
    Direction dir;
    Direction dir2;
    Direction dir3;
    vector<Point> snake;
    Point food;
    vector<Point> obs;
    vector<Point> enemy1;
    vector<Point> enemy2;
    int score;
    bool gameOver;
};

SnakeGame::SnakeGame()
    : window(nullptr), renderer(nullptr), font(nullptr), texture(nullptr),fieldTexture(nullptr), 
      levelTexture(nullptr), backgroundMusic(nullptr), running(true), dir(UP), score(0), 
      gameOver(false) {srand(static_cast<unsigned int>(time(0)));
}

SnakeGame::~SnakeGame() {
    close();
}

bool SnakeGame::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    if (TTF_Init() == -1) {
        cout << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        cout << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cout << "SDL_mixer initialization failed: " << Mix_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }

    font = TTF_OpenFont("NotoSans_ExtraCondensed-MediumItalic.ttf", FONT_SIZE);
    if (!font) {
        cout << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
        return false;
    }

    backgroundMusic = Mix_LoadMUS("snake_music.mp3");
    if (!backgroundMusic) {
        cout << "Failed to load background music! Mix_Error: " << Mix_GetError() << endl;
        return false;
    }

    Mix_PlayMusic(backgroundMusic, -1); //Loop the music indefinitely

    foodTexture = loadTexture("snakefood.jpeg");
    if (!foodTexture) {
        return false;
    }


    reset();
    return true;
}

SDL_Texture* SnakeGame::loadTexture(const string& path) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        cout << "Failed to load image: " << IMG_GetError() << endl;
        return nullptr;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void SnakeGame::reset() {
    snake.clear();
    snake.push_back({ SCREEN_WIDTH/2, SCREEN_HEIGHT  });
    snake.push_back({ SCREEN_WIDTH / 2 - CELL_SIZE, SCREEN_HEIGHT  });
    snake.push_back({ SCREEN_WIDTH / 2 - 2 * CELL_SIZE, SCREEN_HEIGHT });
    spawnFood();
    dir = UP;
    score = 0;
    gameOver = false;

    // Define the obstacle in the middle of the screen with length of 10 cells
    obs.clear();
    int startX = (SCREEN_WIDTH / 2) - (CELL_SIZE * 5);
    int startY = (SCREEN_HEIGHT / 2)   - CELL_SIZE;
    for (int i = 0; i < 10; ++i) {
        obs.push_back({ startX + i * CELL_SIZE, startY });
    }

    //Define the enemies in the border of the screen;
    
    enemy1.clear();
    enemy1.push_back({ SCREEN_WIDTH/2, 0  });
    enemy1.push_back({ SCREEN_WIDTH / 2 - CELL_SIZE, SCREEN_HEIGHT });
    enemy1.push_back({ SCREEN_WIDTH / 2 - 2 * CELL_SIZE, SCREEN_HEIGHT  });
    dir2 = DOWN;

    enemy2.clear();
    enemy2.push_back({ 0-CELL_SIZE, SCREEN_HEIGHT/2 });
    enemy2.push_back({ SCREEN_WIDTH / 2 - CELL_SIZE, SCREEN_HEIGHT });
    enemy2.push_back({ SCREEN_WIDTH / 2 - 2 * CELL_SIZE, SCREEN_HEIGHT });
    dir3 = RIGHT;
    
}

void SnakeGame::spawnFood() {
    bool onSnake;
    bool onObstacle;
    do {
        onSnake = false;
        onObstacle = false;
        food.x = (rand()  % ((SCREEN_WIDTH  / CELL_SIZE) -2) + 1) * CELL_SIZE;
        food.y = (rand()  % ((SCREEN_HEIGHT / CELL_SIZE) - 2) + 1) * CELL_SIZE;

        for (const auto& part : snake) {
            if (part.x == food.x && part.y == food.y) {
                onSnake = true;
                break;
            }
        }

        for (const auto& part : obs) {
            if (part.x == food.x && part.y == food.y) {
                onObstacle = true;
                break;
            }
        }

    } while (onSnake||onObstacle);
}

void SnakeGame::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            running = false;
            
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    if (dir != DOWN) dir = UP;
                    break;
                case SDLK_DOWN:
                    if (dir != UP) dir = DOWN;
                    break;
                case SDLK_LEFT:
                    if (dir != RIGHT) dir = LEFT;
                    break;
                case SDLK_RIGHT:
                    if (dir != LEFT) dir = RIGHT;
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            if (mouseX >= 550 && mouseX <= 620 && mouseY >= 10 && mouseY <= 40) {
                resume();
            } 
        }
        
        
    }
    
    
}

void SnakeGame::resume() {
    SDL_SetRenderDrawColor(renderer, 86, 191, 0, 255);
    SDL_RenderClear(renderer);

    fieldTexture = loadTexture("snakeGameBlank.jpeg");
    if (fieldTexture == nullptr) {
        return;
    }

    SDL_RenderCopy(renderer, fieldTexture, NULL, NULL);

    SDL_Color black = { 0, 0, 0, 255 };
    
    renderText("Press Enter to resume!", SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 10, black);

    SDL_RenderPresent(renderer);

    bool waiting = true;
    while (waiting) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
                waiting = false;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    waiting = false;
                    run();//return to the game
                }
            }
        }
    }
    close();
}


void SnakeGame::help(){
    SDL_SetRenderDrawColor(renderer, 86, 191, 0, 255);
    SDL_RenderClear(renderer);

    fieldTexture = loadTexture("snakeHelp.jpeg");
    if (fieldTexture == nullptr) {
        return;
    }

    SDL_RenderCopy(renderer, fieldTexture, NULL, NULL);

    SDL_Color black = { 0, 0, 0, 255 };
    
    renderText("Press Right to move the snake Right", SCREEN_WIDTH / 2 - 153, SCREEN_HEIGHT / 2 - 50 , black);
    renderText("Press Left to move the snake Left", SCREEN_WIDTH / 2 - 148, SCREEN_HEIGHT / 2 - 10, black);
    renderText("Press Up to move the snake Upward", SCREEN_WIDTH / 2 - 153, SCREEN_HEIGHT / 2 + 30, black);
    renderText("Press Down to move the snake Down", SCREEN_WIDTH / 2 - 153, SCREEN_HEIGHT / 2 + 70, black);
   

    renderText("Press Enter to Back!", SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 120, black);

    SDL_RenderPresent(renderer);

    bool waiting = true;
    while (waiting) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
                waiting = false;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    return;//return to the menu
                }
            }
        }
    }

    close();
}

void SnakeGame::update() {
    if (gameOver) {
        return;
    }

    //Move the snake
    Point newHead = snake[0];
    switch (dir) {
        case UP:    newHead.y -= CELL_SIZE; break;
        case DOWN:  newHead.y += CELL_SIZE; break;
        case LEFT:  newHead.x -= CELL_SIZE; break;
        case RIGHT: newHead.x += CELL_SIZE; break;
    }

    //Check for collisions
    if (newHead.x < 0 || newHead.x >= SCREEN_WIDTH   || newHead.y < 0 || newHead.y >= SCREEN_HEIGHT) {
        gameOver = true;
        return;
    }
    for (const auto& part : snake ) {
        if (newHead.x == part.x && newHead.y == part.y) {
            gameOver = true;

            return;
        }
    }

    if(level2){
        for(const auto& part : obs){
            if(newHead.x==part.x && newHead.y== part.y){
                gameOver=true;
                return;
            }
        }
    }

    if(level3){

        if (dir2 == DOWN) {
            enemy1[0].y += CELL_SIZE;  // Move head down
            if (enemy1[0].y >= SCREEN_HEIGHT) {
                enemy2[0].x += CELL_SIZE;   //Move head right
                if (enemy2[0].x >= SCREEN_WIDTH) {
                    dir2 = UP;  
                } 
            }
        } else {
            enemy1[0].y -= CELL_SIZE;  // Move head up
            if (enemy1[0].y < 0) {
                enemy2[0].x -= CELL_SIZE;  // Move head left
                if (enemy2[0].x < 0) {
                    dir2 = DOWN;  
                }  
            }
        }

        // Move the body segments of enemy1 to follow the head
        for (int i = enemy1.size() - 1; i > 0; --i) {
            enemy1[i] = enemy1[i - 1];  // Each segment follows the previous one
        }

        // Move the body segments of enemy2 to follow the head
        for (int i = enemy2.size() - 1; i > 0; --i) {
            enemy2[i] = enemy2[i - 1];  // Each segment follows the previous one
        }


    }

    if(level3){
        for(const auto& part : snake){
            
                if(enemy1[0].x==part.x && enemy1[0].y== part.y){
                    gameOver=true;
                    return;
                }
            
        }

        for(const auto& part : snake){

                if(enemy2[0].x==part.x && enemy2[0].y== part.y){
                    gameOver=true;
                    return;
                }
            
        }
    }

    snake.insert(snake.begin(), newHead);

    if (newHead.x == food.x && newHead.y == food.y) {
        score += 10;
        spawnFood();
    } else {
        snake.pop_back();
    }
}

void SnakeGame::level(){
    SDL_SetRenderDrawColor(renderer, 86, 191, 0, 255);
    SDL_RenderClear(renderer);

    levelTexture = loadTexture("SnakegameLevel.jpeg");
    if (levelTexture == nullptr) {
        return;
    }

    SDL_RenderCopy(renderer, levelTexture, NULL, NULL);

    bool quit = false;
    SDL_Event e;
    level2 = false;
    level3 = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
                running = false;
            } 
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                if (mouseX >= 130 && mouseX <= 300 && mouseY >= 150 && mouseY <= 200) {
                    run();//level 1
                } else if (mouseX >= 130 && mouseX <= 300 && mouseY >= 220 && mouseY <= 270) {
                    level2 = true;
                    run();//level 2
                }
                else if (mouseX >= 130 && mouseX <= 300 && mouseY >= 290 && mouseY <= 320) {
                    level2 = true;
                    level3 = true;
                    run();//level 3
                }
            }
        }


        SDL_Color black = { 0, 0, 0, 255 };
        renderText("LEVEL 1", 140, 160, black);
        renderText("LEVEL 2", 140, 230, black);
        renderText("LEVEL 3", 140, 295, black);

        SDL_RenderPresent(renderer);
    }

    close();
}


void SnakeGame::renderText(const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int texW = 0, texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}




void SnakeGame::render() {
    SDL_SetRenderDrawColor(renderer, 86, 191, 0, 255);
    SDL_RenderClear(renderer);

    fieldTexture = loadTexture("snakeGameField.jpeg");
    if (fieldTexture == nullptr) {
        return;
    }

    SDL_RenderCopy(renderer, fieldTexture, NULL, NULL);

    
    //Render snake
    
    for (const auto& part : snake) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect fillRect = { part.x, part.y, CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &fillRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect borderRect = { part.x, part.y, CELL_SIZE, CELL_SIZE }; 
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    //Render food
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect foodRect = { food.x, food.y, CELL_SIZE, CELL_SIZE };
    SDL_RenderCopy(renderer, foodTexture, nullptr, &foodRect);

    if(level2){
        for (const auto& part : obs) {
            SDL_SetRenderDrawColor(renderer, 137, 87, 55, 255);
            SDL_Rect fillRect = { part.x, part.y, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &fillRect);

            SDL_SetRenderDrawColor(renderer, 94,48,35, 255);
            SDL_Rect borderRect = { part.x, part.y, CELL_SIZE, CELL_SIZE }; 
            SDL_RenderDrawRect(renderer, &borderRect);
        }
    }

    if(level3){
        for (const auto& part : enemy1) {
            SDL_SetRenderDrawColor(renderer, 250, 0, 50, 255);
            SDL_Rect fillRect = { part.x, part.y, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &fillRect);

            SDL_SetRenderDrawColor(renderer, 94,48,35, 255);
            SDL_Rect borderRect = { part.x, part.y, CELL_SIZE, CELL_SIZE }; 
            SDL_RenderDrawRect(renderer, &borderRect);
        }

        for (const auto& part : enemy2) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 230, 255);
            SDL_Rect fillRect = { part.x, part.y, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &fillRect);

            SDL_SetRenderDrawColor(renderer, 94,48,35, 255);
            SDL_Rect borderRect = { part.x, part.y, CELL_SIZE, CELL_SIZE }; 
            SDL_RenderDrawRect(renderer, &borderRect);
        }
    }

    //Render score
    SDL_Color textColor = { 0, 0, 0, 255 };
    string scoreText = "Score: " + to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);
    SDL_Rect renderQuad = { 10, 10, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    SDL_DestroyTexture(textTexture);

    //Render Pause button
    SDL_Color black = { 0, 0, 0, 255 };
    renderText("Pause", 550, 10, black);

    SDL_RenderPresent(renderer);

        
    
}




void SnakeGame::close() {
    if (backgroundMusic != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }

    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if(foodTexture != nullptr){
        SDL_DestroyTexture(foodTexture);
        foodTexture = nullptr;
    }

    if(fieldTexture != nullptr){
        SDL_DestroyTexture(fieldTexture);
        fieldTexture = nullptr;
    }

    if(levelTexture != nullptr){
        SDL_DestroyTexture(levelTexture);
        levelTexture = nullptr;
    }

    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    if (font != nullptr) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}




void SnakeGame::menu() {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    texture = loadTexture("GameInterface.jpeg");
    if (texture == nullptr) {
        return;
    }

    bool quit = false;
    SDL_Event e;

    

    while (!quit) {

        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_Color black = { 0, 0, 0, 255 };
        renderText("CLICK HERE TO START!", 210, 350, black);
        renderText("Need Help?", 250, 400, black);

        SDL_RenderPresent(renderer);

        

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
                
            } 

            
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (mouseX >= 210 && mouseX <= 460 && mouseY >= 350 && mouseY <= 380) {
                    
                    level();//go to level selection
                    
                }
                else if (mouseX >= 240 && mouseX <= 450 && mouseY >= 400 && mouseY <= 430) {
                    help();// see the instructions
                    
                }
            }
        }

        SDL_Delay(50);
        
    }

    

    close();
}

void SnakeGame::run() {
    Uint32 frameStart;
    int frameTime;

    while (running) {
        frameStart = SDL_GetTicks();


        handleEvents();

        if (!running) {
            break; // If quit event is detected, exit the loop
        }

        update();
        render();

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime < 1000 / FPS) {
            SDL_Delay((1000 / FPS) - frameTime);
        }

        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 86, 191, 0, 255);
            SDL_RenderClear(renderer);

            fieldTexture = loadTexture("snakeGameover.jpeg");
            if (fieldTexture == nullptr) {
                return;
            }

            SDL_RenderCopy(renderer, fieldTexture, NULL, NULL);

            SDL_Color black = { 0, 0, 0, 255 };
            
            string finalScore = "Final Score: " + to_string(score);
            renderText(finalScore.c_str(), SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2, black);
            renderText("Press Enter to return to the menu", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 50, black);

            SDL_RenderPresent(renderer);

            bool waiting = true;
        
            while (waiting) {
                SDL_Event e;
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        
                        running = false;
                        waiting = false;
                    } else if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_RETURN) {
                            waiting = false;
                        }
                    }
                    
                }
            }

            SDL_DestroyTexture(fieldTexture);
            fieldTexture = nullptr; 

            if (!running) { // If the game was quit during gameOver screen
                break;
            }

            reset();
            menu();
        }
    }

    close();
}




int main(int argc, char* args[]) {
    SnakeGame game;
    if (!game.init()) {
        cout << "Failed to initialize!" << endl;
        return -1;
    }
    game.menu();
    return 0;
}

#include "Windows.h"
#include "BitMap.h"
#include "GameEngine.h"
#include "Resource.h"
#include <string>
#include <algorithm>

using namespace std;

GameEngine* game;
BitMap* cards[4][4];
int currMatches;
int numCard[4][4];
bool matchCard[4][4];
POINT tiles[2];
int pairs[8][2]; // Array to store pairs of card numbers
string cardNames[16]; // Declaration of cardNames array

BOOL GameInitialize(HINSTANCE currInstance) {
    game = new GameEngine(currInstance, L"Brainiac", L"Brainiac", IDI_UFO, IDI_UFO_SM, 540, 720);
    if (game == NULL) {
        return FALSE;
    }
    game->setFrameRate(15);
    return TRUE;
}

void GameStart(HWND hwnd) {
    srand(GetTickCount());
    HDC hdc = GetDC(hwnd);

    // Porygon images
    string porygonImages[] = {
        "Res/porygon_1_SM.bmp",
        "Res/porygon_2_SM.bmp",
        "Res/porygon_3_SM.bmp",
        "Res/porygon_4_SM.bmp",
        "Res/porygon_5_SM.bmp",
        "Res/porygon_6_SM.bmp",
        "Res/porygon_7_SM.bmp",
        "Res/porygon_8_SM.bmp"
    };

    // Shuffle porygon images
    random_shuffle(begin(porygonImages), end(porygonImages));

    // Card images
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; ++j) {
            cardNames[i * 8 + j] = porygonImages[j]; // Assign card names
        }
    }

    // Shuffle card names
    random_shuffle(begin(cardNames), end(cardNames));

    /*
    // Display card names in a message box
    string cardNamesString = "Card Names:\n";
    for (const auto& name : cardNames) {
        cardNamesString += name + "\n";
    }
    MessageBox(NULL, wstring(cardNamesString.begin(), cardNamesString.end()).c_str(), L"Card Names", MB_OK);
    */

    // Assign card images to the cards array
    int index = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            cards[i][j] = new BitMap(hdc, wstring(cardNames[index].begin(), cardNames[index].end()).c_str());
            numCard[i][j] = index; // Set the card number to match pairs later
            matchCard[i][j] = false;
            ++index;
        }
    }

    // Assign pairs of card numbers
    int pairIndex = 0;
    for (int i = 0; i < 8; ++i) {
        pairs[i][0] = pairIndex;
        pairs[i][1] = pairIndex;
        pairIndex++;
    }

    // Shuffle pairs
    for (int i = 0; i < 8; ++i) {
        int randomIndex = rand() % 8;
        swap(pairs[i], pairs[randomIndex]);
    }

    // Initialize other game variables
    currMatches = 0;
    memset(tiles, -1, sizeof(tiles)); // Reset tiles to {-1, -1}

    // Release the DC before restarting the game
    ReleaseDC(hwnd, hdc);

    // Trigger window repaint
    InvalidateRect(hwnd, NULL, TRUE);
}

void GameLoop() {
    // Check if all pairs are matched
    int matchedPairs = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 2; ++j) {
            if (matchCard[pairs[i][j] / 4][pairs[i][j] % 4]) {
                ++matchedPairs;
                break; // Break inner loop as we found a match for this pair
            }
        }
    }

    if (matchedPairs == 8) {
        // Prompt the player to restart the game
        int result = MessageBox(NULL, L"All pairs matched! Do you want to restart the game?", L"Congratulations!",
            MB_YESNO | MB_ICONQUESTION);
        if (result == IDYES) {
            // Restart the game
            GameStart(game->getWnd());
        }
        else {
            // Exit the game
            GameEnd();
        }
    }
    else {
        // Check if all cards are matched and displayed
        bool allCardsDisplayed = true;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (!matchCard[i][j]) {
                    allCardsDisplayed = false;
                    break; // Break inner loop if any card is not displayed
                }
            }
            if (!allCardsDisplayed) {
                break; // Break outer loop if any card is not displayed
            }
        }

        // If all cards are displayed and matched, notify the player
        if (allCardsDisplayed) {
            MessageBox(NULL, L"Congratulations! You have completed the game!", L"Game Completed", MB_OK);
            // Exit the game
            GameEnd();
        }
    }
}

void GameEnd() {
    // Clean up resources here

    // Release memory allocated for BitMap objects
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            delete cards[i][j];
        }
    }
    // Reset pointers to NULL to avoid dangling pointers
    memset(cards, 0, sizeof(cards));

    // Set currMatches to 0
    currMatches = 0;

    // Exit the game
    PostQuitMessage(0);
}


void GameActivate(HWND hwnd) {
    // Implement activation logic here
}

void GameDeactivate(HWND hwnd) {
    // Implement deactivation logic here
}

void GamePaint(HDC hdc) {
    // Draw white background
    RECT clientRect;
    GetClientRect(game->getWnd(), &clientRect);
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // White color
    FillRect(hdc, &clientRect, hBrush);
    DeleteObject(hBrush);

    // Draw cards here
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (matchCard[i][j]) {
                // Draw the revealed card
                cards[i][j]->draw(hdc, i * 132, j * 175);
            }
            else {
                // Draw the back of the card
                // Load and draw the back image over the card
                BitMap back(hdc, L"Res/PokemonBack_SM.bmp");
                back.draw(hdc, i * 132, j * 175);
            }
        }
    }
}

void HandleKeys() {
    // Implement key handling logic here
}

void MouseButtonDown(int x, int y, bool left) {
    if (left) {
        int windowWidth = game->getWidth();
        int windowHeight = game->getHeight();

        // Check if the click is within the window bounds
        if (x >= 0 && x < windowWidth && y >= 0 && y < windowHeight) {
            // Calculate the column and row of the clicked card
            int col = x / 132;
            int row = y / 175;

            // Check if the card is already matched or already selected
            if (!matchCard[col][row] && (tiles[0].x != col || tiles[0].y != row)) {
                // Redraw the card over the area of the clicked card
                HDC hdc = GetDC(game->getWnd());
                if (matchCard[col][row]) {
                    // Draw the back of the card
                    cards[col][row]->draw(hdc, col * 132, row * 175);
                }
                else {
                    // Draw the front of the card
                    cards[col][row]->draw(hdc, col * 132, row * 175);
                }
                ReleaseDC(game->getWnd(), hdc);

                // If both tiles are reset, it's the first click and the first card
                if (tiles[0].x == -1 && tiles[0].y == -1) {
                    // First click
                    tiles[0].x = col;
                    tiles[0].y = row;
                }
                // If the first tile is set but the second tile is reset, it's the second click and the second card
                else if (tiles[1].x == -1 && tiles[1].y == -1) {
                    // Second click
                    tiles[1].x = col;
                    tiles[1].y = row;

                    // Check if the two selected cards match
                    if (cardNames[numCard[tiles[0].x][tiles[0].y]] == cardNames[numCard[tiles[1].x][tiles[1].y]]) {
                        // Cards match, show a message
                        //MessageBox(NULL, L"The cards are the same!", L"Match", MB_OK);
                        // Mark the cards as matched
                        matchCard[tiles[0].x][tiles[0].y] = true;
                        matchCard[tiles[1].x][tiles[1].y] = true;
                    }
                    else {
                        // Cards don't match, show a message
                        //MessageBox(NULL, L"The cards do not match!", L"No Match", MB_OK);
                        // Redraw the back of the cards after a short delay
                        Sleep(1000); // Adjust the delay as needed
                        HDC hdc = GetDC(game->getWnd());

                        // Draw the back image over the area of the clicked cards
                        BitMap back(hdc, L"Res/PokemonBack_SM.bmp");
                        for (int i = 0; i < 2; ++i) {
                            back.draw(hdc, tiles[i].x * 132, tiles[i].y * 175);
                        }

                        ReleaseDC(game->getWnd(), hdc);
                    }

                    // Reset tiles and click count for the next pair
                    tiles[0].x = -1;
                    tiles[0].y = -1;
                    tiles[1].x = -1;
                    tiles[1].y = -1;
                }
            }
        }
    }
}

void MouseButtonUp(int x, int y, bool left) {
    // Implement mouse button up logic here
}

void MouseMove(int x, int y) {
    // Implement mouse move logic here
}

/*
Var-Name-JPG License
~~~~~~~~~~~~~~~~~~~~
- This software is Copyright (C) 2025 Var-Name-JPG
- This is free software
- This software comes with absolutely no warranty
- The copyright holder is not liable or responsible for anything
  this software does or does not
- You use this software at your own risk
- You can distribute this software
- You can modify this software
- Redistribution of this software or a derivative of this software
  does not require attribution to the copyright holder named above
  
License
~~~~~~~
Var-Name-JPG Console Application Copyright (C) 2025 Var-Name-JPG
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it

Hello! Feel free to use, modify, and share this code in any way you see fit.
This code was designed with creativity in mind, and I hope it provides
value or at least a bit of fun! By using this code, you agree that I am not
responsible for anything bad that happens as a result of your actions.
Enjoy hacking and experimenting!
*/

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;

#include <stdio.h>
#include <Windows.h>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 1.5f;
float fPlayerY = 1.5f;
// angle player is looking (0-360) technically its (0 - 2pi) because this stupid ass language only uses radians
float fPlayerA = 1.5f;

int nMapHeight = 17;
int nMapWidth = 17;

// These numbers just worked
float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

// Used for later calculation
const float PI = 3.14159f;
const float PI_2 = PI / 2.0f;
const float M_3PI_2 = 3.0f * PI / 2.0f;

// Function to check if the player is colliding with a wall
bool IsCollidingWithWall(wstring map, float newX, float newY)
{
    return map[(int)newY * nMapWidth + (int)newX] == '#';
}

// Function to slide the player along the wall (if a collision occurs)
void SlideAlongWall(wstring map, float& newX, float& newY, float originalX, float originalY)
{
    // Try sliding horizontally if there is a collision in the original direction
    if (IsCollidingWithWall(map, newX, originalY)) // Check if horizontal collision
    {
        newX = originalX; // Keep X the same, slide in Y direction
    }
    // Try sliding vertically if there is a collision in the original direction
    else if (IsCollidingWithWall(map, originalX, newY)) // Check if vertical collision
    {
        newY = originalY; // Keep Y the same, slide in X direction
    }
}

int main()
{
    cout << R"(
 ______  __  __  ____                  ______  ________   ____      
/\__  _\/\ \/\ \/\  _`\        /'\_/`\/\  _  \/\_____  \ /\  _`\    
\/_/\ \/\ \ \_\ \ \ \L\_\     /\      \ \ \L\ \/____//'/'\ \ \L\_\  
   \ \ \ \ \  _  \ \  _\L     \ \ \__\ \ \  __ \   //'/'  \ \  _\L  
    \ \ \ \ \ \ \ \ \ \L\ \    \ \ \_/\ \ \ \/\ \ //'/'___ \ \ \L\ \
     \ \_\ \ \_\ \_\ \____/     \ \_\\ \_\ \_\ \_\/\_______\\ \____/
      \/_/  \/_/\/_/\/___/       \/_/ \/_/\/_/\/_/\/_______/ \/___/ 

                    Created by Var-Name-JPG


                           CONTROLS
                          WASD - Move
                    Left/Right Arrow - Turn

                      Press Enter to Start
)" << endl;
    cin.get();

    system("cls");

    // Allocate memory for the screen buffer.
    // The screen buffer is effectively a 2D array of characters, where each character represents
    // a pixel (or cell) in the console's display. It has a total size of nScreenWidth * nScreenHeight.
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];

    // Create a new console screen buffer.
    // A screen buffer is essentially a virtual representation of the console's output.
    // This function creates a new buffer that can be used to manipulate console output without
    // immediately displaying it. The GENERIC_READ and GENERIC_WRITE flags allow reading and writing
    // operations on the buffer.
    HANDLE hConsole = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE, // Permissions to read from and write to the screen buffer
        0,                            // No sharing mode (not needed for this application)
        NULL,                         // No security attributes (default settings)
        CONSOLE_TEXTMODE_BUFFER,      // Specify that the buffer operates in text mode
        NULL                          // No additional parameters (default behavior)
    );

    // Set the newly created console screen buffer as the active one.
    // This function switches the console's output to the newly created buffer. Any subsequent
    // calls to functions that interact with the console will now target this active buffer,
    // allowing custom rendering of text or graphics.
    SetConsoleActiveScreenBuffer(hConsole);

    // Declare and initialize a variable to track the number of bytes written to the console.
    // `WriteConsoleOutputCharacter` requires this variable to report how many bytes were successfully
    // written. While the variable itself isn't directly critical for rendering, it ensures the function
    // operates correctly.
    DWORD dwBytesWritten = 0; // Initial value set to 0 for safety and completeness.

    // Fill screen with ' ' to start
    for (int i = 0; i < nScreenHeight; i++)
    {
        for (int j = 0; j < nScreenWidth; j++)
        {
            screen[i * nScreenWidth + j] = L' ';
        }
    }

    // Make a map
    // '.' is empy space
    // '#' is a "wall"
    wstring map;
    map += L"#################";
    map += L"#.....#.........#";
    map += L"#####.#.###.###.#";
    map += L"#...#.....#.#...#";
    map += L"###.#.###########";
    map += L"#.#.....#.......#";
    map += L"#.#.###.#.#.#.###";
    map += L"#...#...#.#.#.#.#";
    map += L"#####.#.#.###.#.#";
    map += L"#.....#.....#...#";
    map += L"#.#####.###.#.#.#";
    map += L"#...#.....#.#.#.#";
    map += L"#.###############";
    map += L"#.#.............#";
    map += L"#.#.###.#####.#.#";
    map += L"#.....#...#...#.#";
    map += L"#################";

    // initializing frame delta time
    // basically get two time stamps, let one run, then compare them to eachother
    // every time the buffer updates, then reset both, then repeat
    // Multiply everything movement or render related to this and it will always
    // render the same, regardless of system
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    bool bGameOver = false;

    // Timer Start
    auto start = chrono::high_resolution_clock::now();

    // Game Loop
    while (!bGameOver)
    {
        // Pseudo-delta time, explained above
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Get Controls
        // Because we're using raycasts, we can use that to move the player based on where they are looking
        
        // CCW Rotation
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) // Left arrow makes you rotate left
            fPlayerA -= (1.5f) * fElapsedTime;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) // Right arrow makes you rotate right
            fPlayerA += (1.5f) * fElapsedTime;

        // Forward movement
        // I'm going to explain this one thoroughly, it applies to all horizontal movement
        if (GetAsyncKeyState((unsigned char)'W') & 0x8000) // Check if the 'W' key is currently pressed
        {
            // Calculate the player's potential new X position based on current angle (fPlayerA),
            // movement speed (5.0f units per second), and elapsed time (fElapsedTime).
            // Uses the sine of the angle to determine horizontal movement.
            float newPlayerX = fPlayerX + sinf(fPlayerA) * 5.0f * fElapsedTime;

            // Calculate the player's potential new Y position based on current angle (fPlayerA),
            // movement speed, and elapsed time.
            // Uses the cosine of the angle to determine vertical movement.
            float newPlayerY = fPlayerY + cosf(fPlayerA) * 5.0f * fElapsedTime;

            // Check if the new position (newPlayerX, newPlayerY) would result in a collision with a wall.
            // `IsCollidingWithWall` is a function that evaluates whether the given position intersects
            // with any obstacles defined in the `map`.
            if (!IsCollidingWithWall(map, newPlayerX, newPlayerY))
            {
                // No collision detected, so update the player's position to the new position.
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
            else
            {
                // Collision detected. Handle sliding along the wall.
                // The `SlideAlongWall` function adjusts the player's position to allow movement
                // along the surface of the wall, without penetrating it.
                SlideAlongWall(map, newPlayerX, newPlayerY, fPlayerX, fPlayerY);

                // Update the player's position to the new position after sliding adjustments.
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
        }

        // Backward movement
        if (GetAsyncKeyState((unsigned char)'S') * 0x8000)
        {
            float newPlayerX = fPlayerX - sinf(fPlayerA) * 5.0f * fElapsedTime;
            float newPlayerY = fPlayerY - cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (!IsCollidingWithWall(map, newPlayerX, newPlayerY))
            {
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
            else
            {
                // Slide along wall if collision detected
                SlideAlongWall(map, newPlayerX, newPlayerY, fPlayerX, fPlayerY);
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
        }

        // Strafing Left
        if (GetAsyncKeyState((unsigned char)'A') * 0x8000)
        {
            float newPlayerX = fPlayerX - cosf(fPlayerA) * 5.0f * fElapsedTime / 2.0f;
            float newPlayerY = fPlayerY + sinf(fPlayerA) * 5.0f * fElapsedTime / 2.0f;

            if (!IsCollidingWithWall(map, newPlayerX, newPlayerY))
            {
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
            else
            {
                // Slide along wall if collision detected
                SlideAlongWall(map, newPlayerX, newPlayerY, fPlayerX, fPlayerY);
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
        }

        // Strafing Right
        if (GetAsyncKeyState((unsigned char)'D') * 0x8000)
        {
            float newPlayerX = fPlayerX + cosf(fPlayerA) * 5.0f * fElapsedTime / 2.0f;
            float newPlayerY = fPlayerY - sinf(fPlayerA) * 5.0f * fElapsedTime / 2.0f;

            if (!IsCollidingWithWall(map, newPlayerX, newPlayerY))
            {
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
            else
            {
                // Slide along wall if collision detected
                SlideAlongWall(map, newPlayerX, newPlayerY, fPlayerX, fPlayerY);
                fPlayerX = newPlayerX;
                fPlayerY = newPlayerY;
            }
        }

        for (int x = 0; x < nScreenWidth; x++) // Iterate over each column of the screen
        {
            // Calculate the angle of the ray being projected for this column
            // Start from (player's angle - half field of view) and increment based on screen width
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0.0f;  // Distance from the player to the first wall hit by the ray
            bool bHitWall = false;         // Flag to indicate if a wall is hit
            bool bBoundary = false;        // Flag to indicate if the ray hit a boundary (for edge detection)

            // Unit vector for the ray direction, based on the angle
            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            // Cast the ray until it hits a wall or reaches the maximum render depth
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f; // Increment the distance the ray travels in small steps

                // Calculate the current cell the ray is testing in the map
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Check if the ray is out of the map's bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true; // Ray is out of bounds, treat it as a hit
                    fDistanceToWall = fDepth; // Set distance to maximum depth
                }
                else
                {
                    // Check if the cell the ray is testing contains a wall
                    if (map.c_str()[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true; // Wall hit detected

                        // Boundary detection for edges of walls (visual effect)
                        vector<pair<float, float>> p; // Stores distances and dot products

                        // Test corners of the cell for boundary conditions
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy); // Distance to corner
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d); // Dot product

                                p.push_back(make_pair(d, dot)); // Store distance and dot product
                            }

                        // Sort corners by distance
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) { return left.first < right.first; });

                        // Check if the angle to the nearest corners is within a small boundary threshold
                        float fBound = 0.01f; // Boundary threshold
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                    }
                }
            }

            // Calculate the projected distance to the ceiling and floor on the screen
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' '; // Variable to determine the shading character for the wall

            // Determine the shading based on the distance to the wall
            if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588; // Very close
            else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
            else if (fDistanceToWall < fDepth)				nShade = 0x2591;
            else											nShade = ' ';   // Too far, no shading

            // Apply boundary shading if the ray hit a wall boundary
            if (bBoundary) nShade = ' ';

            // Render the column by iterating over the screen height
            if (bHitWall)
            {
                for (int y = 0; y < nScreenHeight; y++)
                {
                    if (y <= nCeiling)
                        screen[y * nScreenWidth + x] = ' '; // Sky (empty space above the ceiling)
                    else if (y > nCeiling && y <= nFloor)
                        screen[y * nScreenWidth + x] = nShade; // Wall shading
                    else
                    {
                        // Shade the floor based on its distance from the player
                        float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));

                        if (b < 0.25)		nShade = '#'; // Closest floor shading
                        else if (b < 0.5)	nShade = 'x';
                        else if (b < 0.75)	nShade = '-';
                        else if (b < 0.9)	nShade = '.';
                        else				nShade = ' '; // Farthest floor shading

                        screen[y * nScreenWidth + x] = nShade; // Apply floor shading
                    }
                }
            }
        }

        // Ensure player's angle stays within valid range.
        // If the angle exceeds 2π (360 degrees) or is less than -2π (-360 degrees),
        // it is reset to 0 to keep the value normalized. This prevents overflow issues
        // and ensures consistent angle handling.
        if (fPlayerA >= 2.0f * 3.141592f || fPlayerA <= -(2.0f * 3.141592f))
            fPlayerA = 0.0f;

        // Display player stats on the screen.
        // Uses `swprintf_s` to format the player's position (X and Y) and angle (A) as strings.
        // The formatted text is stored in the `screen` buffer for rendering.
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f", fPlayerX, fPlayerY, fPlayerA);

        // Display the map on the screen.
        // Iterate through all cells in the map dimensions.
        // Copy each map cell character into the appropriate position in the `screen` buffer.
        // This renders the map directly into the output buffer for visualization.
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                // Maps each map cell to its corresponding position in the `screen` buffer.
                // Add `1` to ny to avoid overwriting the stats area in the first row of the screen.
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }

        // Render the player's position on the map.
        // Uses `fPlayerX` and `fPlayerY` to find the player's current cell in the map.
        // The player's character (a solid block, 0x2588) is placed in the `screen` buffer,
        // overwriting the map cell at the player's coordinates.
        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 0x2588;

        // Null-terminate the `screen` buffer.
        // Ensures the buffer is correctly terminated for display with `WriteConsoleOutputCharacter`.
        screen[nScreenWidth * nScreenHeight - 1] = '\0';

        // Write the contents of the `screen` buffer to the console.
        // This function updates the console output to match the current state of the `screen` buffer.
        // `hConsole` is the handle to the console output.
        // The buffer size is equal to the total screen dimensions (width * height).
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

        if (fPlayerY >= 15.5f && fPlayerX >= 15.5f)
        {
            bGameOver = true;
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::seconds>(end - start);
            CloseHandle(hConsole);

            system("cls");

            cout << R"(
 __    __  _____   __  __       __      __  ______   __  __     
/\ \  /\ \/\  __`\/\ \/\ \     /\ \  __/\ \/\__  _\ /\ \/\ \    
\ `\`\\/'/\ \ \/\ \ \ \ \ \    \ \ \/\ \ \ \/_/\ \/ \ \ `\\ \   
 `\ `\ /'  \ \ \ \ \ \ \ \ \    \ \ \ \ \ \ \ \ \ \  \ \ , ` \  
   `\ \ \   \ \ \_\ \ \ \_\ \    \ \ \_/ \_\ \ \_\ \__\ \ \`\ \ 
     \ \_\   \ \_____\ \_____\    \ `\___x___/ /\_____\\ \_\ \_\
      \/_/    \/_____/\/_____/     '\/__//__/  \/_____/ \/_/\/_/
)" << endl;

            cout << endl << "                          Time: " << duration.count() << "s" << endl;
            cout << endl << "Automatically closing in three seconds...";
            this_thread::sleep_for(chrono::seconds(3));
        }
    }


    return 0;
}
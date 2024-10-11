#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include "../win32/glfw3.h"
#else
#include <unistd.h>
#include "../linux/glfw3.h"
#endif


/*MazeC 3D Legacy
    Raycasted 3D maze game made in C with OpenGL 2.1 and GLFW

    Copyright (c) 2024 Benjamin Helle. All rights reserved.
    Version: 0.1.0a
*/

#define VERSION "0.1.0a"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define WORLD_SIZE 196 // world size in squares(now 14x14)

typedef struct {
    bool worldmap[WORLD_SIZE]; // 1 = wall, 0 = air
    float playerX, playerY; //player pos
    float dirX, dirY; //player direction
    float planeX, planeY; //2D raycaster projection plane
    float moveSpeed; //movement speed
    float rotSpeed; //rotation speed
} world;

void genWorld(world *W){ // generate random world
    int j = (int)sqrt(WORLD_SIZE);
    for (int i = 0; i < WORLD_SIZE; i++) {
        W->worldmap[i] = rand() % 2;
        if (i == j) {
            printf("\n");
            j += (int)sqrt(WORLD_SIZE);
        }
        printf("%d ", W->worldmap[i]);
    }
    printf("\nWorld generated\n");
}


void initWorld(world *W){ // initialize the world before the game starts
    genWorld(W);
    W->playerX = (int)sqrt(WORLD_SIZE) / 2.0f; //player pos = center of the world
    W->playerY = (int)sqrt(WORLD_SIZE) / 2.0f;
    W->worldmap[(int)(W->playerY) * (int)sqrt(WORLD_SIZE) + (int)(W->playerX)] = 0; // set player pos to air
    W->dirX = -1.0f;
    W->dirY = 0.0f;
    W->planeX = 0.0f;
    W->planeY = 0.66f;
    W->moveSpeed = 0.03f;
    W->rotSpeed = 0.02f; 
}

void mainRenderer(world *W){ // the main renderer of the game
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float cameraX = 2 * x / (float)SCREEN_WIDTH - 1; //x-coordinate in camera space
        float rayDirX = W->dirX + W->planeX * cameraX; 
        float rayDirY = W->dirY + W->planeY * cameraX;

        int mapX = (int)W->playerX; // player coords -> map coords
        int mapY = (int)W->playerY;

        float sideDistX;
        float sideDistY;

        float deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX); 
        float deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
        float perpWallDist;

        int stepX, stepY;
        int hit = 0;
        int side;

        if (rayDirX < 0) { // calculate ray directions
            stepX = -1;
            sideDistX = (W->playerX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - W->playerX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (W->playerY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - W->playerY) * deltaDistY;
        }

        while (hit == 0) { // raycast
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (W->worldmap[mapY * (int)sqrt(WORLD_SIZE) + mapX] > 0) hit = 1; 
        }

        if (side == 0) perpWallDist = (mapX - W->playerX + (1 - stepX) / 2) / rayDirX; 
        else perpWallDist = (mapY - W->playerY + (1 - stepY) / 2) / rayDirY;

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist); //height of the line to draw on screen
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        glColor3f(1.0f, 1.0f, 1.0f); // white, makes front walls brighter
        if (side == 1) glColor3f(0.7f, 0.7f, 0.7f); // light gray, makes side walls darker

        glBegin(GL_LINES); // draw the line
        glVertex2i(x, drawStart);
        glVertex2i(x, drawEnd);
        glEnd();
    }
}

void processInput(GLFWwindow *window, world *W) { // process movement and other inputs
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { // forward
        if (W->worldmap[(int)(W->playerY) * (int)sqrt(WORLD_SIZE) + (int)(W->playerX + W->dirX * W->moveSpeed)] == 0) W->playerX += W->dirX * W->moveSpeed;
        if (W->worldmap[(int)(W->playerY + W->dirY * W->moveSpeed) * (int)sqrt(WORLD_SIZE) + (int)(W->playerX)] == 0) W->playerY += W->dirY * W->moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { // backward
        if (W->worldmap[(int)(W->playerY) * (int)sqrt(WORLD_SIZE) + (int)(W->playerX - W->dirX * W->moveSpeed)] == 0) W->playerX -= W->dirX * W->moveSpeed;
        if (W->worldmap[(int)(W->playerY - W->dirY * W->moveSpeed) * (int)sqrt(WORLD_SIZE) + (int)(W->playerX)] == 0) W->playerY -= W->dirY * W->moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { // left
        float oldDirX = W->dirX;
        W->dirX = W->dirX * cos(W->rotSpeed) - W->dirY * sin(W->rotSpeed);
        W->dirY = oldDirX * sin(W->rotSpeed) + W->dirY * cos(W->rotSpeed);
        float oldPlaneX = W->planeX;
        W->planeX = W->planeX * cos(W->rotSpeed) - W->planeY * sin(W->rotSpeed);
        W->planeY = oldPlaneX * sin(W->rotSpeed) + W->planeY * cos(W->rotSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { // right
        float oldDirX = W->dirX;
        W->dirX = W->dirX * cos(-W->rotSpeed) - W->dirY * sin(-W->rotSpeed);
        W->dirY = oldDirX * sin(-W->rotSpeed) + W->dirY * cos(-W->rotSpeed);
        float oldPlaneX = W->planeX;
        W->planeX = W->planeX * cos(-W->rotSpeed) - W->planeY * sin(-W->rotSpeed);
        W->planeY = oldPlaneX * sin(-W->rotSpeed) + W->planeY * cos(-W->rotSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1); // close the game
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { // print current player position
        printf("X: %.2f, Y: %.2f\n", W->playerX, W->playerY);
    }
}


int main() {

    printf("MazeC 3D Legacy %s\n", VERSION);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    if (!glfwInit()) {
        perror("Failed to initialize GLFW"); // throw an error if GLFW couldn't be initialized
        return -2;
    }

    srand(time(NULL)); // set random seed

    world TestWorld; // create a new world
    initWorld(&TestWorld); // initialize the world

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MazeC 3D Legacy", NULL, NULL);
    if (!window) {
        perror("Failed to create window"); // throw an error if the window couldn't be created
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);

    glClearColor(0.6f, 0.6f, 0.6f, 1.0f); // set the background color, light gray
    glClear(GL_COLOR_BUFFER_BIT);

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    while (!glfwWindowShouldClose(window)) { // main game loop
        glClear(GL_COLOR_BUFFER_BIT); // clear the screen

        mainRenderer(&TestWorld); // call the main renderer
        processInput(window, &TestWorld); // check if any input

        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwSwapInterval(1); // enable vsync
    }

    glfwTerminate();
    return 0;
}
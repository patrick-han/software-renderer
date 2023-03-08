#include <stdlib.h>
#include <utility>
#include <cmath>
#include <SDL.h>
#include <vector>

#define C_w 1920/2
#define C_h 1080/2

class Point {
public:
    Point(int _x, int _y) : x(_x), y(_y) {}

    int x;
    int y;
};

class Color {
public:
    Color(int _r, int _g, int _b) : r(_r), g(_g), b(_b) {}

    int r;
    int g;
    int b;
};


void PutPixel(SDL_Renderer* renderer, int x, int y, Color color) {
    // Origin at the center conversion
    int S_x = C_w / 2 + x;
    int S_y = C_h / 2 - y;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawPoint(renderer, S_x, S_y);
}

std::vector<float> Interpolate(int i0, int d0, int i1, int d1) {
    // Given our known "end" values of our linear function d0 = f(i0) and d1 = f(i1),
    // return a list of all interpolated values of d, assuming i0 < i1
    std::vector<float> values;

    if (i0 == i1) { // In the case we only have a single value of i, since a will encounter divide by 0 if we try
        values.push_back(d0);
        return values;
    }


    float a = (float)(d1 - d0) / (i1 - i0); // Slope
    float d = d0;

    for (int i = i0; i <= i1; i++) {
        values.push_back(d);
        d = d + a; // We realize that to get the next y-value, we simply add the slope to the previous y-value
        
    }
    return values;
}


void DrawLine(SDL_Renderer* renderer, Point p0, Point p1, Color color) {

    // Is the line more horizontal (greater difference in x values) or vertical?
    int dx = p1.x - p0.x;
    int dy = p1.y - p0.y;
    if (std::abs(dx) > std::abs(dy)) {
        // For drawing when we have "more horizontalish" lines (many x, fewer y)
        // We always draw from left to right
        if (p0.x > p1.x) {
            std::swap(p0, p1);
        }
        std::vector<float> vy = Interpolate(p0.x, p0.y, p1.x, p1.y);
        for (int x = p0.x; x <= p1.x; x++) {
            PutPixel(renderer, x, vy[x - p0.x], color);
        }

    }
    else {
        // For drawing when we have "more verticalish" lines (many y, fewer x)
        // Always draw from bottom to top
        if (p0.y > p1.y) {
            std::swap(p0, p1);
        }
        std::vector<float> vx = Interpolate(p0.y, p0.x, p1.y, p1.x);
        for (int y = p0.y; y <= p1.y; y++) {
            PutPixel(renderer, vx[y - p0.y], y, color);

        }
    }
}

void DrawWireframeTriangle(SDL_Renderer* renderer, Point p0, Point p1, Point p2, Color color) {
    DrawLine(renderer, p0, p1, color);
    DrawLine(renderer, p1, p2, color);
    DrawLine(renderer, p2, p0, color);
}


int main(int argc, char* argv[]) {
    SDL_Event event;
    SDL_Renderer* renderer;
    SDL_Window* window;
    int i;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(C_w, C_h, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    // Render
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    Color color = { 0, 255, 0 };
    Point p0 = Point(-50, -200);
    Point p1 = Point(60, 240);
    Point p2 = Point(100, -10);
   
    DrawWireframeTriangle(renderer, p0, p1, p2, color);



    SDL_RenderPresent(renderer);
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
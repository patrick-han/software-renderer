#include <stdlib.h>
#include <utility>
#include <cmath>
#include <SDL.h>
#include <vector>
#include <ranges>
#include <iostream>
#include <algorithm>

#define C_w 1920/2
#define C_h 1080/2

class Point {
public:
    Point(int _x, int _y, float _h) : x(_x), y(_y), h(_h) {}

    int x;
    int y;
    float h; // 0 to 1.0 intensity of color (black to full color)
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

std::vector<float> Interpolate(int i0, float d0, int i1, float d1) {
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

void DrawFilledTriangle(SDL_Renderer* renderer, Point p0, Point p1, Point p2, Color color) {
    // First sort our vertcies by y-value ascending p0.y, p1.y, p2.y
    if (p1.y < p0.y) { std::swap(p1, p0); }
    if (p2.y < p0.y) { std::swap(p2, p0); }
    if (p2.y < p1.y) { std::swap(p2, p1); }

    // We use our interpolate function to get all the x values for each y-value for each of the three sides
    // Same for the h intensity values
    auto x01 = Interpolate(p0.y, p0.x, p1.y, p1.x); // x = f(y)
    auto h01 = Interpolate(p0.y, p0.h, p1.y, p1.h); // h = f(y)

    auto x12 = Interpolate(p1.y, p1.x, p2.y, p2.x);
    auto h12 = Interpolate(p1.y, p1.h, p2.y, p2.h);

    auto x02 = Interpolate(p0.y, p0.x, p2.y, p2.x); // p0 to p2 is the "tall side", the other 2 make up the "short" side
    auto h02 = Interpolate(p0.y, p0.h, p2.y, p2.h);

    // There's a repeated value in x01 and x12
    x01.pop_back();
    std::ranges::copy(x12, std::back_inserter(x01));
    auto x012 = x01;

    h01.pop_back();
    std::ranges::copy(h12, std::back_inserter(h01));
    auto h012 = h01;

    // Determine which side(s) are left and which is right by comparing an arbitrary interpolated value from either side
    
    // List of all x values on the very left and right sides of the triangle (the "edge")
    std::vector<float> x_left;
    std::vector<float> x_right;
    // List of all intensity values for the very left and right sides of the triangle (the "edge")
    std::vector<float> h_left;
    std::vector<float> h_right;

    int middle = std::floor(x02.size() / 2); // This is arbitrary, we could compare any value
    if (x02[middle] < x012[middle]) {
        x_left = x02;
        x_right = x012;
        h_left = h02;
        h_right = h012;
    }
    else {
        x_left = x012;
        x_right = x02;
        h_left = h012;
        h_right = h02;
    }

    // Draw horizontal lines
    for (int y = p0.y; y <= p2.y; y++) {
        // Fetch the current left and right x coordinates, we need to use these to interpolate the intensity across the triangle
        float x_l = x_left[y - p0.y];
        float x_r = x_right[y - p0.y];

        // Interpolate intensity across the scanline h = intensity(x)
        std::vector<float> h_line = Interpolate(x_l, h_left[y - p0.y], x_r, h_right[y - p0.y]);
        
        for (int x = x_l; x <= x_r; x++) {
            float intensity = h_line[x - x_l];
            int new_r = (float)color.r * intensity;
            int new_g = (float)color.g * intensity;
            int new_b = (float)color.b * intensity;
            Color shaded_color = {new_r, new_g, new_b};
            PutPixel(renderer, x, y, shaded_color);
        }
    }

}


int main(int argc, char* argv[]) {
    SDL_Event event;
    SDL_Renderer* renderer;
    SDL_Window* window;
    int i;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(C_w, C_h, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    // Render
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    
    Point p0 = Point(-200, -250, 0.0);
    Point p1 = Point(200, 50, 1.0);
    Point p2 = Point(20, 250, 0.5);
   
    Color color = { 0, 255, 0 };
    DrawFilledTriangle(renderer, p0, p1, p2, color);
    Color color2 = { 0, 0, 0 };
    DrawWireframeTriangle(renderer, p0, p1, p2, color2);

    

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
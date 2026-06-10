#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

struct Vec3{
    float x, y, z;
};

struct Face{
    int a, b, c;
};

struct Point{
    int x, y;
};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color){
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    if(steep){
        std::swap(ax,ay);
        std::swap(bx, by);
    }
    if(ax > bx){
        std::swap(ax,bx);
        std::swap(ay, by);
    }
    int y = ay;
    // bresenham's algorithm
    int ierror = 0;
    for(int x = ax; x <= bx; x++){
        if(steep)
            framebuffer.set(y,x,color);
        else
            framebuffer.set(x, y, color);
        ierror+= 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror-= (2 * (bx-ax)) * (ierror > bx - ax);
    }
}
constexpr int width = 128;
constexpr int height = 128;

std::pair<int,int> projectTo2D(Vec3 &vec){
    return {static_cast<int>((vec.x + 1.0f) * (width) / 2.0f), static_cast<int>((vec.y+1.0f) * (height) / 2.0f)};
}

void triangleLegacy(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
    if(ay > by){
        std::swap(ax, bx);
        std::swap(ay,by);
    }
    if(ay > cy){
        std::swap(ax,cx);
        std::swap(ay,cy);
    }
    if(by > cy){
        std::swap(bx,cx);
        std::swap(by,cy);
    }

    int total_height = cy - ay;
    if(ay != by){
        int segment_height = by - ay;
        for(int y = ay; y <= by; y++){
            // a -> c
            int x1 = ax + ((cx-ax)*(y-ay))/total_height;
            // a -> b
            int x2 = ax + ((bx-ax)*(y-ay))/segment_height;
            for(int x = std::min(x1,x2); x <= std::max(x1,x2); x++){
                framebuffer.set(x, y, color);
            }
        }
    }
    if(by != cy){
        int segment_height = cy - by;
        for(int y = by; y <= cy; y++){
            // a' -> c
            int x1 = ax + ((cx - ax)*(y - ay))/total_height;
            // b -> c
            int x2 = bx + ((cx - bx)*(y - by))/segment_height;
            for(int x = std::min(x1,x2); x <= std::max(x1,x2); x++){
                framebuffer.set(x, y, color);
            }
        }
    }


    // line(ax, ay, bx, by, framebuffer, green);
    //  line(bx, by, cx, cy, framebuffer, green);
    //  line(cx, cy, ax, ay, framebuffer, red);
}


double signedTriangleArea(int ax, int ay, int bx, int by, int cx, int cy){
    return 0.5 * ((bx-ax)*(cy-ay) - (cx-ax)*(by-ay));
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color){
    int bb_minx = std::min(std::min(ax, bx), cx);
    int bb_miny = std::min(std::min(ay, by), cy);
    int bb_maxx = std::max(std::max(ax, bx), cx);
    int bb_maxy = std::max(std::max(ay, by), cy);
    double total_area = signedTriangleArea(ax,ay,bx,by,cx,cy);

    if(total_area == 0) return;

    #pragma omp parallel for
    for(int x = bb_minx; x <= bb_maxx; x++){
        for(int y = bb_miny; y <= bb_maxy; y++){
            double alpha = signedTriangleArea(x,y,bx,by,cx,cy) / total_area;
            double beta = signedTriangleArea(x,y,cx,cy,ax,ay) / total_area;
            double gamma = signedTriangleArea(x,y,ax,ay,bx,by) / total_area;

            if(alpha < 0 || beta < 0 || gamma < 0) continue;

            framebuffer.set(x, y, color);
        }
    }

}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    
    triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangle(115, 83, 80,  90, 85, 120, framebuffer, green);    
    


    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
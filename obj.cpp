#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
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
constexpr int width  = 1024;
constexpr int height = 1024;

std::pair<int,int> projectTo2D(Vec3 &vec){
    return {static_cast<int>((vec.x + 1.0f) * (width-1) / 2.0f), static_cast<int>((vec.y+1.0f) * (height-1) / 2.0f)};
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::ifstream file("diablo3_pose.obj");

    std::vector<Vec3> vertices;
    std::vector<Face> faces;
    std::string in;
    while(file >> in){
        if(in == "v"){
            Vec3 vec3;
            file >> vec3.x >> vec3.y >> vec3.z;
            std::cerr << in << ' ' << vec3.x << ' ' << vec3.y << ' ' << vec3.z << '\n';
            vertices.push_back(vec3);
        }
        else if(in == "f"){
            Face face;
            std::string as, bs, cs;
            file >> as >> bs >> cs;
            face.a = std::stoi(as.substr(0, as.find("/"))) - 1;
            face.b = std::stoi(bs.substr(0,bs.find("/"))) - 1;
            face.c = std::stoi(cs.substr(0, cs.find("/"))) - 1;
            std::cerr << in << ' ' << face.a << ' ' << face.b << ' ' << face.c << '\n';
            faces.push_back(face);
        }
    }

    for(auto &face: faces){
        Vec3 v0 = vertices[face.a];
        Vec3 v1 = vertices[face.b];
        Vec3 v2 = vertices[face.c];

        auto [x0,y0] = projectTo2D(v0);
        auto [x1,y1] = projectTo2D(v1);
        auto [x2,y2] = projectTo2D(v2);

        line(x0,y0,x2,y2, framebuffer, red);
        line(x1,y1,x2,y2, framebuffer, red);
        line(x1,y1,x0,y0, framebuffer, red);
    }

    for(auto &vert: vertices){
        auto [x, y] = projectTo2D(vert);
        framebuffer.set(x, y, white);
    }


    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <conio.h>
#include <chrono>
#include "Console.h"

struct WordD {
    std::string word;
    int x;
    int y;
    int idx;
    std::string typed;
};

std::vector<WordD> game;
std::vector<std::string> words;
const int W = 120;
const int H = 25;
bool GameOver;

std::string strTime(time_t t=0)
{
    if (t < 0)return "#n/a";
    tm lt;
    if (t == 0)t = time(0);
    char buf[50];
    localtime_s(&lt, &t);
    strftime(buf, 50, "%F %T", &lt);
    return std::string(buf);
}

std::string strTime(std::string const& fmt, time_t t=0)
{
    if (t < 0)return "#n/a";
    tm lt;
    if (t == 0)t = time(0);
    char buf[256];
    localtime_s(&lt, &t);
    strftime(buf, 256, fmt.c_str(), &lt);
    return std::string(buf);
}


void LoadWords()
{
    std::ifstream file("words.txt");
    if (file.is_open()) {
        while (!file.eof()) {
            std::string line;
            std::getline(file, line);
            size_t x = line.find_first_of(" -.");
            if (x == std::string::npos) {
                words.push_back(line);
            }
        }
    }
}

void DrawWords()
{
    Con::Cls();
    for (int i = 0; i < game.size(); i++) {
        Con::Print(game[i].x, game[i].y, game[i].word, 0x07);
        Con::Print(game[i].x, game[i].y, game[i].typed, 0x0e);
    }
}

void UpdateWords()
{
    for (auto it = begin(game); it != end(game); ++it){
        if (it->y < H) it->y++;
        if (it->y >= H) {
            GameOver = true;
        }
    }
}

void TypeWords(char const c)
{
    for (auto it = begin(game); it != end(game); ++it) {
        if (it->word[it->idx] == c) {
            it->typed += c;
            it->idx++;
            if (it->typed == it->word)it = game.erase(it);
        }
        else {
            it->idx = 0;
            it->typed = "";
        }
    }
    DrawWords();
}

void Run()
{
    int speed = 500;
    int nwords = 5;
    Con::Print(10, 10, "Loading Words...", 0xB);
    LoadWords();
    auto t0 = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point t1;
    bool done = false;
    GameOver = false;
    while (!done && !GameOver) {
        if (_kbhit()) {
            char c = _getch();
            switch (c) {
                case 27:
                    done = true;
                    break;
                default:
                    TypeWords(c);
                    break;
            }
        }
        t1 = std::chrono::steady_clock::now();
        auto elapsed = t1 - t0;
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        if (dur >= speed) {
            t0 = t1;
            if (game.size() < nwords) {
                WordD w;
                w.typed = "";
                w.word = words[rand() % (int)words.size()];
                w.x = rand() % W;
                w.y = 0;
                w.idx = 0;
                game.push_back(w);
            }
            UpdateWords();
            DrawWords();
        }
    }
}

int main()
{
    srand(time(0));
    Con::Init(true);
    Con::SetSize(W, H);
    Con::SetFixedWindow();
    Con::Cls();
    Run();
    Con::Restore();
    if (GameOver) {
        printf("You lost!\n");
    }
}

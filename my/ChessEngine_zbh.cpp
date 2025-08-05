#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <queue>
#include <cassert>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <random>
#include <chrono>
#include "ChessEngine.h"

using namespace std;

int getMedian(std::vector<int>& nums) {
    size_t n = nums.size();
    if (n == 0) {return 8;}
    std::nth_element(nums.begin(), nums.begin() + n / 2, nums.end());
    return n % 2 ? nums[n / 2] : (nums[n / 2 - 1] + nums[n / 2]) / 2;
}


ChessEngine::ChessEngine() = default;

int (*ChessEngine::m_map)[16] = nullptr;
int ChessEngine::maxMoveCount = 8;
bool ChessEngine::isBlack = true;
int ChessEngine::searchFloor = SEARCH_FLOOR;

ACEngine ChessEngine::blackEngine = {
        {"11111",  50000},
        {"011110", 4320},
        {"011100", 720},
        {"001110", 720},
        {"011010", 720},
        {"010110", 720},
        {"11110",  720},
        {"01111",  720},
        {"11011",  720},
        {"10111",  720},
        {"11101",  720},
        {"001100", 120},
        {"001010", 120},
        {"010100", 120},
        {"000100", 20},
        {"001000", 20},
};

ACEngine ChessEngine::whiteEngine = {
        {"22222",  50000},
        {"022220", 4320},
        {"022200", 720},
        {"002220", 720},
        {"022020", 720},
        {"020220", 720},
        {"22220",  720},
        {"02222",  720},
        {"22022",  720},
        {"20222",  720},
        {"22202",  720},
        {"002200", 120},
        {"002020", 120},
        {"020200", 120},
        {"000200", 20},
        {"002000", 20},};

ACEngine ChessEngine::blackForbidden({
    {"111111", -10000},       // 长连禁手 - 6子及以上
    {"011110", -1},       // 活四
    {"011112", -1},       // 冲四
    {"211110", -1},       // 冲四
    {"11110", -1},        // 冲四
    {"01111", -1},        // 冲四
    {"11011", -1},        // 跳四
    {"10111", -1},        // 跳四
    {"11101", -1},        // 跳四
    {"011100", -1},       // 活三
    {"001110", -1},       // 活三
    {"011010", -1},       // 活三
    {"010110", -1}        // 活三
});

ACEngine ChessEngine::blackFiveLoose({
    {"11111", 1000000},
});

void ChessEngine::initMap(int (*map)[16]) {
    m_map = map;
}

void ChessEngine::initMapWithSeq(vector<Coord> seq){
    int curChess = BLACK_CHESS;
    for (auto &coord: seq) {
        if (coord.x < 1 or coord.x > 15 or coord.y < 1 or coord.y > 15) continue;
        m_map[coord.x][coord.y] = curChess;
        curChess = WHITE_CHESS + BLACK_CHESS - curChess;
    }
}

bool ChessEngine::isEmptyBoard(){
    for (int i = 1; i <= 15; ++i) {
        for (int j = 1; j <= 15; ++j) {
            if (m_map[i][j] != NO_CHESS)return false;
        }
    }
    return true;
}

Coord ChessEngine::getMaxCoord() {
    Coord ret{};
    if (isEmptyBoard()) return {8, 8}; // center
    abSearch(searchFloor, -SEARCH_INFINITY, SEARCH_INFINITY, isBlack, ret);
    return ret;
}

//检查这个点有没有人赢
bool ChessEngine::someoneWin(Coord coord){
    return (checkByStep(coord, 0, 1) + checkByStep(coord, 0, -1) >= 4) or
        (checkByStep(coord, 1, 0) + checkByStep(coord, -1, 0) >= 4) or
        (checkByStep(coord, 1, 1) + checkByStep(coord, -1, -1) >= 4) or
        (checkByStep(coord, 1, -1) + checkByStep(coord, -1, 1) >= 4);
}

//检查这个点在这个方向上有几个连续的棋子
int ChessEngine::checkByStep(Coord now, int x_step, int y_step){
    int cc = 0;
    auto origin = now;
    now.x += x_step;
    now.y += y_step;
    while ((now.x > 0 and now.x < 16 and now.y > 0 and now.y < 16) and (m_map[now.x][now.y] == m_map[origin.x][origin.y])) {
        cc++;
        now.x += x_step;
        now.y += y_step;
    }
    return cc;
}

//返回这个棋子的字符表示
inline char ChessEngine::chessChar(int chessType){
    switch (chessType) {
        case 0:
            return '0';
        case WHITE_CHESS:
            return '2';
        case BLACK_CHESS:
            return '1';
        default:
            assert(false);
    }
}

//判断这个点附近有没有棋子 附近隔着一个棋子也算
inline bool ChessEngine::thereIsNoChessNearby(Coord coord){ // 5 * 5 的范围内没有棋子
    for (int i = max(1, coord.x - 2); i <= min(15, coord.x + 2); ++i) {
        for (int j = max(1, coord.y - 2); j <= min(15, coord.y + 2); ++j) {
            if (m_map[i][j] != NO_CHESS)return false;
        }
    }
    return true;
}


inline bool ChessEngine::isValidInMap(Coord coord){
    bool isEmpty = m_map[coord.x][coord.y] == NO_CHESS;
    if (!isBlack) return isEmpty;
    else{
        if (!isEmpty) return false;
        m_map[coord.x][coord.y] = BLACK_CHESS;
        bool isforbodden = isForbidden(coord);
        m_map[coord.x][coord.y] = NO_CHESS;
        return !isforbodden;
    }
}

//评估一个点所在位置放射状的四条线的评分和 越大对当前棋子越有利
bool ChessEngine::isForbidden(Coord coord){
    int ret = 0;
    char line[17] = {};
    //coord所在位置的竖向
    for (int x = 1, lineIndex = 0; x <= 15; ++x, lineIndex++) {
        line[lineIndex] = chessChar(m_map[x][coord.y]);
    }
    ret += getLineForbiddenScore(line, isBlack);
    memset(line, 0, sizeof line);
    //coord所在位置的横向
    for (int y = 1, lineIndex = 0; y <= 15; ++y, lineIndex++) {
        line[lineIndex] = chessChar(m_map[coord.x][y]);
    }
    ret += getLineForbiddenScore(line, isBlack);
    memset(line, 0, sizeof line);
    // y = -x + y0 + x0
    int b = coord.x + coord.y;
    if (b <= 16) {
        for (int x = b - 1, y = 1, lineIndex = 0; x >= 1; x--, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    } else {
        for (int x = 15, y = b - 15, lineIndex = 0; y <= 15; x--, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    }
    ret += getLineForbiddenScore(line, isBlack);
    memset(line, 0, sizeof line);
    // y = x + y0 - x0
    int a = coord.x - coord.y;
    if (a >= 0) {
        for (int x = a + 1, y = 1, lineIndex = 0; x <= 15; x++, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    } else {
        for (int x = 1, y = -a + 1, lineIndex = 0; y <= 15; x++, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    }
    ret += getLineForbiddenScore(line, isBlack);
    return ret <= -2;
}

int ChessEngine::getLineForbiddenScore(const char *line, bool isBlack){
    assert(isBlack);
    int forbiddenScore = blackForbidden.ACSearch(line);
    if (forbiddenScore <= -10000) return -1; // 长连
    else if (forbiddenScore <= -1){
        int fiveScore = blackFiveLoose.ACSearch(line);
        return fiveScore > 0? fiveScore: -1;
    } 
    return 0;
}

//现在要下的是黑棋
bool ChessEngine::isBlackNow(){ // TODO
    int blackCC = 0, whiteCC = 0;
    for (int i = 1; i <= 15; ++i) {
        for (int j = 1; j <= 15; ++j) {
            switch (m_map[i][j]) {
                case NO_CHESS:
                    continue;
                case WHITE_CHESS:
                    whiteCC++;
                    continue;
                case BLACK_CHESS:
                    blackCC++;
                    continue;
                default:
                    assert(false);
            }
        }
    }
    if (blackCC == whiteCC)return true;
    return false;
}
    
//评估一条线上的分 差值评分 越大对当前棋子越有利
int ChessEngine::getLineScore(const char *line, bool isBlack){
    int score = blackEngine.ACSearch(line) - whiteEngine.ACSearch(line);
    return isBlack ? score : -score;
}

//评估一个点所在位置放射状的四条线的评分和 越大对当前棋子越有利
int ChessEngine::evaluateOnePoint(bool isBlack, Coord coord){
    int ret = 0;
    char line[17] = {};
    //coord所在位置的竖向
    for (int x = 1, lineIndex = 0; x <= 15; ++x, lineIndex++) {
        line[lineIndex] = chessChar(m_map[x][coord.y]);
    }
    ret += getLineScore(line, isBlack);
    memset(line, 0, sizeof line);
    //coord所在位置的横向
    for (int y = 1, lineIndex = 0; y <= 15; ++y, lineIndex++) {
        line[lineIndex] = chessChar(m_map[coord.x][y]);
    }
    ret += getLineScore(line, isBlack);
    memset(line, 0, sizeof line);
    // y = -x + y0 + x0
    int b = coord.x + coord.y;
    if (b <= 16) {
        for (int x = b - 1, y = 1, lineIndex = 0; x >= 1; x--, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    } else {
        for (int x = 15, y = b - 15, lineIndex = 0; y <= 15; x--, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    }
    ret += getLineScore(line, isBlack);
    memset(line, 0, sizeof line);
    // y = x + y0 - x0
    int a = coord.x - coord.y;
    if (a >= 0) {
        for (int x = a + 1, y = 1, lineIndex = 0; x <= 15; x++, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    } else {
        for (int x = 1, y = -a + 1, lineIndex = 0; y <= 15; x++, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
    }
    ret += getLineScore(line, isBlack);
    return ret;
}

//评估整个棋盘 如果是黑棋返回黑棋的得分 如果是白棋返回白棋的得分
int ChessEngine::evaluateAll(bool isBlack){
    int ret = 0;
    char line[17] = {};

    //竖向
    for (int i = 1; i <= 15; ++i) {
        for (int j = 1, lineIndex = 0; j <= 15; ++j, lineIndex++) {
            line[lineIndex] = chessChar(m_map[i][j]);
        }
        int score = getLineScore(line, isBlack);
        ret += score;
    }
    //横向
    for (int j = 1; j <= 15; ++j) {
        for (int i = 1, lineIndex = 0; i <= 15; ++i, lineIndex++) {
            line[lineIndex] = chessChar(m_map[i][j]);
        }
        ret += getLineScore(line, isBlack);
    }
    //右上到左下
    for (int i = 5; i <= 15; ++i) {
        memset(line, 0, sizeof line);
        for (int x = i, y = 1, lineIndex = 0; x >= 1; x--, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
        ret += getLineScore(line, isBlack);
    }
    for (int i = 2; i <= 11; ++i) {
        memset(line, 0, sizeof line);
        for (int x = 15, y = 15 + i - x, lineIndex = 0; y <= 15; x--, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
        ret += getLineScore(line, isBlack);
    }
    // 左上到右下
    for (int i = 10; i >= 0; --i) {
        memset(line, 0, sizeof line);
        for (int x = 1, y = x + i, lineIndex = 0; y <= 15; x++, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
        ret += getLineScore(line, isBlack);
    }
    for (int i = -1; i >= -10; --i) {
        memset(line, 0, sizeof line);
        for (int y = 1, x = y - i, lineIndex = 0; x <= 15; x++, y++, lineIndex++) {
            line[lineIndex] = chessChar(m_map[x][y]);
        }
        ret += getLineScore(line, isBlack);
    }
    return ret; 
}

//生成所有可能的走法
ScoreCoordQueue ChessEngine::generatePossibleMove(bool isBlack){
    fstream file("save.txt", ios::in);
    vector<int> x_list, y_list;
    int x, y;
    while (file >> x >> y) {
        x_list.push_back(x);
        y_list.push_back(y);
    }
    file.close();

    ScoreCoordQueue ret;
    x = getMedian(x_list), y = getMedian(y_list);
    if ((!thereIsNoChessNearby({x, y}) and isValidInMap({x, y}))){
        int baseScore = evaluateOnePoint(isBlack, {x, y});//没有落子前的分数
        m_map[x][y] = isBlack ? BLACK_CHESS : WHITE_CHESS;
        int myScore = evaluateOnePoint(isBlack, {x, y});//我下这点我会得到的分数
        m_map[x][y] = isBlack ? WHITE_CHESS : BLACK_CHESS;
        int rivalScore = evaluateOnePoint(!isBlack, {x, y});//敌方下这点会得到的分数
        m_map[x][y] = NO_CHESS;
        ret.push({(myScore - baseScore) + (rivalScore - (-baseScore)), {x, y}});//要让我获益最大 或者能让敌方获益最大的点下棋
    }
    int NoChessNearbyCount = 0;
    int direction = 0, step_size = 1, steps_taken = 0;
    int directions[4][2] = {
        {0, 1},  // right
        {1, 0},  // down
        {0, -1}, // left
        {-1, 0}  // up
    };
    while(true){
        for (int i = 0; i < step_size; ++i) {
            x += directions[direction][0];
            y += directions[direction][1];
            if (x < 1 or x > 15 or y < 1 or y > 15) return ret;

            bool isNoChessNearby = thereIsNoChessNearby({x, y});
            NoChessNearbyCount += isNoChessNearby;
            if (NoChessNearbyCount > 25) return ret; // prune: 如果周围没有棋子 就停止搜索
            if (isNoChessNearby or !isValidInMap({x, y}))continue;
            int baseScore = evaluateOnePoint(isBlack, {x, y});//没有落子前的分数
            m_map[x][y] = isBlack ? BLACK_CHESS : WHITE_CHESS;
            int myScore = evaluateOnePoint(isBlack, {x, y});//我下这点我会得到的分数
            m_map[x][y] = isBlack ? WHITE_CHESS : BLACK_CHESS;
            int rivalScore = evaluateOnePoint(!isBlack, {x, y});//敌方下这点会得到的分数
            m_map[x][y] = NO_CHESS;
            ret.push({(myScore - baseScore) + (rivalScore - (-baseScore)), {x, y}});//要让我获益最大 或者能让敌方获益最大的点下棋
        }

        // Change direction
        direction = (direction + 1) % 4;

        // Increment steps after every two direction changes (right-left, down-up)
        steps_taken++;
        if (steps_taken == 2) {
            step_size++;
            steps_taken = 0;
        }
    }
    return ret;
}

//alpha-beta剪枝搜索
int ChessEngine::abSearch(int floor, int alpha, int beta, bool isBlack, Coord &searchResult){
    int tmpScore, moveCount = maxMoveCount;
    Coord tempSearchResult{};
    ScoreCoordQueue possibleMove = generatePossibleMove(isBlack);
    while(!possibleMove.empty() and moveCount--){
        auto now = possibleMove.top();
        possibleMove.pop();
        int x = now.coord.x, y = now.coord.y;
        m_map[x][y] = isBlack ? BLACK_CHESS : WHITE_CHESS;
        if (someoneWin({x, y})) {//如果有人赢了 必定是下这个子的人赢了
            searchResult = {x, y};
            tmpScore = evaluateAll(isBlack);//返回这个局面最高的得分，也就是赢局的分数
            m_map[x][y] = NO_CHESS;
            return tmpScore;
        }
        if (floor == 1) {//如果只看这一步子 那就是这一步子所有可能的得分中的最大值
            tmpScore = evaluateAll(isBlack);
            m_map[x][y] = NO_CHESS;
            if (tmpScore > alpha) {
                alpha = tmpScore;
                searchResult = {x, y};
            }
            continue;
        }
        tmpScore = -abSearch(floor - 1, -beta, -alpha, !isBlack, tempSearchResult);//不然得分就是我下了之后的对方的所能得到的最高分取负
        m_map[x][y] = NO_CHESS;
        if (tmpScore >= beta) {
            return beta;
        }
        if (tmpScore > alpha) {//取对方尽所有努力后得到最大值中的最小的一个 取负值后变成最大的一个
            alpha = tmpScore;
            searchResult = {x, y};
        }
    }
    return alpha;
}


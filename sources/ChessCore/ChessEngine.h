//
// Created by Cyril on 2023/11/24.
//

#ifndef AI_BACKGAMMON_CHESSENGINE_H
#define AI_BACKGAMMON_CHESSENGINE_H


#include <queue>
#include "General/General.h"
#include "ACEngine.h"

//可以5 7 9（比较慢了）
#ifndef SEARCH_FLOOR
#define SEARCH_FLOOR 4
#endif

//给possibleMove使用的结构体
//储存在这个坐标下的棋子的分数 方便排序
struct ScoreCoord {
    int score;
    Coord coord;
};

struct CompareScoreCoord {
    bool operator()(const ScoreCoord& a, const ScoreCoord& b) {
        // Return true if 'a' should come after 'b' (lower score, for descending order)
        return a.score < b.score;
    }
};

typedef std::priority_queue<ScoreCoord, std::vector<ScoreCoord>, CompareScoreCoord> ScoreCoordQueue;

class ChessEngine {
private:
    ChessEngine();

public:
    ChessEngine(const ChessEngine &) = delete;

    ChessEngine &operator=(const ChessEngine &) = delete;

    //初始化棋盘
    static void initMap(int map[16][16]);

    //初始化棋盘
    static void initMapWithSeq(vector<Coord> seq);

    //返回棋盘的最大得分点
    static Coord getMaxCoord();

    //检查这个点有没有人赢
    static bool someoneWin(Coord coord);

    static bool isValidInMap(Coord coord);
    //现在要下的是黑棋
    static bool isBlackNow();

private:
    //检查这个点在这个方向上有几个连续的棋子
    static int checkByStep(Coord now, int x_step, int y_step);

    //评估整个棋盘 如果是黑棋返回黑棋的得分 如果是白棋返回白棋的得分
    static int evaluateAll(bool isBlackNow);

    //返回这个棋子的字符表示
    static inline char chessChar(int chessType);

    //没有棋子可以下了
    static bool isNoChessDown();

    static bool isEmptyBoard();

    //判断这个点附近有没有棋子 附近隔着一个棋子也算
    static inline bool thereIsNoChessNearby(Coord coord);

    //评估一条线上的分 差值评分 越大对当前棋子越有利
    static int getLineScore(const char *line, bool isBlackNow);

    //alpha-beta剪枝搜索
    static int abSearch(int floor, int alpha, int beta, bool isBlackNow, Coord &searchResult);

    //生成所有可能的走法
    static ScoreCoordQueue generatePossibleMove(bool isBlack);

    static std::vector<ScoreCoord> generatePossibleMoveVec(bool isBlackNow);

    //评估一个点所在位置放射状的四条线的评分和 越大对当前棋子越有利
    static int evaluateOnePoint(bool isBlackNow, Coord coord);

    //判断这个点是否是禁手
    static bool isForbidden(Coord coord);

    //评估一条线上的禁手分 越大对当前棋子越有利
    static int getLineForbiddenScore(const char *line, bool isBlack);

private:
    //[横坐标→][纵坐标↓] 坐标边界1开始15结束 0--NO_CHESS
    static int (*m_map)[16];
    static int maxMoveCount;
    static ACEngine blackForbidden;
    static ACEngine blackFiveLoose;
    static ACEngine blackEngine;
    static ACEngine whiteEngine;

public:
    static bool isBlack;
    static int searchFloor;
};


#endif //AI_BACKGAMMON_CHESSENGINE_H

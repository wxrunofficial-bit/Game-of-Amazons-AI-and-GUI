/*
* 计算概论A 2025-2026 大作业：Amazons-bot
* 王显然 2500011736 北京大学化学与分子工程学院
* 运行环境：Visual Studio + EasyX
* UI界面用到的素材（如果助教老师要在网盘下载验收，请保证下面的素材均在项目根目录下）：
* 1.black.png(Merida黑皇后图片，透明背景)
* 2.white.png(Merdia白皇后图片，透明背景）
* 3.background.jpg(摄于开发者曾经的家周围，中国人民大学附属中学)
* 注：本人有意以后把这个文件公开，作为一个不够完美的基础范本供后来人参考，所以在下面自主编写了一些注释。
*/

#define _CRT_SECURE_NO_WARNINGS//一定要禁用安全警告
#undef UNICODE
#undef _UNICODE
//禁用unicode，使用多字节字符集来处理字符串

#include <graphics.h>//EasyX
#include <conio.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>


#define GRIDSIZE 8
#define OBSTACLE 2
#define judge_black 0
#define judge_white 1
#define grid_black 1
#define grid_white -1
#define INF 100000000
#define MAXDEPTH 256
//棋盘状态：0空，1黑，-1白，2障碍物

using namespace std;

//注意要加上前置声明，用于全局指针访问
class GraphicsEngine;
GraphicsEngine* g_gameInstance = nullptr;

int currBotColor;//AI持方
int gridInfo[GRIDSIZE][GRIDSIZE] = { 0 };
int dx[] = { -1,-1,-1,0,0,1,1,1 };
int dy[] = { -1,0,1,-1,1,-1,0,1 };

clock_t startTime;
const double TIME_LIMIT = 0.95;
bool timeOut = false;

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }//重载运算符，方便地判断两个点相同
};

struct MoveStep {
    int x0, y0; int x1, y1; int x2, y2;
    int score;
};

inline bool inMap(int x, int y) {
    return x >= 0 && x < GRIDSIZE && y >= 0 && y < GRIDSIZE;
}//不越界

bool ProcStep(int x0, int y0, int x1, int y1, int x2, int y2, int color, bool zhishishishi) {
    if ((!inMap(x0, y0)) || (!inMap(x1, y1)) || (!inMap(x2, y2))) return false;
    if (gridInfo[x0][y0] != color || gridInfo[x1][y1] != 0) return false;
    if ((gridInfo[x2][y2] != 0) && !(x2 == x0 && y2 == y0)) return false;//检查合法性，请你注意：可以在起点放障碍物，要特判
    if (!zhishishishi) {
        gridInfo[x0][y0] = 0;
        gridInfo[x1][y1] = color;
        gridInfo[x2][y2] = OBSTACLE;//模拟半回合实操
    }
    return true;
}


namespace AmazonAI {

    double mobilityTable[65];
    //回调函数指针，用于在思考时刷新UI
    void (*onProgressCallback)() = nullptr;

    void InitAI() {
        for (int i = 0; i <= 64; i++) {
            if (i == 0) mobilityTable[i] = -10.0;
            else mobilityTable[i] = log(i + 1.0) * 10.0;
            //对数打分表：“1-0”>>“10-9”，直觉上显然比线性优秀。似乎存在更优秀的打分方式，但是我尝试搓的遗传算法效果极烂
        }
    }
    //bfs,但是静态数组：比queue更快；搜索一方所有棋子到某一位置的最少步数；除了不是queue其实完全是标准的bfs
    void FastBFS(int color, int distMap[GRIDSIZE][GRIDSIZE]) {
        static Point q[100];
        int head = 0, tail = 0;
        for (int i = 0; i < GRIDSIZE; ++i)
            for (int j = 0; j < GRIDSIZE; ++j)
                distMap[i][j] = INF;

        for (int i = 0; i < GRIDSIZE; ++i) {
            for (int j = 0; j < GRIDSIZE; ++j) {
                if (gridInfo[i][j] == color) {
                    distMap[i][j] = 0;
                    q[tail++] = { i,j };
                }
            }
        }

        while (head < tail) {
            Point p = q[head++];
            int currentDist = distMap[p.x][p.y];

            for (int k = 0; k < 8; ++k) {
                int nx = p.x + dx[k];
                int ny = p.y + dy[k];
                if (inMap(nx, ny) && gridInfo[nx][ny] == 0) {
                    if (distMap[nx][ny] == INF) {
                        distMap[nx][ny] = currentDist + 1;
                        q[tail++] = { nx, ny };
                    }
                }
            }
        }
    }
    //计算某个棋子旁边有多少个空位（灵活度）：一条路走到黑
    inline int GetMobilityFast(int r, int c) {
        int count = 0;
        for (int k = 0; k < 8; ++k) {
            int x = r + dx[k];
            int y = c + dy[k];
            while (inMap(x, y) && gridInfo[x][y] == 0) {
                count++;
                x += dx[k];
                y += dy[k];
            }
        }
        return count;
    }
    //估值函数：对局面打分
    double Evaluate(int myColor) {
        int opColor = -myColor;

        static int distMy[GRIDSIZE][GRIDSIZE];
        static int distOp[GRIDSIZE][GRIDSIZE];

        FastBFS(myColor, distMy);
        FastBFS(opColor, distOp);

        double territoryScore = 0;//领地分
        double posScore = 0;//位置分

        for (int i = 0; i < GRIDSIZE; ++i) {
            for (int j = 0; j < GRIDSIZE; ++j) {
                if (gridInfo[i][j] != 0) continue;

                int d1 = distMy[i][j];
                int d2 = distOp[i][j];
                //领地分计算：比对方近就是我的，+1；位置分计算：离我更近且距离<=2，就为它位置的优越性做出贡献，+0.1
                if (d1 < d2) {
                    territoryScore += 1.0;
                    if (d1 <= 2) posScore += 0.1;
                }
                else if (d1 > d2) {
                    territoryScore -= 1.0;
                    if (d2 <= 2) posScore -= 0.1;
                }
            }
        }

        double mobilityMy = 0;//机动性分
        double mobilityOp = 0;//位置控制分
        double centerControlMy = 0;
        double centerControlOp = 0;

        for (int i = 0; i < GRIDSIZE; ++i) {
            for (int j = 0; j < GRIDSIZE; ++j) {
                if (gridInfo[i][j] == myColor) {
                    mobilityMy += mobilityTable[GetMobilityFast(i, j)];//机动性查表
                    centerControlMy -= (pow(i - 3.5, 2) + pow(j - 3.5, 2));//经验地认为3.5；3.5是最好的，递减值为距离的平方的位置分
                }
                else if (gridInfo[i][j] == opColor) {
                    mobilityOp += mobilityTable[GetMobilityFast(i, j)];
                    centerControlOp -= (pow(i - 3.5, 2) + pow(j - 3.5, 2));
                }
            }
        }

        double wTerritory = 100.0;
        double wMobility = 5.0;
        double wPos = 2.0;
        /*调节上述三者的权重，这是我粗略试的几种权重中最好的，我试想的梯度下降法和遗传方法都坠机了，所以这里很粗糙。
        * 但我很确信领地分>>机动性分>位置分
        */
        return territoryScore * wTerritory + (mobilityMy - mobilityOp) * wMobility + posScore * 5.0 + (centerControlMy - centerControlOp) * wPos;
    }

    bool compareMoves(const MoveStep& a, const MoveStep& b) {
        return a.score > b.score;
    }

    vector<MoveStep> GenerateMoves(int color) {//生成所有符合要求的走法
        vector<MoveStep> moves;
        moves.reserve(500);

        for (int i = 0; i < GRIDSIZE; ++i) {
            for (int j = 0; j < GRIDSIZE; ++j) {
                if (gridInfo[i][j] == color) {
                    for (int k = 0; k < 8; ++k) {
                        int x = i, y = j;
                        while (true) {
                            x += dx[k];
                            y += dy[k];
                            if (!inMap(x, y) || gridInfo[x][y] != 0) break;

                            gridInfo[i][j] = 0;
                            gridInfo[x][y] = color;//暂时移走棋子以便后面射箭模拟

                            for (int l = 0; l < 8; ++l) {
                                int sx = x, sy = y;
                                while (true) {
                                    sx += dx[l];
                                    sy += dy[l];
                                    if (!inMap(sx, sy)) break;
                                    if (gridInfo[sx][sy] != 0) break;
                                    /*roughScore：优化剪枝的粗略打分，尝试让好棋靠前，从而让ab剪枝更早发生，提升搜索速度
                                    *粗暴的公式lol：可达格子的数目+（bool 是否紧邻对方棋子）*5
                                    *粗糙地表达了作者对机动性和攻击性的向往。但是这里依然乱写权重+对攻击性解读很糙
                                    */
                                    int roughScore = 0;
                                    roughScore += GetMobilityFast(x, y);

                                    for (int m = 0; m < 8; ++m) {
                                        int tx = sx + dx[m];
                                        int ty = sy + dy[m];
                                        if (inMap(tx, ty) && gridInfo[tx][ty] == -color) {
                                            roughScore += 5;
                                        }
                                    }
                                    moves.push_back({ i,j,x,y,sx,sy,roughScore });
                                }
                            }
                            gridInfo[x][y] = 0;
                            gridInfo[i][j] = color;//记得回溯刚才暂时改变的棋盘！
                        }
                    }
                }
            }
        }
        if (!moves.empty()) {
            sort(moves.begin(), moves.end(), compareMoves);//排序，先搜好棋，roughScore降序
        }
        return moves;
    }
    //模拟己方回合落子全过程，封装成函数简化许多。
    void MakeMove(const MoveStep& m, int color) {
        gridInfo[m.x0][m.y0] = 0;
        gridInfo[m.x1][m.y1] = color;
        gridInfo[m.x2][m.y2] = OBSTACLE;
    }
    //回溯封装
    void UnmakeMove(const MoveStep& m, int color) {
        gridInfo[m.x2][m.y2] = 0;
        gridInfo[m.x1][m.y1] = 0;
        gridInfo[m.x0][m.y0] = color;
    }

    double AlphaBeta(int depth, double alpha, double beta, int player) {//depth,还要往下搜几层
        static int checkCounter = 0;//防超时操作：每500个节点检查一次是否超时
        static clock_t lastUiUpdate = 0; // UI刷新计时器

        if (++checkCounter > 500) {
            checkCounter = 0;
            clock_t now = clock();
            if ((double)(now - startTime) / CLOCKS_PER_SEC > TIME_LIMIT) {//注意要转换成秒再比
                timeOut = true;
            }
            // 如果回调函数存在，且距离上次刷新超过0.03秒，则刷新UI
            // 既保证了雪花不卡顿，又不会因为频繁重绘导致AI算力严重下降
            if (onProgressCallback && (double)(now - lastUiUpdate) / CLOCKS_PER_SEC > 0.03) {
                onProgressCallback();
                lastUiUpdate = now;
            }
        }
        if (timeOut) return 0;

        if (depth == 0) {
            return Evaluate(currBotColor);//搜到头了给叶子打分
        }

        vector<MoveStep> moves = GenerateMoves(player);
        if (moves.empty()) {//没得走了，只可能是我已赢或者它已赢
            if (player == currBotColor) return -200000.0 + depth;//输了
            else return 200000.0 - depth;//赢了
        }
        //minimax逻辑+ab剪枝的dfs过程
        if (player == currBotColor) {
            double maxEval = -INF;
            for (const auto& move : moves) {
                MakeMove(move, player);
                double eval = AlphaBeta(depth - 1, alpha, beta, -player);
                UnmakeMove(move, player);

                if (timeOut) return maxEval;

                maxEval = max(maxEval, eval);
                alpha = max(alpha, maxEval);//更新max能保证的下限
                if (beta <= alpha) break;
            }
            return maxEval;
        }
        else {
            double minEval = INF;
            for (const auto& move : moves) {
                MakeMove(move, player);
                double eval = AlphaBeta(depth - 1, alpha, beta, -player);
                UnmakeMove(move, player);

                if (timeOut) return minEval;

                minEval = min(minEval, eval);
                beta = min(beta, minEval);//更新min能锁死的上限
                if (beta <= alpha) break;
            }
            return minEval;
        }
    }

    MoveStep GetBestMove() {
        InitAI();
        startTime = clock();
        timeOut = false;

        MoveStep bestMove = { -1, -1, -1, -1, -1, -1 };
        //只要没超时就一直搜
        for (int depth = 1; depth <= MAXDEPTH; ++depth) {
            double bestVal = -INF;
            double alpha = -INF;
            double beta = INF;
            vector<MoveStep> moves = GenerateMoves(currBotColor);
            MoveStep currentDepthBestMove = moves.empty() ? bestMove : moves[0];
            //记录深度为depth时的本层BestMove，先存上一个防止没搜就超时输出不了
            bool foundBetter = false;

            for (auto& move : moves) {
                MakeMove(move, currBotColor);
                double val = AlphaBeta(depth - 1, alpha, beta, -currBotColor);
                UnmakeMove(move, currBotColor);

                if (timeOut) break;

                if (val > bestVal) {
                    bestVal = val;
                    currentDepthBestMove = move;
                    foundBetter = true;
                }
                alpha = max(alpha, bestVal);
            }

            if (timeOut) {
                break;//这一层没搜完必须把整层扔了，留下上一层的bestmove
            }
            else {
                bestMove = currentDepthBestMove;//搜完了更新本层bestmove
                if (bestVal > 100000.0) break;
            }
        }
        return bestMove;
    }
}


//EasyX实现的图形界面模块
const COLORREF C_BOARD_LIGHT = RGB(196, 224, 226);//浅色格 
const COLORREF C_BOARD_DARK = RGB(74, 137, 145);//深色格 
const COLORREF C_BOARD_BORDER = RGB(50, 90, 100);//格子分隔线
const COLORREF C_OBSTACLE = RGB(196, 137, 78);
const COLORREF C_HINT_MOVE = RGB(184, 217, 156);//移动提示：绿
const COLORREF C_HINT_SHOOT = RGB(65, 105, 225);//射箭提示：深蓝
const COLORREF C_SELECTED = RGB(0, 255, 255);//选中框：青色
const COLORREF C_BTN_NORMAL = RGB(70, 70, 70);//与按钮区分
const COLORREF C_BTN_HOVER = RGB(100, 100, 100);//按钮
const COLORREF C_BTN_DISABLED = RGB(50, 50, 50);//禁用按钮颜色
const COLORREF C_TEXT = RGB(255, 255, 255);//文字颜色
const COLORREF C_TEXT_DISABLED = RGB(100, 100, 100);//禁用文字颜色
const COLORREF C_WATERMARK = RGB(200, 200, 200);//水印颜色
const COLORREF C_HINT_TEXT = RGB(255, 255, 0);//操作提示文字颜色(黄)
const int CELL_SIZE = 80;     //格子大小
const int BOARD_PIXEL_SIZE = 640;//棋盘大小
const int BOARD_OFFSET_X = 40; //棋盘左边距
const int BOARD_OFFSET_Y = 40; //棋盘上边距
const int MENU_WIDTH = 220;    //菜单宽度

//窗口总大小
const int WIN_WIDTH = BOARD_OFFSET_X + BOARD_PIXEL_SIZE + 40 + MENU_WIDTH + 20;
const int WIN_HEIGHT = BOARD_OFFSET_Y + BOARD_PIXEL_SIZE + 40;

//图片缩放大小
const int IMG_SIZE = 70;

//文件名
const string HISTORY_FILE = "history_record.txt";
const string BG_FILE = "background.jpg";

//PNG 透明绘图辅助函数,EasyX原生的putimage会直接把透明变黑
void putimagePNG(int x, int y, IMAGE* pSrcImg) {
    DWORD* pDst = GetImageBuffer(NULL);
    DWORD* pSrc = GetImageBuffer(pSrcImg);//分别获取屏幕和图片的显存指针
    int srcW = pSrcImg->getwidth();
    int srcH = pSrcImg->getheight();
    int dstW = getwidth();
    int dstH = getheight();

    int iX = 0, iY = 0, iW = srcW, iH = srcH;
    if (x < 0) { iX = -x; iW += x; x = 0; }
    if (y < 0) { iY = -y; iH += y; y = 0; }
    if (x + iW > dstW) iW = dstW - x;
    if (y + iH > dstH) iH = dstH - y;//越界处理

    if (iW <= 0 || iH <= 0) return;

    for (int iy = 0; iy < iH; iy++) {
        DWORD* srcRow = pSrc + (iy + iY) * srcW + iX;
        DWORD* dstRow = pDst + (y + iy) * dstW + x;
        for (int ix = 0; ix < iW; ix++) {
            int a = srcRow[ix] >> 24;//像素透明度
            if (a == 0) continue;//全透直接不画
            if (a == 255) {
                dstRow[ix] = srcRow[ix];//全不透直接覆盖
            }
            else {//提取二者的RGB
                int r1 = (srcRow[ix] & 0xFF0000) >> 16;
                int g1 = (srcRow[ix] & 0x00FF00) >> 8;
                int b1 = srcRow[ix] & 0x0000FF;
                int r2 = (dstRow[ix] & 0xFF0000) >> 16;
                int g2 = (dstRow[ix] & 0x00FF00) >> 8;
                int b2 = dstRow[ix] & 0x0000FF;
                int r = (r1 * a + r2 * (255 - a)) >> 8;
                int g = (g1 * a + g2 * (255 - a)) >> 8;
                int b = (b1 * a + b2 * (255 - a)) >> 8;
                dstRow[ix] = (r << 16) | (g << 8) | b;//半透混合，按照透明度比例
            }
        }
    }
}

//获取时间戳，用于输入历史记录
string GetTimeStamp() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
    return string(buf);
}

//棋盘状态
struct BoardState {
    int grid[GRIDSIZE][GRIDSIZE];
    int turn;
};

//按钮类
struct Button {
    int x, y, w, h;
    string text;
    bool hover;//鼠标是否悬停

    bool isClicked(int mx, int my) {//鼠标是否在按钮范围
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }
};

//历史记录条目
struct HistoryRecordEntry {
    string timestamp;//时间戳
    long fileOffset;//文件中的偏移量
};

//雪花！灵感来自aistudio.google.com的"Let it snow"
struct SnowFlake {
    double x, y;
    double speed;
    double drift;//漂移！
    double radius;
};

class GraphicsEngine {//游戏主引擎类
private:
    vector<BoardState> history;//悔棋栈
    int humanColor;
    int currentTurn;

    enum State { MENU, IDLE, SELECTED, MOVED, HISTORY_SELECT };//菜单；等待点击；选了棋子；移了棋子
    State state;
    Point selectedSrc;
    Point selectedDst;

    // UI按钮
    vector<Button> buttons;
    string statusMsg;

    // 图片资源
    IMAGE imgBlack;//黑皇后
    IMAGE imgWhite;//白皇后
    IMAGE imgBg; //背景图

    //历史记录选择列表
    vector<HistoryRecordEntry> recordList;
    vector<Button> recordButtons;

    //游戏进行中状态标志
    bool isReplaying;

    vector<SnowFlake> snowFlakes;
    const int MAX_SNOW = 2000; //雪花数量

public:
    //保存AI开始计算前的稳定棋盘状态，这是为了实现AI思考时画面中的雪花不会卡住且盘面不会乱动
    int stableGrid[GRIDSIZE][GRIDSIZE];

    GraphicsEngine() {
        g_gameInstance = this;//将自己注册到全局指针，方便AI回调

        state = MENU;//初始化
        humanColor = grid_black;
        currentTurn = grid_black;
        selectedSrc = { -1,-1 };
        selectedDst = { -1,-1 };
        isReplaying = false;

        initgraph(WIN_WIDTH, WIN_HEIGHT);//EasyX：创建绘图窗口
        setbkmode(TRANSPARENT);//文字背景透明

        //加载图片资源
        loadimage(&imgBlack, _T("black.png"), IMG_SIZE, IMG_SIZE);
        loadimage(&imgWhite, _T("white.png"), IMG_SIZE, IMG_SIZE);
        loadimage(&imgBg, _T(BG_FILE.c_str()), WIN_WIDTH, WIN_HEIGHT);

        //初始化雪花
        InitSnow();

        //初始化按钮位置和间距
        int bx = BOARD_OFFSET_X + BOARD_PIXEL_SIZE + 40;
        int by = BOARD_OFFSET_Y + 20;
        int gap = 75;

        buttons.push_back({ bx,by,180,50,"新游戏（您执黑）",false });
        buttons.push_back({ bx,by + gap,180,50,"新游戏（您执白）",false });
        buttons.push_back({ bx,by + gap * 2,180,50,"存盘",false });
        buttons.push_back({ bx,by + gap * 3,180,50,"读盘",false });
        buttons.push_back({ bx,by + gap * 4,180,50,"悔棋",false });
        buttons.push_back({ bx,by + gap * 5,180,50,"历史回放",false });
        buttons.push_back({ bx,by + gap * 6,180,50,"退出",false });
    }

    ~GraphicsEngine() {//程序退出后自动关闭EasyX窗口，析构函数
        closegraph();
    }


    static void KeepAlive() {//我最喜欢的创意，可惜改进的有点晚没写实验报告里。用于在AI计算的间隙偷偷刷新界面让雪花继续飘
        if (g_gameInstance) {
            //保存当前混乱的AI搜索现场（盘面信息此刻是AI算到一半的残局）
            int tempGrid[GRIDSIZE][GRIDSIZE];
            memcpy(tempGrid, gridInfo, sizeof(gridInfo));

            //载入进入AI前的稳定盘面信息（这样渲染出来的就是静止的）
            memcpy(gridInfo, g_gameInstance->stableGrid, sizeof(gridInfo));

            //刷新一帧画面
            g_gameInstance->Render();

            //恢复混乱的AI现场，让AI继续去算
            memcpy(gridInfo, tempGrid, sizeof(gridInfo));
        }
    }

    //初始化雪花
    void InitSnow() {
        snowFlakes.clear();
        for (int i = 0; i < MAX_SNOW; i++) {
            snowFlakes.push_back({ (double)(rand() % WIN_WIDTH),
            (double)(rand() % WIN_HEIGHT),
            (rand() % 100) / 50.0 + 1.0,
            (rand() % 100) / 1000.0,
            (rand() % 10) / 5 + 0.2
                });
        }
    }

    //雪花
    void DrawAndMoveSnow() {
        setfillcolor(WHITE);
        setlinecolor(WHITE);

        for (auto& f : snowFlakes) {
            //更新位置
            f.y += f.speed;
            f.x += sin(f.y * 0.02 + f.drift) * 0.5; //正弦飘动

            //到边界后重置位置
            if (f.y > WIN_HEIGHT)
            {
                f.y = -5;
                f.x = rand() % WIN_WIDTH;
            }
            if (f.x > WIN_WIDTH) f.x = 0;
            if (f.x < 0) f.x = WIN_WIDTH;

            //绘制：我本来想弄半透明效果，但 EasyX 默认 solidcircle 不支持 alpha
            //直接画实心圆，效果也不错
            solidcircle((int)f.x, (int)f.y, f.radius);
        }
    }


    void DrawDot(int cx, int cy, int size, COLORREF color) {
        setfillcolor(color);
        setlinecolor(color);
        fillcircle(cx, cy, size / 2);
    }//画提示小圆点

    void DrawImagePiece(int cx, int cy, IMAGE* img) {
        int x = cx - IMG_SIZE / 2;
        int y = cy - IMG_SIZE / 2;
        putimagePNG(x, y, img);
    }//放置图片棋子

    void Render() {
        BeginBatchDraw();

        //画背景图
        putimage(0, 0, &imgBg);

        //图上叠雪花
        DrawAndMoveSnow();

        if (state == HISTORY_SELECT) {
            //画历史记录选择界面
            setfillcolor(RGB(30, 30, 30));
            // solidrectangle(20, 20, WIN_WIDTH - 20, WIN_HEIGHT - 20);

            settextcolor(WHITE);
            settextstyle(30, 0, _T("Consolas"));
            outtextxy(40, 40, "请选择要回放的历史对局：");

            settextstyle(20, 0, _T("Consolas"));
            for (auto& btn : recordButtons) {
                setfillcolor(btn.hover ? C_BTN_HOVER : C_BTN_NORMAL);//根据光标位置改变按钮颜色
                setlinecolor(WHITE); // 按钮边框
                setlinestyle(PS_SOLID, 1);
                fillrectangle(btn.x, btn.y, btn.x + btn.w, btn.y + btn.h);//矩形背景

                int tx = btn.x + 20;
                int ty = btn.y + (btn.h - textheight(btn.text.c_str())) / 2;
                outtextxy(tx, ty, btn.text.c_str());
            }

            outtextxy(40, WIN_HEIGHT - 60, "按鼠标右键返回游戏");
        }
        else {
            //画棋盘
            for (int i = 0; i < GRIDSIZE; i++) {
                for (int j = 0; j < GRIDSIZE; j++) {
                    //如果是障碍物，直接跳过不画棋盘格，从而露出下方的背景和雪花
                    if (gridInfo[i][j] == OBSTACLE) continue;

                    int x = BOARD_OFFSET_X + j * CELL_SIZE;
                    int y = BOARD_OFFSET_Y + i * CELL_SIZE;

                    setfillcolor(((i + j) % 2 == 0) ? C_BOARD_DARK : C_BOARD_LIGHT);//深浅格，类似国际象棋
                    solidrectangle(x, y, x + CELL_SIZE, y + CELL_SIZE);

                    setlinecolor(C_BOARD_BORDER);
                    setlinestyle(PS_SOLID, 1);
                    rectangle(x, y, x + CELL_SIZE, y + CELL_SIZE);

                    if (state == SELECTED && i == selectedSrc.x && j == selectedSrc.y) {//格子被鼠标选中时画选中框
                        setlinecolor(C_SELECTED);
                        setlinestyle(PS_SOLID, 3);
                        rectangle(x + 2, y + 2, x + CELL_SIZE - 2, y + CELL_SIZE - 2);
                    }
                }
            }

            //画提示点
            vector<Point> hints;
            COLORREF hintColor = C_HINT_MOVE;

            if (state == SELECTED) {
                hints = GetReachable(selectedSrc.x, selectedSrc.y);
                hintColor = C_HINT_MOVE;
            }
            else if (state == MOVED) {
                hints = GetReachable(selectedDst.x, selectedDst.y);
                hintColor = C_HINT_SHOOT;
            }
            for (auto& p : hints) {
                int cx = BOARD_OFFSET_X + p.y * CELL_SIZE + CELL_SIZE / 2;
                int cy = BOARD_OFFSET_Y + p.x * CELL_SIZE + CELL_SIZE / 2;
                DrawDot(cx, cy, CELL_SIZE * 0.2, hintColor);
            }

            //画棋子
            for (int i = 0; i < GRIDSIZE; i++) {
                for (int j = 0; j < GRIDSIZE; j++) {
                    int cx = BOARD_OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2;
                    int cy = BOARD_OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2;
                    int type = gridInfo[i][j];

                    if (type == OBSTACLE) {

                    }
                    else if (type == grid_black) {
                        DrawImagePiece(cx, cy, &imgBlack);
                    }
                    else if (type == grid_white) {
                        DrawImagePiece(cx, cy, &imgWhite);
                    }
                }
            }
        }

        //画菜单栏
        int menuX = BOARD_OFFSET_X + BOARD_PIXEL_SIZE + 20;

        //右上角状态文字
        settextcolor(WHITE);
        setbkmode(TRANSPARENT);
        settextstyle(20, 0, _T("Consolas"));
        outtextxy(menuX, BOARD_OFFSET_Y - 20, statusMsg.c_str());

        //画按钮
        settextstyle(16, 0, _T("Arial"));
        for (int i = 0; i < buttons.size(); ++i) {
            Button& btn = buttons[i];

            if (i == 4 && state == MOVED) {
                setfillcolor(C_BTN_DISABLED);
                settextcolor(C_TEXT_DISABLED);//要求悔棋悔整步可以简化逻辑，此时在屏幕上禁用悔棋按钮
            }
            else {
                setfillcolor(btn.hover ? C_BTN_HOVER : C_BTN_NORMAL);
                settextcolor(C_TEXT);
            }

            //加按钮边框
            setlinecolor(WHITE);
            setlinestyle(PS_SOLID, 1);
            fillrectangle(btn.x, btn.y, btn.x + btn.w, btn.y + btn.h);

            int tx = btn.x + (btn.w - textwidth(btn.text.c_str())) / 2;
            int ty = btn.y + (btn.h - textheight(btn.text.c_str())) / 2;
            outtextxy(tx, ty, btn.text.c_str());
        }

        //author：王显然 北京大学化学与分子工程学院
        settextcolor(C_WATERMARK);
        settextstyle(16, 0, _T("Arial"));
        string wm1 = "Developed by";
        string wm2 = "王显然 CCME PKU";
        int wmWidth1 = textwidth(wm1.c_str());
        int wmWidth2 = textwidth(wm2.c_str());
        int wmX = WIN_WIDTH - max(wmWidth1, wmWidth2) - 10;
        int wmY = WIN_HEIGHT - 40;

        outtextxy(wmX, wmY, wm1.c_str());
        outtextxy(wmX, wmY + 20, wm2.c_str());

        //回放操作提示，黄字
        if (isReplaying) {
            settextcolor(C_HINT_TEXT);
            string hint = "【提示】左键暂停 / 右键返回";
            outtextxy(menuX, wmY - 40, hint.c_str());
        }

        EndBatchDraw();
    }

    void InitBoard() {//初始化盘面
        memset(gridInfo, 0, sizeof(gridInfo));
        gridInfo[0][2] = gridInfo[2][0] = gridInfo[5][0] = gridInfo[7][2] = grid_black;
        gridInfo[0][5] = gridInfo[2][7] = gridInfo[5][7] = gridInfo[7][5] = grid_white;
        currentTurn = grid_black;
        history.clear();
        SaveState();
    }

    void SaveState() {//存状态以备悔棋
        BoardState s; memcpy(s.grid, gridInfo, sizeof(gridInfo)); s.turn = currentTurn; history.push_back(s);
    }

    void Undo() {
        if (state == MOVED) return;

        if (history.size() > 2) {//悔棋同时去除自己刚下的和AI刚下的
            history.pop_back(); history.pop_back();
            BoardState s = history.back();
            memcpy(gridInfo, s.grid, sizeof(gridInfo));
            currentTurn = s.turn;
            state = IDLE;
        }
        else if (history.size() == 2) {//特判：万一人手速极快在AI落子之前悔棋且是全局第一回合的话，则只去一个
            history.pop_back();
            BoardState s = history.back();
            memcpy(gridInfo, s.grid, sizeof(gridInfo));
            currentTurn = s.turn;
            state = IDLE;
        }
    }
    //存盘
    void SaveToFile() {
        ofstream out("savegame.txt");
        if (out) {
            out << currentTurn << " " << humanColor << endl;
            out << history.size() << endl;
            for (const auto& s : history) {
                out << s.turn << endl;
                for (int i = 0; i < GRIDSIZE; i++) {
                    for (int j = 0; j < GRIDSIZE; j++) out << s.grid[i][j] << " ";
                    out << endl;
                }
            }//保存历史盘面
            out << state << " " << selectedSrc.x << " " << selectedSrc.y << " "
                << selectedDst.x << " " << selectedDst.y << endl;
            //保存UI状态：是否选中棋子？是否移动完了？是否射箭完了？
            for (int i = 0; i < GRIDSIZE; i++) {
                for (int j = 0; j < GRIDSIZE; j++) out << gridInfo[i][j] << " ";
                out << endl;
            }
            out.close();
            statusMsg = "已保存！";
        }
    }
    //读盘
    void LoadFromFile() {
        ifstream in("savegame.txt");
        if (in) {
            in >> currentTurn >> humanColor;
            size_t histSize;
            in >> histSize;
            history.clear();
            //恢复栈
            for (size_t k = 0; k < histSize; k++) {
                BoardState s;
                in >> s.turn;
                for (int i = 0; i < GRIDSIZE; i++)
                    for (int j = 0; j < GRIDSIZE; j++)
                        in >> s.grid[i][j];
                history.push_back(s);
            }
            //恢复UI状态
            int sVal, sx, sy, dx, dy;
            in >> sVal >> sx >> sy >> dx >> dy;
            state = (State)sVal;
            selectedSrc = { sx, sy };
            selectedDst = { dx, dy };
            for (int i = 0; i < GRIDSIZE; i++)
                for (int j = 0; j < GRIDSIZE; j++)
                    in >> gridInfo[i][j];
            in.close();
            //恢复棋盘
            statusMsg = "已载入！";
        }
    }
    //结束游戏自动保存历史记录
    void AutoSaveRecord(int winnerColor) {
        ofstream out(HISTORY_FILE, ios::app);
        if (out) {
            out << "START_GAME" << endl;
            out << GetTimeStamp() << endl;
            out << humanColor << " " << winnerColor << endl; 
            out << history.size() << endl;
            //戳
            for (const auto& s : history) {
                out << s.turn << endl;
                for (int i = 0; i < GRIDSIZE; i++) {
                    for (int j = 0; j < GRIDSIZE; j++) out << s.grid[i][j] << " ";
                    out << endl;
                }
            }//历史过程
            out << "END_GAME" << endl;
            out.close();
            statusMsg = "对局已自动记录";
        }
    }
    //菜单录像列表
    void LoadRecordList() {
        recordList.clear();
        recordButtons.clear();
        ifstream in(HISTORY_FILE);
        if (!in) {
            statusMsg = "你发现了一片无人的旷野...";
            return;
        }

        string line;
        long currentPos = 0;
        int count = 0;

        currentPos = in.tellg(); // 预读位置

        while (getline(in, line)) {
            if (line == "START_GAME") {
                long gameStartOffset = currentPos;
                string ts; getline(in, ts);

                //读取同一行的两个颜色
                string lineColors; getline(in, lineColors);
                stringstream ss(lineColors);
                int hColor, wColor;
                ss >> hColor >> wColor;

                //读取步数
                string lineSteps; getline(in, lineSteps);
                int steps = atoi(lineSteps.c_str());

                string sideStr = (hColor == grid_black) ? "[执黑]" : "[执白]";
                string resStr = (wColor == hColor) ? "结果:胜" : "结果:负";
                string btnText = ts.substr(0, 16) + " " + sideStr + " " + resStr;

                recordList.push_back({ ts, gameStartOffset });

                int by = 80 + count * 45;
                if (by < WIN_HEIGHT - 80) {
                    recordButtons.push_back({ 40, by, 500, 35, btnText, false });//按钮生成
                    count++;
                }

                //2026.1.11debug: 快速跳过数据行，不再逐行getline，解决卡顿
                string dummy;
                // 每一步数据：1行turn + 8行grid = 9行 (GRIDSIZE+1)
                for (int k = 0; k < steps * (GRIDSIZE + 1); k++) {
                    getline(in, dummy);
                }
            }
            currentPos = in.tellg();
        }
        in.close();
        if (recordList.empty()) statusMsg = "暂无记录，快来下棋吧！";
    }
    //回放一局
    void PlayBackSpecificGame(long fileOffset) {
        ifstream in(HISTORY_FILE);
        if (!in) return;

        in.seekg(fileOffset);//跳转文件指定位置

        string line;
        getline(in, line); // 吃掉 START_GAME
        string ts; getline(in, ts); // 吃掉 timestamp

        int hColor, wColor;
        in >> hColor >> wColor;

        size_t steps;
        in >> steps;

        // 关键：吃掉步数后的换行符，防止下面读取空行
        string dummy; getline(in, dummy);

        //按序加载所有盘面
        vector<BoardState> replayData;
        for (size_t k = 0; k < steps; k++) {
            BoardState s;
            in >> s.turn;
            for (int i = 0; i < GRIDSIZE; i++)
                for (int j = 0; j < GRIDSIZE; j++)
                    in >> s.grid[i][j];
            replayData.push_back(s);
        }
        in.close();

        if (replayData.empty()) return;
        BoardState backupState; memcpy(backupState.grid, gridInfo, sizeof(gridInfo));//可能是下一半开始回放另一局，所以要备份盘面
        State backupUIState = state;
        string backupMsg = statusMsg;

        state = IDLE;
        isReplaying = true;
        bool isPaused = false;

        for (size_t i = 0; i < replayData.size(); ) {
            ExMessage msg;
            while (peekmessage(&msg, EM_MOUSE)) {
                if (msg.message == WM_LBUTTONDOWN) isPaused = !isPaused;//左键暂停
                else if (msg.message == WM_RBUTTONDOWN) goto END_PLAYBACK;//右键继续
            }

            if (isPaused) {
                statusMsg = "暂停 (左键继续)";
                Render();
                Sleep(50);
                continue;
            }

            memcpy(gridInfo, replayData[i].grid, sizeof(gridInfo));
            statusMsg = "回放中: " + to_string(i + 1) + "/" + to_string(replayData.size());
            Render();
            Sleep(800);//0.8s放一步
            i++;
        }

        statusMsg = "回放结束 (右键返回)";
        while (true) {
            ExMessage msg;
            if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_RBUTTONDOWN) break;
            Render();
            Sleep(50);
        }

    END_PLAYBACK:
        //恢复原盘
        memcpy(gridInfo, backupState.grid, sizeof(gridInfo));
        state = backupUIState;
        statusMsg = backupMsg;
        isReplaying = false;
    }
    //获得可达位置，用于提示小圆点和合法移动检查
    vector<Point> GetReachable(int r, int c) {
        vector<Point> pts;
        for (int k = 0; k < 8; ++k) {
            int x = r + dx[k]; int y = c + dy[k];
            while (inMap(x, y) && gridInfo[x][y] == 0) { pts.push_back({ x, y }); x += dx[k]; y += dy[k]; }
        }
        return pts;
    }
    //主循环
    void MainLoop() {
        bool running = true;
        ExMessage msg;

        while (running) {
            bool hasMsg = peekmessage(&msg, EM_MOUSE | EM_KEY);//读鼠标

            int mx = msg.x;
            int my = msg.y;
            //鼠标悬停变色
            if (state != HISTORY_SELECT) {
                for (auto& btn : buttons) btn.hover = btn.isClicked(mx, my);
            }
            else {
                for (auto& btn : recordButtons) btn.hover = btn.isClicked(mx, my);
            }

            if (hasMsg && msg.message == WM_LBUTTONDOWN) {//处理左键点击

                if (state == HISTORY_SELECT) {//历史记录点击
                    for (int i = 0; i < recordButtons.size(); i++) {
                        if (recordButtons[i].isClicked(mx, my)) {
                            PlayBackSpecificGame(recordList[i].fileOffset);
                            state = IDLE;
                            break;
                        }
                    }
                }
                else {//主界面点击
                    for (int i = 0; i < buttons.size(); i++) {
                        if (buttons[i].isClicked(mx, my)) {
                            if (i == 4 && state == MOVED) continue;//放障碍物时悔棋，禁用

                            if (i == 0) { humanColor = grid_black; InitBoard(); state = IDLE; statusMsg = "Play as Black"; }
                            if (i == 1) { humanColor = grid_white; InitBoard(); state = IDLE; statusMsg = "Play as White"; }
                            if (i == 2) SaveToFile();
                            if (i == 3) LoadFromFile();
                            if (i == 4) Undo();
                            if (i == 5) {
                                LoadRecordList();
                                state = HISTORY_SELECT;
                            }
                            if (i == 6) running = false;
                        }
                    }

                    int boardX = mx - BOARD_OFFSET_X;
                    int boardY = my - BOARD_OFFSET_Y;
                    //检查是否点击棋盘
                    if (boardX >= 0 && boardX < BOARD_PIXEL_SIZE &&
                        boardY >= 0 && boardY < BOARD_PIXEL_SIZE && state != MENU) {

                        if (currentTurn != humanColor) {
                            statusMsg = "等候AI落子...";
                        }
                        else {
                            int c = boardX / CELL_SIZE;
                            int r = boardY / CELL_SIZE;
                            HandleGameClick(r, c);
                        }
                    }
                }
            }
            else if (hasMsg && msg.message == WM_RBUTTONDOWN && state == HISTORY_SELECT) {
                state = IDLE;
            }//右键返回

            if (state != MENU && state != HISTORY_SELECT && currentTurn != humanColor) {
                Render();//先渲染防卡顿

                /*AI思考时，AB剪枝会定期调用 KeepAlive，KeepAlive 会先恢复这个稳定盘面再Render，从而避免显示乱跳的AI思考摆盘过程
                */
                memcpy(stableGrid, gridInfo, sizeof(gridInfo));//备份稳定盘面
                AmazonAI::onProgressCallback = KeepAlive;//绑定回调

                vector<MoveStep> check = AmazonAI::GenerateMoves(currentTurn);//裁判，检查AI是否有棋可走
                if (check.empty()) {
                    AmazonAI::onProgressCallback = nullptr;
                    statusMsg = "您获胜！";
                    AutoSaveRecord(humanColor);
                    MessageBox(GetHWnd(), _T("您打爆了wxr的AI，tql！"), _T("Game Over"), MB_OK);
                    state = MENU;
                }
                else {
                    currBotColor = currentTurn;
                    MoveStep best = AmazonAI::GetBestMove();

                    AmazonAI::onProgressCallback = nullptr; //算棋完毕，清空回调

                    if (best.x0 == -1) {
                        statusMsg = "wxr认输了";
                        AutoSaveRecord(humanColor);
                        MessageBox(GetHWnd(), _T("您把wxr打跑了！"), _T("Game Over"), MB_OK);
                        state = MENU;
                    }
                    else {//AI落子
                        ProcStep(best.x0, best.y0, best.x1, best.y1, best.x2, best.y2, currentTurn, false);
                        currentTurn = -currentTurn;
                        SaveState();

                        check = AmazonAI::GenerateMoves(currentTurn);//裁判看看人是不是输了
                        if (check.empty()) {
                            statusMsg = "您输了";
                            Render();
                            AutoSaveRecord(-humanColor);
                            MessageBox(GetHWnd(), _T("精彩的对局，谢谢您"), _T("Game Over"), MB_OK);
                            state = MENU;
                        }
                    }
                }
            }

            if (state == IDLE && currentTurn == humanColor) statusMsg = "1：选定要移动的棋子";
            else if (state == MOVED && currentTurn == humanColor) statusMsg = "3：放置障碍物(禁止悔棋)";

            Render();
            Sleep(10);//休眠一小会儿可以降CPU占用
        }
    }
    //具体的处理棋盘点击逻辑
    void HandleGameClick(int r, int c) {
        if (state == IDLE) {//空闲等选子
            if (gridInfo[r][c] == humanColor) {
                selectedSrc = { r, c };
                state = SELECTED;
                statusMsg = "2：移动棋子";
            }
        }
        else if (state == SELECTED) {//已选中
            if (r == selectedSrc.x && c == selectedSrc.y) { state = IDLE; return; }//取消选中
            if (gridInfo[r][c] == humanColor) { selectedSrc = { r, c }; return; }//点另一个子，换子

            vector<Point> valids = GetReachable(selectedSrc.x, selectedSrc.y);
            bool isValid = false;
            for (auto& p : valids) if (p.x == r && p.y == c) isValid = true;//检查落点是否合法

            if (isValid) {//移动棋子
                gridInfo[selectedSrc.x][selectedSrc.y] = 0;
                gridInfo[r][c] = humanColor;
                selectedDst = { r, c };
                state = MOVED;
                statusMsg = "3：放置障碍物";
            }
        }
        else if (state == MOVED) {//该放障碍物了
            if (r == selectedDst.x && c == selectedDst.y) {//隐藏彩蛋：半步悔棋方式——点回刚刚移动的位置
                gridInfo[selectedDst.x][selectedDst.y] = 0;
                gridInfo[selectedSrc.x][selectedSrc.y] = humanColor;
                state = SELECTED;
                statusMsg = "2：移动棋子";
                return;
            }

            vector<Point> valids = GetReachable(selectedDst.x, selectedDst.y);
            bool isValid = false;
            for (auto& p : valids) if (p.x == r && p.y == c) isValid = true;//检查障碍物是否合法

            if (isValid) {
                gridInfo[r][c] = OBSTACLE;
                currentTurn = -currentTurn;
                state = IDLE;
                SaveState();//换人，存状态
            }
        }
    }
};

int main() {
    srand((unsigned)time(NULL));
    GraphicsEngine game;
    game.MainLoop();
    return 0;
}
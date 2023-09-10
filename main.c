#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/*
Это игра САПЕР!!!

Выполнено с помощью OpenGL 1.0
В папке с игрой может быть файл "config.cfg" в формате как я его написал, если его нет, будут применены
стандартные настройки и выдано уведомление об этом
*/




LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
#define PARAM_Q 2


int mapH;
int mapW;
int initMines, mines;

time_t rawtime;
struct tm starttime;
struct tm currenttime;
struct tm* timeinfo;
char stopTime = 0;


char isGameEndPopup = 0;


#define UNDEF -10003

const float counterLinesData[][14] = { //не хочется использовать битмап-атлас шрифта, поэтому закодировал цифры в виде наборов линий
                                {0.35, 0.15, 0.7, 0.15, 0.7, 0.85, 0.35, 0.85, 0.35, 0.15, UNDEF, UNDEF, UNDEF, UNDEF},
                                {0.7, 0.85, 0.7, 0.15, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF},
                                {0.35, 0.85, 0.7, 0.85, 0.7, 0.5, 0.35, 0.5, 0.35, 0.15, 0.7, 0.15, UNDEF, UNDEF},
                                {0.35, 0.85, 0.7, 0.85, 0.7, 0.5, 0.35, 0.5, 0.7, 0.5, 0.7, 0.15, 0.35, 0.15},
                                {0.35, 0.85, 0.35, 0.5, 0.7, 0.5, 0.7, 0.85, 0.7, 0.15, UNDEF, UNDEF, UNDEF, UNDEF},
                                {0.7, 0.85, 0.35, 0.85, 0.35, 0.5, 0.7, 0.5, 0.7, 0.15, 0.35, 0.15, UNDEF, UNDEF},
                                {0.7, 0.85, 0.35, 0.85, 0.35, 0.15, 0.7, 0.15, 0.7, 0.5, 0.35, 0.5, UNDEF, UNDEF},
                                {0.35, 0.85, 0.7, 0.85, 0.7, 0.15, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF},
                                {0.35, 0.15, 0.7, 0.15, 0.7, 0.85, 0.35, 0.85, 0.35, 0.15, 0.35, 0.5, 0.7, 0.5},
                                {0.35, 0.15, 0.7, 0.15, 0.7, 0.85, 0.35, 0.85, 0.35, 0.5, 0.7, 0.5, UNDEF, UNDEF}
                                };





typedef struct {
    BOOL mine;
    BOOL flag;
    BOOL open;
    int cntAround;
}  TCell;

TCell **map;
int closedCell;
enum GAME_STATE {WIN, LOSE, ACTIVE} gameState; //состояние игры, она может быть выиграна, проиграна или быть в процессе
void getCellFromScreen(HWND hwnd, int x, int y, float *ox, float *oy); //функция получает координаты мыши и переводит их в координаты клеток
BOOL isCellInMap(int x, int y); //проверяет, входит ли клетка в поле
void startGame(); //инициализация игры
void renderCounter(int x); //рисует цифру, заданную переменной x
void renderMine(); //рисует мину
void renderField(); //рисует закрытую клетку
void renderOpened(); //рисует открытую клетку
void renderGame(); // рисует поле в общем с использованием всех остальных функций рисования
void renderFlag(); //нарисовать флажок
void openEmpty(int x, int y); //открыть клетку (также работает механика с открытием нескольких безопасных клеток для экономии времени)
void showRules(); //показать правила игры в MessageBox
void drawClock(); //нарисовать иконку часов
void drawColon(); //нарисовать двоеточие
void drawMinus(); //нарисовать минус
void showGameInfo(); //показывает данные игры вверху (время и количество мин)
int displayChoiceMsgBox(LPCSTR caption, LPCSTR message); //диалоговое окно с заголовком caption и содержанием message, с вариантами ответа "да" и "нет"



int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "Minesweeper",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          800,
                          600,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, SW_MAXIMIZE);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);



    FILE *file;
    if ((file = fopen("config.cfg", "r")) == NULL)
    {
        MessageBoxA(hwnd, "Couldn't find config.cfg\nApplying default settings:\n10x10 field, 15 mines", "ERROR", MB_ICONINFORMATION);
        mapH = 10;
        mapW = 10;
        initMines = 15;
        mines = 15;
    }
    else
    {
        int parameters[2], order=0;
        while (fscanf(file, "%*s%d", &parameters[order++]) != EOF && order < PARAM_Q);
        mapH = parameters[0];
        mapW = parameters[0];
        initMines = parameters[1];
        mines = parameters[1];
    }

    map = (TCell **)malloc(mapH*sizeof(TCell *));
    for (int i = 0; i < mapH; i++)
    {
        map[i] = (TCell *)malloc(mapH * sizeof(TCell));
    }



    showRules();
    startGame();

    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            renderGame();
            showGameInfo();

            if (gameState != ACTIVE)
            {
                stopTime = 1;
                if (gameState == WIN && !isGameEndPopup)
                {
                    MessageBoxA(hwnd, "Поздравляю, вы победили!\nESC - выйти, SPACE - заново", "ПОБЕДА", NULL);

                }
                if (gameState == LOSE && !isGameEndPopup)
                {
                    for (int i = 0; i < mapH; i++)
                    {
                        for (int j = 0; j < mapW; j++)
                        {
                            openEmpty(i, j);
                        }
                    }
                    MessageBoxA(hwnd, "К сожалению, вы проиграли((\nESC - выйти, SPACE - заново", "ПРОДУЛ", NULL);
                }
                isGameEndPopup = 1;
            }


            SwapBuffers(hDC);
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_LBUTTONDOWN:
        {
            POINTFLOAT point;
            getCellFromScreen(hwnd, LOWORD(lParam), HIWORD(lParam), &point.x, &point.y);
            int x = (int)point.x;
            int y = (int)point.y;
            if (isCellInMap(x, y) && !map[x][y].flag)
                openEmpty(x, y);
        }
        break;
        case WM_RBUTTONDOWN:
        {
            POINTFLOAT point;
            getCellFromScreen(hwnd, LOWORD(lParam), HIWORD(lParam), &point.x, &point.y);
            int x = (int)point.x;
            int y = (int)point.y;
            if (isCellInMap(x, y) && !map[x][y].open)
            {

                map[x][y].flag = !map[x][y].flag;
                mines += (map[x][y].flag) ? -1 : 1;
            }
        }
        break;

        case WM_DESTROY:
        {
            for (int i = 0; i < mapH; i++)
                free(map[i]);
            free(map);

            return 0;
        }
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    if (gameState == ACTIVE)
                    {
                        int otvet = displayChoiceMsgBox("Вы уверены, что хотите выйти?", "Выход");
                        if (otvet == IDNO)
                                break;
                    }
                    MessageBox(NULL, "ДО СВИДАНИЯ!", "ПОКАА!!", NULL);
                    PostQuitMessage(0);
                case VK_SPACE:
                    if (gameState == ACTIVE)
                    {
                        int otvet = displayChoiceMsgBox("Вы уверены, что хотите начать новую игру?", "Новая игра");
                        if (otvet == IDNO)
                            break;
                    }
                startGame();
                default:
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


void getCellFromScreen(HWND hwnd, int x, int y, float *ox, float *oy)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    rect.top = rect.top + rect.bottom / 20;
    *ox = (x - rect.left) / (float)(rect.right - rect.left) * mapW;
    *oy = mapH - (y - rect.top) / (float)(rect.bottom - rect.top) * mapH;
}

BOOL isCellInMap(int x, int y)
{
    return (x >= 0) && (y >= 0) && (y < mapH) && (x < mapW);
}


void startGame()
{
    srand(time(NULL));
    stopTime = 0;
    isGameEndPopup = 0;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    starttime = *timeinfo;

    for (int i = 0; i < mapH; i++)
        for (int j = 0; j < mapW; j++)
        {
            map[i][j].flag = 0;
            map[i][j].mine = 0;
            map[i][j].open = 0;
            map[i][j].cntAround = 0;
        }
    gameState = ACTIVE;
    closedCell = mapW * mapH - initMines;
    mines = initMines;
    for (int i = 0; i < mines; i++)
    {
        int x = rand() % mapW;
        int y = rand() % mapH;
        if (map[x][y].mine)
            i--;
        else
        {
            map[x][y].mine = TRUE;
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++)
                    if (isCellInMap(x + dx, y + dy))
                        map[x + dx][y + dy].cntAround += 1;

        }
    }
}


void renderCounter(int x)
{
    void line(float x1, float y1, float x2, float y2)
    {
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    glLineWidth(3);
    glPointSize(3);
    glColor3f(1, 1, 0);
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i < sizeof(counterLinesData[i]) / sizeof(float); i += 2)
        {
            if (counterLinesData[x][i] != UNDEF)
                glVertex2f(counterLinesData[x][i], counterLinesData[x][i + 1]);
            else
                break;
        }
    glEnd();


}



void renderMine()
{





    glColor3f(0.6, 0.6, 0.6);
    glLineWidth(3);
    glBegin(GL_LINES);
        glPushMatrix();
            glTranslatef(0.5, 0.5, 0.0);
            for (int i = 0; i < 360; i += 45)
            {
                float rads = (float)i / 180 * M_PI;
                glVertex2f(0.5, 0.5);
                glVertex2f(0.5 + cos(rads) / 2, 0.5 + sin(rads) / 2);
            }
            glTranslatef(-0.5, -0.5, 0.0);
        glPopMatrix();
    glEnd();
    glBegin(GL_QUADS);
        glVertex2f(0.3, 0.3);
        glVertex2f(0.7, 0.3);
        glVertex2f(0.7, 0.7);
        glVertex2f(0.3, 0.7);
        glColor3f(1.0, 0.0, 0.0);

        glVertex2f(0.4, 0.4);
        glVertex2f(0.6, 0.4);
        glVertex2f(0.6, 0.6);
        glVertex2f(0.4, 0.6);
    glEnd();

}


void renderField()
{
    glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.8, 0.8, 0.0); glVertex2f(0, 1);
        glColor3f(0.9, 0.7, 0.1); glVertex2f(1, 1); glVertex2f(0, 0);
        glColor3f(0.9, 0.5, 0.2); glVertex2f(1, 0);
    glEnd();
}

void renderOpened()
{
    glBegin(GL_TRIANGLE_STRIP);
        glColor3f(0.1, 0.7, 0.7); glVertex2f(1, 0);
        glColor3f(0.0, 0.7, 0.6); glVertex2f(1, 1); glVertex2f(0, 0);
        glColor3f(0.0, 0.6, 0.5); glVertex2f(0, 1);
    glEnd();
}

void renderGame()
{
    glLoadIdentity();
    glTranslatef(0.0f, -0.05f, 0.0f);
    glScalef(2.0 / mapH, 1.9 / mapW, 1);
    glTranslatef(-mapW * 0.5, -mapH * 0.5, 0);


    for (int j = 0; j < mapH; j++)
    {
        for (int i = 0; i < mapW; i++)
        {
            glPushMatrix();
                glTranslatef(i, j, 0);
                if (map[i][j].open)
                {
                    renderOpened();
                    if (map[i][j].mine)
                        renderMine();
                    else if (map[i][j].cntAround > 0)
                        renderCounter(map[i][j].cntAround);
                }
                else
                {
                    renderField();
                    if (map[i][j].flag)
                        renderFlag();
                }
            glPopMatrix();
        }
    }
}


void renderFlag()
{
    glBegin(GL_TRIANGLES);
        glColor3f(0, 0.5, 1);
        glVertex2f(0.4, 0.90);
        glVertex2f(0.4, 0.45);
        glVertex2f(0.85, 0.65);
    glEnd();
    glLineWidth(3);
    glBegin(GL_LINES);
        glColor3f(0, 0, 0);
        glVertex2f(0.4, 0.85);
        glVertex2f(0.4, 0);
    glEnd();


}


void openEmpty(int x, int y)
{

    if (!isCellInMap(x, y) || map[x][y].open || gameState != ACTIVE)
        return;

    map[x][y].open = TRUE;
    closedCell--;
    if (map[x][y].cntAround == 0)
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
            {
                openEmpty(x + dx, y + dy);
            }
    if (map[x][y].mine)
    {
        gameState = LOSE;
    }
    if (closedCell <= 0)
    {
        gameState = WIN;
    }

}



void showRules()
{
    MessageBox(NULL, "Игра \"Сапер\" \nЧтобы начать новую игру, нажмите SPACE\nЧтобы выйти - ESC", "Правила", NULL);
    return;
}


void drawClock()
{
    glBegin(GL_LINE_LOOP);
        glColor3f(1, 1, 1);
        int facets = 35;
        for (int i = 0; i < facets; i++)
        {
            glVertex2f(cos(M_PI * 2 / facets * i) / 2 + 0.5,
                       sin(M_PI * 2 / facets * i) / 2 + 0.5);
        }
    glEnd();
    glBegin(GL_LINES);
        glVertex2f(0.5, 0.5); glVertex2f(0.5, 1.0);
        glVertex2f(0.5, 0.5); glVertex2f(cos(-M_PI/4)/2+0.5,
                                         sin(-M_PI/4)/2+0.5);
    glEnd();
}

void drawColon()
{
    glBegin(GL_QUADS);
        glColor3f(1, 1, 0);
        glVertex2f(0.4, 0.2);
        glVertex2f(0.6, 0.2);
        glVertex2f(0.6, 0.4);
        glVertex2f(0.4, 0.4);

        glVertex2f(0.4, 0.6);
        glVertex2f(0.6, 0.6);
        glVertex2f(0.6, 0.8);
        glVertex2f(0.4, 0.8);
    glEnd();
}

void drawMinus()
{
    glBegin(GL_QUADS);
        glColor3f(1, 1, 0);
        glVertex2f(0.2, 0.4);
        glVertex2f(0.8, 0.4);
        glVertex2f(0.8, 0.6);
        glVertex2f(0.2, 0.6);
    glEnd();
}


void showGameInfo()
{
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (!stopTime)
        currenttime = *timeinfo;
    int hours = currenttime.tm_hour - starttime.tm_hour,
        mins = currenttime.tm_min - starttime.tm_min,
        secs = currenttime.tm_sec - starttime.tm_sec;

        if (secs < 0) {mins--; secs += 60;}
        if (mins < 0) {hours--; mins += 60;}


    glPushMatrix();
        glLoadIdentity();
        glTranslatef(-1.0, 0.91, 0.0);
        glScalef((float)1/10, (float)1/12, 1);

        char info[250]; //значки и цифры на верхней панели, память выделена с запасом
        info[0] = 'c'; //значок часиков
        info[1] = ' ';
        info[2] = hours / 10;
        info[3] = hours % 10;
        info[4] = ':';
        info[5] = mins / 10;
        info[6] = mins % 10;
        info[7] = ':';
        info[8] = secs / 10;
        info[9] = secs % 10;
        info[10] = ' ';
        info[11] = ' ';
        info[12] = ' ';
        info[13] = 'm';
        info[14] = (mines > 0) ? ' ' : '-';
        info[15] = (abs(mines) >= 1000) ? abs(mines) / 1000 : ' ';
        info[16] = (abs(mines) >= 100) ? abs(mines) / 100 % 10 : ' ';
        info[17] = (abs(mines) >= 10) ? abs(mines) / 10 % 10 : ' ';
        info[18] = abs(mines) % 10;
        info[19] = 'e';

        for (int i = 0; info[i] != 'e'; i++)
        {
            if (info[i] == 'c')
                drawClock();
            if (info[i] == ':')
                drawColon();
            if (info[i] == 'm')
                renderMine();
            if (info[i] == '-')
                drawMinus();
            if (info[i] >= 0 && info[i] <= 9)
                renderCounter(info[i]);
            glTranslatef(0.6, 0, 0);
        }

    glPopMatrix();
}

int displayChoiceMsgBox(LPCSTR caption, LPCSTR message)
{
    int msgboxID = MessageBox(
        NULL,
        caption,
        message,
        MB_ICONEXCLAMATION | MB_YESNO
    );

    return msgboxID;
}




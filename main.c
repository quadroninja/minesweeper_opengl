#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <time.h>
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);



int mapH;
int mapW;
int mines;
#define UNDEF -10003

const float counterLinesData[][14] = {
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
enum GAME_STATE {WIN, LOSE, ACTIVE} gameState;

void getCellFromScreen(HWND hwnd, int x, int y, float *ox, float *oy)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    *ox = x / (float)rect.right * mapW;
    *oy = mapH - y / (float)rect.bottom * mapH;
}

BOOL isCellInMap(int x, int y)
{
    return (x >= 0) && (y >= 0) && (y < mapH) && (x < mapW);
}


void startGame()
{
    srand(time(NULL));
    for (int i = 0; i < mapH; i++)
        for (int j = 0; j < mapW; j++)
        {
            map[i][j].flag = 0;
            map[i][j].mine = 0;
            map[i][j].open = 0;
            map[i][j].cntAround = 0;
        }
    gameState = ACTIVE;
    closedCell = mapW * mapH - mines;
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
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0, 0, 0);
        glVertex2f(0.25, 0.25);
        glVertex2f(0.75, 0.25);
        glVertex2f(0.75, 0.75);
        glVertex2f(0.25, 0.75);
    glEnd();
    glLineWidth(5);
    glBegin(GL_LINES);
        glColor3f(0, 0, 0);
        glVertex2f(0, 0.5); glVertex2f(1, 0.5);
        glVertex2f(0.5, 0); glVertex2f(0.5, 1);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1, 0, 0);
        glVertex2f(0.35, 0.35);
        glVertex2f(0.65, 0.35);
        glVertex2f(0.65, 0.65);
        glVertex2f(0.35, 0.65);
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
    glScalef(2.0 / mapH, 2.0 / mapW, 1);
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
                openEmpty(x + dx, y + dy);
    if (map[x][y].mine)
    {
        gameState = LOSE;
        for (int i = 0; i < mapH; i++)
            for (int j = 0; j < mapW; j++)
                map[i][j].open = TRUE;
    }

}












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

    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_POPUP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1280,
                          1024,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, SW_MAXIMIZE);

    EnableOpenGL(hwnd, &hDC, &hRC);



    FILE *file;
    if ((file = fopen("config.cfg", "r")) == NULL)
    {
        MessageBoxA(hwnd, "ERROR", "Couldn't find config.cfg", MB_ICONERROR);
        return 0;
    }
    int parameters[2], order=0;
    while (fscanf(file, "%*s%d", &parameters[order++]) != EOF && order < 2);
    mapH = parameters[0];
    mapW = parameters[0];
    mines = parameters[1];

    map = (TCell **)malloc(mapH*sizeof(TCell *));
    for (int i = 0; i < mapH; i++)
    {
        map[i] = (TCell *)malloc(mapH * sizeof(TCell));
    }


    startGame();
    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
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

            if (closedCell <= 0)
            {

            }


            SwapBuffers(hDC);
            Sleep (1);
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);

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
            if (isCellInMap(x, y))
                map[x][y].flag = !map[x][y].flag;
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
                    PostQuitMessage(0);
                case VK_SPACE:
                    startGame();

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

    *hDC = GetDC(hwnd);

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

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


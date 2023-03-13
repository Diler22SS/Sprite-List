#include <windows.h>
#include <gl/gl.h>

#include "stb-master/stb_easy_font.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

//Структура кнопки
typedef struct
{
 char name[10]; // длина имени кнопки
 float vertx[8]; // 4 вершины по 2 координаты
 //text
 BOOL visible;
 BOOL hover;
} Button;

Button btn[4];
int btnCnt = sizeof(btn)/sizeof(btn[0]);
BOOL PointInButton(int x, int y, Button btn){
    return (x > btn.vertx[0]) && (x < btn.vertx[4]) && (y > btn.vertx[1]) && (y < btn.vertx[5]);
}

void Add_button(Button *btn, char *name, int x, int y, int width, int height, float scale)
{
    strcpy(btn -> name, name);
    if (btn -> visible){
        float buffer[1000];
        int num_quads;
        btn -> vertx[0] = btn -> vertx[6] = x;
        btn -> vertx[2] = btn -> vertx[4] = x + width;
        btn -> vertx[1] = btn -> vertx[3] = y;
        btn -> vertx[5] = btn -> vertx[7] = y + height;

        num_quads = stb_easy_font_print(0, 0, name, 0, buffer, sizeof(buffer)); // запись
        //координат вершин элементов имени
        float textPosX = x +(width-stb_easy_font_width(name)*scale)/2.0;
        float textPosY = y +(height-stb_easy_font_height(name)*scale)/2.0;
        textPosY+= scale*2;
        scale = scale;


        glEnableClientState(GL_VERTEX_ARRAY);

        if (btn -> hover){
            glColor3f(1, 1, 1);
        }
        else{
            glColor3f(0.5, 0.5, 0.5);
        }
        glVertexPointer(2, GL_FLOAT, 0, btn -> vertx);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glColor3f(0.3,0.3,0.3); //цвет обводки
        glLineWidth(5); // толщина обводки кнопки
        glDrawArrays(GL_LINE_LOOP,0,4); //отрисовка обводки
        glDisableClientState(GL_VERTEX_ARRAY);


        glPushMatrix(); //матрицу в стек
        glColor3f(0.7,0.7,0.7); //цвет текста
        glTranslatef(textPosX,textPosY,0); //перенос матрицы для отрисовки текста
        glScalef(scale , scale,0); //масштабирование текста
        glEnableClientState(GL_VERTEX_ARRAY); // разрешение
        glVertexPointer(2, GL_FLOAT, 16, buffer); //вектор для отрисовки
        glDrawArrays(GL_QUADS, 0, num_quads*4); //отрисовка текста
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopMatrix();
    }

}

unsigned int texture;
void Game_Init()
{
    int width, height, cnt;
    unsigned char *data = stbi_load("Спрайт_лист.png", &width, &height, &cnt, 0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}
float vertex[] = {496,100,0, 832,100,0, 832,370,0, 496,370,0};
float texCoord[] = {0,0, 1,0, 1,1, 0,1};

void Game_Show()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glColor3f(1,1,1);
    glPushMatrix();
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
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
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1024,
                          576,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    Game_Init();

    RECT rct; //создание переменной с координатами прямоуголника
    GetClientRect(hwnd, &rct); //получение текущих координат окна
    glOrtho(0, rct.right, 0, rct.bottom, 1, -1); //выставляем их как координаты окна

    /* program main loop */
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
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();

            glRotatef(theta, 0.0f, 0.0f, 1.0f);

            glLoadIdentity();
            glOrtho(0, rct.right, rct.bottom, 0, 1, -1);

            Add_button(&btn[0], ">||", 128, 192, 100, 50, 3);
            btn[0].visible = TRUE;
            Add_button(&btn[1], "EXIT", 128, 329, 100, 50, 3);
            btn[1].visible = TRUE;
            Add_button(&btn[2], "NEXT", 512, 384, 100, 50, 3);
            btn[2].visible = TRUE;
            Add_button(&btn[3], "LAST", 768, 384, 100, 50, 3);
            btn[3].visible = TRUE;

            Game_Show();
            glPopMatrix();

            SwapBuffers(hDC);

            //theta += 1.0f;
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
        case WM_MOUSEMOVE:
            for (int i =0; i < btnCnt; i++){
                btn[i].hover=PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i]);
            }
            break;
        case WM_LBUTTONDOWN:
            for (int i =0; i < btnCnt; i++){
                if (PointInButton(LOWORD(lParam), HIWORD(lParam), btn[i])){
                    if (strcmp(btn[i].name, "EXIT") == 0){
                        printf("EXIT");
                        PostQuitMessage(0);
                    }
                    if (strcmp(btn[i].name, ">||") == 0){
                        printf(">||");
                    }
                    if (strcmp(btn[i].name, "NEXT") == 0){
                        printf("NEXT");
                    }
                    if (strcmp(btn[i].name, "LAST") == 0){
                        printf("LAST");
                    }
                }

            }

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
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


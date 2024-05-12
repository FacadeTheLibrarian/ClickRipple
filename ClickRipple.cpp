#include "DxLib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string>
#include "data.h"

using std::vector;
using std::string;

#define SCRHEIGHT 640
#define SCRWIDTH 960

#define OBSMAX 16

#define RADMAX 100
#define PI 3.141592653589

enum eFlags {
	NONE = 0,
	COLLISION = 1,
};

enum eState {
	TITLE = 0,
	MENU = 1,
	PLAY = 2,
	OVER = 3
};

enum eDirection {
	WEST = 0,
	EAST = 1,
	SOUTH = 2,
	NORTH = 3,
	DIRMAX = 4,
};

typedef struct sAxis {
	int X;
	int Y;
}sAxis;

typedef struct sDblAxis {
	float X;
	float Y;
} sDblAxis;

typedef struct sStatus {
	float X;
	float Y;
	int RAD;
} sStatus;

const unsigned int colWhite = GetColor(255, 255, 255);
const unsigned int colGray = GetColor(150, 150, 150);

float RadToDeg(float);
sAxis GetInput(void);

class cRipple {
    private:
		sAxis sMousePos = { 0, 0 };
		int rippleRad = 0, rippleExpandSpeed = 2;

		int timer = 0, timeWait = (30 + 1);
		sStatus sBallPos = { (float)SCRWIDTH / 2, (float)SCRHEIGHT / 2 , 5 };
		float accRate = 8.0f;
		sDblAxis sAcc = { 0.0f, -3.0f };
		float gravity = 0.15f;

		eFlags flag = NONE;

    public:
		cRipple() {

		}
		void ripple() {
			timer += 1;
			DrawCircle((int) sBallPos.X, (int) sBallPos.Y, sBallPos.RAD, colWhite); //ボール

			GetMousePoint(&sMousePos.X, &sMousePos.Y); 

			if (GetMouseInput() & MOUSE_INPUT_LEFT && !(flag & COLLISION)) {
				timer = 1;
				rippleRad = (rippleRad + rippleExpandSpeed) % RADMAX;
				DrawCircle(sMousePos.X, sMousePos.Y, rippleRad, colWhite, FALSE);

				if (sBallPos.RAD + rippleRad > sqrtf((sBallPos.X - sMousePos.X) * (sBallPos.X - sMousePos.X) + (sBallPos.Y - sMousePos.Y) * (sBallPos.Y - sMousePos.Y))) {
					flag = COLLISION;

					bounce(sMousePos.X, sMousePos.Y, rippleRad);

				}
			}
			else {
				rippleRad = 0;
			}

			if (sBallPos.Y < 0 || SCRHEIGHT < sBallPos.Y) {
				sAcc.Y *= -1;
			}
			if (sBallPos.X < 0 || SCRWIDTH < sBallPos.X) {
				sAcc.X *= -1;
			}

			if ((timer % timeWait) == 0) {
				flag = NONE;
			}

			sAcc.Y += gravity;
			sBallPos.X += sAcc.X;
			sBallPos.Y += sAcc.Y;
		}
		sStatus showBallStatus(void){
			return sBallPos;
		}
		void Debug(void) {
			DrawLine((int) sBallPos.X, (int) sBallPos.Y, sMousePos.X, sMousePos.Y, colWhite); //アシスト表示

			if (CheckHitKey(KEY_INPUT_SPACE)) {
				sAcc.X = 0.0f;
				sAcc.Y = -5.0f;
			}
			if (CheckHitKey(KEY_INPUT_B)) {
				sBallPos.X = sBallPos.Y = 100;
			}
		}
		void bounce(float x, float y, int power) {
			double ballDegFromMouse = atan2(y - sBallPos.Y, x - sBallPos.X); //マウスから見たボールの角度 atanは座標から角度を求める
			sAcc.X = -((cos(ballDegFromMouse) * power) / accRate);
			sAcc.Y = -((sin(ballDegFromMouse) * power) / accRate);
		}
};

class cObstacle {
    private:
		sAxis sObsPos = { 0, 0 };
		unsigned short int ObsDirection = EAST;
		int obsRad = 0;
		int collisionCount = 0;
		int movement = 0;
    public:
		cObstacle() : ObsDirection(rand() % DIRMAX), obsRad(20), movement(rand() % 11) {
			switch (ObsDirection) {
				case(EAST): //右から左
					sObsPos = { SCRWIDTH + 100, rand() % SCRHEIGHT };
					break;
				case(WEST): //左から右
					sObsPos = { -100, rand() % SCRHEIGHT };
					break;
				case(SOUTH): //下から上
					sObsPos = { rand() % SCRWIDTH, SCRHEIGHT + 100 };
					break;
				case(NORTH): //上から下
					sObsPos = { rand() % SCRWIDTH, -100 };
					break;
				default:
					break;
			}
		};
		void drawObstacle(void) {
			DrawFormatString(0, 10, colWhite, "y座標: %d", sObsPos.Y);
			switch (ObsDirection) {
				case(EAST): //右から左
					sObsPos.X = (sObsPos.X + movement) % (SCRWIDTH);
					DrawCircle((SCRWIDTH - sObsPos.X), sObsPos.Y, obsRad, colWhite, TRUE);
					break;
				case(WEST): //左から右
					sObsPos.X = (sObsPos.X + movement) % (SCRWIDTH);
					DrawCircle(sObsPos.X, sObsPos.Y, obsRad, colWhite, TRUE);
					break;
				case(SOUTH): //下から上
					sObsPos.Y = ((sObsPos.Y + movement) % (SCRHEIGHT));
					DrawCircle(sObsPos.X, (SCRHEIGHT - sObsPos.Y), obsRad, colWhite, TRUE);
					break;
				case(NORTH): //上から下
					sObsPos.Y = (sObsPos.Y + movement) % (SCRHEIGHT);
					DrawCircle(sObsPos.X, sObsPos.Y, obsRad, colWhite, TRUE);
					break;
				default:
					break;
			}
			
		}
		bool collision(sStatus ball) {
			collisionCount++;
			return (obsRad + ball.RAD) > sqrtf((sObsPos.X - ball.X) * (sObsPos.X - ball.X) + (sObsPos.Y - ball.Y) * (sObsPos.Y - ball.Y)) ? TRUE : FALSE;
		}
		sAxis showObsStatus(void) {
			return sObsPos;
		}
};

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	if (DxLib_Init() == -1){
		return -1;
	}
	srand((unsigned)time(NULL));
	eState state = MENU;
	sAxis sInput = { 0, 0 };
	
	cRipple iPlayer;
	cObstacle iObstacle[5];

	SetWindowText("ClickRipple");
	SetGraphMode(SCRWIDTH, SCRHEIGHT, 32);
	ChangeWindowMode(TRUE);
	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);
	SetMouseDispFlag(TRUE);

	while (!CheckHitKey(KEY_INPUT_ESCAPE)) {
		ClearDrawScreen();
		for (int i = 0; i < 5; i++) {
			iObstacle[i].drawObstacle();
		}
		iPlayer.ripple();
		iPlayer.Debug();
		
		for (int i = 0; i < 5; i++) {
			if (iObstacle[i].collision(iPlayer.showBallStatus())) {
				iPlayer.bounce(iObstacle[i].showObsStatus().X, iObstacle[i].showObsStatus().Y, 30);
			}
		}
		
		ScreenFlip();
	}

	DxLib_End();
	return 0;
}

float RadToDeg(float rad) {
	return (float) ((rad * 180) / PI);
}


sAxis GetInput(void) {
	if (CheckHitKey(KEY_INPUT_W)) {
		return { 0, -1 };
	}
	if (CheckHitKey(KEY_INPUT_A)) {
		return { -1, 0 };
	}
	if (CheckHitKey(KEY_INPUT_S)) {
		return { 0, 1 };
	}
	if (CheckHitKey(KEY_INPUT_D)) {
		return { 1, 0 };
	}

	return { 0, 0 };
}
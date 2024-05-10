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

#define OBSPOSMAX 16

#define RADMAX 100
#define BALLRAD 5

#define PI 3.141592 

#define DEBUG 1

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

typedef struct sAxis {
	int X;
	int Y;
}sAxis;

typedef struct sDblAxis {
	double X;
	double Y;
} sDblAxis;

const int white = GetColor(255, 255, 255);

class cObstacle {
    private:
		sAxis obsPos = { 0, 0 };
		int obsRad = 0;
    public:
		cObstacle(int index = 8) {
			index = index % OBSPOSMAX;
			obsRad = rand() % RADMAX;
			obsPos = { SCRWIDTH + 100, obsPosData[index] };
		};
		void drawObstacle(void) {
			DrawCircle(obsPos.X, obsPos.Y, obsRad, white, TRUE);
			obsPos.X--;
		}
};

double RadToDig(double);
sAxis GetInput(void);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	if (DxLib_Init() == -1){
		return -1;
	}
	srand((unsigned)time(NULL));

	int rad = 0, radMod = rad, radSpeed = 2;
	int ballColor = 255;
	double ballATT = 0.0f;
	int timer = 0, timeWait = (30 + 1);
	unsigned short flag = NONE;
	unsigned short state = MENU;
	double dist = 0.0f;
	double ballCos = 0.0f;
	double ballSin = 0.0f;
	sAxis sMouse = { 0, 0 };
	sDblAxis sBall = { 250.0f, 250.0f };
	double accRate = 8.0f;
	sDblAxis sAcc = { 0.0f, -3.0f };
	double gravity = 0.15;
	sAxis sInput = { 0, 0 };

	SetWindowText("ClickRipple");
	SetGraphMode(SCRWIDTH, SCRHEIGHT, 32);
	ChangeWindowMode(TRUE);
	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);
	SetMouseDispFlag(TRUE);

	/*while (!CheckHitKey(KEY_INPUT_ESCAPE)) {
	DrawCircle(sBall.X, sBall.Y, BALLRAD, white);
	ScreenFlip();
	if (CheckHitKey(KEY_INPUT_SPACE)) {
	sVel.Y += gravity;
	sBall.X += sVel.X;
	sBall.Y += sVel.Y;
	}

	if (sBall.Y > SCRHEIGHT) {
	sVel.Y *= -1;
	}
	if (sBall.X < 0 || SCRWIDTH < sBall.X) {
	sVel.X *= -1;
	}

	}*/ //ボールの演算のサンプル

	//while (!CheckHitKey(KEY_INPUT_ESCAPE)) {
	//	switch (state) {
	//		case(TITLE):
	//			ClearDrawScreen();
	//			timer++;
	//			SetFontSize(80);
	//			DrawString(230, SCRHEIGHT / 3, "CLICK RIPPLE", white);//12字
	//			if (timer < 30) {
	//				SetFontSize(30);
	//				DrawString(320, (SCRHEIGHT / 3) * 2, "Press SPACE to Start", white);//20字
	//				//DrawFormatString(0, 0, white, "%d", timer);
	//			}
	//			else if (timer > 60) {
	//				timer = 0;
	//			}

	//			if (CheckHitKey(KEY_INPUT_SPACE)) {
	//				state = MENU;
	//			}
	//			break;
	//		case(MENU):
	//			ClearDrawScreen();
	//			//sInput = GetInput();
	//			//DrawFormatString(0, 0, white, "x: %d, y: %d", sInput.X, sInput.Y);
	//			break;
	//		case(PLAY):
	//			break;
	//		case(OVER):
	//			break;
	//	}
	//	ScreenFlip();
	//}

	cObstacle test;

	while (!CheckHitKey(KEY_INPUT_ESCAPE)) {
		timer += 1;
		ClearDrawScreen();
		DrawCircle(sBall.X, sBall.Y, BALLRAD, white); //ボール
		GetMousePoint(&sMouse.X, &sMouse.Y);
		dist = sqrtf((sBall.X - sMouse.X) * (sBall.X - sMouse.X) + (sBall.Y - sMouse.Y) * (sBall.Y - sMouse.Y));
		ballATT = atan2(sMouse.Y - sBall.Y, sMouse.X - sBall.X);
		ballCos = cos(ballATT);
		ballSin = sin(ballATT);
		if (GetMouseInput() & MOUSE_INPUT_LEFT && !(flag & COLLISION)) {
			timer = 1;
			rad = (rad + radSpeed) % RADMAX;
			DrawCircle(sMouse.X, sMouse.Y, rad, white, FALSE);
			if (BALLRAD + rad > dist) {
				flag = COLLISION;
				if (ballCos < 0 && 0 < ballSin ) { //ボールがマウスの右方向
					sAcc.X = -((ballCos * rad) / accRate);
					sAcc.Y = -((ballSin * rad) / accRate);
				}
				else if(0 < ballSin){
					sAcc.X = -((ballCos * rad) / accRate);
					sAcc.Y = -((ballSin * rad) / accRate);
				}
			}
		}
		else {
			rad = 0;
		}

		sAcc.Y += gravity;
		sBall.X += sAcc.X;
		sBall.Y += sAcc.Y;

		if (sBall.Y > SCRHEIGHT) {
			sAcc = { 0, 0 };
			DrawFormatString(SCRWIDTH / 3, SCRHEIGHT / 2, white, "GAME OVER!");
		}
		if (sBall.Y < 0) {
			sAcc.Y *= -1;
		}
		if (sBall.X < 0 || SCRWIDTH < sBall.X) {
			sAcc.X *= -1;
		}

		DrawLine(sBall.X, sBall.Y, sMouse.X, sMouse.Y, white); //アシスト表示
#ifdef DEBUG
		DrawFormatString(0, 0, white, "atan2の値: %f, 角度に直した値: %f", ballATT, RadToDig(ballATT));
		DrawFormatString(0, 20, white, "ボールのcos(x座標): %f\nボールのsin(y座標)：%f", ballCos, ballSin);
		DrawFormatString(0, 60, white, "%ボールとマウスの距離: %4f\nボールとマウスの半径の合計: %d リップルの半径: %d", dist, (BALLRAD + rad), rad);
		DrawFormatString(0, 100, white, "マウス x: %d, y: %d, ボール x: %2f, y: %2f", sMouse.X, sMouse.Y, sBall.X, sBall.Y);
		DrawFormatString(0, 120, white, "タイマー: %d, X軸加速度: %2f, Y軸加速度: %2f", timer, sAcc.X, sAcc.Y);

		if (CheckHitKey(KEY_INPUT_SPACE)) {
			sAcc.X = 0.0f;
			sAcc.Y = -5.0f;
		}
#endif
		if ((timer % timeWait) == 0) {
			flag = NONE;
		}

		test.drawObstacle();
		ScreenFlip();
	}

	DxLib_End();
	return 0;
}

double RadToDig(double rad) {
	return (rad * 180) / PI;
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
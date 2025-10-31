#include <windows.h>
#include <string>
#include <math.h>

struct {
	HWND hWnd;
	HDC device_context, context;
	int width, height;
} window;

struct obj {
	float x, y;
	int width, height, speed, dirx, diry;
	HBITMAP hBitmap;
};

struct box {
	float x, y;
	int width;
	int height;
};

obj racket, ball;
const int dotsHsphere = 19;
box boxes[39]{};
box dots[dotsHsphere]{};
float lives = 3;
int score = 0;
int level = 1;
int ballHit = -200;
const int boxesInL1 = 21;
const int boxesInL2 = 18;
int past = 0;
int rayx, rayy, hitx, hity, hitd, trigger1x, trigger1y, trigger2x, trigger2y;
bool diry;
HBITMAP hBack, mandrill, tiger;

box boxes_lvl1[boxesInL1];
box boxes_lvl2[boxesInL2];
box* box_ptrs[] = { &boxes_lvl1[0] ,&boxes_lvl2[0] };

box* box_ptr = box_ptrs[level - 1];//NULL

HBITMAP loadImage(const char* name)
{
	return (HBITMAP)LoadImageA(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void InitGame() {
	racket.x = window.width / 2 - 75;
	racket.y = window.height - 30;
	racket.width = 150;
	racket.height = 30;
	racket.speed = 10;
	ball.x = window.width / 2 - 50;
	ball.y = window.height - 130;
	ball.width = ball.height = 100;
	ball.speed = 8;
	ball.dirx = ball.diry = 0;
	diry = false;

	racket.hBitmap = loadImage("racket.bmp");
	ball.hBitmap = loadImage("ball.bmp");
	mandrill = loadImage("mandrill.bmp");
	tiger = loadImage("tiger.bmp");
	hBack = loadImage("jungle.bmp");

	int boxToTop = window.height / 20;
	int boxLen = 8;
	int offset = 0;
	int rowlengths[6] = { 0, 8, 15, 0, 7, 13 };
	for (int i = 0; i < 39; i++) {
		//size
		//boxes[i].width = window.width / (8 - i / boxesInL1);
		//boxes[i].height = window.height / 8;
	}

	for (int y = 0; y < std::size(rowlengths); y++)
		//position
	{
		if (y == std::size(rowlengths) / 2) {
			boxLen += 2;
			level = 2;
		}
		for (int x = 0; x < boxLen; x++) {
			box_ptr[rowlengths[y] + x].x = offset + box_ptr[rowlengths[y] + x].width * x;
			box_ptr[rowlengths[y] + x].height = window.height / 8;
			box_ptr[rowlengths[y] + x].width = window.width / (9 - level);
			if (y < 3) {
				box_ptr[rowlengths[y] + x].y = boxToTop + box_ptr[rowlengths[y] + x].height * y;
			}
			else {
				box_ptr[rowlengths[y] + x].y = ballHit - box_ptr[rowlengths[y] + x].height * (5 - y);
			}
		}
		boxLen--;
		offset += box_ptr[y].width / 2;
	}
	level = 1;
}

void setText() {
	SetTextColor(window.context, RGB(250, 250, 250));
	SetBkMode(window.context, TRANSPARENT);
	auto hFont = CreateFont(50, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 10, 0, L"CALIBRI");
	auto hTmp = (HFONT)SelectObject(window.context, hFont);
}

void InitWindow() {
	SetProcessDPIAware();
	window.hWnd = CreateWindow(L"edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

	RECT r;
	GetClientRect(window.hWnd, &r);
	window.device_context = GetDC(window.hWnd);
	window.width = r.right - r.left;
	window.height = r.bottom - r.top;
	window.context = CreateCompatibleDC(window.device_context);
	SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));
	GetClientRect(window.hWnd, &r);
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
	HBITMAP hbm, hOldbm;
	HDC hMemDC;
	BITMAP bm;

	hMemDC = CreateCompatibleDC(hDC);
	hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

	if (hOldbm) {
		GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm);
		StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		SelectObject(hMemDC, hOldbm);
	}
	DeleteDC(hMemDC);
}

void ballMove() {
	int move = 0;
	move = ball.speed - (past - ball.speed) * (past != ball.speed * 2);
	ball.x += move * ball.dirx;
	ball.y += move * (diry * 2 - 1);
}

void collWall() {
	// unused function, stored just in case
	if (ball.x - ball.speed <= 0 || ball.x + ball.width + ball.speed >= window.width) {
		ball.x += min(ball.x, window.height - ball.x) * ball.dirx;
		ball.dirx *= -1;
	}
	if (ball.y - ball.speed <= 0) {
		ball.y = 1;
		diry = true;
	}
}

void collBox() {
	// unused function, stored just in case
	bool hita = false;
	for (int i = 0; i < boxesInL1; i++) {
		if (level == 2) {
			i += 21;
		}

		if (ball.x + ball.width + ball.speed >= boxes[i].x && ball.y < boxes[i].y + boxes[i].height && ball.y + ball.height > boxes[i].y && ball.x + ball.width <= boxes[i].x + ball.speed * 2) {
			past += ball.x + ball.width - boxes[i].x - 1;
			ball.x = boxes[i].x - ball.width - 1;
			ball.dirx = -1;
			hita = true;
		}
		if (ball.x - ball.speed <= boxes[i].x + boxes[i].width && ball.y < boxes[i].y + boxes[i].height && ball.y + ball.height > boxes[i].y && ball.x >= boxes[i].x + boxes[i].width - ball.speed * 2) {
			past += ball.x - boxes[i].x - boxes[i].width + 1;
			ball.x = boxes[i].x + boxes[i].width + 1;
			ball.dirx = 1;
			hita = true;
		}
		if (ball.y - ball.speed <= boxes[i].y + boxes[i].height && ball.x < boxes[i].x + boxes[i].width && ball.x + ball.width > boxes[i].x && ball.y >= boxes[i].y + boxes[i].height - ball.speed * 2) {
			past += ball.y - boxes[i].y - boxes[i].height + 1;
			ball.y = boxes[i].y + boxes[i].height + 1;
			ball.diry = 1;
			hita = true;
		}
		if (ball.y + ball.height + ball.speed >= boxes[i].y && ball.x < boxes[i].x + boxes[i].width && ball.x + ball.width > boxes[i].x && ball.y + ball.height <= boxes[i].y + ball.speed * 2) {
			past += ball.y + ball.height - boxes[i].y - 1;
			ball.y = boxes[i].y - ball.height - 1;
			ball.diry = -1;
			hita = true;
		}
		if (hita == true) {
			score++;
			boxes[i].x = boxes[i].y = ballHit;
			break;
		}
		if (level == 2) {
			i -= 21;
		}
	}
	if (hita == false) {
		past = ball.speed * 2;
	}
}

void collRacket() {
	if (ball.x + ball.width + ball.speed >= racket.x && ball.y < racket.y + racket.height && ball.y + ball.height > racket.y && ball.x + ball.width <= racket.x + ball.speed * 2 ||
		ball.x - ball.speed <= racket.x + racket.width && ball.y < racket.y + racket.height && ball.y + ball.height > racket.y && ball.x >= racket.x + racket.width - ball.speed * 2) {
		ball.x = racket.x - (ball.width - 1) * (ball.dirx == 1) + (racket.width + 1) * (ball.dirx == -1);
		ball.dirx *= -1;
	}
	if (ball.y + ball.height + ball.speed >= racket.y && ball.x < racket.x + racket.width && ball.x + ball.width > racket.x && ball.y + ball.height <= racket.y + ball.speed * 2) {
		ball.y = racket.y - ball.height - 1;
		diry = false;
	}
}

void tutorial() {
	ball.x = racket.x + 25;
	ball.y = window.height - 130;
	diry = false;
	while (true) {
		ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);
		ShowBitmap(window.context, racket.x, racket.y, racket.width, racket.height, racket.hBitmap);
		ShowBitmap(window.context, ball.x, ball.y, ball.width, ball.height, ball.hBitmap);
		TextOutA(window.context, window.width / 2 - 100, window.height / 2, "Rumble in the Jungle", 20);
		TextOutA(window.context, window.width / 2 - 105, window.height / 2 + 100, "Press A or D to begin", 21);
		if (GetAsyncKeyState('A')) {
			ball.dirx = -1;
			break;
		}
		else if (GetAsyncKeyState('D')) {
			ball.dirx = 1;
			break;
		}
		BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);
		Sleep(100);
	}
}

void newLevel() {
	int hit = 0;
	for (int i = 0; i < boxesInL1; i++) {
		if (boxes[i].x == ballHit) {
			hit++;
		}
	}
	if (hit == boxesInL1) {
		level = 2;
		ball.x = window.width / 2 - 50;
		ball.y = window.height - 130;
		for (int i = boxesInL1; i < 39; i++) {
			boxes[i].y += 250 + window.height / 4;
		}
		tutorial();
	}
}

void die() {
	if (ball.y + 8 >= window.height - ball.height) {
		lives -= 0.5;
		diry = false;
		tutorial();
	}
	int hit = 0;
	for (int i = 0; i < 39; i++) {
		if (boxes[i].x == ballHit) {
			hit++;
		}
	}
	if (hit == 39) {
		lives -= 3;
	}
}

void collBoxes() {
	bool hita = false;
	for (int i = 0; i < boxesInL1; i++) {
		i += boxesInL1 * (1 / (3 - level));
		for (int j = 0; j < dotsHsphere; j++) {
			if (dots[j].x + ball.speed >= boxes[i].x &&
				dots[j].x - ball.speed <= boxes[i].x + boxes[i].width &&
				dots[j].y + ball.speed >= boxes[i].y &&
				dots[j].y - ball.speed <= boxes[i].y + boxes[i].height) {
				hita = true;
				int paststep = 0;
				if (j < 9) {
					paststep = ball.speed - abs(boxes[i].x + boxes[i].width * (ball.dirx == -1) - dots[j].x);
				}
				else {
					paststep = ball.speed - abs(boxes[i].y + boxes[i].height * not(diry)-dots[j].y);
				}
				past += paststep;
				ball.x += paststep * ball.dirx;
				ball.y += paststep * (2 * diry - 1);
				if (j < 9) {
					ball.dirx = -ball.dirx;
				}
				else {
					diry = not(diry);
				}
				boxes[i].x = boxes[i].y = -200;
				score++;
				break;
			}
		}
		i -= boxesInL1 * (1 / (3 - level));
	}
	if (hita == false) {
		ball.x += (ball.speed - past) * ball.dirx;
		ball.y += (ball.speed - past) * (2 * diry - 1);
		past = ball.speed;
	}
	/*if (ball.y - ball.speed <= hity && diry == false || ball.y + ball.height + ball.speed >= hity && diry == true) {
		hita = true;
		int paststep = (ball.y - hity) * (not(diry) * 2 - 1) - ball.height * diry;
		past += paststep;
		ball.x += paststep * ball.dirx;
		ball.y = hity - ball.height * diry;
		if (hitx <= 4 || hitx >= window.width - 4 || hitd != -1 && boxes[hitd].y + ball.speed < hity && boxes[hitd].y + boxes[hitd].height - ball.speed > hity) {
			ball.dirx = -ball.dirx;
		}
		else if (hitd != -1 && (boxes[hitd].y + ball.speed >= hity || boxes[hitd].y + boxes[hitd].height - ball.speed <= hity) || hity <= 4) {
			diry = not(diry);
		}
		if (hitd != -1) {
			boxes[hitd].x = boxes[hitd].y = ballHit;
			score++;
		}
		hitd = -1;
	}
	if (hita == false) {
		past += ball.speed + ball.speed * (past == 0);
	}*/
}

void collWalls() {
	for (int j = 0; j < dotsHsphere; j++) {
		if (dots[j].x + ball.speed >= window.width ||
			dots[j].x - ball.speed <= 0 ||
			dots[j].y + ball.speed >= window.height ||
			dots[j].y - ball.speed <= 0) {
			int paststep = 0;
			if (j < 9) {
				paststep = ball.speed - abs(dots[j].x - window.width * (ball.dirx == 1));
			}
			else {
				paststep = ball.speed - abs(dots[j].y - window.height * (diry == true));
			}
			past += paststep;
			ball.x += paststep * ball.dirx;
			ball.y += paststep * (2 * diry - 1);
			if (j < 9) {
				ball.dirx = -ball.dirx;
			}
			else {
				diry = not(diry);
			}
			break;
		}
	}
}

void racketMove() {
	if (GetAsyncKeyState('A') && racket.x > 0) {
		racket.x -= racket.speed;
	}
	else if (GetAsyncKeyState('D') && racket.x + racket.width < window.width) {
		racket.x += racket.speed;
	}
}

void ShowImage() {

	ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);
	ShowBitmap(window.context, racket.x, racket.y, racket.width, racket.height, racket.hBitmap);
	ShowBitmap(window.context, ball.x, ball.y, ball.width, ball.height, ball.hBitmap);
	for (int i = 0; i < boxesInL1; i++) {
		ShowBitmap(window.context, box_ptr[i].x, box_ptr[i].y, box_ptr[i].width, box_ptr[i].height, mandrill);
	}
	if (level == 2) {
		for (int i = 0; i < boxesInL2; i++) {
			ShowBitmap(window.context, box_ptr[i].x, box_ptr[i].y, box_ptr[i].width, box_ptr[i].height, tiger);
		}
	}
}

void triggerPoints() {
	trigger1x = ball.x + ball.width * (ball.dirx == -1);
	trigger2x = ball.x + ball.width * (ball.dirx == 1);
	trigger1y = ball.y + ball.height * diry;
	trigger2y = ball.y + ball.height * not(diry);
	for (int i = 0; i < boxesInL1; i++) {
		if (trigger1x + ball.speed >= boxes[i].x &&
			trigger1x - ball.speed <= boxes[i].x + boxes[i].width &&
			trigger1y + ball.speed >= boxes[i].y &&
			trigger1y - ball.speed <= boxes[i].y + boxes[i].height) {
			hitd = i;
			hity = trigger1y;
			hitx = trigger1x;
			collBoxes();
		}
		if (trigger2x + ball.speed >= boxes[i].x && trigger2x - ball.speed <= boxes[i].x + boxes[i].width && trigger2y + ball.speed >= boxes[i].y && trigger2y - ball.speed <= boxes[i].y + boxes[i].height) {
			hitd = i;
			hity = trigger2y;
			hitx = trigger2x;
			collBoxes();
		}
	}
}

void rayConstr() {
	rayx = ball.x + ball.width * (ball.dirx == 1);
	rayy = ball.y + ball.height * diry;
	while (rayx < window.width && rayx > 0 && rayy < window.height && rayy > 0) {
		ShowBitmap(window.context, rayx, rayy, 6, 6, ball.hBitmap);
		bool hita = false;
		for (int i = 0; i < boxesInL1; i++) {
			i += 21 * (1 / (3 - level));
			if (rayy >= boxes[i].y && rayy <= boxes[i].y + boxes[i].height && rayx >= boxes[i].x && rayx <= boxes[i].x + boxes[i].width) {
				if (i < 39 && boxes[i].y == boxes[i - ball.dirx].y && (rayx - boxes[i].x) * ball.dirx + boxes[i].width * (ball.dirx == -1) < ball.width / 2) {
					hitd = i - ball.dirx;
				}
				else {
					hitd = i;
				}
				hity = rayy;
				hitx = rayx + ball.width / 2 * ball.dirx;
				hita = true;
				if (diry == true) {
					break;
				}
			}
			i += boxesInL1 * (1 / (3 - level));
			if (level == 2 && i == boxesInL2 - 1) {
				break;
			}
		}
		if (hita == true) {
			break;
		}
		else {
			hity = rayy;
			hitx = rayx;
		}
		rayx = rayx + 4 * ball.dirx;
		rayy += 4 * (diry * 2 - 1);
	}
}

void sphere() {
	float poluhorda = sqrt(pow(ball.width / 2, 2) / 2);
	dots[0].y = ball.y + ball.height / 2 - poluhorda * (2 * diry - 1);
	dots[0].x = ball.x + ball.width / 2 + poluhorda * ball.dirx;

	float vecAngleRad = atan2(2 * diry - 1, ball.dirx);
	float d90 = 90.f * 3.141519f / 180.f;
	int i = 0;
	bool hita = false;
	for (float angle = vecAngleRad - d90; angle <= vecAngleRad + d90; angle += d90 * 2.f / 10.f)
	{
		float x = ball.x - sin(angle) * ball.width / 2 + ball.width / 2 + ball.speed * ball.dirx;
		float y = ball.y - cos(angle) * ball.height / 2 + ball.height / 2 + ball.speed * (2 * diry - 1);
		SetPixel(window.context, x, y, RGB(0, 0, 255));
		i++;

		for (int i = 0; i < boxesInL1; i++) {
			i += boxesInL1 * (1 / (3 - level));
			if (x + ball.speed >= boxes[i].x &&
				x - ball.speed <= boxes[i].x + boxes[i].width &&
				y + ball.speed >= boxes[i].y &&
				y - ball.speed <= boxes[i].y + boxes[i].height) {
				hita = true;
				int paststep = 0;
				if (angle < vecAngleRad) {
					paststep = ball.speed - abs(boxes[i].x + boxes[i].width * (ball.dirx == -1) - x);
				}
				else {
					paststep = ball.speed - abs(boxes[i].y + boxes[i].height * not(diry)-y);
				}
				past += paststep;
				ball.x += paststep * ball.dirx;
				ball.y += paststep * (2 * diry - 1);
				if (angle < vecAngleRad) {
					ball.dirx = -ball.dirx;
				}
				else {
					diry = not(diry);
				}
				boxes[i].x = boxes[i].y = ballHit;
				score++;
				break;
			}
			i -= boxesInL1 * (1 / (3 - level));
		}

		if (x + ball.speed >= window.width ||
			x - ball.speed <= 0 ||
			y + ball.speed >= window.height ||
			y - ball.speed <= 0) {
			int paststep = 0;
			hita = true;
			if (angle < vecAngleRad) {
				paststep = ball.speed - abs(x - window.width * (ball.dirx == 1));
			}
			else {
				paststep = ball.speed - abs(y - window.height * (diry == true));
			}
			past += paststep;
			ball.x += paststep * ball.dirx;
			ball.y += paststep * (2 * diry - 1);
			if (angle < vecAngleRad) {
				ball.dirx = -ball.dirx;
			}
			else {
				diry = not(diry);
			}
			break;
		}
	}
	if (hita == false) {
		ball.x += (ball.speed - past) * ball.dirx;
		ball.y += (ball.speed - past) * (2 * diry - 1);
		past = ball.speed;
	}
	/*for (int i = 1; i < 10; i++) {
		dots[i].y = dots[i - 1].y + poluhorda / 5 * (2 * diry - 1) + ball.speed * (2 * diry - 1);
		dots[i].x = ball.x + ball.width / 2 + sqrt(pow(ball.width / 2, 2) - pow(abs(ball.y + ball.height / 2 - dots[i].y), 2)) * ball.dirx + ball.speed * ball.dirx;
	}
	for (int i = 10; i < dotsHsphere; i++) {
		dots[i].x = dots[i - 1].x - poluhorda / 5 * ball.dirx + ball.speed * ball.dirx;
		dots[i].y = ball.y + ball.height / 2 + sqrt(pow(ball.height / 2, 2) - pow(abs(ball.x + ball.width / 2 - dots[i].x), 2)) * (2 * diry - 1) + ball.speed * (2 * diry - 1);
	}
	for (int i = 0; i < dotsHsphere; i++) {
		ShowBitmap(window.context, dots[i].x, dots[i].y, 6, 6, ball.hBitmap);
	}
	*/
}

std::string A;
std::string B;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	InitWindow();
	InitGame();
	setText();
	ShowCursor(NULL);
	tutorial();
	while (!(GetAsyncKeyState(VK_ESCAPE))) {

		ShowImage();

		A = "score: " + std::to_string((int)score);
		B = "lives: " + std::to_string((int)lives);
		TextOutA(window.context, 10, window.height - 100, A.c_str(), 9);
		TextOutA(window.context, 10, window.height - 50, B.c_str(), 8);

		while (past < ball.speed) {
			//collWall();
			//collBox();
			sphere();
			//collBoxes();
			//collWalls();
			collRacket();
			//triggerPoints();
			//rayConstr();
		}
		//ballMove();
		past = 0;

		if (level == 1) {
			newLevel();
		}

		racketMove();
		die();

		if (lives == 0) {
			//break;
		}

		BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);
		Sleep(16);
	}
}
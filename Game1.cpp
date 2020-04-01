#include <iostream>
#include <Windows.h>
#include <utility>
#include <chrono>
#include <vector>
#include <stdio.h>
#include <algorithm>

int nScreenW = 120;
int nScreenH = 40;

float fPlayerX = 1.0f;
float fPlayerY = 1.0f;
float fPlayerA = 0.0f;

int nMapW = 17;
int nMapH = 14;

float fFOV = 3.14159 / 3; //viewing angle 
float fDepth = 30.0f; //max distanse

int main() {
	//create screen buffer
	wchar_t* screen = new wchar_t[nScreenW * nScreenH + 1]; //massiv to write in buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); //SCREEN BUFFER
	SetConsoleActiveScreenBuffer(hConsole); //tool console
	DWORD dwBytesWritten = 0; //for debug

	std::wstring map;
	map += L"#################";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#...............#";
	map += L"#################";

	auto tp1 = std::chrono::system_clock::now(); //count time
	auto tp2 = std::chrono::system_clock::now();
	while (1) //game cycle
	{
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (1.5f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (1.5f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapW + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapW + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}
		//ray casting
		for (int x = 0; x < nScreenW; x++) {
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenW) * fFOV; //beam direction
			
			float fDistanceToWall = 0.0f;

			bool bHitWall = false; //beam hits wall block
			bool bBoundary = false; //beam hits boundary between two walls blocks

			float fEyeX = sinf(fRayAngle); //coo once vector fRayAngle
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f; //step size

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); //point where beam hit
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				if (nTestX < 0 || nTestX >= nMapW || nTestY < 0 || nTestY >= nMapH)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}

				else if (map[nTestY * nMapW + nTestX] == '#') {
					bHitWall = true;

					std::vector <std::pair<float, float>> p;
					for (int tx = 0; tx < 2; tx++)
						for (int ty = 0; ty < 2; ty++)
							for (int ty = 0; ty < 2; ty++)  //go on all 4 ribs
							{
								float vx = (float)nTestX + tx - fPlayerX; //vector coo from me to rib
								float vy = (float)nTestY + ty - fPlayerX;
								float d = sqrt(vx * vx + vy * vy); //modul
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d); //scalar mult once vectors
								p.push_back(std::make_pair(d, dot)); //save result on massiv
							}
					std::sort(p.begin(), p.end(), [](const std::pair <float, float>& left, const std::pair <float, float>& right) {
						return left.first < right.first; //sort for modul vector
						});

					float fBound = 0.005; //angle when we star view rib
					if (acos(p.at(0).second) < fBound) bBoundary = true;
					if (acos(p.at(1).second) < fBound) bBoundary = true;
				}

			}

			int nCeiling = (float)(nScreenH / 2.0f) - nScreenH / ((float)fDistanceToWall); //beging wall
			int nFloor = nScreenH - nCeiling; //end wall

			short nShade;

			if (fDistanceToWall <= fDepth / 3.0f)
				nShade = 0x2588; //if wall near, than draw lightly, else darkly
			else if (fDistanceToWall < fDepth / 2.0f)
				nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 1.5f)
				nShade = 0x2592;
			else if (fDistanceToWall < fDepth)
				nShade = 0x2591;
			else nShade = ' ';

			for (int y = 0; y < nScreenH; y++) {
				if (y <= nCeiling)
					screen[y * nScreenW + x] = ' ';
				else if (y > nCeiling&& y <= nFloor)
					screen[y * nScreenW + x] = nShade;
				else {
					float b = 1.0f - ((float)y - nScreenH / 2.0) / ((float)nScreenH / 2.0);
					if (b < 0.25)
						nShade = '#';
					else if (b < 0,5)
						nShade = 'X';
					else if (b < 0,75)
						nShade = '~';
					else if (b < 0,9)
						nShade = '-';
					else nShade = ' ';

					screen[y * nScreenW + x] = nShade;
				}
			}
		}
	//display stats
		std::swprintf(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

	//display map
		for (int nx = 0; nx < nMapW; nx++) 
			for (int ny = 0; ny < nMapH; ny++) {
				screen[(ny + 1) * nScreenW + nx] = map[ny * nMapW + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenW + (int)fPlayerY] = 'P';

	//display frame
		screen[nScreenW * nScreenH - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenW* nScreenH, { 0,0 }, & dwBytesWritten);
	}
	return 0;
}
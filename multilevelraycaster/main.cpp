#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"

#define PI 3.14159f
#define SCREEN_X 960
#define SCREEN_Y 600
#define PIXEL_X 1
#define PIXEL_Y 1

#define COL_CEIL olc::DARK_BLUE
#define COL_FLOOR olc::DARK_YELLOW
#define COL_WALL olc::GREY
#define COL_TEXT olc::MAGENTA

#define SPEED_ROTATE 60.0f
#define SPEED_MOVE 5.0f
#define SPEED_STRAFE 5.0f

class RayCaster : public olc::PixelGameEngine
{
public:
	RayCaster()
	{
		sAppName = "RayCaster";
	}

private:
	std::string sMap;
	int* nMap;
	int nMapX = 32;
	int nMapY = 32;

	float fMaxDistance = 40.0f;
	
	float fPlayerX = 2.0f;
	float fPlayerY = 2.0f;
	float fPlayerA_deg = 0.0f;

	float fPlayerH = 0.5f;
	float fPlayerfoV_deg = 60.0f;

	float fDistToProjPlane;

	olc::Sprite* pWallSprite = nullptr;
	olc::Sprite* pFloorSprite = nullptr;
	olc::Sprite* pCeilSprite = nullptr;

public:

#define  GRND_FLOOR '.'
#define FRST_FLOOR '#'
#define SCND_FLOOR '@'
#define THRD_FLOOR '*'
#define FRTH_FLOOR '-'
#define FFTH_FLOOR '+'
#define SXTH_FLOOR '='

    bool OnUserCreate() override
    {
        sMap.append("*##############################*");
        sMap.append("#..............................#");
        sMap.append("#........#@*#..................@");
        sMap.append("#..................##########..#");
        sMap.append("#...#.....#........#....#......@");
        sMap.append("#...@..............#.##.##..#..#");
        sMap.append("#...*@##..............#...#.#..@");
        sMap.append("#..................#..#.....#..#");
        sMap.append("#..................##########..@");
        sMap.append("#...#..........................#");
        sMap.append("#.......*#.#*..................@");
        sMap.append("#...@...#...#..................#");
        sMap.append("#.......#...#..................@");
        sMap.append("#...*....@@@...................#");
        sMap.append("#..............................@");
        sMap.append("#...-..........................#");
        sMap.append("#..............................@");
        sMap.append("#...+..........................#");
        sMap.append("#..............................@");
        sMap.append("#...=..........................#");
        sMap.append("#..............................@");
        sMap.append("#..............................#");
        sMap.append("#..............................@");
        sMap.append("***---+++===###..###===+++---***");
        sMap.append("#..............................@");
        sMap.append("#..............................#");
        sMap.append("#..............................@");
        sMap.append("#..............................#");
        sMap.append("#..............................@");
        sMap.append("#..............................#");
        sMap.append("#..............................@");
        sMap.append("***---+++===###..###===+++---***");

        nMap = new int[nMapX * nMapY];
        for (int y = 0; y < nMapY; y++)
        {
            for (int x = 0; x < nMapX; x++)
            {
                switch (sMap[y * nMapX + x])
                {
                case GRND_FLOOR: nMap[y * nMapX + x] = 0; break;
                case FRST_FLOOR: nMap[y * nMapX + x] = 1; break;
                case SCND_FLOOR: nMap[y * nMapX + x] = 2; break;
                case THRD_FLOOR: nMap[y * nMapX + x] = 3; break;
                case FRTH_FLOOR: nMap[y * nMapX + x] = 4; break;
                case FFTH_FLOOR: nMap[y * nMapX + x] = 5; break;
                case SXTH_FLOOR: nMap[y * nMapX + x] = 6; break;
                }
            }
        }

        fDistToProjPlane = ((ScreenWidth() / 2.0f) / sin((fPlayerfoV_deg / 2.0f) * PI / 180.0f)) * cos((fPlayerfoV_deg / 2.0f) * PI / 180.0f);

        pWallSprite = new olc::Sprite("wall_brd.png");
        pFloorSprite = new olc::Sprite("wood.png");
        pCeilSprite = new olc::Sprite("brick.png");

        return true;
    }


    struct IntersectInfo
    {
        float fHitX, fHitY;
        float fDistance;
        int nMapCoordX, nMapCoordY;
        int nHeight;
    };

    bool GetDistanceToWalls(float fRayAngle, std::vector<IntersectInfo>& vHitList)
    {
        float fFromX = fPlayerX;
        float fFromY = fPlayerY;
     

        float fToX = fPlayerX + fMaxDistance * cos(fRayAngle * PI / 180.0f);
        float fToY = fPlayerY + fMaxDistance * sin(fRayAngle * PI / 180.0f);

        float fDX = fToX - fFromX;
        float fDY = fToY - fFromY;
        float fRayLen = sqrt(fDX * fDX + fDY * fDY);
        fDX /= fRayLen;
        fDY /= fRayLen;

        float fSX = (fDX == 0.0f) ? FLT_MAX : sqrt(1.0f + (fDY / fDX) * (fDY / fDX));
        float fSY = (fDY == 0.0f) ? FLT_MAX : sqrt(1.0f + (fDX / fDY) * (fDX / fDY));

        float fLengthPartialRayX = 0.0f;
        float fLengthPartialRayY = 0.0f;

        int nGridStepX = (fDX > 0.0f) ? +1 : -1;
        int nGridStepY = (fDY > 0.0f) ? +1 : -1;
        int nCurX = int(fFromX);
        int nCurY = int(fFromY);

        if (nGridStepX < 0)
        {
            fLengthPartialRayX = (fFromX - float(nCurX)) * fSX;
        }
        else {
            fLengthPartialRayX = (float(nCurX + 1.0f) - fFromX) * fSX;
        }

        if (nGridStepY < 0)
        {
            fLengthPartialRayY = (fFromY - float(nCurY)) * fSY;
        }
        else {
            fLengthPartialRayY = (float(nCurY + 1.0f) - fFromY) * fSY;
        }

        bool bOutOfBounds = (nCurX < 0 || nCurX >= nMapX ||
            nCurY < 0 || nCurY >= nMapY);

        bool bHitFound = bOutOfBounds ? false : sMap[nCurY * nMapX + nCurX] != '.';
        bool bDestCellFound = (nCurX == int(fToX) && nCurY == int(fToY));
        float fDistIfFound = 0.0f;

        while (!bOutOfBounds && !bDestCellFound && fDistIfFound < fMaxDistance)
        {
            if (fLengthPartialRayX < fLengthPartialRayY)
            {
                nCurX += nGridStepX;
                fDistIfFound = fLengthPartialRayX;
                fLengthPartialRayX += fSX;
            }
            else
            {
                nCurY += nGridStepY;
                fDistIfFound = fLengthPartialRayY;
                fLengthPartialRayY += fSY;
            }

            bOutOfBounds = (nCurX < 0 || nCurX >= nMapX ||
                nCurY < 0 || nCurY >= nMapY);

            if (bOutOfBounds)
            {
                bHitFound = false;
                bDestCellFound = false;
            }
            else {
                bHitFound = bOutOfBounds ? false : sMap[nCurY * nMapX + nCurX] != '.';
                bDestCellFound = (nCurX == int(fToX) && nCurY == int(fToY));

                if (bHitFound)
                {
                    IntersectInfo sInfo;

                    sInfo.fDistance = fDistIfFound;
                    sInfo.fHitX = fFromX + fDistIfFound * fDX;
                    sInfo.fHitY = fFromY + fDistIfFound * fDY;
                    sInfo.nMapCoordX = nCurX;
                    sInfo.nMapCoordY = nCurY;
                    sInfo.nHeight = nMap[nCurY * nMapX + nCurX];

                    vHitList.push_back(sInfo);
                }
            }
        }
        return (vHitList.size() > 0);

    }

    void CalculateWallBottomAndTop(float fCorrectedDistToWall, int nWallHeight, int& nWallTop, int& nWallBottom)
    {
        int nSliceHeight = int((1.0f / fCorrectedDistToWall) * fDistToProjPlane);

        nWallTop = (ScreenHeight() / 2) - (nSliceHeight / 2.0f) - (nWallHeight - 1) * nSliceHeight;
        nWallBottom = (ScreenHeight() / 2) + (nSliceHeight / 2.0f);
    }

    bool OnUserUpdate(float fElapsedTime) override
    {

        // Rotate - collision detection not necessary. Keep fPlayerA_deg between 0 and 360 degrees
        if (GetKey(olc::D).bHeld) { fPlayerA_deg += SPEED_ROTATE * fElapsedTime; if (fPlayerA_deg >= 360.0f) fPlayerA_deg -= 360.0f; }
        if (GetKey(olc::A).bHeld) { fPlayerA_deg -= SPEED_ROTATE * fElapsedTime; if (fPlayerA_deg < 0.0f) fPlayerA_deg += 360.0f; }

        // variables used for collision detection - work out the new location in a seperate coordinate pair, and only alter
        // the players coordinate if there's no collision
        float fNewX = fPlayerX;
        float fNewY = fPlayerY;

        // walking forward, backward and strafing left, right
        float fPlayerA_rad = fPlayerA_deg * PI / 180.0f;
        if (GetKey(olc::W).bHeld) { fNewX += cos(fPlayerA_rad) * SPEED_MOVE * fElapsedTime; fNewY += sin(fPlayerA_rad) * SPEED_MOVE * fElapsedTime; }   // walk forward
        if (GetKey(olc::S).bHeld) { fNewX -= cos(fPlayerA_rad) * SPEED_MOVE * fElapsedTime; fNewY -= sin(fPlayerA_rad) * SPEED_MOVE * fElapsedTime; }   // walk backwards
        if (GetKey(olc::Q).bHeld) { fNewX += sin(fPlayerA_rad) * SPEED_STRAFE * fElapsedTime; fNewY -= cos(fPlayerA_rad) * SPEED_STRAFE * fElapsedTime; }   // strafe left
        if (GetKey(olc::E).bHeld) { fNewX -= sin(fPlayerA_rad) * SPEED_STRAFE * fElapsedTime; fNewY += cos(fPlayerA_rad) * SPEED_STRAFE * fElapsedTime; }   // strafe right
        // collision detection - check if out of bounds or inside non-empty tile
        // only update position if no collision
        if (fNewX >= 0 && fNewX < nMapX &&
            fNewY >= 0 && fNewY < nMapY &&
            sMap[int(fNewY) * nMapX + int(fNewX)] == GRND_FLOOR) {
            fPlayerX = fNewX;
            fPlayerY = fNewY;
        }

        Clear(olc::BLACK);

        int nHalfScreenWidth = ScreenWidth() / 2;
        int nHalfScreenHeight = ScreenHeight() / 2;
        float fAngleStep = fPlayerfoV_deg / float(ScreenWidth());

        for (int x = 0; x < ScreenWidth(); x++)
        {
            float fViewAngle = float(x - nHalfScreenWidth) * fAngleStep;

            float fCurAngle = fPlayerA_deg + fViewAngle;

            float fRawDistToWall, fCorrectedDistToWall;
            float fX_hit, fY_hit;
            int nX_hit, nY_hit;

            int nWallCeil, int nWallFloor;

            // this lambda returns a sample of the ceiling through the pixel at screen coord (px, py)
            auto get_ceil_sample = [=](int px, int py) -> olc::Pixel {
                // work out the distance to the location on the ceiling you are looking at through this pixel
                // (the pixel is given since you know the x and y screen coordinate to draw to)
                float fCeilProjDistance = ((fPlayerH / float(nHalfScreenHeight - py)) * fDistToProjPlane) / cos(fViewAngle * PI / 180.0f);
                // calculate the world ceiling coordinate from the player's position, the distance and the view angle + player angle
                float fCeilProjX = fPlayerX + fCeilProjDistance * cos(fCurAngle * PI / 180.0f);
                float fCeilProjY = fPlayerY + fCeilProjDistance * sin(fCurAngle * PI / 180.0f);
                // calculate the sample coordinates for that world ceiling coordinate, by subtracting the
                // integer part and only keeping the fractional part
                float fSampleX = fCeilProjX - int(fCeilProjX);
                float fSampleY = fCeilProjY - int(fCeilProjY);
                // having both sample coordinates, get the sample and draw the pixel
                return pCeilSprite->Sample(fSampleX, fSampleY);
            };

            // this lambda returns a sample of the floor through the pixel at screen coord (px, py)
            auto get_floor_sample = [=](int px, int py) -> olc::Pixel {
                // work out the distance to the location on the floor you are looking at through this pixel
                // (the pixel is given since you know the x and y to draw to)
                float fFloorProjDistance = ((fPlayerH / float(py - nHalfScreenHeight)) * fDistToProjPlane) / cos(fViewAngle * PI / 180.0f);
                // calculate the world floor coordinate from the distance and the view angle + player angle
                float fFloorProjX = fPlayerX + fFloorProjDistance * cos(fCurAngle * PI / 180.0f);
                float fFloorProjY = fPlayerY + fFloorProjDistance * sin(fCurAngle * PI / 180.0f);
                // calculate the sample coordinates for that world floor coordinate, by subtracting the
                // integer part and only keeping the fractional part
                float fSampleX = fFloorProjX - int(fFloorProjX);
                float fSampleY = fFloorProjY - int(fFloorProjY);
                // having both sample coordinates, get the sample and draw the pixel
                return pFloorSprite->Sample(fSampleX, fSampleY);
            };

            std::vector<IntersectInfo> vColHitlist;
            if (GetDistanceToWalls(fCurAngle, vColHitlist)) {
                // a wall was hit - set bottom and top value depending on the distance found

                // get the info from first hit point
                fX_hit = vColHitlist[0].fHitX;
                fY_hit = vColHitlist[0].fHitY;
                nX_hit = vColHitlist[0].nMapCoordX;
                nY_hit = vColHitlist[0].nMapCoordY;
                fRawDistToWall = vColHitlist[0].fDistance;
                int nColHeight = vColHitlist[0].nHeight;

                // make correction for the fish eye effect
                fCorrectedDistToWall = fRawDistToWall * cos(fViewAngle * PI / 180.0f);
                CalculateWallBottomAndTop(fCorrectedDistToWall, nColHeight, nWallCeil, nWallFloor);
            }
            else {
                // no wall was hit - set bottom and top value for wall at the horizon
                nWallCeil = nHalfScreenHeight;
                nWallFloor = nHalfScreenHeight;
            }

            // now render this slice using the info of the hit list
            int nHitListIndex = 0;
            // note that we are working upwards
        }

        return true;
    }
};


int main()
{


	return 0;
}
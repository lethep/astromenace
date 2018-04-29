/************************************************************************************

	AstroMenace
	Hardcore 3D space scroll-shooter with spaceship upgrade possibilities.
	Copyright (c) 2006-2018 Mikhail Kurinnoi, Viewizard


	AstroMenace is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AstroMenace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AstroMenace. If not, see <http://www.gnu.org/licenses/>.


	Web Site: http://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

// TODO translate comments
// TODO move to fixed size static draw buffer, no reason allocate/release memory all the time
// TODO remove vw_FindTextureByName() call from main loop
// TODO local variables should be moved to unnamed namespace, don't allow external usage

#include "../game.h"
#include "../object3d/space_object/space_object.h"
#include "skybox.h"

namespace {

// StarSystem initialization status
bool StarSystem_InitedAll{false};
bool StarSystem_Inited{false};
// StarSystem rotation
sVECTOR3D StarSystem_BaseRotation(0.0f, 0.0f, 0.0f);

} // unnamed namespace

extern cSpaceObject *StartSpaceObject;
extern std::weak_ptr<cParticleSystem> psSpace;

// для прорисовки подложки с тайловой анимацией
float StarsTile{0.0f};
float StarsTileUpdateTime{0.0f};
float StarsTile2{0.0f};
float StarsTileUpdateTime2{0.0f};
float StarsTileStartTransparentLayer1{0.0f};
float StarsTileEndTransparentLayer1{0.0f};
float StarsTileStartTransparentLayer2{0.0f};
float StarsTileEndTransparentLayer2{0.0f};


//------------------------------------------------------------------------------------
// StarSystem initialiation
//------------------------------------------------------------------------------------
void StarSystemInit(int Num, sVECTOR3D SetBaseRotation)
{
	// SkyBox setup
	switch (Num) {
	case 1:
		SkyBoxCreate(0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 100.0f);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/1/skybox_back6.tga"), eSide::BACK);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/1/skybox_bottom4.tga"), eSide::BOTTOM);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/1/skybox_front5.tga"), eSide::FRONT);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/1/skybox_left2.tga"), eSide::LEFT);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/1/skybox_right1.tga"), eSide::RIGHT);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/1/skybox_top3.tga"), eSide::TOP);
		StarSystem_Inited = true;
		break;
	case 2:
		SkyBoxCreate(0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 100.0f);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/2/skybox_back6.tga"), eSide::BACK);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/2/skybox_bottom4.tga"), eSide::BOTTOM);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/2/skybox_front5.tga"), eSide::FRONT);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/2/skybox_left2.tga"), eSide::LEFT);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/2/skybox_right1.tga"), eSide::RIGHT);
		SkyBoxSetTexture(vw_FindTextureByName("skybox/2/skybox_top3.tga"), eSide::TOP);
		StarSystem_Inited = true;
		break;
	default:
		std::cerr << __func__ << "(): " << "wrong Num.\n";
		break;
	}

	// StarSystem setup
	StarSystem_InitedAll = true;
	StarSystem_BaseRotation = SetBaseRotation;
}

//------------------------------------------------------------------------------------
// StarSystem data release function
//------------------------------------------------------------------------------------
void StarSystemRelease()
{
	for (unsigned i = 0; i < static_cast<unsigned>(eSide::size); i++)
		SkyBoxSetTexture(0, static_cast<eSide>(i));

	StarSystem_Inited = false;
}

//------------------------------------------------------------------------------------
// StarSystem draw function
//------------------------------------------------------------------------------------
void StarSystemDraw(int DrawType)
{
	if (!StarSystem_InitedAll) return;

	// current camera location
	sVECTOR3D CurrentCameraLocation;
	vw_GetCameraLocation(&CurrentCameraLocation);

	if (StarSystem_Inited) {
		vw_DepthTest(false, eCompareFunc::LESS);

		// SkyBox
		vw_PushMatrix();
		vw_Translate(CurrentCameraLocation);
		vw_Rotate(StarSystem_BaseRotation.x, 1.0f, 0.0f, 0.0f);
		vw_Rotate(StarSystem_BaseRotation.y, 0.0f, 1.0f, 0.0f);
		vw_Rotate(StarSystem_BaseRotation.z, 0.0f, 0.0f, 1.0f);
		SkyBoxDraw();
		vw_PopMatrix();

		vw_DepthTest(true, eCompareFunc::LEQUAL);
	}

	// космические объекты
	// рисуем планеты и большие астероиды _до_ тайловой анимации
	cSpaceObject *tmpSpaceObject = StartSpaceObject;
	while (tmpSpaceObject) {
		cSpaceObject *tmp2 = tmpSpaceObject->Next;

		// если это планета на заднем фоне
		if (tmpSpaceObject->ObjectType == 14) {
			if (DrawType == 2) {
				vw_PushMatrix();
				vw_Translate(sVECTOR3D(CurrentCameraLocation.x * 0.90f - GameCameraGetDeviation() * 4.0f,
						       GameCameraGetDeviation() * 2.0f, 0.0f));
			}
			tmpSpaceObject->Draw(false);
			if (DrawType == 2)
				vw_PopMatrix();
		} else {
			// если это большой астероид летящий на заднем фоне
			if (tmpSpaceObject->ObjectType == 15 &&
			    ((tmpSpaceObject->ObjectCreationType > 10) && (tmpSpaceObject->ObjectCreationType < 20))) {
				if (DrawType == 2) {
					vw_PushMatrix();
					vw_Translate(sVECTOR3D(CurrentCameraLocation.x * 0.70f - GameCameraGetDeviation() * 4.0f,
							       GameCameraGetDeviation() * 2.0f, 0.0f));
				}
				tmpSpaceObject->Draw(false);
				if (DrawType == 2)
					vw_PopMatrix();
			}
		}

		tmpSpaceObject = tmp2;
	}
	// очищаем буфер глубины, чтобы "3д подложка" не участвовала в проверке глубины
	vw_Clear(RI_DEPTH_BUFFER);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Прорисовка подложки с тайловой анимацией
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	float *buff = new float[4 * 9];

	float width_2{0.0f};
	float heigh_2{110.0f};
	float length_2{110.0f};
	float x{GamePoint.x};
	float y{GamePoint.y};
	float z{GamePoint.z};
	float StartTransparentLayer1{0.7f};
	float EndTransparentLayer1{0.7f};

	if (DrawType == 2) {
		width_2 = length_2 = 175.0f;
		heigh_2 = 0.0f;
		// чем ниже слой, тем меньше его двигаем при перемещении камеры (при стандартном аспект рейшен)
		x = GamePoint.x + GameCameraGetDeviation() + CurrentCameraLocation.x * 0.8f;
		y = GamePoint.y - GameCameraGetDeviation() * 0.5f;
		z = GamePoint.z + 25.0f;

		StartTransparentLayer1 = StarsTileStartTransparentLayer1;
		EndTransparentLayer1 = StarsTileEndTransparentLayer1;
	}

	int k=0;

	buff[k++] = x + width_2;
	buff[k++] = y + heigh_2;
	buff[k++] = z + length_2 + length_2 / 2.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = StartTransparentLayer1;
	buff[k++] = 1.0f;
	buff[k++] = 0.0f + StarsTile / 3.0f;

	buff[k++] = x + width_2;
	buff[k++] = y + heigh_2;
	buff[k++] = z - length_2 / 2.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = EndTransparentLayer1;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f + StarsTile / 3.0f;

	buff[k++] = x - width_2;
	buff[k++] = y - heigh_2;
	buff[k++] = z + length_2 + length_2 / 2.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = StartTransparentLayer1;
	buff[k++] = 0.0f;
	buff[k++] = 0.0f + StarsTile / 3.0f;

	buff[k++] = x - width_2;
	buff[k++] = y - heigh_2;
	buff[k++] = z - length_2/2;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = EndTransparentLayer1;
	buff[k++] = 0.0f;
	buff[k++] = 1.0f + StarsTile / 3.0f;

	if (DrawType == 1) {
		StarsTile -= 0.015f * (vw_GetTimeThread(0) - StarsTileUpdateTime);
		StarsTileUpdateTime = vw_GetTimeThread(0);
	} else {
		StarsTile -= 0.035f * (vw_GetTimeThread(1) - StarsTileUpdateTime);
		StarsTileUpdateTime = vw_GetTimeThread(1);
	}

	if (StarsTile < -3.0f)
		StarsTile += 3.0f;


	GLtexture TileTexture = vw_FindTextureByName("skybox/tile_back.tga");
	vw_BindTexture(0, TileTexture);
	vw_SetTextureBlend(true, eTextureBlendFactor::SRC_ALPHA, eTextureBlendFactor::ONE);

	vw_DepthTest(false, eCompareFunc::LESS);

	if (DrawType == 1) {
		vw_PushMatrix();
		vw_Rotate(-20.0f, 0.0f, 0.0f, 1.0f);
		vw_Rotate(-45.0f, 0.0f, 1.0f, 0.0f);
		vw_Rotate(30.0f, 1.0f, 0.0f, 0.0f);
	}

	vw_Draw3D(ePrimitiveType::TRIANGLE_STRIP, 4, RI_3f_XYZ | RI_4f_COLOR | RI_1_TEX, buff, 9 * sizeof(buff[0]));

	// звезды рисуем отдельно, четкими (со своими текстурными координатами)

	k = 0;

	buff[k++] = x + width_2;
	buff[k++] = y + heigh_2;
	buff[k++] = z + length_2 + length_2 / 2.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = StartTransparentLayer1;
	buff[k++] = 3.0f;
	buff[k++] = 0.0f + StarsTile;

	buff[k++] = x + width_2;
	buff[k++] = y + heigh_2;
	buff[k++] = z - length_2 / 2.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = EndTransparentLayer1;
	buff[k++] = 3.0f;
	buff[k++] = 3.0f + StarsTile;

	buff[k++] = x - width_2;
	buff[k++] = y - heigh_2;
	buff[k++] = z + length_2 + length_2 / 2.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = StartTransparentLayer1;
	buff[k++] = 0.0f;
	buff[k++] = 0.0f + StarsTile;

	buff[k++] = x - width_2;
	buff[k++] = y - heigh_2;
	buff[k++] = z - length_2/2;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = 1.0f;
	buff[k++] = EndTransparentLayer1;
	buff[k++] = 0.0f;
	buff[k++] = 3.0f + StarsTile;

	vw_BindTexture(0, vw_FindTextureByName("skybox/tile_stars.tga"));

	vw_Draw3D(ePrimitiveType::TRIANGLE_STRIP, 4, RI_3f_XYZ | RI_4f_COLOR | RI_1_TEX, buff, 9 * sizeof(buff[0]));

	if (DrawType == 1)
		vw_PopMatrix();

	vw_DepthTest(true, eCompareFunc::LEQUAL);

	vw_SetTextureBlend(false, eTextureBlendFactor::ONE, eTextureBlendFactor::ZERO);
	vw_BindTexture(0, 0);
	if (buff)
		delete [] buff;

	// корректируем положение частиц "космической пыли", если в игре и камера движется
	if (DrawType == 2) {
		if (auto sharedSpace = psSpace.lock()) {
			sharedSpace->SetStartLocation(sharedSpace->GetLocation());
			sharedSpace->MoveSystemLocation(sVECTOR3D(0, 10, 250) + GamePoint);
		}
	}
}

//------------------------------------------------------------------------------------
// Прорисовка второго слоя "пыли" с тайловой анимацией
//------------------------------------------------------------------------------------
void StarSystemDrawSecondLayer(int DrawType)
{
	if (Setup.VisualEffectsQuality <= 1) {
		float *buff = new float[4 * 9];

		float width_2{0.0f};
		float heigh_2{110.0f};
		float length_2{110.0f};
		float x{GamePoint.x};
		float y{GamePoint.y};
		float z{GamePoint.z};
		float StartTransparentLayer2{0.9f};
		float EndTransparentLayer2{0.7f};

		if (DrawType == 2) {
			width_2 = length_2 = 175.0f;
			heigh_2 = 0.0f;

			sVECTOR3D CurrentCameraLocation;
			vw_GetCameraLocation(&CurrentCameraLocation);

			x = GamePoint.x+GameCameraGetDeviation() * 2.0f + CurrentCameraLocation.x * 0.5f;
			y = GamePoint.y-GameCameraGetDeviation();
			z = GamePoint.z+25.0f;

			StartTransparentLayer2 = StarsTileStartTransparentLayer2;
			EndTransparentLayer2 = StarsTileEndTransparentLayer2;
		}

		int k = 0;

		buff[k++] = x + width_2;
		buff[k++] = y + heigh_2;
		buff[k++] = z + length_2 + length_2/ 2.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = StartTransparentLayer2;
		buff[k++] = 3.2f;
		buff[k++] = 0.0f + StarsTile2;

		buff[k++] = x + width_2;
		buff[k++] = y + heigh_2;
		buff[k++] = z - length_2 / 2.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = EndTransparentLayer2;
		buff[k++] = 3.2f;
		buff[k++] = 3.0f + StarsTile2;

		buff[k++] = x - width_2;
		buff[k++] = y - heigh_2;
		buff[k++] = z + length_2 + length_2 / 2.0f ;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = StartTransparentLayer2;
		buff[k++] = 0.2f;
		buff[k++] = 0.0f + StarsTile2;

		buff[k++] = x - width_2;
		buff[k++] = y - heigh_2;
		buff[k++] = z - length_2 / 2.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = 1.0f;
		buff[k++] = EndTransparentLayer2;
		buff[k++] = 0.2f;
		buff[k++] = 3.0f + StarsTile2;

		if (DrawType == 1) {
			StarsTile2 -= 0.04f * (vw_GetTimeThread(0) - StarsTileUpdateTime2);
			StarsTileUpdateTime2 = vw_GetTimeThread(0);
		} else {
			StarsTile2 -= 0.12f * (vw_GetTimeThread(1) - StarsTileUpdateTime2);
			StarsTileUpdateTime2 = vw_GetTimeThread(1);
		}
		if (StarsTile2 < -3.0f)
			StarsTile2 += 3.0f;

		GLtexture TileTexture = vw_FindTextureByName("skybox/tile_stars.tga");
		vw_BindTexture(0, TileTexture);
		vw_SetTextureBlend(true, eTextureBlendFactor::SRC_ALPHA, eTextureBlendFactor::ONE);
		vw_DepthTest(false, eCompareFunc::LESS);

		if (DrawType == 1) {
			vw_PushMatrix();
			vw_Rotate(-20.0f, 0.0f, 0.0f, 1.0f);
			vw_Rotate(-45.0f, 0.0f, 1.0f, 0.0f);
			vw_Rotate(30.0f, 1.0f, 0.0f, 0.0f);
		}

		vw_Draw3D(ePrimitiveType::TRIANGLE_STRIP, 4, RI_3f_XYZ | RI_4f_COLOR | RI_1_TEX, buff, 9 * sizeof(buff[0]));

		if (DrawType == 1)
			vw_PopMatrix();

		vw_DepthTest(true, eCompareFunc::LEQUAL);
		vw_SetTextureBlend(false, eTextureBlendFactor::ONE, eTextureBlendFactor::ZERO);
		vw_BindTexture(0, 0);
		if (buff)
			delete [] buff;
	}
}

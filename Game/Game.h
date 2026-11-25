#pragma once

#include "Level3DRender/LevelRender.h"
#include "Stage.h"

class Player;
class GameCamera;
class BackGround;
class GameUI;
class Timer;
class Score;

class Game : public IGameObject
{
public:
	Game() {}
	~Game() {}
	bool Start();
	void Update();
	void Render(RenderContext& rc);

	void Finish();   // ゲームを終わらせる(ゲーム中のものを全部消す)
	void StopBgm();   // BGMを止める(すでに止まっていれば何もしない)
	void GoToNextStage();   // いまの面を片づけて、次の面へ進む(最後の面なら、ゲームクリア)

	Player* player;
	GameCamera* gameCamera;
	BackGround* backGround;
	GameUI* gameUI;
	Timer* timer;
	Score* score;

	bool isGameOver = false;   // ゲームオーバー画面を出したか

private:
	void CreateBackGround();   // いまの面の背景を作る
	void CreateStageObject();  // いまの面の進行役(敵を出す係)を作る
	void ClearStage();         // いまの面のもの(背景・進行役・敵・弾・BGM)を全部消す

private:
	EnStage stage = enStage_Seaofclouds;   // いまの面
	IGameObject* stageObject = nullptr;    // いまの面の進行役(Seaofclouds など)
};
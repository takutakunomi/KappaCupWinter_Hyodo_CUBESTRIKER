#include "stdafx.h"
#include "Game.h"
#include "Player.h"
#include "GameCamera.h"
#include "BackGround.h"
#include "GameUI.h"
#include "Timer.h"
#include "Score.h"
#include "Seaofclouds.h"
#include "Sunsetsky.h"
#include "Enemy.h"
#include "GameOver.h"
#include "GameClear.h"
#include "BossHpBar.h"
#include "EnemyBullet.h"

bool Game::Start()
{
	player = NewGO<Player>(0, "player");

	gameCamera = NewGO<GameCamera>(0, "gamecamra");

	CreateBackGround();

	gameUI = NewGO<GameUI>(0, "gameui");

	timer = NewGO<Timer>(0, "timer");

	score = NewGO<Score>(1, "score");

	CreateStageObject();

	return true;
}

void Game::Update()
{
	// ライフが0になり、最後の点滅が終わったら、ゲームオーバーにする
	if (!isGameOver && player->GetLife() <= 0 && !player->IsInvincible())
	{
		isGameOver = true;
		NewGO<GameOver>(3, "gameover");
		StopBgm();
	}
}

void Game::Render(RenderContext& rc)
{

}


// ゲームを終わらせる:ゲーム中のものを全部消して、最後に自分も消える
void Game::Finish()
{
	DeleteGO(player);
	DeleteGO(gameCamera);
	DeleteGO(gameUI);
	DeleteGO(timer);
	DeleteGO(score);
	ClearStage();

	DeleteGO(this);
}

// いまの面を片づけて、次の面へ進む。最後の面だったら、ゲームクリアにする
void Game::GoToNextStage()
{
	bool isLastStage = (stage + 1 >= enStage_Count);
	if (isLastStage)
	{
		StopBgm();
		NewGO<GameClear>(3, "gameclear");
		return;
	}

	ClearStage();
	stage = static_cast<EnStage>(stage + 1);
	CreateBackGround();
	CreateStageObject();
}

// いまの面の背景を作る
void Game::CreateBackGround()
{
	backGround = NewGO<BackGround>(0, "background");
	backGround->SetStage(stage);   // Start が動く前(NewGO の直後)に、面を決める
}

// いまの面の進行役(敵を出す係)を作る
void Game::CreateStageObject()
{
	switch (stage)
	{
	case enStage_Seaofclouds:
		stageObject = NewGO<Seaofclouds>(0, "seaofclouds");
		break;
	case enStage_Sunsetsky:
		stageObject = NewGO<Sunsetsky>(0, "sunsetsky");
		break;
	}
}

// いまの面のものを、全部消す(プレイヤーなど、面をまたいで続くものは消さない)
void Game::ClearStage()
{
	StopBgm();

	DeleteGO(backGround);

	if (stageObject != nullptr)
	{
		DeleteGO(stageObject);
		stageObject = nullptr;
	}

	BossHpBar* hpBar = FindGO<BossHpBar>("bosshpbar");
	if (hpBar != nullptr)
	{
		DeleteGO(hpBar);
	}

	for (Enemy* enemy : FindGOs<Enemy>("enemy"))
	{
		DeleteGO(enemy);
	}

	for (EnemyBullet* bullet : FindGOs<EnemyBullet>("enemybullet"))
	{
		DeleteGO(bullet);
	}
}

// BGMを止める(すでに止まっていれば、何もしない)
void Game::StopBgm()
{
	SoundSource* bgm = FindGO<SoundSource>("bgm");
	if (bgm != nullptr)
	{
		DeleteGO(bgm);
	}
}
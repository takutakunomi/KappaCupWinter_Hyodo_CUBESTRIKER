#include "stdafx.h"
#include "Seaofclouds.h"
#include "Enemy.h"
#include "Orbspike.h"
#include "Grainspike.h"
#include "Pelletspike.h"
#include "PurpleBaron.h"
#include "FauxOni.h"
#include "BossHpBar.h"
#include "Game.h"
#include "EnemyBullet.h"

namespace
{
	const float ENEMY_Z = 1500.0f;      // 敵がいる奥行き(固定)

	const float MIDBOSS_TIME = 15.0f;   // 中ボスが出る時間(ステージ開始から)
	const float MIDBOSS_X = 0.0f;       // 中ボスの出現位置
	const float MIDBOSS_Y = 60.0f;

	const float BOSS_TIME = 9.0f;      // ボスが出る時間(中ボス撃破から)
	const float BOSS_X = 0.0f;          // ボスの出現位置
	const float BOSS_Y = 60.0f;

	// ---- BGM ----
	const int BGM_NUMBER = 0;                                     // 音の登録番号(ほかの音と重ならない番号にする)
	const char* BGM_PATH = "Assets/music/bgm/ingame/seaofclouds/seaofclouds.wav";
	const char* BGM_NAME = "bgm";                                 // 止めるときに探す名前
	const float BGM_VOLUME = 0.5f;                                // 音量(0.0〜1.0)

	const float CLEAR_WAIT = 2.0f;   // ボスを倒してから、次の面へ進むまでの時間(秒)

	// ---- 中ボスの前の出現予定(ステージ開始からの時間) ----
	// 書き方: { 時間, 敵の種類, X, Y }
	const SpawnData BEFORE_MIDBOSS[] =
	{
		{ 3.0f, enType_Orbspike,      0.0f,   60.0f },   // 真ん中

		{ 6.0f, enType_Grainspike,    0.0f,   60.0f },   // 真ん中

		{ 9.0f, enType_Pelletspike,   0.0f,   60.0f },   // 真ん中

		{ 12.0f, enType_Orbspike,  -450.0f,   60.0f },   // 左
		{ 12.0f, enType_Orbspike,   450.0f,   60.0f },   // 右
	};

	// ---- 中ボス撃破後の出現予定(中ボスを倒してからの時間) ----
	const SpawnData AFTER_MIDBOSS[] =
	{
		{ 3.0f, enType_Grainspike, -450.0f,   60.0f },   // 左
		{ 3.0f, enType_Grainspike,  450.0f,   60.0f },   // 右

		{ 6.0f, enType_Pelletspike,    0.0f,   60.0f },   // 真ん中
		{ 6.0f, enType_Orbspike,    -450.0f,   60.0f },   // 左
		{ 6.0f, enType_Orbspike,     450.0f,   60.0f },   // 右
	};

	// 表の数(配列全体の大きさ ÷ 1件の大きさ)
	const int BEFORE_MIDBOSS_COUNT = sizeof(BEFORE_MIDBOSS) / sizeof(BEFORE_MIDBOSS[0]);
	const int AFTER_MIDBOSS_COUNT = sizeof(AFTER_MIDBOSS) / sizeof(AFTER_MIDBOSS[0]);

	// 画面に残っている敵の弾を、全部消す
	void DeleteAllEnemyBullets()
	{
		for (EnemyBullet* bullet : FindGOs<EnemyBullet>("enemybullet"))
		{
			DeleteGO(bullet);
		}
	}

	// 雑魚(や中ボス・ボス)が、まだ生きているか
	bool IsAnyEnemyAlive()
	{
		for (Enemy* enemy : FindGOs<Enemy>("enemy"))
		{
			if (enemy->IsAlive())
			{
				return true;
			}
		}
		return false;
	}
}

bool Seaofclouds::Start()
{
	StartBgm();
	return true;
}

// 道中BGMを、ループで流す(止めるのは Game::ClearStage)
void Seaofclouds::StartBgm()
{
	g_soundEngine->ResistWaveFileBank(BGM_NUMBER, BGM_PATH);   // 同じ番号は2回目から何もしないので、やり直しても大丈夫

	SoundSource* bgm = NewGO<SoundSource>(0, BGM_NAME);
	bgm->Init(BGM_NUMBER);
	bgm->SetVolume(BGM_VOLUME);
	bgm->Play(true);
}

void Seaofclouds::Update()
{
	// 雑魚が残っているあいだは、出現の時間を止める(倒し切るまで、次へ進まない)
	bool isWaitingDefeat = (phase == enPhase_BeforeMidBoss || phase == enPhase_AfterMidBoss) && IsAnyEnemyAlive();
	if (!isWaitingDefeat)
	{
		phaseTimer += g_gameTime->GetFrameDeltaTime();
	}

	switch (phase)
	{
	case enPhase_BeforeMidBoss:
		UpdateSpawn(BEFORE_MIDBOSS, BEFORE_MIDBOSS_COUNT);

		// 中ボスの時間になったら、中ボスを出す
		if (phaseTimer >= MIDBOSS_TIME)
		{
			SpawnMidBoss();
		}
		break;

	case enPhase_MidBoss:
		// 中ボスが倒されたら、撃破後の区間へ進む
		if (midBoss == nullptr || !midBoss->IsAlive())
		{
			midBoss = nullptr;
			StartAfterMidBoss();
		}
		break;

	case enPhase_AfterMidBoss:
		UpdateSpawn(AFTER_MIDBOSS, AFTER_MIDBOSS_COUNT);

		// ボスの時間になったら、ボスを出す
		if (phaseTimer >= BOSS_TIME)
		{
			SpawnBoss();
		}
		break;

	case enPhase_Boss:
		// ボスが倒されたら、ステージクリア
		if (boss == nullptr || !boss->IsAlive())
		{
			boss = nullptr;
			phase = enPhase_Clear;
			phaseTimer = 0.0f;         // クリア画面を出すまでの待ち時間を数え直す
			DeleteAllEnemyBullets();   // 残っている敵の弾を消す
		}
		break;

	case enPhase_Clear:
		UpdateClear();
		break;
	}
}

// 時間が来た予定を、順番に出していく
void Seaofclouds::UpdateSpawn(const SpawnData* table, int tableCount)
{
	while (nextIndex < tableCount && table[nextIndex].time <= phaseTimer)
	{
		Spawn(table[nextIndex]);
		nextIndex++;
	}
}

// 敵を1体、予定の座標に出す
void Seaofclouds::Spawn(const SpawnData& data)
{
	Enemy* enemy = nullptr;

	switch (data.type)
	{
	case enType_Orbspike:
		enemy = NewGO<Orbspike>(0, "enemy");
		break;
	case enType_Grainspike:
		enemy = NewGO<Grainspike>(0, "enemy");
		break;
	case enType_Pelletspike:
		enemy = NewGO<Pelletspike>(0, "enemy");
		break;
	}

	if (enemy == nullptr)
	{
		return;
	}

	enemy->SetPosition(Vector3(data.x, data.y, ENEMY_Z));
}

// 中ボス「ムラサキング」を出す
void Seaofclouds::SpawnMidBoss()
{
	midBoss = NewGO<PurpleBaron>(0, "enemy");   // 名前を "enemy" にすると、プレイヤーの弾が当たる
	midBoss->SetPosition(Vector3(MIDBOSS_X, MIDBOSS_Y, ENEMY_Z));
	CreateHpBar(midBoss);
	phase = enPhase_MidBoss;
}

// ボス「ニセオニ」を出す
void Seaofclouds::SpawnBoss()
{
	boss = NewGO<FauxOni>(0, "enemy");   // 名前を "enemy" にすると、プレイヤーの弾が当たる
	boss->SetPosition(Vector3(BOSS_X, BOSS_Y, ENEMY_Z));
	CreateHpBar(boss);
	phase = enPhase_Boss;
}

// 敵の体力バーを出す(敵が倒されると、バーも消える)
void Seaofclouds::CreateHpBar(Enemy* target)
{
	BossHpBar* hpBar = NewGO<BossHpBar>(1, "bosshpbar");
	hpBar->SetTarget(target);
}

// 中ボス撃破後の区間へ進む(時間と予定の番号を最初に戻す)
void Seaofclouds::StartAfterMidBoss()
{
	phase = enPhase_AfterMidBoss;
	phaseTimer = 0.0f;
	nextIndex = 0;
}

// ボス撃破から少し待って、次の面へ進む(最後の面なら、Game がゲームクリアにする)
void Seaofclouds::UpdateClear()
{
	if (isStageFinished)
	{
		return;
	}

	if (phaseTimer < CLEAR_WAIT)
	{
		return;
	}

	isStageFinished = true;

	Game* game = FindGO<Game>("game");
	if (game != nullptr)
	{
		game->GoToNextStage();
	}
}
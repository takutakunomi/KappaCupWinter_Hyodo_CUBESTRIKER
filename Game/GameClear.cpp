#include "stdafx.h"
#include "GameClear.h"
#include "Game.h"
#include "Title.h"

namespace
{
	const char* SPRITE_PATH = "Assets/sprite/outgame/gameclear/gameclear.DDS";
	const float SCREEN_WIDTH = 1920.0f;
	const float SCREEN_HEIGHT = 1080.0f;
	const float INPUT_WAIT = 1.0f;   // この時間(秒)がたつまで、ボタンを受け付けない

	// ---- BGM ----
	const int BGM_NUMBER = 4;                                     // 音の登録番号(0:道中 1:ショット 2:被弾 3:ゲームオーバー)
	const char* BGM_PATH = "Assets/music/bgm/outgame/gameclear.wav";
	const char* BGM_NAME = "gameclearbgm";                        // 止めるときに探す名前
	const float BGM_VOLUME = 0.5f;                                // 音量(0.0〜1.0)
}

bool GameClear::Start()
{
	spriteRender.Init(SPRITE_PATH, SCREEN_WIDTH, SCREEN_HEIGHT);
	spriteRender.Update();
	StartBgm();
	return true;
}

void GameClear::Update()
{
	// 出た直後は、撃っていたAボタンで飛ばさないように少し待つ
	waitTimer += g_gameTime->GetFrameDeltaTime();
	if (waitTimer < INPUT_WAIT)
	{
		return;
	}

	if (g_pad[0]->IsTrigger(enButtonA))
	{
		ReturnToTitle();
	}
}

void GameClear::StartBgm()
{
	g_soundEngine->ResistWaveFileBank(BGM_NUMBER, BGM_PATH);   // 同じ番号は2回目から何もしないので、やり直しても大丈夫

	SoundSource* bgm = NewGO<SoundSource>(0, BGM_NAME);
	bgm->Init(BGM_NUMBER);
	bgm->SetVolume(BGM_VOLUME);
	bgm->Play(true);
}

void GameClear::StopBgm()
{
	SoundSource* bgm = FindGO<SoundSource>(BGM_NAME);
	if (bgm != nullptr)
	{
		DeleteGO(bgm);
	}
}

void GameClear::ReturnToTitle()
{
	StopBgm();

	Game* game = FindGO<Game>("game");
	if (game != nullptr)
	{
		game->Finish();   // ゲーム中のものを全部消す
	}

	NewGO<Title>(0, "title");
	DeleteGO(this);
}

void GameClear::Render(RenderContext& rc)
{
	spriteRender.Draw(rc);
}
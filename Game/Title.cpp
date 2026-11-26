#include "stdafx.h"
#include "Title.h"
#include "Game.h"

namespace
{
	const char* BACK_PATH = "Assets/sprite/outgame/title/title.DDS";

	// ---- ロゴ ----
	const char* LOGO_PATH = "Assets/sprite/outgame/title/titlenoglow.DDS";   // 光なし
	const char* LOGO_GLOW_PATH = "Assets/sprite/outgame/title/titleglow.DDS";     // 光あり
	const float LOGO_WIDTH = 1560.0f;
	const float LOGO_HEIGHT = 420.0f;
	const float LOGO_Y = 0.0f;              // ロゴの高さ(画面の中央が0、上がプラス)

	const float SLIDE_START_X = -1740.0f;   // 滑り込みを始める位置(ロゴが画面の左の外に隠れる位置)
	const float SLIDE_TIME = 1.2f;          // 中央に着くまでの時間(秒)
	const float LEAVE_TIME = 0.8f;          // Aボタンのあと、ロゴが右の外へ出きるまでの時間(秒)
	const float LEAVE_END_X = 1740.0f;      // 出ていく先の位置(ロゴが画面の右の外に隠れる位置)

	// ---- ボタン(0:ゲームスタート 1:操作説明) ----
	const char* MENU_PATH[Title::MENU_COUNT] =
	{
		"Assets/sprite/outgame/title/gamestartnoglow.DDS",
		"Assets/sprite/outgame/title/controlsbutton_noglow.DDS",
	};
	const char* MENU_GLOW_PATH[Title::MENU_COUNT] =
	{
		"Assets/sprite/outgame/title/gamestartglow.DDS",
		"Assets/sprite/outgame/title/controlsbuttonglow.DDS",
	};
	const float MENU_WIDTH = 1080.0f;
	const float MENU_HEIGHT = 320.0f;
	const float MENU_Y[Title::MENU_COUNT] = { 80.0f, -200.0f };   // 各ボタンの高さ

	const float MENU_START_X = -1500.0f;    // 流れ始める位置(ボタンが画面の左の外に隠れる位置)
	const float MENU_SLIDE_TIME = 0.9f;     // 1つのボタンが中央に着くまでの時間(秒)
	const float MENU_DELAY = 0.2f;          // 次のボタンが流れ始めるまでの遅れ(秒)
	const float MENU_TOTAL_TIME = MENU_SLIDE_TIME + MENU_DELAY * (Title::MENU_COUNT - 1);   // 全部のボタンが着くまでの時間(秒)

	const float STICK_THRESHOLD = 0.5f;     // スティックを、これ以上倒したら「動かした」とする

	// ---- 光の点滅(ロゴもボタンも共通) ----
	const float BLINK_PERIOD = 2.0f;        // 光が1往復する時間(秒)
	const float MIN_GLOW_ALPHA = 0.1f;      // 一番暗いときの光の濃さ(0.0 で完全に消える)
	const float PI = 3.14159265f;

	const float GLOW_CHANGE_SPEED = 6.0f;   // ボタンの光が、点いたり消えたりする速さ(1秒あたりの濃さの変化量)

	// 最初は速く、終わりに近づくほどゆっくり止まる(t は 0 〜 1)
	float EaseOut(float t)
	{
		return 1.0f - powf(1.0f - t, 3.0f);
	}

	// 最初はゆっくり、だんだん速くなる(t は 0 〜 1)
	float EaseIn(float t)
	{
		return t * t;
	}

	// 光の濃さ:timer が 0 のとき一番暗く、BLINK_PERIOD の半分のとき一番明るい
	float CalcGlowAlpha(float timer)
	{
		float wave = 0.5f - 0.5f * cosf(timer * 2.0f * PI / BLINK_PERIOD);
		return MIN_GLOW_ALPHA + (1.0f - MIN_GLOW_ALPHA) * wave;
	}

	float Clamp01(float value)
	{
		return max(0.0f, min(value, 1.0f));
	}
}

bool Title::Start()
{
	titleRender.Init(BACK_PATH, 1920.0f, 1080.0f);
	titleRender.Update();

	// ロゴ:左の外に置いて、光は一番弱くしておく
	logoRender.Init(LOGO_PATH, LOGO_WIDTH, LOGO_HEIGHT);
	logoGlowRender.Init(LOGO_GLOW_PATH, LOGO_WIDTH, LOGO_HEIGHT);
	logoGlowRender.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, MIN_GLOW_ALPHA));
	SetLogoPosition(SLIDE_START_X);

	// ボタン:左の外に置いて、光は消しておく
	for (int i = 0; i < MENU_COUNT; i++)
	{
		menuRender[i].Init(MENU_PATH[i], MENU_WIDTH, MENU_HEIGHT);
		menuGlowRender[i].Init(MENU_GLOW_PATH[i], MENU_WIDTH, MENU_HEIGHT);
		menuGlowRender[i].SetMulColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
		SetMenuPosition(i, MENU_START_X);
	}

	return true;
}

void Title::Update()
{
	switch (state)
	{
	case enState_Logo:
		UpdateLogo();
		break;
	case enState_LogoOut:
		UpdateLogoOut();
		break;
	case enState_MenuIn:
		UpdateMenuIn();
		break;
	case enState_Menu:
		UpdateMenu();
		break;
	case enState_MenuOut:
		UpdateMenuOut();
		break;
	case enState_LogoIn:
		UpdateLogoIn();
		break;
	}
}

// ---------- 段階ごとの更新 ----------

// ロゴが出てきて光る。Aボタンで、ロゴを右へ出していく
void Title::UpdateLogo()
{
	UpdateSlide();
	UpdateBlink();

	if (g_pad[0]->IsTrigger(enButtonA))
	{
		state = enState_LogoOut;
		leaveStartX = logoX;   // 途中で押されたら、いまの位置から出ていく
	}
}

// ロゴを右の外へ動かす。出きったら、ボタンを流し始める
void Title::UpdateLogoOut()
{
	leaveTimer = min(leaveTimer + g_gameTime->GetFrameDeltaTime(), LEAVE_TIME);
	SetLogoLeavePosition();

	if (leaveTimer >= LEAVE_TIME)
	{
		state = enState_MenuIn;
	}
}

// ボタンを、上から順に少しずつ遅らせて、左から中央へ流す
void Title::UpdateMenuIn()
{
	menuTimer = min(menuTimer + g_gameTime->GetFrameDeltaTime(), MENU_TOTAL_TIME);
	SetMenuSlidePosition();

	// 最後のボタンが着いたら、選べるようにする
	if (menuTimer >= MENU_TOTAL_TIME)
	{
		state = enState_Menu;
	}

	UpdateMenuGlow();
}

// ボタンを選んで、Aボタンで決める
void Title::UpdateMenu()
{
	int move = ReadMenuMove();   // 1フレームに1回だけ読む(min/max の中で呼ぶと、何回も呼ばれてしまう)
	selected = max(0, min(selected + move, MENU_COUNT - 1));

	UpdateMenuGlow();

	if (g_pad[0]->IsTrigger(enButtonA))
	{
		if (selected == 0)
		{
			StartGame();
		}
		// selected == 1(操作説明)は、まだ作っていない
	}

	// Bボタンで、ロゴの画面へ戻る
	if (g_pad[0]->IsTrigger(enButtonB))
	{
		state = enState_MenuOut;
	}
}

// ボタンが流れてきた動きを、逆戻しにする(あとから来たボタンが、先に戻る)
void Title::UpdateMenuOut()
{
	menuTimer = max(menuTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
	SetMenuSlidePosition();

	UpdateMenuGlow();   // 光は消えていく

	// 全部戻ったら、ロゴを戻し始める
	if (menuTimer <= 0.0f)
	{
		state = enState_LogoIn;
		selected = 0;   // 次に開いたときは、ゲームスタートから
	}
}

// ロゴが出ていった動きを、逆戻しにする。戻りきったら、またAボタン待ち
void Title::UpdateLogoIn()
{
	leaveTimer = max(leaveTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
	SetLogoLeavePosition();

	UpdateBlink();

	if (leaveTimer <= 0.0f)
	{
		state = enState_Logo;
	}
}

// ---------- ロゴ ----------

// ロゴを、画面の左の外から中央へ動かす
void Title::UpdateSlide()
{
	// 着いたあとは、動かさない
	if (slideTimer >= SLIDE_TIME)
	{
		return;
	}

	slideTimer = min(slideTimer + g_gameTime->GetFrameDeltaTime(), SLIDE_TIME);

	float eased = EaseOut(slideTimer / SLIDE_TIME);
	SetLogoPosition(SLIDE_START_X * (1.0f - eased));   // 左の外 → 中央(0)
}

// 光ありのロゴの透明度を、なめらかに上げ下げする(光なし ⇔ 光あり)
void Title::UpdateBlink()
{
	// 滑り込んでいる間は、光らせない(着いてから点滅を始める)
	if (slideTimer < SLIDE_TIME)
	{
		return;
	}

	blinkTimer += g_gameTime->GetFrameDeltaTime();

	logoGlowRender.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, CalcGlowAlpha(blinkTimer)));
	logoGlowRender.Update();
}

// ロゴを、leaveTimer に合わせた位置に置く(0 で出ていく前、LEAVE_TIME で右の外)
void Title::SetLogoLeavePosition()
{
	float eased = EaseIn(leaveTimer / LEAVE_TIME);
	SetLogoPosition(leaveStartX + (LEAVE_END_X - leaveStartX) * eased);
}

// 光なしと光ありのロゴを、同じ位置に動かす
void Title::SetLogoPosition(float x)
{
	logoX = x;

	Vector3 position(x, LOGO_Y, 0.0f);
	logoRender.SetPosition(position);
	logoRender.Update();
	logoGlowRender.SetPosition(position);
	logoGlowRender.Update();
}

// ---------- ボタン ----------

// ボタン(光なしと光あり)を、横の位置 x に動かす
void Title::SetMenuPosition(int index, float x)
{
	Vector3 position(x, MENU_Y[index], 0.0f);
	menuRender[index].SetPosition(position);
	menuRender[index].Update();
	menuGlowRender[index].SetPosition(position);
	menuGlowRender[index].Update();
}

// ボタンを、menuTimer に合わせた位置に置く(0 で左の外、MENU_TOTAL_TIME で全部中央)
void Title::SetMenuSlidePosition()
{
	for (int i = 0; i < MENU_COUNT; i++)
	{
		float t = Clamp01((menuTimer - MENU_DELAY * i) / MENU_SLIDE_TIME);
		SetMenuPosition(i, MENU_START_X * (1.0f - EaseOut(t)));
	}
}

// 選んでいるボタンだけ光らせる。濃さを少しずつ目標に近づけて、切り替わりをなめらかにする
// (流れてくる間は、全部消しておく)
void Title::UpdateMenuGlow()
{
	float step = GLOW_CHANGE_SPEED * g_gameTime->GetFrameDeltaTime();

	for (int i = 0; i < MENU_COUNT; i++)
	{
		bool isLit = (state == enState_Menu && i == selected);
		float target = isLit ? 1.0f : 0.0f;

		// 目標に向かって、step ずつ近づける(行き過ぎないようにする)
		if (menuGlowAlpha[i] < target)
		{
			menuGlowAlpha[i] = min(menuGlowAlpha[i] + step, target);
		}
		else
		{
			menuGlowAlpha[i] = max(menuGlowAlpha[i] - step, target);
		}

		menuGlowRender[i].SetMulColor(Vector4(1.0f, 1.0f, 1.0f, menuGlowAlpha[i]));
		menuGlowRender[i].Update();
	}
}

// 十字キー、または左スティックで、選ぶボタンを動かす向きを読む
int Title::ReadMenuMove()
{
	if (g_pad[0]->IsTrigger(enButtonUp))
	{
		return -1;
	}
	if (g_pad[0]->IsTrigger(enButtonDown))
	{
		return 1;
	}

	// スティックは、倒した瞬間だけ「動かした」とする(倒しっぱなしで走り続けないように)
	float stickY = g_pad[0]->GetLStickYF();
	bool isTilted = fabsf(stickY) > STICK_THRESHOLD;
	int move = 0;
	if (isTilted && !wasStickTilted)
	{
		move = (stickY > 0.0f) ? -1 : 1;   // 上に倒すと、上のボタン
	}

	wasStickTilted = isTilted;

	return move;
}

void Title::StartGame()
{
	NewGO<Game>(0, "game");
	DeleteGO(this);
}

void Title::Render(RenderContext& rc)
{
	titleRender.Draw(rc);

	if (state == enState_Logo || state == enState_LogoOut || state == enState_LogoIn)
	{
		logoRender.Draw(rc);
		logoGlowRender.Draw(rc);   // 光ありを、光なしの上に重ねる
		return;
	}

	for (int i = 0; i < MENU_COUNT; i++)
	{
		menuRender[i].Draw(rc);
		menuGlowRender[i].Draw(rc);
	}
}
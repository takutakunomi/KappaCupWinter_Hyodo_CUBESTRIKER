#pragma once

// タイトル画面
//   1. ロゴが左から出てきて、光る
//   2. Aボタンで、ロゴが右へ出ていく
//   3. 「ゲームスタート」「操作説明」のボタンが左から流れてくる
//   4. ボタンを選んで、Aボタンで決める
//   5. Bボタンで、3 → 2 を逆戻しにして、ロゴの画面へ戻る
class Title : public IGameObject
{
public:
	static const int MENU_COUNT = 2;   // ボタンの数(0:ゲームスタート 1:操作説明)

public:
	Title() {}
	~Title() {}

	bool Start();
	void Update();
	void Render(RenderContext& rc);

private:
	// タイトル画面の段階
	enum EnState
	{
		enState_Logo,      // ロゴが出てきて、光っている(Aボタン待ち)
		enState_LogoOut,   // ロゴが右へ出ていく
		enState_MenuIn,    // ボタンが左から流れてくる
		enState_Menu,      // ボタンを選ぶ
		enState_MenuOut,   // ボタンが左へ戻っていく(流れてきた動きの逆戻し)
		enState_LogoIn,    // ロゴが右から戻ってくる(出ていった動きの逆戻し)
	};

	// 段階ごとの更新
	void UpdateLogo();
	void UpdateLogoOut();
	void UpdateMenuIn();
	void UpdateMenu();
	void UpdateMenuOut();
	void UpdateLogoIn();

	// ロゴ
	void UpdateSlide();   // ロゴを、左の外から中央へ滑り込ませる
	void UpdateBlink();   // ロゴの光を、脈打つように点滅させる
	void SetLogoPosition(float x);
	void SetLogoLeavePosition();   // leaveTimer に合わせて、ロゴを出ていく動きの位置に置く

	// ボタン
	void SetMenuPosition(int index, float x);
	void SetMenuSlidePosition();   // menuTimer に合わせて、ボタンを流れてくる動きの位置に置く
	void UpdateMenuGlow();   // 選んでいるボタンだけ光らせる(切り替わりはなめらか)
	int ReadMenuMove();      // 上に1つ:-1  下に1つ:+1  動かない:0

	void StartGame();

private:
	SpriteRender titleRender;                       // 背景
	SpriteRender logoRender;                        // ロゴ(光なし):いつも出す
	SpriteRender logoGlowRender;                    // ロゴ(光あり):透明度を変えて重ね、点滅させる
	SpriteRender menuRender[MENU_COUNT];            // ボタン(光なし)
	SpriteRender menuGlowRender[MENU_COUNT];        // ボタン(光あり):選んでいるものだけ重ねる

	EnState state = enState_Logo;
	float slideTimer = 0.0f;       // ロゴが滑り込み始めてからの時間(秒)
	float blinkTimer = 0.0f;       // ロゴの点滅を始めてからの時間(秒)
	float logoX = 0.0f;            // いまのロゴの横の位置
	float leaveTimer = 0.0f;       // ロゴが出ていき始めてからの時間(秒)
	float leaveStartX = 0.0f;      // ロゴが出ていき始めたときの位置
	float menuTimer = 0.0f;        // ボタンが流れ始めてからの時間(秒)
	float menuGlowAlpha[MENU_COUNT] = {};   // 各ボタンの、いまの光の濃さ(0.0〜1.0)
	int selected = 0;              // 選んでいるボタンの番号
	bool wasStickTilted = false;   // 前のフレームで、スティックを倒していたか
};
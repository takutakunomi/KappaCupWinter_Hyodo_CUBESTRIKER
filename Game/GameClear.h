#pragma once

// ゲームクリア画面:画像を出して、Aボタンでタイトルへ戻る
class GameClear : public IGameObject
{
public:
	GameClear() {}
	~GameClear() {}

	bool Start();
	void Update();
	void Render(RenderContext& rc);

private:
	void StartBgm();        // クリアのBGMを流す
	void StopBgm();         // クリアのBGMを止める
	void ReturnToTitle();   // ゲームを片づけて、タイトルに戻る

private:
	SpriteRender spriteRender;
	float waitTimer = 0.0f;   // 出てからの時間(秒)
};
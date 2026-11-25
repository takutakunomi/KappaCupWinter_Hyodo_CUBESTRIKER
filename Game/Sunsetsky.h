#pragma once

// 2面「夕焼けの空」:ここに、時間に合わせて敵を出す仕組みを作っていく
//   (いまは、面の進行役の入れ物だけ。敵は、まだ出ない)
class Sunsetsky : public IGameObject
{
public:
	Sunsetsky() {}
	~Sunsetsky() {}

	bool Start();
	void Update();

private:
	float stageTimer = 0.0f;   // 面が始まってからの経過時間(秒)
};
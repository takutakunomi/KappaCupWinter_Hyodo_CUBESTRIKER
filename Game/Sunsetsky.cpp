#include "stdafx.h"
#include "Sunsetsky.h"

bool Sunsetsky::Start()
{
	// ここで BGM を流す予定(2面のBGMのファイルが、まだない)
	return true;
}

void Sunsetsky::Update()
{
	stageTimer += g_gameTime->GetFrameDeltaTime();

	// ここに、時間が来たら敵を出す処理を書いていく(Seaofclouds.cpp と同じ作り)
}
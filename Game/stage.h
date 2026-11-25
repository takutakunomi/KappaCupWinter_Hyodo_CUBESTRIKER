#pragma once

// 面の種類(Game・BackGround など、面ごとに切りかえるものが共通で使う)
// 順番がそのまま面の順番。BackGround.cpp の表も、この順番で書く
enum EnStage
{
	enStage_Seaofclouds,   // 1面:昼の雲海
	enStage_Sunsetsky,        // 2面:夕焼けの空
	enStage_Count,         // 面の数(いちばん最後に置く)
};
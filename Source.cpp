#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <wchar.h>
#include <mmsystem.h>
// BGM, SE用ライブラリ
#pragma comment(lib, "winmm.lib")
// 透過色画像用ライブラリ
#pragma comment(lib, "msimg32.lib")
// IME監視用ライブラリ
#pragma comment(lib, "imm32.lib")

#ifndef UNICODE
#define swprintf snprintf
#endif

//ウィンドウサイズ
#define WIDTH 800
#define HEIGHT 600

#define CL 128  //テキストの最大文字数
#define ST 1100 //文の最大数

#define TITLE TEXT("Typing") //タイトル
//DATA
#define ENGLISH_DATA TEXT("data/statement.dat") //英文データの場所
#define WORD_DATA TEXT("data/word.dat") //単語データの場所
//IMAGE
#define START_IMG TEXT("image/start.bmp")
#define SELECT_IMG TEXT("image/select.bmp")
#define SETTING_IMG TEXT("image/setting.bmp")
#define TYPING_IMG TEXT("image/typing.bmp")
#define KEYBORD_IMG TEXT("image/keybord.bmp")
#define RESULT_IMG TEXT("image/result.bmp")
#define CHARACTER_IMG TEXT("image/story/stage%d/%d.bmp")
#define STAGE_IMG TEXT("image/story/stage%d/stage.bmp")
#define ATTACK_IMG TEXT("image/attack.bmp")
//BGM
#define TYPING_BGM TEXT("music/typing.mp3")
#define MAIN_BGM TEXT("music/main.mp3")
#define RESULT_BGM TEXT("music/result.mp3")
#define SILENT_BGM TEXT("music/silent.mp3")
//SE
#define COUNTDOWN1_SE TEXT("music/countdown1.wav")
#define COUNTDOWN2_SE TEXT("music/countdown2.wav")
#define DECIDE_SE TEXT("music/decide.wav")
#define UPDOWN_SE TEXT("music/updown.wav")
#define LEFTRIGHT_SE TEXT("music/leftright.wav")
#define KEY_SE TEXT("music/key.wav")
#define NG_SE TEXT("music/ng.wav")
#define ATTACK_SE TEXT("music/attack.wav")
#define DAMAGE_SE TEXT("music/damage.wav")
#define CLEAR_SE TEXT("music/clear.wav")
#define GAMEOVER_SE TEXT("music/gameover.wav")
#define BACK_SE TEXT("music/back.wav")
#define RECOVER_SE TEXT("music/recover.wav")

// 問題出題数の最小値、最大値
#define MIN 1
#define MAX 20

//ランダムモードで生成する文字列の長さの範囲
#define RANDOM_MODE_MIN 4
#define RANDOM_MODE_MAX 15

#define ENEMY_MAX 10//1ステージの敵の数
#define DAMAGE_INTERVAL 3000 //敵の攻撃間隔
#define DEFAULT_LIFE 500 //ユーザーの初期ライフ
#define ENEMY_LIFE(eCount, stage) (5 * (eCount + 1) + stage) //敵の初期ライフ
#define ENEMY_POWER(eCount, stage) (eCount + 1 + stage)	//敵の攻撃力
#define ATTACK_TIME 150 //攻撃モーション時間
#define RECOVER 5 //敵を倒したときの回復値

#define Rect(hdc, x, y, w, h) Rectangle(hdc, x, y, x + w, y + h)
#define RoundRect(hdc, x, y, w, h) RoundRect(hdc, x - 10, y - 10, x + w + 10, y + h + 10, 30, 30)
#define Font(height, weight, italic, underline, pitchAndFamily) CreateFont(height, 0, 0, 0, weight, italic, underline, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, pitchAndFamily, NULL)
#define Stop(src) mciSendString(TEXT("stop ") src, NULL, 0, NULL)
#define Open(src) mciSendString(TEXT("open ") src, NULL, 0, NULL)
#define PlayBGM(src, hwnd) mciSendString(TEXT("play ") src TEXT(" from 0 notify"), NULL, 0, hwnd)
#define PlaySE(src) mciSendString(TEXT("play ") src TEXT(" from 0"), NULL, 0, NULL)
#define GetSystemTimeAsULARGE_INTEGER(ui) {FILETIME ft; GetSystemTimeAsFileTime(&ft); (ui)->HighPart = ft.dwHighDateTime; (ui)->LowPart = ft.dwLowDateTime;}

//タイマーのID
enum TimerID {
	COUNT_DOWN_TIMER_ID = 1,
	PRESS_ENTER_TIMER_ID,
	DAMAGE_TIMER_ID,
	ATTACK_TIMER_ID
};

//エラーの種類
enum ErrorType {
	NONE,			//エラーなし
	INSERTION,		//挿入エラー
	SUBSTITUTION,	//置換エラー
	DELETION		//脱落エラー
};

typedef struct {
	ErrorType error;//エラーの種類
	int start, end;	//訂正後の文字列に対応する出題文のインデックス範囲
} Error;

//画面の状態
enum ScreenState {
	START,			//スタート画面
	SELECT,			//選択画面(メニュー)
	TYPING,			//タイピング中
	STAGE_SELECT,	//ステージセレクト画面
	GAMEOVER,		//タイピング終了画面
	SCORE,			//スコア表示画面
	REVIEW,			//復習(誤り箇所指摘)画面
	SETTING			//設定画面
};
ScreenState screenState = START;//現在の画面状態

//メニュー項目
enum MenuItem {
	E_SENTENCE,
	E_WORD,
	RANDOM,
	STORY,
	CONFIG,
	EXIT,
	MENU_ITEM_COUNT	//メニュー項目の数
};
const TCHAR MENU_STR[][64] = { TEXT("英文タイピング"), TEXT("英単語タイピング"), TEXT("ランダムタイピング"), TEXT("ストーリーモード"), TEXT("設定"), TEXT("終了") };
MenuItem selectedMenuItem = E_SENTENCE;//現在選択されているメニュー項目

//設定メニュー項目
enum ConfigMenuItem {
	TYPING_MODE,
	NUMBER_OF_QUESTION,
	ASSIST,
	BGM,
	SE,
	CONFIG_BACK,
	CONFIG_MENU_ITEM_COUNT	//設定メニュー項目の数
};
const TCHAR CONFIG_MENU_STR[][64] = { TEXT("タイピングモード"), TEXT("出題数"), TEXT("アシスト"), TEXT("BGM"), TEXT("SE"), TEXT("戻る") };
ConfigMenuItem selectedConfigMenuItem = TYPING_MODE;//現在選択されている設定メニュー項目

//ストーリーメニュー項目
enum StoryMenuItem {
	STAGE_0,
	STAGE_1,
	STAGE_2,
	STAGE_3,
	STAGE_4,
	STAGE_5,
	STAGE_6,
	STAGE_7,
	STAGE_8,
	STAGE_9,
	STORY_BACK,
	STORY_ITEM_COUNT //ストーリーメニュー項目の数
};
const TCHAR STORY_MENU_STR[][64] = { TEXT("0. 草原ステージ"), TEXT("1. 水中ステージ"), TEXT("2. 石壁ステージ"), TEXT("3. 洞窟ステージ"), TEXT("4. 遺跡ステージ"),
TEXT("5. 砂漠ステージ"), TEXT("6. 荒地ステージ"), TEXT("7. 湿地ステージ"), TEXT("8. 森ステージ"), TEXT("9. 溶岩ステージ"), TEXT("戻る") };
StoryMenuItem selectedStoryItem = STAGE_0;//現在選択されている設定メニュー項目

TCHAR lines[ST][CL]; //行(文)の集合
TCHAR typingString[MAX][CL]; //入力中の文字列
int selectedString[MAX]; // 選択された文字列のインデックス
int count; //現在何問目か
int typingIndex; //入力文字列のインデックス
BOOL dpmatching = TRUE; //DPマッチングを行うかどうか (true:DP Matchinモード、false:Normalモード)
int numberOfQuestion = 10; //問題出題数

int insertion, deletion, substitution; //挿入エラー、脱落エラー、置換エラー
Error result[MAX][CL]; //結果

ULARGE_INTEGER startTime, endTime;// 開始時間、終了時間 (単位:100ナノ秒)
int countdown; //タイピング開始前のカウントダウン

int totalInputLength; //入力総文字数
int totalTextLength;//出題文の合計文字数

BOOL pressEnterVisible = TRUE; // "Press Enter"を表示するかどうか
BOOL assist = TRUE; // 入力アシストを表示するかどうか
BOOL bgm = TRUE, se = TRUE;//BGM、SEを鳴らすかどうか
BOOL correct = FALSE;//入力が正しいかどうか(Normalモードのみ)

//キャラクター定義
typedef struct {
	int life, power;
}Character;
int dataLength;// ロードしたデータの個数
int enemyCount;// 現在、何番目の敵か
Character enemy; // 敵
Character user = { DEFAULT_LIFE, 1 };//ユーザー
int attack;//敵に与えたダメージ

int gd[CL][CL], ld[CL][CL], path[CL][CL], trace[CL][CL];

//プロトタイプ宣言
void drawStart(HDC hdc);
void drawSelect(HDC hdc);
void drawTyping(HDC hdc);
void drawScore(HDC hdc);
void drawReview(HDC hdc);
void drawSetting(HDC hdc);
void drawStageSelect(HDC hdc);
void drawAssist(HDC hdc);
void drawGameover(HDC hdc);
void drawString(HDC hdc, int x, int y, const TCHAR *text, int n);
void drawBMP(HDC hdc, const TCHAR *path, int x, int y);
int drawBMP(HDC hdc, const TCHAR *path, int x, int y, UINT rgb);
void initialization(int jfr, int ifr);
double dp(int jframe, int iframe, TCHAR *reftext, TCHAR *inputtext);
void distancemap(int jframe, int iframe, Error *result, int &insertion, int &deletion, int &substitution);
void swap(int &a, int &b);
void makenum(int *num, int length, int selectLength);
int readLines(const TCHAR *filename, TCHAR lines[][CL], int length);

//ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	HDC hdc;
	PAINTSTRUCT ps;
	static HBITMAP hBitmap; // ビットマップ
	static HDC hMemDC; // オフスクリーン
	static HIMC himc;// IME操作用 入力コンテキスト

	switch (msg) {
	case WM_CREATE://ウィンドウ作成時---------------------------------------------------------------------------
		//入力コンテキストの作成
		himc = ImmCreateContext();
		ImmAssociateContext(hwnd, himc);
		// DCコンパチブルの作成
		hdc = GetDC(hwnd);
		hMemDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
		SelectObject(hMemDC, hBitmap);
		ReleaseDC(hwnd, hdc);
		//サウンドロード
		Open(TYPING_BGM);
		Open(MAIN_BGM);
		Open(RESULT_BGM);
		Open(SILENT_BGM);
		Open(COUNTDOWN1_SE);
		Open(COUNTDOWN2_SE);
		Open(DECIDE_SE);
		Open(UPDOWN_SE);
		Open(LEFTRIGHT_SE);
		Open(KEY_SE);
		Open(NG_SE);
		Open(ATTACK_SE);
		Open(DAMAGE_SE);
		Open(CLEAR_SE);
		Open(GAMEOVER_SE);
		Open(BACK_SE);
		Open(RECOVER_SE);

		PlayBGM(SILENT_BGM, hwnd);
		if (bgm) PlayBGM(MAIN_BGM, hwnd);
		//乱数の種セット
		srand((unsigned)time(NULL));
		SetTimer(hwnd, PRESS_ENTER_TIMER_ID, 500, NULL);
		return 0;
	case WM_DESTROY://ウィンドウ終了時--------------------------------------------------------------------------
		//SE, BGM削除
		mciSendString(TEXT("close all"), NULL, 0, NULL);
		// DCコンパチブルの破棄
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
		//入力コンテキストの破棄
		ImmDestroyContext(himc);
		PostQuitMessage(0);
		return 0;
	case WM_PAINT://描画処理-----------------------------------------------------------------------------
		// 白で塗りつぶす
		SelectObject(hMemDC, GetStockObject(WHITE_BRUSH));
		SelectObject(hMemDC, GetStockObject(WHITE_PEN));
		Rect(hMemDC, 0, 0, WIDTH, HEIGHT);
		switch (screenState) {
		case START:
			drawStart(hMemDC);
			break;
		case SELECT:
			drawSelect(hMemDC);
			break;
		case STAGE_SELECT:
			drawStageSelect(hMemDC);
			break;
		case TYPING:
			drawTyping(hMemDC);
			break;
		case GAMEOVER:
			drawGameover(hMemDC);
			break;
		case SCORE:
			drawScore(hMemDC);
			break;
		case REVIEW:
			drawReview(hMemDC);
			break;
		case SETTING:
			drawSetting(hMemDC);
			break;
		}
		//オフスクリーンをメインスクリーンに転送
		hdc = BeginPaint(hwnd, &ps);
		BitBlt(hdc, 0, 0, WIDTH, HEIGHT, hMemDC, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_TIMER://タイマー-------------------------------------------------------------------------
		switch (wp) {
		case COUNT_DOWN_TIMER_ID://カウントダウンタイマー
			if (--countdown <= 0) {
				KillTimer(hwnd, COUNT_DOWN_TIMER_ID);
				if (se) PlaySE(COUNTDOWN2_SE);
				if (bgm) PlayBGM(TYPING_BGM, hwnd);
				if (selectedMenuItem == STORY)//ストーリーモードのとき
					SetTimer(hwnd, DAMAGE_TIMER_ID, DAMAGE_INTERVAL, NULL);
				GetSystemTimeAsULARGE_INTEGER(&startTime); //計測開始
			} else {
				if (se) PlaySE(COUNTDOWN1_SE);
			}
			break;
		case PRESS_ENTER_TIMER_ID://PRESS ENTER点滅タイマー
			pressEnterVisible = !pressEnterVisible;
			break;
		case DAMAGE_TIMER_ID://ユーザーに与えるダメージタイマー
			if (enemy.life <= 0) return 0;
			if (se) PlaySE(DAMAGE_SE);
			user.life -= enemy.power;
			if (user.life <= 0) {//ゲームオーバーのとき
				GetSystemTimeAsULARGE_INTEGER(&endTime); //計測終了
				KillTimer(hwnd, DAMAGE_TIMER_ID);
				KillTimer(hwnd, ATTACK_TIMER_ID);
				screenState = GAMEOVER;
				if (bgm) Stop(TYPING_BGM);
				if (se) PlaySE(GAMEOVER_SE);
			}
			break;
		case ATTACK_TIMER_ID://ユーザーの敵に対する攻撃モーション表示タイマー
			KillTimer(hwnd, ATTACK_TIMER_ID);
			startTime.QuadPart += ATTACK_TIME * 10000; //モーション表示中の時間は除く
			attack = 0;
			//次の文の選択
			selectedString[0] = rand() % dataLength;
			typingString[0][0] = TEXT('\0');
			typingIndex = 0;
			if (enemy.life <= 0) {//敵がやられたとき
				if (++enemyCount >= ENEMY_MAX) {//敵を最後まで倒し終えたら
					GetSystemTimeAsULARGE_INTEGER(&endTime); //計測終了
					KillTimer(hwnd, DAMAGE_TIMER_ID);
					screenState = GAMEOVER;
					if (bgm) Stop(TYPING_BGM);
					if (se) PlaySE(CLEAR_SE);
				}
				enemy.life = ENEMY_LIFE(enemyCount, selectedStoryItem);
				enemy.power = ENEMY_POWER(enemyCount, selectedStoryItem);
			}
			break;
		}
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	case WM_KEYDOWN://キー入力時-------------------------------------------------------------------------
		if (ImmGetOpenStatus(himc))//日本語入力(IME)がONのときOFFにする
			ImmSetOpenStatus(himc, false);
		switch (screenState) {
		case START://スタート画面
			if (wp != VK_RETURN)
				return 0;
			if (se) PlaySE(DECIDE_SE);
			screenState = SELECT;
			selectedMenuItem = E_SENTENCE;
			KillTimer(hwnd, PRESS_ENTER_TIMER_ID);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SELECT://セレクト画面
			switch (wp) {
			case VK_ESCAPE:
			case VK_BACK:
				if (se) PlaySE(BACK_SE);
				screenState = START;
				SetTimer(hwnd, PRESS_ENTER_TIMER_ID, 500, NULL);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_UP:
				if (se) PlaySE(UPDOWN_SE);
				selectedMenuItem = (MenuItem)((selectedMenuItem - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_DOWN:
				if (se) PlaySE(UPDOWN_SE);
				selectedMenuItem = (MenuItem)((selectedMenuItem + 1) % MENU_ITEM_COUNT);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_RETURN:
				if (se) PlaySE(DECIDE_SE);
				switch (selectedMenuItem) {
				case E_SENTENCE://英文タイプ
				case E_WORD://英単語タイプ
					dataLength = readLines((selectedMenuItem == E_SENTENCE) ? ENGLISH_DATA : WORD_DATA, lines, ST);
					if (dataLength <= 0) {
						MessageBox(hwnd, TEXT("データがありません"), TEXT("エラー"), MB_OK);
						return 0;
					}
					if (dataLength < numberOfQuestion) {
						MessageBox(hwnd, TEXT("データが少ないため、出題数を変更します"), TEXT("確認"), MB_OK);
						numberOfQuestion = dataLength;
					}
					makenum(selectedString, dataLength, numberOfQuestion);
					screenState = TYPING;
					count = typingIndex = totalInputLength = totalTextLength = insertion = deletion = substitution = 0;
					for (int i = 0; i < numberOfQuestion; ++i)
						typingString[i][0] = TEXT('\0');
					correct = TRUE;
					countdown = 3;
					SetTimer(hwnd, COUNT_DOWN_TIMER_ID, 1000, NULL);
					InvalidateRect(hwnd, NULL, FALSE);
					if (bgm) Stop(MAIN_BGM);
					if (se) PlaySE(COUNTDOWN1_SE);
					break;
				case RANDOM://ランダムタイプ
					for (int i = 0; i < numberOfQuestion; ++i) {
						int length = rand() % (RANDOM_MODE_MAX - RANDOM_MODE_MIN + 1) + RANDOM_MODE_MIN;
						for (int j = 0; j < length; ++j)
							lines[i][j] = TEXT('a') + rand() % 26;
						lines[i][length] = TEXT('\0');
						selectedString[i] = i;
						typingString[i][0] = TEXT('\0');
					}
					screenState = TYPING;
					count = typingIndex = totalInputLength = totalTextLength = insertion = deletion = substitution = 0;
					correct = TRUE;
					countdown = 3;
					SetTimer(hwnd, COUNT_DOWN_TIMER_ID, 1000, NULL);
					InvalidateRect(hwnd, NULL, FALSE);
					if (bgm) Stop(MAIN_BGM);
					if (se) PlaySE(COUNTDOWN1_SE);
					break;
				case STORY://ストーリーモード
					screenState = STAGE_SELECT;
					selectedStoryItem = STAGE_0;
					InvalidateRect(hwnd, NULL, FALSE);
					break;
				case CONFIG://設定
					screenState = SETTING;
					selectedConfigMenuItem = TYPING_MODE;
					InvalidateRect(hwnd, NULL, FALSE);
					break;
				case EXIT://終了
					DestroyWindow(hwnd);
					break;
				}
				break;
			}
			break;
		case STAGE_SELECT://ステージセレクト画面
			switch (wp) {
			case VK_ESCAPE:
			case VK_BACK:
				if (se) PlaySE(BACK_SE);
				screenState = SELECT;
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_UP:
				if (se) PlaySE(UPDOWN_SE);
				selectedStoryItem = (StoryMenuItem)((selectedStoryItem - 1 + STORY_ITEM_COUNT) % STORY_ITEM_COUNT);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_DOWN:
				if (se) PlaySE(UPDOWN_SE);
				selectedStoryItem = (StoryMenuItem)((selectedStoryItem + 1) % STORY_ITEM_COUNT);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_RETURN:
				if (se) PlaySE(DECIDE_SE);
				switch (selectedStoryItem) {
				case STORY_BACK://戻る
					screenState = SELECT;
					InvalidateRect(hwnd, NULL, FALSE);
					break;
				default://ステージ
					dataLength = readLines(WORD_DATA, lines, ST);
					if (dataLength <= 0) {
						MessageBox(hwnd, TEXT("データがありません"), TEXT("エラー"), MB_OK);
						return 0;
					}
					//ストーリーモードでは、入力を終えるたびに、次の文をランダムに選ぶ。0番目しか使わない。
					selectedString[0] = rand() % dataLength;
					typingString[0][0] = TEXT('\0');
					screenState = TYPING;
					attack = enemyCount = count = typingIndex = totalInputLength = totalTextLength = insertion = deletion = substitution = 0;
					enemy.life = ENEMY_LIFE(enemyCount, selectedStoryItem);
					enemy.power = ENEMY_POWER(enemyCount, selectedStoryItem);
					user.life = DEFAULT_LIFE;
					correct = TRUE;
					countdown = 3;
					SetTimer(hwnd, COUNT_DOWN_TIMER_ID, 1000, NULL);
					InvalidateRect(hwnd, NULL, FALSE);
					if (bgm) Stop(MAIN_BGM);
					if (se) PlaySE(COUNTDOWN1_SE);
					break;
				}
			}
			break;
		case TYPING://タイピング中
			if (countdown != 0)
				return 0;
			switch (wp) {
			case VK_RETURN:
				if (!dpmatching || (selectedMenuItem == STORY && attack > 0))
					return 0;
				//DPマッチングモードのとき
				if (se) PlaySE(DECIDE_SE);
				if (selectedMenuItem == STORY) {//ストーリーモードのとき
					//DPマッチングを行う
					int jframe = lstrlen(lines[selectedString[0]]), iframe = lstrlen(typingString[0]);
					totalTextLength += jframe, totalInputLength += iframe;
					dp(jframe, iframe, lines[selectedString[0]], typingString[0]);
					int ins, del, sub;
					distancemap(jframe, iframe, result[0], ins, del, sub);
					insertion += ins, deletion += del, substitution += sub;
					//与えるダメージ、受けるダメージの計算
					attack = (jframe - del - sub) * user.power;
					user.life -= ins + del + sub;
					enemy.life -= attack;
					if (user.life <= 0) {//ゲームオーバー時
						GetSystemTimeAsULARGE_INTEGER(&endTime); //計測終了
						KillTimer(hwnd, DAMAGE_TIMER_ID);
						screenState = GAMEOVER;
						if (bgm) Stop(TYPING_BGM);
						if (se) PlaySE(GAMEOVER_SE);
					} else if (attack > 0) {
						if (se) PlaySE(ATTACK_SE);
						if (enemy.life <= 0) {
							enemy.life = 0;
							user.life += RECOVER;
							if (se) PlaySE(RECOVER_SE);
						}
						SetTimer(hwnd, ATTACK_TIMER_ID, ATTACK_TIME, NULL);
					} else {
						//次の文の選択
						selectedString[0] = rand() % dataLength;
						typingString[0][0] = TEXT('\0');
						typingIndex = 0;
					}
				} else {//ストーリーモードでないとき
					typingIndex = 0;
					if (++count >= numberOfQuestion) {//出題し終えたら
						GetSystemTimeAsULARGE_INTEGER(&endTime); //計測終了
						screenState = GAMEOVER;
						//DPマッチング
						for (int i = 0; i < numberOfQuestion; ++i) {
							int jframe = lstrlen(lines[selectedString[i]]), iframe = lstrlen(typingString[i]);
							totalTextLength += jframe, totalInputLength += iframe;
							dp(jframe, iframe, lines[selectedString[i]], typingString[i]);
							int ins, del, sub;
							distancemap(jframe, iframe, result[i], ins, del, sub);
							insertion += ins, deletion += del, substitution += sub;
						}
						if (bgm) Stop(TYPING_BGM);
						if (se) PlaySE(CLEAR_SE);
					}
				}
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_ESCAPE:
				if (se) PlaySE(BACK_SE);
				screenState = (selectedMenuItem == STORY) ? STAGE_SELECT : SELECT;
				InvalidateRect(hwnd, NULL, FALSE);
				KillTimer(hwnd, DAMAGE_TIMER_ID);
				KillTimer(hwnd, ATTACK_TIMER_ID);
				if (bgm) {
					Stop(TYPING_BGM);
					PlayBGM(MAIN_BGM, hwnd);
				}
				break;
			}
			break;
		case GAMEOVER://タイピング終了画面
			if (wp != VK_RETURN)
				return 0;
			screenState = SCORE;
			if (se) PlaySE(DECIDE_SE);
			if (bgm) PlayBGM(RESULT_BGM, hwnd);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SCORE://スコア画面
			if (wp != VK_RETURN)
				return 0;
			if (se) PlaySE(DECIDE_SE);
			if (!dpmatching || selectedMenuItem == STORY) {//DPマッチングでないか、ストーリーモードのとき
				if (selectedMenuItem == STORY && user.life <= 0) {
					screenState = STAGE_SELECT;
				} else {
					screenState = START;
					SetTimer(hwnd, PRESS_ENTER_TIMER_ID, 500, NULL);
				}
				if (bgm) {
					Stop(RESULT_BGM);
					PlayBGM(MAIN_BGM, hwnd);
				}
			} else {//ストーリーモードでなく、DPマッチングモードのとき
				count = 0;
				screenState = REVIEW;
			}
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case REVIEW://レビュー(エラー箇所指摘)画面
			if (wp != VK_RETURN)
				return 0;
			if (se) PlaySE(DECIDE_SE);
			if (++count >= numberOfQuestion) {
				screenState = START;
				SetTimer(hwnd, PRESS_ENTER_TIMER_ID, 500, NULL);
				if (bgm) {
					Stop(RESULT_BGM);
					PlayBGM(MAIN_BGM, hwnd);
				}
			}
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SETTING://設定画面
			switch (wp) {
			case VK_ESCAPE:
			case VK_BACK:
				if (se) PlaySE(BACK_SE);
				screenState = SELECT;
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_RETURN:
				if (selectedConfigMenuItem != CONFIG_BACK)
					return 0;
				if (se) PlaySE(DECIDE_SE);
				screenState = SELECT;
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_UP:
				if (se) PlaySE(UPDOWN_SE);
				selectedConfigMenuItem = (ConfigMenuItem)((selectedConfigMenuItem - 1 + CONFIG_MENU_ITEM_COUNT) % CONFIG_MENU_ITEM_COUNT);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_DOWN:
				if (se) PlaySE(UPDOWN_SE);
				selectedConfigMenuItem = (ConfigMenuItem)((selectedConfigMenuItem + 1) % CONFIG_MENU_ITEM_COUNT);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_LEFT:
				switch (selectedConfigMenuItem) {
				case TYPING_MODE:
					if (se) PlaySE(LEFTRIGHT_SE);
					dpmatching = !dpmatching;
					break;
				case NUMBER_OF_QUESTION:
					if (--numberOfQuestion < MIN)
						numberOfQuestion = MIN;
					else if (se)
						PlaySE(LEFTRIGHT_SE);
					break;
				case ASSIST:
					if (se) PlaySE(LEFTRIGHT_SE);
					assist = !assist;
					break;
				case BGM:
					if (se) PlaySE(LEFTRIGHT_SE);
					bgm = !bgm;
					if (bgm) PlayBGM(MAIN_BGM, hwnd);
					else Stop(MAIN_BGM);
					break;
				case SE:
					se = !se;
					if (se) PlaySE(LEFTRIGHT_SE);
					break;
				}
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case VK_RIGHT:
				switch (selectedConfigMenuItem) {
				case TYPING_MODE:
					if (se) PlaySE(LEFTRIGHT_SE);
					dpmatching = !dpmatching;
					break;
				case NUMBER_OF_QUESTION:
					if (++numberOfQuestion > MAX)
						numberOfQuestion = MAX;
					else if (se)
						PlaySE(LEFTRIGHT_SE);
					break;
				case ASSIST:
					if (se) PlaySE(LEFTRIGHT_SE);
					assist = !assist;
					break;
				case BGM:
					if (se) PlaySE(LEFTRIGHT_SE);
					bgm = !bgm;
					if (bgm) PlayBGM(MAIN_BGM, hwnd);
					else Stop(MAIN_BGM);
					break;
				case SE:
					se = !se;
					if (se) PlaySE(LEFTRIGHT_SE);
					break;
				}
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			}
			break;
		}
		return 0;
	case MM_MCINOTIFY://BGMに関する通知--------------------------------------------------------------------
		if (wp != MCI_NOTIFY_SUCCESSFUL)
			return 0;
		if (lp == mciGetDeviceID(TYPING_BGM))
			PlayBGM(TYPING_BGM, hwnd);
		else if (lp == mciGetDeviceID(MAIN_BGM))
			PlayBGM(MAIN_BGM, hwnd);
		else if (lp == mciGetDeviceID(RESULT_BGM))
			PlayBGM(RESULT_BGM, hwnd);
		else if (lp == mciGetDeviceID(SILENT_BGM))
			PlayBGM(SILENT_BGM, hwnd);
		return 0;
	case WM_CHAR://文字入力された時-----------------------------------------------------------------------
		if (countdown != 0 || screenState != TYPING || wp == VK_RETURN || wp == VK_TAB || (selectedMenuItem == STORY && attack > 0))
			return 0;
		if (!dpmatching) {//DPマッチングモードでないとき
			totalInputLength++;//入力総文字数
			correct = ((TCHAR)wp == lines[selectedString[count]][typingIndex]);//入力が正しいかどうか
			if (correct) {//入力が正しい時
				if (se) PlaySE(KEY_SE);
				totalTextLength++;//正しい入力文字数
				typingString[count][typingIndex++] = (TCHAR)wp;
				typingString[count][typingIndex] = TEXT('\0');
				if (lstrlen(lines[selectedString[count]]) == typingIndex) {//打ち終えた時
					if (selectedMenuItem == STORY) {//ストーリーモードのとき
						attack = typingIndex * user.power;
						enemy.life -= attack;
						if (enemy.life <= 0) {
							enemy.life = 0;
							user.life += RECOVER;
							if (se) PlaySE(RECOVER_SE);
						}
						if (se) PlaySE(ATTACK_SE);
						SetTimer(hwnd, ATTACK_TIMER_ID, ATTACK_TIME, NULL);
					} else {//ストーリーモードでないとき
						typingIndex = 0;
						if (++count >= numberOfQuestion) {//出題し終えたら
							GetSystemTimeAsULARGE_INTEGER(&endTime); //計測終了
							screenState = GAMEOVER;
							if (bgm) Stop(TYPING_BGM);
							if (se) PlaySE(CLEAR_SE);
						}
					}
				}
			} else {//入力が間違っていた時
				if (se) PlaySE(NG_SE);
				if (selectedMenuItem == STORY && --user.life <= 0) {//ストーリーモードのとき、ユーザーにダメージを与え、0以下になったら
					GetSystemTimeAsULARGE_INTEGER(&endTime); //計測終了
					KillTimer(hwnd, DAMAGE_TIMER_ID);
					screenState = GAMEOVER;
					if (bgm) Stop(TYPING_BGM);
					if (se) PlaySE(GAMEOVER_SE);
				}
			}
		} else { //DPマッチングモードの時
			if (se) PlaySE(KEY_SE);
			if (wp == VK_BACK) {//1文字削除
				if (--typingIndex < 0)
					typingIndex = 0;
				typingString[count][typingIndex] = TEXT('\0');
			} else {//普通の入力
				typingString[count][typingIndex] = (TCHAR)wp;
				if (++typingIndex >= CL - 1)
					typingIndex = CL - 1;
				typingString[count][typingIndex] = TEXT('\0');
			}
		}
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

//メイン関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {

	//ウィンドウクラスの作成
	WNDCLASS winc;
	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = WndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInstance;
	winc.hIcon = LoadIcon(hInstance, TEXT("TYPING_ICON"));
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT("TYPING");

	//ウィンドウクラスの登録
	if (!RegisterClass(&winc)) return -1;

	//ウィンドウの生成
	HWND hwnd = CreateWindow(
		TEXT("TYPING"), TITLE,
		(WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL
		);

	if (hwnd == NULL) return -1;

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

/**
* スタート画面を描画する
* @param hdc
*/
void drawStart(HDC hdc) {
	drawBMP(hdc, START_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	//タイトル描画
	SetTextColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, Font(100, FW_BOLD, TRUE, TRUE, VARIABLE_PITCH | FF_ROMAN));
	TextOut(hdc, WIDTH / 2, HEIGHT / 3, TITLE, lstrlen(TITLE));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//PRESS ENTER描画
	if (pressEnterVisible) {
		const TCHAR *PRESS_ENTER_STR = TEXT("Press Enter");
		SetTextColor(hdc, RGB(255, 255, 0));
		SelectObject(hdc, Font(30, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_ROMAN));
		TextOut(hdc, WIDTH / 2, HEIGHT * 2 / 3, PRESS_ENTER_STR, lstrlen(PRESS_ENTER_STR));
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	}
}

/**
* 選択(メニュー)画面を描画する
* @param hdc
*/
void drawSelect(HDC hdc) {
	drawBMP(hdc, SELECT_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	//メニュー項目描画
	SelectObject(hdc, Font(50, FW_BOLD, TRUE, FALSE, VARIABLE_PITCH | FF_ROMAN));
	for (int i = 0; i < MENU_ITEM_COUNT; ++i) {
		SetTextColor(hdc, (i == selectedMenuItem) ? RGB(255, 0, 0) : RGB(0, 0, 0));
		TextOut(hdc, 100, 50 + i * 80, MENU_STR[i], lstrlen(MENU_STR[i]));
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}
/**
* ステージ選択画面を描画する
* @param hdc
*/
void drawStageSelect(HDC hdc) {
	drawBMP(hdc, SELECT_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	//ステージ選択項目を描画
	SelectObject(hdc, Font(35, FW_BOLD, TRUE, FALSE, FIXED_PITCH | FF_ROMAN));
	for (int i = 0; i < STORY_ITEM_COUNT; ++i) {
		SetTextColor(hdc, (i == selectedStoryItem) ? RGB(255, 0, 0) : RGB(0, 0, 0));
		TextOut(hdc, 100, 40 + i * 45, STORY_MENU_STR[i], lstrlen(STORY_MENU_STR[i]));
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}

/**
* キーボードアシストを描画する
* @param hdc
*/
void drawAssist(HDC hdc) {
	drawBMP(hdc, KEYBORD_IMG, 50, 320);
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	SelectObject(hdc, CreatePen(PS_SOLID, 3, RGB(255, 0, 0)));
	//Enterを囲う
	if (lstrlen(lines[selectedString[count]]) <= typingIndex) {
		POINT po[6];
		po[0].x = 682, po[0].y = 372;
		po[1].x = 682, po[1].y = 418;
		po[2].x = 695, po[2].y = 418;
		po[3].x = 695, po[3].y = 464;
		po[4].x = 744, po[4].y = 464;
		po[5].x = 744, po[5].y = 372;
		Polygon(hdc, po, 6);
		DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
		return;
	}
	const int KEY_SIZE = 46; //キーの縦および横の長さ
	TCHAR ch = lines[selectedString[count]][typingIndex];
	//スペースの時
	if (TEXT(' ') == ch) {
		Rect(hdc, 311, 510, 117, KEY_SIZE);
		DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
		return;
	}
	for (int i = 0; i < 13; ++i) {
		if (TEXT("1234567890-^\\")[i] == ch) {
			Rect(hdc, 100 + KEY_SIZE*i, 326, KEY_SIZE, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		} else if (TEXT("!\"#$%&'()0=~|")[i] == ch) {
			Rect(hdc, 100 + KEY_SIZE*i, 326, KEY_SIZE, KEY_SIZE);
			if (i <= 4)//シフトキー
				Rect(hdc, 679, 464, 65, KEY_SIZE);
			else
				Rect(hdc, 56, 464, 118, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		}
	}
	for (int i = 0; i < 12; ++i) {
		if (TEXT("qwertyuiop@[")[i] == ch) {
			Rect(hdc, 130 + KEY_SIZE*i, 372, KEY_SIZE, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		} else if (TEXT("QWERTYUIOP`{")[i] == ch) {
			Rect(hdc, 130 + KEY_SIZE*i, 372, KEY_SIZE, KEY_SIZE);
			if (i <= 4)//シフトキー
				Rect(hdc, 679, 464, 65, KEY_SIZE);
			else
				Rect(hdc, 56, 464, 118, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		} else if (TEXT("asdfghjkl;:]")[i] == ch) {
			Rect(hdc, 143 + KEY_SIZE*i, 418, KEY_SIZE, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		} else if (TEXT("ASDFGHJKL+*}")[i] == ch) {
			Rect(hdc, 143 + KEY_SIZE*i, 418, KEY_SIZE, KEY_SIZE);
			if (i <= 4)//シフトキー
				Rect(hdc, 679, 464, 65, KEY_SIZE);
			else
				Rect(hdc, 56, 464, 118, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		}
	}
	for (int i = 0; i < 10; ++i) {
		if (TEXT("zxcvbnm,./")[i] == ch) {
			Rect(hdc, 173 + KEY_SIZE*i, 464, KEY_SIZE, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		} else if (TEXT("ZXCVBNM<>?_")[i] == ch) {
			Rect(hdc, 173 + KEY_SIZE*i, 464, KEY_SIZE, KEY_SIZE);
			if (i <= 4)//シフトキー
				Rect(hdc, 679, 464, 65, KEY_SIZE);
			else
				Rect(hdc, 56, 464, 118, KEY_SIZE);
			DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
			return;
		}
	}
	DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
}

/**
* タイピング終了画面を描画する
* @param hdc
*/
void drawGameover(HDC hdc) {
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	Rect(hdc, 0, 0, WIDTH, HEIGHT);

	SelectObject(hdc, Font(100, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_SCRIPT));
	SetTextColor(hdc, RGB(255, 255, 255));
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	if (selectedMenuItem == STORY) {
		if (user.life <= 0) {
			const TCHAR *GAMEOVER_STR = TEXT("GAME OVER");
			TextOut(hdc, WIDTH / 2, HEIGHT / 2, GAMEOVER_STR, lstrlen(GAMEOVER_STR));
		} else {
			const TCHAR *CLEAR_STR = TEXT("CLEAR");
			TextOut(hdc, WIDTH / 2, HEIGHT / 2, CLEAR_STR, lstrlen(CLEAR_STR));
		}
	} else {
		const TCHAR *COMPLETE_STR = TEXT("終了");
		TextOut(hdc, WIDTH / 2, HEIGHT / 2, COMPLETE_STR, lstrlen(COMPLETE_STR));
	}
}

/**
* 指定文字数で折り返す文字列を描画する
* @param hdc
* @param x 描画始点の左上x座標
* @param y 描画始点の左上y座標
* @param text 描画する文字列
* @param n 折り返す文字数
*/
void drawString(HDC hdc, int x, int y, const TCHAR *text, int n) {
	int length = lstrlen(text);
	SIZE size;
	SetTextAlign(hdc, TA_LEFT);
	GetTextExtentPoint32(hdc, text, length, &size);
	for (int i = 0; i < length / n; ++i) {
		TextOut(hdc, x, y, &text[i * n], n);
		y += size.cy;
	}
	if (length % n != 0)
		TextOut(hdc, x, y, &text[(length / n) * n], length % n);
}

/**
* タイピング中の画面を描画する
* @param hdc
*/
void drawTyping(HDC hdc) {
	SetBkMode(hdc, TRANSPARENT);

	TCHAR tempString[128];
	if (selectedMenuItem == STORY) {
		wsprintf(tempString, STAGE_IMG, selectedStoryItem);
		drawBMP(hdc, tempString, 0, 0);
	} else {
		drawBMP(hdc, TYPING_IMG, 0, 0);
	}

	//カウントダウン
	if (countdown != 0) {
		SelectObject(hdc, Font(200, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_SCRIPT));
		SetTextColor(hdc, RGB(0, 0, 0));
		wsprintf(tempString, TEXT("%d"), countdown);
		SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
		TextOut(hdc, WIDTH / 2, HEIGHT / 2, tempString, lstrlen(tempString));
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
		return;
	}
	//何問目、何体目 描画
	SetTextAlign(hdc, TA_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, Font(40, FW_BOLD, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));
	if (selectedMenuItem == STORY)
		wsprintf(tempString, TEXT("%2d体目"), enemyCount + 1);
	else
		wsprintf(tempString, TEXT("%2d問目"), count + 1);
	TextOut(hdc, 20, 20, tempString, lstrlen(tempString));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	if (selectedMenuItem == STORY) {//ストーリーモード
		//ライフ描画
		SelectObject(hdc, CreateSolidBrush(RGB(200, 200, 0)));
		SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(200, 200, 0)));
		Rect(hdc, 200, 25, user.life, 30);
		Rect(hdc, WIDTH / 2 - enemy.life / 2, HEIGHT / 2 - 50, enemy.life, 15);
		DeleteObject(SelectObject(hdc, GetStockObject(WHITE_BRUSH)));
		DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
		//ライフ、パワーの文字列描画
		SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, FALSE, FIXED_PITCH | FF_SWISS));
		SetTextColor(hdc, RGB(0, 0, 0));
		SetTextAlign(hdc, TA_LEFT);
		wsprintf(tempString, TEXT(" Life:%4d "), user.life);
		TextOut(hdc, 200, 30, tempString, lstrlen(tempString));
		SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
		wsprintf(tempString, TEXT(" Life:%3d  Power:%3d "), enemy.life, enemy.power);
		TextOut(hdc, WIDTH / 2, HEIGHT / 2 - 55, tempString, lstrlen(tempString));
		//モンスター描画
		wsprintf(tempString, CHARACTER_IMG, selectedStoryItem, enemyCount);
		int h = drawBMP(hdc, tempString, WIDTH / 2, HEIGHT / 2 - 30, RGB(255, 255, 255));
		//攻撃モーション描画
		if (attack > 0) {
			drawBMP(hdc, ATTACK_IMG, WIDTH / 2, HEIGHT / 2 - 30 + h / 2 - 60, RGB(0, 0, 0));
			SetTextColor(hdc, RGB(255, 0, 0));
			SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
			wsprintf(tempString, TEXT("%d"), -attack);
			TextOut(hdc, WIDTH / 2, HEIGHT / 2 - 35, tempString, lstrlen(tempString));
			if (enemy.life <= 0) {// 回復値を描画
				SetTextColor(hdc, RGB(0, 0, 255));
				SetTextAlign(hdc, TA_LEFT);
				wsprintf(tempString, TEXT("           +%d"), RECOVER);
				TextOut(hdc, 200, 30, tempString, lstrlen(tempString));
			}
		}
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	} else if (assist) {//アシスト描画
		drawAssist(hdc);
	}

	if (!correct) {//タイプミスのとき
		SelectObject(hdc, Font(30, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_ROMAN));
		SetTextAlign(hdc, TA_RIGHT);
		SetTextColor(hdc, RGB(255, 0, 0));
		const TCHAR *MISS_STR = TEXT("MISS!!");
		TextOut(hdc, WIDTH - 50, 25, MISS_STR, lstrlen(MISS_STR));
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	}
	//出題文と入力文の描画
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, FALSE, FIXED_PITCH | FF_SWISS));
	SetTextAlign(hdc, TA_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));
	SIZE size;
	GetTextExtentPoint32(hdc, TEXT(" "), 1, &size);
	const int MARGIN = 50;
	RECT rect;
	rect.left = MARGIN, rect.right = WIDTH - MARGIN;
	rect.top = 100, rect.bottom = HEIGHT;
	DrawText(hdc, lines[selectedString[count]], -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	int width = rect.right - rect.left, n = width / size.cx;
	rect.left = (WIDTH - width) / 2;
	drawString(hdc, rect.left, rect.top, lines[selectedString[count]], n);
	int length = lstrlen(lines[selectedString[count]]);
	rect.bottom = rect.top, rect.top += (1 + length / n + ((length % n != 0) ? 1 : 0)) * size.cy;
	if (rect.top - rect.bottom <= size.cy * 2)// 問題文が一行のときは、画面右端で折り返す(そうでないときは、同じ幅で折り返す)
		rect.right = WIDTH - MARGIN, width = rect.right - rect.left, n = width / size.cx;
	drawString(hdc, rect.left, rect.top, typingString[count], n);
	rect.left += (typingIndex % n) * size.cx, rect.top += (typingIndex / n) * size.cy;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//キャレット描画
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	MoveToEx(hdc, rect.left, rect.top, NULL);
	LineTo(hdc, rect.left, rect.top + size.cy);
}

/**
* スコア画面を描画する
* @param hdc
*/
void drawScore(HDC hdc) {
	drawBMP(hdc, RESULT_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));

	const int MARGIN = 30;
	RECT rect;
	rect.left = MARGIN, rect.right = WIDTH - MARGIN;
	rect.top = MARGIN, rect.bottom = HEIGHT;

	TCHAR tempString[512];
	SelectObject(hdc, Font(40, FW_BOLD, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));
	DrawText(hdc, TEXT("成績"), -1, &rect, DT_LEFT);
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//入力時間
	double time = (endTime.QuadPart - startTime.QuadPart) / 10000000.0;

	rect.left = 50, rect.top = 100;
	SelectObject(hdc, Font(25, FW_BOLD, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));

	SetTextColor(hdc, RGB(0, 0, 0));
	wsprintf(tempString, TEXT("出題文字総数 %10d [文字]\n\n入力文字総数 %10d [文字]\n"), totalTextLength, totalInputLength);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;

	SetTextColor(hdc, RGB(255, 0, 0));
	if (dpmatching)
		wsprintf(tempString, TEXT("脱落誤り総数 %10d [個]\n\n挿入誤り総数 %10d [個]\n\n置換誤り総数 %10d [個]\n"), deletion, insertion, substitution);
	else
		wsprintf(tempString, TEXT("入力ミス総数 %10d [個]\n"), totalInputLength - totalTextLength);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;

	SetTextColor(hdc, RGB(0, 0, 255));
	if (totalInputLength == 0)//入力が0文字のとき
		swprintf(tempString, 512, TEXT("合計入力時間　　　　  %10.3lf [s]\n\n1文字あたりの入力時間         ∞ [s]\n\n1秒あたりの入力文字数 %10.3lf [文字]\n"), time, totalInputLength / time);
	else
		swprintf(tempString, 512, TEXT("合計入力時間　　　　  %10.3lf [s]\n\n1文字あたりの入力時間 %10.3lf [s]\n\n1秒あたりの入力文字数 %10.3lf [文字]\n"), time, time / totalInputLength, totalInputLength / time);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;

	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	SelectObject(hdc, Font(45, FW_HEAVY, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(0, 255, 0));
	swprintf(tempString, 512, TEXT("スコア %7.3lf [%%]"), (dpmatching ? (1 - (double)(deletion + insertion + substitution) / totalTextLength) : (totalTextLength / (double)totalInputLength)) * 100);
	DrawText(hdc, tempString, -1, &rect, DT_RIGHT);
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}

/**
* レビュー画面を描画する
* @param hdc
*/
void drawReview(HDC hdc) {
	drawBMP(hdc, RESULT_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));

	const int MARGIN = 30;
	RECT rect;
	rect.left = MARGIN, rect.right = WIDTH - MARGIN;
	rect.top = MARGIN, rect.bottom = HEIGHT;

	TCHAR tempString[32];
	SelectObject(hdc, Font(40, FW_BOLD, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));
	wsprintf(tempString, TEXT("復習 %2d問目"), count + 1);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT);
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	rect.left = 50, rect.top = 100, rect.right = WIDTH - rect.left;
	SelectObject(hdc, Font(30, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	DrawText(hdc, TEXT("出題文"), -1, &rect, DT_LEFT);
	DrawText(hdc, TEXT("出題文\n"), -1, &rect, DT_LEFT | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//出題文描画
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	SIZE size;
	GetTextExtentPoint32(hdc, TEXT(" "), 1, &size);
	int n = (rect.right - rect.left) / size.cx, length = lstrlen(lines[selectedString[count]]);
	drawString(hdc, rect.left, rect.top, lines[selectedString[count]], n);
	rect.top += (1 + length / n + ((length % n != 0) ? 1 : 0)) * size.cy;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	SelectObject(hdc, Font(30, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	DrawText(hdc, TEXT("入力文"), -1, &rect, DT_LEFT);
	DrawText(hdc, TEXT("入力文\n"), -1, &rect, DT_LEFT | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//入力文描画
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	drawString(hdc, rect.left, rect.top, typingString[count], n);
	length = lstrlen(typingString[count]);
	rect.top += (1 + length / n + ((length % n != 0) ? 1 : 0)) * size.cy;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	SelectObject(hdc, Font(30, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	SetTextColor(hdc, RGB(255, 0, 0));
	DrawText(hdc, TEXT("　　　　　　　　 ■"), -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 255, 0));
	DrawText(hdc, TEXT("　　　　　　　　　　　　 ■"), -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 0, 255));
	DrawText(hdc, TEXT("　　　　　　　　　　　　　　　　 ■"), -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));
	DrawText(hdc, TEXT("エラー箇所 (脱落:　 挿入:　 置換:　)"), -1, &rect, DT_LEFT);
	DrawText(hdc, TEXT("エラー箇所 (脱落:　 挿入:　 置換:　)\n"), -1, &rect, DT_LEFT | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//エラー箇所描画
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	int x = rect.left, y = rect.top;
	// 先頭の脱落エラー
	if (result[count][length].error == DELETION) {
		SetTextColor(hdc, RGB(255, 0, 0));
		for (int i = result[count][length].start; i <= result[count][length].end; ++i) {
			TextOut(hdc, x, y, &lines[selectedString[count]][i], 1);
			x += size.cx;
			if (x + size.cx >= rect.right)
				x = rect.left, y += 2 * size.cy;
		}
	}
	for (int i = 0; i < length; ++i) {
		switch (result[count][i].error) {
		case INSERTION:
			SetTextColor(hdc, RGB(0, 255, 0));
			break;
		case SUBSTITUTION:
			SetTextColor(hdc, RGB(0, 0, 255));
			TextOut(hdc, x, y + size.cy, &lines[selectedString[count]][result[count][i].start], 1);
			SetTextColor(hdc, RGB(0, 0, 255));
			break;
		default:
			SetTextColor(hdc, RGB(0, 0, 0));
		}
		// 入力文字の出力
		TextOut(hdc, x, y, &typingString[count][i], 1);
		x += size.cx;
		if (x + size.cx >= rect.right)
			x = rect.left, y += 2 * size.cy;
		// 脱落エラー
		if (result[count][i].error == DELETION) {
			SetTextColor(hdc, RGB(255, 0, 0));
			for (int j = result[count][i].start; j <= result[count][i].end; ++j) {
				TextOut(hdc, x, y, &lines[selectedString[count]][j], 1);
				x += size.cx;
				if (x + size.cx >= rect.right)
					x = rect.left, y += 2 * size.cy;
			}
		}
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}

/**
* 設定画面を描画する
* @param hdc
*/
void drawSetting(HDC hdc) {
	drawBMP(hdc, SETTING_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);

	SelectObject(hdc, Font(60, FW_BOLD, FALSE, TRUE, VARIABLE_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(255, 200, 10));
	const TCHAR *SETTING_STR = TEXT("設定");
	TextOut(hdc, 30, 30, SETTING_STR, lstrlen(SETTING_STR));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	SelectObject(hdc, Font(20, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(0, 0, 0));
	const TCHAR *EXPLAIN_STR = TEXT("※出題数とアシストの設定は、ストーリーモード以外で有効です");
	TextOut(hdc, 180, 50, EXPLAIN_STR, lstrlen(EXPLAIN_STR));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//設定項目の描画
	const int INTERVAL = 70, TOP = 120;
	SelectObject(hdc, Font(40, FW_BOLD, TRUE, FALSE, VARIABLE_PITCH | FF_ROMAN));
	for (int i = 0; i < CONFIG_MENU_ITEM_COUNT; ++i) {
		SetTextColor(hdc, (selectedConfigMenuItem == i) ? RGB(255, 0, 0) : RGB(0, 0, 0));
		TextOut(hdc, 50, TOP + INTERVAL*i, CONFIG_MENU_STR[i], lstrlen(CONFIG_MENU_STR[i]));
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//タイピングモードの描画
	SelectObject(hdc, Font(30, FW_MEDIUM, FALSE, FALSE, FIXED_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(0, 0, 0));
	SIZE size1, size2;
	const TCHAR *DP_STR = TEXT("DP Matching"), *NORMAL_STR = TEXT("Normal");
	TextOut(hdc, WIDTH / 2, TOP, DP_STR, lstrlen(DP_STR));
	GetTextExtentPoint32(hdc, DP_STR, lstrlen(DP_STR), &size1);
	TextOut(hdc, WIDTH / 2 + size1.cx + 50, TOP, NORMAL_STR, lstrlen(NORMAL_STR));
	GetTextExtentPoint32(hdc, NORMAL_STR, lstrlen(NORMAL_STR), &size2);
	//現在のタイピングモードを囲う
	SelectObject(hdc, CreatePen(PS_SOLID, 3, RGB(0, 0, 255)));
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	if (dpmatching)
		RoundRect(hdc, WIDTH / 2, TOP, size1.cx, size1.cy);
	else
		RoundRect(hdc, WIDTH / 2 + size1.cx + 50, TOP, size2.cx, size2.cy);
	//出題数を描画
	TCHAR number[8];
	wsprintf(number, TEXT(" %2d "), numberOfQuestion);
	TextOut(hdc, WIDTH / 2 + 30, TOP + INTERVAL, number, lstrlen(number));
	GetTextExtentPoint32(hdc, number, lstrlen(number), &size1);
	POINT po1[3], po2[3];
	po1[0].x = WIDTH / 2, po1[0].y = TOP + INTERVAL + 15;
	po1[1].x = po1[0].x + 30, po1[1].y = po1[0].y - 15;
	po1[2].x = po1[0].x + 30, po1[2].y = po1[0].y + 15;
	po2[0].x = po1[2].x + size1.cx, po2[0].y = po1[1].y;
	po2[1].x = po2[0].x, po2[1].y = po1[2].y;
	po2[2].x = po2[0].x + 30, po2[2].y = po1[0].y;
	if (numberOfQuestion != MIN)
		Polygon(hdc, po1, 3);
	if (numberOfQuestion != MAX)
		Polygon(hdc, po2, 3);
	//ON、OFFの描画、選択されたものを囲う
	const TCHAR *ON_STR = TEXT("ON"), *OFF_STR = TEXT("OFF");
	GetTextExtentPoint32(hdc, ON_STR, lstrlen(ON_STR), &size1);
	GetTextExtentPoint32(hdc, OFF_STR, lstrlen(OFF_STR), &size2);
	for (int i = ASSIST; i < CONFIG_BACK; ++i) {
		TextOut(hdc, WIDTH / 2, TOP + INTERVAL * i, ON_STR, lstrlen(ON_STR));
		TextOut(hdc, WIDTH / 2 + size1.cx + 50, TOP + INTERVAL * i, OFF_STR, lstrlen(OFF_STR));
		if ((assist && i == ASSIST) || (bgm && i == BGM) || (se && i == SE))
			RoundRect(hdc, WIDTH / 2, TOP + INTERVAL * i, size1.cx, size1.cy);
		else
			RoundRect(hdc, WIDTH / 2 + size1.cx + 50, TOP + INTERVAL * i, size2.cx, size2.cy);
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
}

/**
* BMPの描画
* @param hdc
* @param path BMPファイルのパス
* @param x 描画する左上x座標
* @param y 描画する左上y座標
*/
void drawBMP(HDC hdc, const TCHAR *path, int x, int y) {
	HBITMAP hBmp;
	hBmp = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (hBmp != NULL) {
		BITMAP bm;
		HDC hBmpDC;
		hBmpDC = CreateCompatibleDC(hdc);
		SelectObject(hBmpDC, hBmp);
		GetObject(hBmp, sizeof(bm), &bm);
		BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hBmpDC, 0, 0, SRCCOPY);
		DeleteDC(hBmpDC);
		DeleteObject(hBmp);
	}
}

/**
* BMPの描画(透過色指定あり)
* @param hdc
* @param path BMPファイルのパス
* @param x 描画する中心x座標
* @param y 描画する上y座標
* @param rgb 透過色
* @param 画像の高さ
*/
int drawBMP(HDC hdc, const TCHAR *path, int x, int y, UINT rgb) {
	HBITMAP hBmp;
	hBmp = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (hBmp != NULL) {
		BITMAP bm;
		HDC hBmpDC;
		hBmpDC = CreateCompatibleDC(hdc);
		SelectObject(hBmpDC, hBmp);
		GetObject(hBmp, sizeof(bm), &bm);
		TransparentBlt(hdc, x - bm.bmWidth / 2, y, bm.bmWidth, bm.bmHeight, hBmpDC, 0, 0, bm.bmWidth, bm.bmHeight, rgb);
		DeleteDC(hBmpDC);
		DeleteObject(hBmp);
		return bm.bmHeight;
	}
	return 0;
}

/**
* ファイルから文字列を読み取る
* @param filename ファイル名
* @param lines 文字列の配列
* @param length 最大読み取り行数
* @return 実際に読み取った行数
*/
int readLines(const TCHAR *filename, TCHAR lines[][CL], int length) {
	HANDLE hFile;
	hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;
	DWORD wReadSize;
	char ch;
	int i, j;
	for (i = 0; i < length; ++i) {
		for (j = 0; j < CL; ++j) {
			ReadFile(hFile, &ch, 1, &wReadSize, NULL);
			if (wReadSize == 0) {
				lines[i][j] = TEXT('\0');
				CloseHandle(hFile);
				return (j != 0) ? i + 1 : i;
			}
			if (ch == '\r' || ch == '\n') {
				lines[i][j] = TEXT('\0');
				if (j == 0)
					--i;
				break;
			}
			lines[i][j] = (TCHAR)ch;
		}
		if (j == CL) { //最大文字数を超えたとき
			lines[i][CL - 1] = TEXT('\0');
			while (TRUE) {//この行を読み飛ばす
				ReadFile(hFile, &ch, 1, &wReadSize, NULL);
				if (wReadSize == 0 || ch == '\r' || ch == '\n')
					break;
			}
		}
	}
	CloseHandle(hFile);
	return i;
}

/**
* 値の入れ替え
* @param a int型
* @param b int型
*/
void swap(int &a, int &b) {
	int t = a;
	a = b, b = t;
}

/**
* 重複のないランダムな数列を取得する(0からlength未満の数字からselectLength個選ばれる)
* @param num 得られた数列
* @param length 生成する数字の最大値+1
* @param selectLength 得たい数列の長さ
*/
void makenum(int *num, int length, int selectLength) {
	int i, rnum[ST];
	int offset = rand() % (length - selectLength + 1);
	for (i = 0; i < length; ++i)
		rnum[i] = i;
	for (i = length - 1; i >= 1; --i)
		swap(rnum[i], rnum[rand() % (i + 1)]);
	for (i = 0; i < selectLength; ++i)
		num[i] = rnum[offset + i];
}

/**
* DPマッチング
* @param jframe 出題文の文字数
* @param iframe 入力文の文字数
* @param reftext 出題文
* @param inputtext 入力文
* @return 入力文と出題文の距離
*/
double dp(int jframe, int iframe, TCHAR *reftext, TCHAR *inputtext) {
	int i, j, dd, dmin;
	double d;

	initialization(jframe, iframe);
	for (i = 1; i < iframe + 1; i++) {
		for (j = 1; j < jframe + 1; j++) {
			ld[i][j] = 0;
			if (*(reftext + j - 1) != *(inputtext + i - 1))
				ld[i][j] = 1;
			dmin = gd[i - 1][j - 1] + ld[i][j];
			path[i][j] = 2;
			dd = gd[i - 1][j];
			if (dd < dmin) {
				dmin = dd;
				path[i][j] = 1;
			}
			dd = gd[i][j - 1];
			if (dd < dmin) {
				dmin = dd;
				path[i][j] = 3;
			}
			gd[i][j] = dmin + ld[i][j];
		}
	}
	d = (double)gd[iframe][jframe] / (double)(iframe + jframe);
	return d;
}

/**
* バックトレースする
* @param jframe 出題文の文字数
* @param iframe 入力文の文字数
* @param result エラー情報を返す
* @param insetion 挿入エラー数を返す
* @param deletion 脱落エラー数を返す
* @param substitution 置換エラー数を返す
*/
void distancemap(int jframe, int iframe, Error *result, int &insertion, int &deletion, int &substitution) {
	int i, j;
	for (i = 0; i < iframe + 1; i++) {
		for (j = 0; j < jframe + 1; j++)
			trace[i][j] = 0;
	}
	insertion = deletion = substitution = 0;
	trace[iframe][jframe] = 1;
	result[iframe].error = NONE;
	for (i = iframe; i >= 1; i--) {
		result[i - 1].error = NONE;
		for (j = jframe; j >= 1; j--) {
			if (trace[i][j] == 1) {
				switch (path[i][j]) {
				case 2:
					if (gd[i][j] > gd[i - 1][j - 1]) {
						substitution++;
						result[i - 1].error = SUBSTITUTION;
						result[i - 1].start = result[i - 1].end = j - 1;
					}
					trace[i - 1][j - 1] = 1;
					break;
				case 1:
					trace[i - 1][j] = 1;
					insertion++;
					result[i - 1].error = INSERTION;
					break;
				case 3:
					trace[i][j - 1] = 1;
					deletion++;
					if (result[i - 1].error == DELETION) {
						result[i - 1].start = j - 1;
					} else {
						result[i - 1].error = DELETION;
						result[i - 1].start = result[i - 1].end = j - 1;
					}
					break;
				}
			}
		}
	}
	for (i = iframe; i >= 1; --i) {//先頭の挿入エラー
		if (trace[i][0] == 1) {
			trace[i - 1][0] = 1;
			result[i - 1].error = INSERTION;
			insertion++;
		}
	}
	for (j = jframe; j >= 1; --j) {//先頭の脱落エラーをiframeに記録
		if (trace[0][j] == 1) {
			trace[0][j - 1] = 1;
			deletion++;
			if (result[iframe].error == DELETION) {
				result[iframe].start = j - 1;
			} else {
				result[iframe].error = DELETION;
				result[iframe].start = result[iframe].end = j - 1;
			}
		}
	}
}

/**
* DPマッチングの初期化処理
* @param jframe 出題文の文字数
* @param iframe 入力文の文字数
*/
void initialization(int jframe, int iframe) {
	for (int i = 1; i < iframe + 1; i++)
		gd[i][0] = i;
	for (int j = 1; j < jframe + 1; j++)
		gd[0][j] = j;
	gd[0][0] = 0;
}

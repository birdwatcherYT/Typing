#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <wchar.h>
#include <mmsystem.h>
// BGM, SE�p���C�u����
#pragma comment(lib, "winmm.lib")
// ���ߐF�摜�p���C�u����
#pragma comment(lib, "msimg32.lib")
// IME�Ď��p���C�u����
#pragma comment(lib, "imm32.lib")

#ifndef UNICODE
#define swprintf snprintf
#endif

//�E�B���h�E�T�C�Y
#define WIDTH 800
#define HEIGHT 600

#define CL 128  //�e�L�X�g�̍ő啶����
#define ST 1100 //���̍ő吔

#define TITLE TEXT("Typing") //�^�C�g��
//DATA
#define ENGLISH_DATA TEXT("data/statement.dat") //�p���f�[�^�̏ꏊ
#define WORD_DATA TEXT("data/word.dat") //�P��f�[�^�̏ꏊ
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

// ���o�萔�̍ŏ��l�A�ő�l
#define MIN 1
#define MAX 20

//�����_�����[�h�Ő������镶����̒����͈̔�
#define RANDOM_MODE_MIN 4
#define RANDOM_MODE_MAX 15

#define ENEMY_MAX 10//1�X�e�[�W�̓G�̐�
#define DAMAGE_INTERVAL 3000 //�G�̍U���Ԋu
#define DEFAULT_LIFE 500 //���[�U�[�̏������C�t
#define ENEMY_LIFE(eCount, stage) (5 * (eCount + 1) + stage) //�G�̏������C�t
#define ENEMY_POWER(eCount, stage) (eCount + 1 + stage)	//�G�̍U����
#define ATTACK_TIME 150 //�U�����[�V��������
#define RECOVER 5 //�G��|�����Ƃ��̉񕜒l

#define Rect(hdc, x, y, w, h) Rectangle(hdc, x, y, x + w, y + h)
#define RoundRect(hdc, x, y, w, h) RoundRect(hdc, x - 10, y - 10, x + w + 10, y + h + 10, 30, 30)
#define Font(height, weight, italic, underline, pitchAndFamily) CreateFont(height, 0, 0, 0, weight, italic, underline, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, pitchAndFamily, NULL)
#define Stop(src) mciSendString(TEXT("stop ") src, NULL, 0, NULL)
#define Open(src) mciSendString(TEXT("open ") src, NULL, 0, NULL)
#define PlayBGM(src, hwnd) mciSendString(TEXT("play ") src TEXT(" from 0 notify"), NULL, 0, hwnd)
#define PlaySE(src) mciSendString(TEXT("play ") src TEXT(" from 0"), NULL, 0, NULL)
#define GetSystemTimeAsULARGE_INTEGER(ui) {FILETIME ft; GetSystemTimeAsFileTime(&ft); (ui)->HighPart = ft.dwHighDateTime; (ui)->LowPart = ft.dwLowDateTime;}

//�^�C�}�[��ID
enum TimerID {
	COUNT_DOWN_TIMER_ID = 1,
	PRESS_ENTER_TIMER_ID,
	DAMAGE_TIMER_ID,
	ATTACK_TIMER_ID
};

//�G���[�̎��
enum ErrorType {
	NONE,			//�G���[�Ȃ�
	INSERTION,		//�}���G���[
	SUBSTITUTION,	//�u���G���[
	DELETION		//�E���G���[
};

typedef struct {
	ErrorType error;//�G���[�̎��
	int start, end;	//������̕�����ɑΉ�����o�蕶�̃C���f�b�N�X�͈�
} Error;

//��ʂ̏��
enum ScreenState {
	START,			//�X�^�[�g���
	SELECT,			//�I�����(���j���[)
	TYPING,			//�^�C�s���O��
	STAGE_SELECT,	//�X�e�[�W�Z���N�g���
	GAMEOVER,		//�^�C�s���O�I�����
	SCORE,			//�X�R�A�\�����
	REVIEW,			//���K(���ӏ��w�E)���
	SETTING			//�ݒ���
};
ScreenState screenState = START;//���݂̉�ʏ��

//���j���[����
enum MenuItem {
	E_SENTENCE,
	E_WORD,
	RANDOM,
	STORY,
	CONFIG,
	EXIT,
	MENU_ITEM_COUNT	//���j���[���ڂ̐�
};
const TCHAR MENU_STR[][64] = { TEXT("�p���^�C�s���O"), TEXT("�p�P��^�C�s���O"), TEXT("�����_���^�C�s���O"), TEXT("�X�g�[���[���[�h"), TEXT("�ݒ�"), TEXT("�I��") };
MenuItem selectedMenuItem = E_SENTENCE;//���ݑI������Ă��郁�j���[����

//�ݒ胁�j���[����
enum ConfigMenuItem {
	TYPING_MODE,
	NUMBER_OF_QUESTION,
	ASSIST,
	BGM,
	SE,
	CONFIG_BACK,
	CONFIG_MENU_ITEM_COUNT	//�ݒ胁�j���[���ڂ̐�
};
const TCHAR CONFIG_MENU_STR[][64] = { TEXT("�^�C�s���O���[�h"), TEXT("�o�萔"), TEXT("�A�V�X�g"), TEXT("BGM"), TEXT("SE"), TEXT("�߂�") };
ConfigMenuItem selectedConfigMenuItem = TYPING_MODE;//���ݑI������Ă���ݒ胁�j���[����

//�X�g�[���[���j���[����
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
	STORY_ITEM_COUNT //�X�g�[���[���j���[���ڂ̐�
};
const TCHAR STORY_MENU_STR[][64] = { TEXT("0. �����X�e�[�W"), TEXT("1. �����X�e�[�W"), TEXT("2. �ΕǃX�e�[�W"), TEXT("3. ���A�X�e�[�W"), TEXT("4. ��ՃX�e�[�W"),
TEXT("5. �����X�e�[�W"), TEXT("6. �r�n�X�e�[�W"), TEXT("7. ���n�X�e�[�W"), TEXT("8. �X�X�e�[�W"), TEXT("9. �n��X�e�[�W"), TEXT("�߂�") };
StoryMenuItem selectedStoryItem = STAGE_0;//���ݑI������Ă���ݒ胁�j���[����

TCHAR lines[ST][CL]; //�s(��)�̏W��
TCHAR typingString[MAX][CL]; //���͒��̕�����
int selectedString[MAX]; // �I�����ꂽ������̃C���f�b�N�X
int count; //���݉���ڂ�
int typingIndex; //���͕�����̃C���f�b�N�X
BOOL dpmatching = TRUE; //DP�}�b�`���O���s�����ǂ��� (true:DP Matchin���[�h�Afalse:Normal���[�h)
int numberOfQuestion = 10; //���o�萔

int insertion, deletion, substitution; //�}���G���[�A�E���G���[�A�u���G���[
Error result[MAX][CL]; //����

ULARGE_INTEGER startTime, endTime;// �J�n���ԁA�I������ (�P��:100�i�m�b)
int countdown; //�^�C�s���O�J�n�O�̃J�E���g�_�E��

int totalInputLength; //���͑�������
int totalTextLength;//�o�蕶�̍��v������

BOOL pressEnterVisible = TRUE; // "Press Enter"��\�����邩�ǂ���
BOOL assist = TRUE; // ���̓A�V�X�g��\�����邩�ǂ���
BOOL bgm = TRUE, se = TRUE;//BGM�ASE��炷���ǂ���
BOOL correct = FALSE;//���͂����������ǂ���(Normal���[�h�̂�)

//�L�����N�^�[��`
typedef struct {
	int life, power;
}Character;
int dataLength;// ���[�h�����f�[�^�̌�
int enemyCount;// ���݁A���Ԗڂ̓G��
Character enemy; // �G
Character user = { DEFAULT_LIFE, 1 };//���[�U�[
int attack;//�G�ɗ^�����_���[�W

int gd[CL][CL], ld[CL][CL], path[CL][CL], trace[CL][CL];

//�v���g�^�C�v�錾
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

//�E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	HDC hdc;
	PAINTSTRUCT ps;
	static HBITMAP hBitmap; // �r�b�g�}�b�v
	static HDC hMemDC; // �I�t�X�N���[��
	static HIMC himc;// IME����p ���̓R���e�L�X�g

	switch (msg) {
	case WM_CREATE://�E�B���h�E�쐬��---------------------------------------------------------------------------
		//���̓R���e�L�X�g�̍쐬
		himc = ImmCreateContext();
		ImmAssociateContext(hwnd, himc);
		// DC�R���p�`�u���̍쐬
		hdc = GetDC(hwnd);
		hMemDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
		SelectObject(hMemDC, hBitmap);
		ReleaseDC(hwnd, hdc);
		//�T�E���h���[�h
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
		//�����̎�Z�b�g
		srand((unsigned)time(NULL));
		SetTimer(hwnd, PRESS_ENTER_TIMER_ID, 500, NULL);
		return 0;
	case WM_DESTROY://�E�B���h�E�I����--------------------------------------------------------------------------
		//SE, BGM�폜
		mciSendString(TEXT("close all"), NULL, 0, NULL);
		// DC�R���p�`�u���̔j��
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
		//���̓R���e�L�X�g�̔j��
		ImmDestroyContext(himc);
		PostQuitMessage(0);
		return 0;
	case WM_PAINT://�`�揈��-----------------------------------------------------------------------------
		// ���œh��Ԃ�
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
		//�I�t�X�N���[�������C���X�N���[���ɓ]��
		hdc = BeginPaint(hwnd, &ps);
		BitBlt(hdc, 0, 0, WIDTH, HEIGHT, hMemDC, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_TIMER://�^�C�}�[-------------------------------------------------------------------------
		switch (wp) {
		case COUNT_DOWN_TIMER_ID://�J�E���g�_�E���^�C�}�[
			if (--countdown <= 0) {
				KillTimer(hwnd, COUNT_DOWN_TIMER_ID);
				if (se) PlaySE(COUNTDOWN2_SE);
				if (bgm) PlayBGM(TYPING_BGM, hwnd);
				if (selectedMenuItem == STORY)//�X�g�[���[���[�h�̂Ƃ�
					SetTimer(hwnd, DAMAGE_TIMER_ID, DAMAGE_INTERVAL, NULL);
				GetSystemTimeAsULARGE_INTEGER(&startTime); //�v���J�n
			} else {
				if (se) PlaySE(COUNTDOWN1_SE);
			}
			break;
		case PRESS_ENTER_TIMER_ID://PRESS ENTER�_�Ń^�C�}�[
			pressEnterVisible = !pressEnterVisible;
			break;
		case DAMAGE_TIMER_ID://���[�U�[�ɗ^����_���[�W�^�C�}�[
			if (enemy.life <= 0) return 0;
			if (se) PlaySE(DAMAGE_SE);
			user.life -= enemy.power;
			if (user.life <= 0) {//�Q�[���I�[�o�[�̂Ƃ�
				GetSystemTimeAsULARGE_INTEGER(&endTime); //�v���I��
				KillTimer(hwnd, DAMAGE_TIMER_ID);
				KillTimer(hwnd, ATTACK_TIMER_ID);
				screenState = GAMEOVER;
				if (bgm) Stop(TYPING_BGM);
				if (se) PlaySE(GAMEOVER_SE);
			}
			break;
		case ATTACK_TIMER_ID://���[�U�[�̓G�ɑ΂���U�����[�V�����\���^�C�}�[
			KillTimer(hwnd, ATTACK_TIMER_ID);
			startTime.QuadPart += ATTACK_TIME * 10000; //���[�V�����\�����̎��Ԃ͏���
			attack = 0;
			//���̕��̑I��
			selectedString[0] = rand() % dataLength;
			typingString[0][0] = TEXT('\0');
			typingIndex = 0;
			if (enemy.life <= 0) {//�G�����ꂽ�Ƃ�
				if (++enemyCount >= ENEMY_MAX) {//�G���Ō�܂œ|���I������
					GetSystemTimeAsULARGE_INTEGER(&endTime); //�v���I��
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
	case WM_KEYDOWN://�L�[���͎�-------------------------------------------------------------------------
		if (ImmGetOpenStatus(himc))//���{�����(IME)��ON�̂Ƃ�OFF�ɂ���
			ImmSetOpenStatus(himc, false);
		switch (screenState) {
		case START://�X�^�[�g���
			if (wp != VK_RETURN)
				return 0;
			if (se) PlaySE(DECIDE_SE);
			screenState = SELECT;
			selectedMenuItem = E_SENTENCE;
			KillTimer(hwnd, PRESS_ENTER_TIMER_ID);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SELECT://�Z���N�g���
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
				case E_SENTENCE://�p���^�C�v
				case E_WORD://�p�P��^�C�v
					dataLength = readLines((selectedMenuItem == E_SENTENCE) ? ENGLISH_DATA : WORD_DATA, lines, ST);
					if (dataLength <= 0) {
						MessageBox(hwnd, TEXT("�f�[�^������܂���"), TEXT("�G���["), MB_OK);
						return 0;
					}
					if (dataLength < numberOfQuestion) {
						MessageBox(hwnd, TEXT("�f�[�^�����Ȃ����߁A�o�萔��ύX���܂�"), TEXT("�m�F"), MB_OK);
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
				case RANDOM://�����_���^�C�v
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
				case STORY://�X�g�[���[���[�h
					screenState = STAGE_SELECT;
					selectedStoryItem = STAGE_0;
					InvalidateRect(hwnd, NULL, FALSE);
					break;
				case CONFIG://�ݒ�
					screenState = SETTING;
					selectedConfigMenuItem = TYPING_MODE;
					InvalidateRect(hwnd, NULL, FALSE);
					break;
				case EXIT://�I��
					DestroyWindow(hwnd);
					break;
				}
				break;
			}
			break;
		case STAGE_SELECT://�X�e�[�W�Z���N�g���
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
				case STORY_BACK://�߂�
					screenState = SELECT;
					InvalidateRect(hwnd, NULL, FALSE);
					break;
				default://�X�e�[�W
					dataLength = readLines(WORD_DATA, lines, ST);
					if (dataLength <= 0) {
						MessageBox(hwnd, TEXT("�f�[�^������܂���"), TEXT("�G���["), MB_OK);
						return 0;
					}
					//�X�g�[���[���[�h�ł́A���͂��I���邽�тɁA���̕��������_���ɑI�ԁB0�Ԗڂ����g��Ȃ��B
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
		case TYPING://�^�C�s���O��
			if (countdown != 0)
				return 0;
			switch (wp) {
			case VK_RETURN:
				if (!dpmatching || (selectedMenuItem == STORY && attack > 0))
					return 0;
				//DP�}�b�`���O���[�h�̂Ƃ�
				if (se) PlaySE(DECIDE_SE);
				if (selectedMenuItem == STORY) {//�X�g�[���[���[�h�̂Ƃ�
					//DP�}�b�`���O���s��
					int jframe = lstrlen(lines[selectedString[0]]), iframe = lstrlen(typingString[0]);
					totalTextLength += jframe, totalInputLength += iframe;
					dp(jframe, iframe, lines[selectedString[0]], typingString[0]);
					int ins, del, sub;
					distancemap(jframe, iframe, result[0], ins, del, sub);
					insertion += ins, deletion += del, substitution += sub;
					//�^����_���[�W�A�󂯂�_���[�W�̌v�Z
					attack = (jframe - del - sub) * user.power;
					user.life -= ins + del + sub;
					enemy.life -= attack;
					if (user.life <= 0) {//�Q�[���I�[�o�[��
						GetSystemTimeAsULARGE_INTEGER(&endTime); //�v���I��
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
						//���̕��̑I��
						selectedString[0] = rand() % dataLength;
						typingString[0][0] = TEXT('\0');
						typingIndex = 0;
					}
				} else {//�X�g�[���[���[�h�łȂ��Ƃ�
					typingIndex = 0;
					if (++count >= numberOfQuestion) {//�o�肵�I������
						GetSystemTimeAsULARGE_INTEGER(&endTime); //�v���I��
						screenState = GAMEOVER;
						//DP�}�b�`���O
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
		case GAMEOVER://�^�C�s���O�I�����
			if (wp != VK_RETURN)
				return 0;
			screenState = SCORE;
			if (se) PlaySE(DECIDE_SE);
			if (bgm) PlayBGM(RESULT_BGM, hwnd);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case SCORE://�X�R�A���
			if (wp != VK_RETURN)
				return 0;
			if (se) PlaySE(DECIDE_SE);
			if (!dpmatching || selectedMenuItem == STORY) {//DP�}�b�`���O�łȂ����A�X�g�[���[���[�h�̂Ƃ�
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
			} else {//�X�g�[���[���[�h�łȂ��ADP�}�b�`���O���[�h�̂Ƃ�
				count = 0;
				screenState = REVIEW;
			}
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case REVIEW://���r���[(�G���[�ӏ��w�E)���
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
		case SETTING://�ݒ���
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
	case MM_MCINOTIFY://BGM�Ɋւ���ʒm--------------------------------------------------------------------
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
	case WM_CHAR://�������͂��ꂽ��-----------------------------------------------------------------------
		if (countdown != 0 || screenState != TYPING || wp == VK_RETURN || wp == VK_TAB || (selectedMenuItem == STORY && attack > 0))
			return 0;
		if (!dpmatching) {//DP�}�b�`���O���[�h�łȂ��Ƃ�
			totalInputLength++;//���͑�������
			correct = ((TCHAR)wp == lines[selectedString[count]][typingIndex]);//���͂����������ǂ���
			if (correct) {//���͂���������
				if (se) PlaySE(KEY_SE);
				totalTextLength++;//���������͕�����
				typingString[count][typingIndex++] = (TCHAR)wp;
				typingString[count][typingIndex] = TEXT('\0');
				if (lstrlen(lines[selectedString[count]]) == typingIndex) {//�ł��I������
					if (selectedMenuItem == STORY) {//�X�g�[���[���[�h�̂Ƃ�
						attack = typingIndex * user.power;
						enemy.life -= attack;
						if (enemy.life <= 0) {
							enemy.life = 0;
							user.life += RECOVER;
							if (se) PlaySE(RECOVER_SE);
						}
						if (se) PlaySE(ATTACK_SE);
						SetTimer(hwnd, ATTACK_TIMER_ID, ATTACK_TIME, NULL);
					} else {//�X�g�[���[���[�h�łȂ��Ƃ�
						typingIndex = 0;
						if (++count >= numberOfQuestion) {//�o�肵�I������
							GetSystemTimeAsULARGE_INTEGER(&endTime); //�v���I��
							screenState = GAMEOVER;
							if (bgm) Stop(TYPING_BGM);
							if (se) PlaySE(CLEAR_SE);
						}
					}
				}
			} else {//���͂��Ԉ���Ă�����
				if (se) PlaySE(NG_SE);
				if (selectedMenuItem == STORY && --user.life <= 0) {//�X�g�[���[���[�h�̂Ƃ��A���[�U�[�Ƀ_���[�W��^���A0�ȉ��ɂȂ�����
					GetSystemTimeAsULARGE_INTEGER(&endTime); //�v���I��
					KillTimer(hwnd, DAMAGE_TIMER_ID);
					screenState = GAMEOVER;
					if (bgm) Stop(TYPING_BGM);
					if (se) PlaySE(GAMEOVER_SE);
				}
			}
		} else { //DP�}�b�`���O���[�h�̎�
			if (se) PlaySE(KEY_SE);
			if (wp == VK_BACK) {//1�����폜
				if (--typingIndex < 0)
					typingIndex = 0;
				typingString[count][typingIndex] = TEXT('\0');
			} else {//���ʂ̓���
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

//���C���֐�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {

	//�E�B���h�E�N���X�̍쐬
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

	//�E�B���h�E�N���X�̓o�^
	if (!RegisterClass(&winc)) return -1;

	//�E�B���h�E�̐���
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
* �X�^�[�g��ʂ�`�悷��
* @param hdc
*/
void drawStart(HDC hdc) {
	drawBMP(hdc, START_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	//�^�C�g���`��
	SetTextColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, Font(100, FW_BOLD, TRUE, TRUE, VARIABLE_PITCH | FF_ROMAN));
	TextOut(hdc, WIDTH / 2, HEIGHT / 3, TITLE, lstrlen(TITLE));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//PRESS ENTER�`��
	if (pressEnterVisible) {
		const TCHAR *PRESS_ENTER_STR = TEXT("Press Enter");
		SetTextColor(hdc, RGB(255, 255, 0));
		SelectObject(hdc, Font(30, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_ROMAN));
		TextOut(hdc, WIDTH / 2, HEIGHT * 2 / 3, PRESS_ENTER_STR, lstrlen(PRESS_ENTER_STR));
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	}
}

/**
* �I��(���j���[)��ʂ�`�悷��
* @param hdc
*/
void drawSelect(HDC hdc) {
	drawBMP(hdc, SELECT_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	//���j���[���ڕ`��
	SelectObject(hdc, Font(50, FW_BOLD, TRUE, FALSE, VARIABLE_PITCH | FF_ROMAN));
	for (int i = 0; i < MENU_ITEM_COUNT; ++i) {
		SetTextColor(hdc, (i == selectedMenuItem) ? RGB(255, 0, 0) : RGB(0, 0, 0));
		TextOut(hdc, 100, 50 + i * 80, MENU_STR[i], lstrlen(MENU_STR[i]));
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}
/**
* �X�e�[�W�I����ʂ�`�悷��
* @param hdc
*/
void drawStageSelect(HDC hdc) {
	drawBMP(hdc, SELECT_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	//�X�e�[�W�I�����ڂ�`��
	SelectObject(hdc, Font(35, FW_BOLD, TRUE, FALSE, FIXED_PITCH | FF_ROMAN));
	for (int i = 0; i < STORY_ITEM_COUNT; ++i) {
		SetTextColor(hdc, (i == selectedStoryItem) ? RGB(255, 0, 0) : RGB(0, 0, 0));
		TextOut(hdc, 100, 40 + i * 45, STORY_MENU_STR[i], lstrlen(STORY_MENU_STR[i]));
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}

/**
* �L�[�{�[�h�A�V�X�g��`�悷��
* @param hdc
*/
void drawAssist(HDC hdc) {
	drawBMP(hdc, KEYBORD_IMG, 50, 320);
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	SelectObject(hdc, CreatePen(PS_SOLID, 3, RGB(255, 0, 0)));
	//Enter���͂�
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
	const int KEY_SIZE = 46; //�L�[�̏c����щ��̒���
	TCHAR ch = lines[selectedString[count]][typingIndex];
	//�X�y�[�X�̎�
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
			if (i <= 4)//�V�t�g�L�[
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
			if (i <= 4)//�V�t�g�L�[
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
			if (i <= 4)//�V�t�g�L�[
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
			if (i <= 4)//�V�t�g�L�[
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
* �^�C�s���O�I����ʂ�`�悷��
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
		const TCHAR *COMPLETE_STR = TEXT("�I��");
		TextOut(hdc, WIDTH / 2, HEIGHT / 2, COMPLETE_STR, lstrlen(COMPLETE_STR));
	}
}

/**
* �w�蕶�����Ő܂�Ԃ��������`�悷��
* @param hdc
* @param x �`��n�_�̍���x���W
* @param y �`��n�_�̍���y���W
* @param text �`�悷�镶����
* @param n �܂�Ԃ�������
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
* �^�C�s���O���̉�ʂ�`�悷��
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

	//�J�E���g�_�E��
	if (countdown != 0) {
		SelectObject(hdc, Font(200, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_SCRIPT));
		SetTextColor(hdc, RGB(0, 0, 0));
		wsprintf(tempString, TEXT("%d"), countdown);
		SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
		TextOut(hdc, WIDTH / 2, HEIGHT / 2, tempString, lstrlen(tempString));
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
		return;
	}
	//����ځA���̖� �`��
	SetTextAlign(hdc, TA_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, Font(40, FW_BOLD, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));
	if (selectedMenuItem == STORY)
		wsprintf(tempString, TEXT("%2d�̖�"), enemyCount + 1);
	else
		wsprintf(tempString, TEXT("%2d���"), count + 1);
	TextOut(hdc, 20, 20, tempString, lstrlen(tempString));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	if (selectedMenuItem == STORY) {//�X�g�[���[���[�h
		//���C�t�`��
		SelectObject(hdc, CreateSolidBrush(RGB(200, 200, 0)));
		SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(200, 200, 0)));
		Rect(hdc, 200, 25, user.life, 30);
		Rect(hdc, WIDTH / 2 - enemy.life / 2, HEIGHT / 2 - 50, enemy.life, 15);
		DeleteObject(SelectObject(hdc, GetStockObject(WHITE_BRUSH)));
		DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
		//���C�t�A�p���[�̕�����`��
		SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, FALSE, FIXED_PITCH | FF_SWISS));
		SetTextColor(hdc, RGB(0, 0, 0));
		SetTextAlign(hdc, TA_LEFT);
		wsprintf(tempString, TEXT(" Life:%4d "), user.life);
		TextOut(hdc, 200, 30, tempString, lstrlen(tempString));
		SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
		wsprintf(tempString, TEXT(" Life:%3d  Power:%3d "), enemy.life, enemy.power);
		TextOut(hdc, WIDTH / 2, HEIGHT / 2 - 55, tempString, lstrlen(tempString));
		//�����X�^�[�`��
		wsprintf(tempString, CHARACTER_IMG, selectedStoryItem, enemyCount);
		int h = drawBMP(hdc, tempString, WIDTH / 2, HEIGHT / 2 - 30, RGB(255, 255, 255));
		//�U�����[�V�����`��
		if (attack > 0) {
			drawBMP(hdc, ATTACK_IMG, WIDTH / 2, HEIGHT / 2 - 30 + h / 2 - 60, RGB(0, 0, 0));
			SetTextColor(hdc, RGB(255, 0, 0));
			SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
			wsprintf(tempString, TEXT("%d"), -attack);
			TextOut(hdc, WIDTH / 2, HEIGHT / 2 - 35, tempString, lstrlen(tempString));
			if (enemy.life <= 0) {// �񕜒l��`��
				SetTextColor(hdc, RGB(0, 0, 255));
				SetTextAlign(hdc, TA_LEFT);
				wsprintf(tempString, TEXT("           +%d"), RECOVER);
				TextOut(hdc, 200, 30, tempString, lstrlen(tempString));
			}
		}
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	} else if (assist) {//�A�V�X�g�`��
		drawAssist(hdc);
	}

	if (!correct) {//�^�C�v�~�X�̂Ƃ�
		SelectObject(hdc, Font(30, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_ROMAN));
		SetTextAlign(hdc, TA_RIGHT);
		SetTextColor(hdc, RGB(255, 0, 0));
		const TCHAR *MISS_STR = TEXT("MISS!!");
		TextOut(hdc, WIDTH - 50, 25, MISS_STR, lstrlen(MISS_STR));
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	}
	//�o�蕶�Ɠ��͕��̕`��
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
	if (rect.top - rect.bottom <= size.cy * 2)// ��蕶����s�̂Ƃ��́A��ʉE�[�Ő܂�Ԃ�(�����łȂ��Ƃ��́A�������Ő܂�Ԃ�)
		rect.right = WIDTH - MARGIN, width = rect.right - rect.left, n = width / size.cx;
	drawString(hdc, rect.left, rect.top, typingString[count], n);
	rect.left += (typingIndex % n) * size.cx, rect.top += (typingIndex / n) * size.cy;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//�L�����b�g�`��
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	MoveToEx(hdc, rect.left, rect.top, NULL);
	LineTo(hdc, rect.left, rect.top + size.cy);
}

/**
* �X�R�A��ʂ�`�悷��
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
	DrawText(hdc, TEXT("����"), -1, &rect, DT_LEFT);
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//���͎���
	double time = (endTime.QuadPart - startTime.QuadPart) / 10000000.0;

	rect.left = 50, rect.top = 100;
	SelectObject(hdc, Font(25, FW_BOLD, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));

	SetTextColor(hdc, RGB(0, 0, 0));
	wsprintf(tempString, TEXT("�o�蕶������ %10d [����]\n\n���͕������� %10d [����]\n"), totalTextLength, totalInputLength);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;

	SetTextColor(hdc, RGB(255, 0, 0));
	if (dpmatching)
		wsprintf(tempString, TEXT("�E����葍�� %10d [��]\n\n�}����葍�� %10d [��]\n\n�u����葍�� %10d [��]\n"), deletion, insertion, substitution);
	else
		wsprintf(tempString, TEXT("���̓~�X���� %10d [��]\n"), totalInputLength - totalTextLength);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;

	SetTextColor(hdc, RGB(0, 0, 255));
	if (totalInputLength == 0)//���͂�0�����̂Ƃ�
		swprintf(tempString, 512, TEXT("���v���͎��ԁ@�@�@�@  %10.3lf [s]\n\n1����������̓��͎���         �� [s]\n\n1�b������̓��͕����� %10.3lf [����]\n"), time, totalInputLength / time);
	else
		swprintf(tempString, 512, TEXT("���v���͎��ԁ@�@�@�@  %10.3lf [s]\n\n1����������̓��͎��� %10.3lf [s]\n\n1�b������̓��͕����� %10.3lf [����]\n"), time, time / totalInputLength, totalInputLength / time);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;

	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	SelectObject(hdc, Font(45, FW_HEAVY, FALSE, TRUE, FIXED_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(0, 255, 0));
	swprintf(tempString, 512, TEXT("�X�R�A %7.3lf [%%]"), (dpmatching ? (1 - (double)(deletion + insertion + substitution) / totalTextLength) : (totalTextLength / (double)totalInputLength)) * 100);
	DrawText(hdc, tempString, -1, &rect, DT_RIGHT);
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
}

/**
* ���r���[��ʂ�`�悷��
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
	wsprintf(tempString, TEXT("���K %2d���"), count + 1);
	DrawText(hdc, tempString, -1, &rect, DT_LEFT);
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	rect.left = 50, rect.top = 100, rect.right = WIDTH - rect.left;
	SelectObject(hdc, Font(30, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	DrawText(hdc, TEXT("�o�蕶"), -1, &rect, DT_LEFT);
	DrawText(hdc, TEXT("�o�蕶\n"), -1, &rect, DT_LEFT | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//�o�蕶�`��
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	SIZE size;
	GetTextExtentPoint32(hdc, TEXT(" "), 1, &size);
	int n = (rect.right - rect.left) / size.cx, length = lstrlen(lines[selectedString[count]]);
	drawString(hdc, rect.left, rect.top, lines[selectedString[count]], n);
	rect.top += (1 + length / n + ((length % n != 0) ? 1 : 0)) * size.cy;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	SelectObject(hdc, Font(30, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	DrawText(hdc, TEXT("���͕�"), -1, &rect, DT_LEFT);
	DrawText(hdc, TEXT("���͕�\n"), -1, &rect, DT_LEFT | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//���͕��`��
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	drawString(hdc, rect.left, rect.top, typingString[count], n);
	length = lstrlen(typingString[count]);
	rect.top += (1 + length / n + ((length % n != 0) ? 1 : 0)) * size.cy;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	SelectObject(hdc, Font(30, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	SetTextColor(hdc, RGB(255, 0, 0));
	DrawText(hdc, TEXT("�@�@�@�@�@�@�@�@ ��"), -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 255, 0));
	DrawText(hdc, TEXT("�@�@�@�@�@�@�@�@�@�@�@�@ ��"), -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 0, 255));
	DrawText(hdc, TEXT("�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@ ��"), -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));
	DrawText(hdc, TEXT("�G���[�ӏ� (�E��:�@ �}��:�@ �u��:�@)"), -1, &rect, DT_LEFT);
	DrawText(hdc, TEXT("�G���[�ӏ� (�E��:�@ �}��:�@ �u��:�@)\n"), -1, &rect, DT_LEFT | DT_CALCRECT);
	rect.top = rect.bottom, rect.bottom = HEIGHT, rect.right = WIDTH - rect.left;
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//�G���[�ӏ��`��
	SelectObject(hdc, Font(20, FW_SEMIBOLD, FALSE, TRUE, FIXED_PITCH | FF_SWISS));
	int x = rect.left, y = rect.top;
	// �擪�̒E���G���[
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
		// ���͕����̏o��
		TextOut(hdc, x, y, &typingString[count][i], 1);
		x += size.cx;
		if (x + size.cx >= rect.right)
			x = rect.left, y += 2 * size.cy;
		// �E���G���[
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
* �ݒ��ʂ�`�悷��
* @param hdc
*/
void drawSetting(HDC hdc) {
	drawBMP(hdc, SETTING_IMG, 0, 0);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);

	SelectObject(hdc, Font(60, FW_BOLD, FALSE, TRUE, VARIABLE_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(255, 200, 10));
	const TCHAR *SETTING_STR = TEXT("�ݒ�");
	TextOut(hdc, 30, 30, SETTING_STR, lstrlen(SETTING_STR));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

	SelectObject(hdc, Font(20, FW_BOLD, FALSE, FALSE, VARIABLE_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(0, 0, 0));
	const TCHAR *EXPLAIN_STR = TEXT("���o�萔�ƃA�V�X�g�̐ݒ�́A�X�g�[���[���[�h�ȊO�ŗL���ł�");
	TextOut(hdc, 180, 50, EXPLAIN_STR, lstrlen(EXPLAIN_STR));
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//�ݒ荀�ڂ̕`��
	const int INTERVAL = 70, TOP = 120;
	SelectObject(hdc, Font(40, FW_BOLD, TRUE, FALSE, VARIABLE_PITCH | FF_ROMAN));
	for (int i = 0; i < CONFIG_MENU_ITEM_COUNT; ++i) {
		SetTextColor(hdc, (selectedConfigMenuItem == i) ? RGB(255, 0, 0) : RGB(0, 0, 0));
		TextOut(hdc, 50, TOP + INTERVAL*i, CONFIG_MENU_STR[i], lstrlen(CONFIG_MENU_STR[i]));
	}
	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	//�^�C�s���O���[�h�̕`��
	SelectObject(hdc, Font(30, FW_MEDIUM, FALSE, FALSE, FIXED_PITCH | FF_ROMAN));
	SetTextColor(hdc, RGB(0, 0, 0));
	SIZE size1, size2;
	const TCHAR *DP_STR = TEXT("DP Matching"), *NORMAL_STR = TEXT("Normal");
	TextOut(hdc, WIDTH / 2, TOP, DP_STR, lstrlen(DP_STR));
	GetTextExtentPoint32(hdc, DP_STR, lstrlen(DP_STR), &size1);
	TextOut(hdc, WIDTH / 2 + size1.cx + 50, TOP, NORMAL_STR, lstrlen(NORMAL_STR));
	GetTextExtentPoint32(hdc, NORMAL_STR, lstrlen(NORMAL_STR), &size2);
	//���݂̃^�C�s���O���[�h���͂�
	SelectObject(hdc, CreatePen(PS_SOLID, 3, RGB(0, 0, 255)));
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	if (dpmatching)
		RoundRect(hdc, WIDTH / 2, TOP, size1.cx, size1.cy);
	else
		RoundRect(hdc, WIDTH / 2 + size1.cx + 50, TOP, size2.cx, size2.cy);
	//�o�萔��`��
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
	//ON�AOFF�̕`��A�I�����ꂽ���̂��͂�
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
* BMP�̕`��
* @param hdc
* @param path BMP�t�@�C���̃p�X
* @param x �`�悷�鍶��x���W
* @param y �`�悷�鍶��y���W
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
* BMP�̕`��(���ߐF�w�肠��)
* @param hdc
* @param path BMP�t�@�C���̃p�X
* @param x �`�悷�钆�Sx���W
* @param y �`�悷���y���W
* @param rgb ���ߐF
* @param �摜�̍���
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
* �t�@�C�����當�����ǂݎ��
* @param filename �t�@�C����
* @param lines ������̔z��
* @param length �ő�ǂݎ��s��
* @return ���ۂɓǂݎ�����s��
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
		if (j == CL) { //�ő啶�����𒴂����Ƃ�
			lines[i][CL - 1] = TEXT('\0');
			while (TRUE) {//���̍s��ǂݔ�΂�
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
* �l�̓���ւ�
* @param a int�^
* @param b int�^
*/
void swap(int &a, int &b) {
	int t = a;
	a = b, b = t;
}

/**
* �d���̂Ȃ������_���Ȑ�����擾����(0����length�����̐�������selectLength�I�΂��)
* @param num ����ꂽ����
* @param length �������鐔���̍ő�l+1
* @param selectLength ����������̒���
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
* DP�}�b�`���O
* @param jframe �o�蕶�̕�����
* @param iframe ���͕��̕�����
* @param reftext �o�蕶
* @param inputtext ���͕�
* @return ���͕��Əo�蕶�̋���
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
* �o�b�N�g���[�X����
* @param jframe �o�蕶�̕�����
* @param iframe ���͕��̕�����
* @param result �G���[����Ԃ�
* @param insetion �}���G���[����Ԃ�
* @param deletion �E���G���[����Ԃ�
* @param substitution �u���G���[����Ԃ�
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
	for (i = iframe; i >= 1; --i) {//�擪�̑}���G���[
		if (trace[i][0] == 1) {
			trace[i - 1][0] = 1;
			result[i - 1].error = INSERTION;
			insertion++;
		}
	}
	for (j = jframe; j >= 1; --j) {//�擪�̒E���G���[��iframe�ɋL�^
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
* DP�}�b�`���O�̏���������
* @param jframe �o�蕶�̕�����
* @param iframe ���͕��̕�����
*/
void initialization(int jframe, int iframe) {
	for (int i = 1; i < iframe + 1; i++)
		gd[i][0] = i;
	for (int j = 1; j < jframe + 1; j++)
		gd[0][j] = j;
	gd[0][0] = 0;
}

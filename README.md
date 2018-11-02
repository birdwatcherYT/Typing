# Typing
2017年1月16日

学部時代に作った汚いソースコード

## 各ファイルについて

- Typing
    - data
        - statement.dat : 英文1000文のテキストデータ
        - word.dat      : 英単語のテキストデータ
    - image : 画像データフォルダ
    - music : 音楽データフォルダ
    - Source.cpp  : ソースファイル
    - report.docx : レポート
    - Typing (Compiled by Borland C++).exe   : タイピングソフト(Source.cppをBorland C++でコンパイルした)
    - Typing (Compiled by Cygwin g++).exe    : タイピングソフト(Source.cppをCygwin g++でコンパイルした)
    - Typing (Compiled by Visual Studio).exe : タイピングソフト(Source.cppをVisual Studioでコンパイルした)
    - icon.ico : アイコン画像
    - icon.rc  : アイコンのためのリソースファイル
    - README.txt : このファイル

--------------------------------------------------
## Source.cpp, icon.rcのコンパイル方法

- g++でのコンパイル
    - cygwin1.dll依存
        - windres icon.rc -O coff -o icon.res
        - g++ -mwindows -o Typing Source.cpp -lwinmm -lmsimg32 -limm32 icon.res
    - cygwin1.dll非依存(64bit)
        - x86_64-w64-mingw32-windres icon.rc -O coff -o icon.res
        - x86_64-w64-mingw32-g++ -mwindows -o Typing Source.cpp -lwinmm -lmsimg32 -limm32 icon.res
    - cygwin1.dll非依存(32bit)
        - i686-w64-mingw32-windres icon.rc -O coff -o icon.res
        - i686-w64-mingw32-g++ -mwindows -o Typing Source.cpp -lwinmm -lmsimg32 -limm32 icon.res

- Borland C++でのコンパイル
    - bcc32 -eTyping -oTyping -W Source.cpp
    - brc32 icon.rc Typing.exe

- Visual Studioでのコンパイル
    - プロジェクトを作成して、Source.cppとicon.rcを追加し、そのままコンパイルすればよい。
    - ランタイムライブラリ非依存にするには、プロジェクトのプロパティを開き、
        C/C++→コード生成→ランタイムライブラリを「マルチスレッド (/MT)」または「マルチスレッド デバッグ (/MTd)」に変更する。
--------------------------------------------------
## プログラムについて
音が出るので、音量に注意してください。

以下のファイルのうち、起動できるものを利用してください。
- Typing (Compiled by Borland C++).exe
- Typing (Compiled by Cygwin g++).exe
- Typing (Compiled by Visual Studio).exe

### 操作方法
全てキーボードで操作します。

- Enterキー: 
メニュー項目の決定、次の出題文への移行(DP Matchingモード)、結果画面の移行をします。

- ↑キー、↓キー: 
メニュー画面、設定画面、ステージセレクト画面の選択項目を上下します。

- ←キー、→キー: 
設定画面において、設定を変更します。

- ESCAPEキー: 
タイピングの終了、設定の終了、メニューの終了をします。

- BACKSPACEキー: 
タイピング中の1文字削除、設定の終了、メニューの終了をします。

### メニュー項目
- 英文タイピング: 
英文のタイピングをします。

- 英単語タイピング: 
英単語のタイピングをします。

- ランダムタイピング: 
ランダムな文字列のタイピングをします。

- ストーリーモード: 
0から9のステージがあり、数字が大きいほど難易度が上がります。
ユーザーのライフが無くなる前に、モンスターを10体倒すことができれば、ステージクリアです。
モンスターの攻撃力に従い、ある時間間隔でユーザーのライフは減っていきます。
また、敵を倒すとライフを少し回復します。
DPマッチングモードでは、Enterを押して入力を決定すると、
入力のうち正しかった文字数のダメージをモンスターに与え、エラー数分のダメージをユーザーが受けることになります。
Normalモードでは、出題文と同じ文字列を打ち終えると、その文字数だけモンスターにダメージを与えます。
入力中にミスをすると、その瞬間にユーザーはダメージを受けることになります。
なお、ストーリーモードでは、タイピング終了後にエラー箇所の指摘を行いません。

- 設定: 
設定を変更します。

- 終了: 
プログラムを終了します。

### 設定項目
- タイピングモード
    - DP Matching : DPマッチングによるモードです。自由な入力ができ、Enterキーで次の出題文へ移行します。
    - Normal      : 出題文と同じ文字以外入力できません。出題文と同じ文字列を打ち終えると、自動的に次の出題文へ移行します。

- 出題数: 
1から20まで変更可能です。ストーリーモード以外で有効です。

- アシスト: 
キーボードの入力アシストを表示するかどうかを設定できます。ストーリーモード以外で有効です。

- BGM: 
BGMを流すかどうかを設定できます。

- SE: 
効果音を鳴らすかどうかを設定できます。

--------------------------------------------------
## 素材について

以下のサイト様から、使用させていただきました。
画像は、ファイル形式の変換、トリミング、明るさ変更など加工しています。
音楽は、音量を変更しています。

- 背景画像
    - 無料の壁紙,無料の写真,画像を自由に使用する,無料壁紙素材 - FIXABAY: 
http://www.fixabay.com/%E5%A4%96%E5%81%B4%E3%81%AE%E7%94%BB%E5%83%8F%E3%83%96%E3%83%AB%E3%83%BC%E5%A3%81%E7%B4%99%E5%AE%87%E5%AE%99%E3%81%AE%E5%86%99%E7%9C%9F%E6%83%91%E6%98%9F%E3%81%AE%E8%83%8C%E6%99%AF/
    - 商用、加工OK!! 無料で使える写真素材サイト - まちふぉと: 
http://machi-photo.net/detail/photoId/842/
    - フリー素材や写真素材のサイト - AFFEXCEED: 
http://affexceed.art.coocan.jp/water/unit_01/
    - フリー画像、写真素材集 - GATAG: 
http://free-photos.gatag.net/2014/12/15/190000.html
    - 高品質、高解像度の写真素材を無料で配布しているストックフォトサービス - PAKUTASO: 
https://www.pakutaso.com/20150802230post-5911.html
    - ぴぽや倉庫: 
http://piposozai.wiki.fc2.com/wiki/%E8%83%8C%E6%99%AF%E7%94%BB%E5%83%8F

- キーボード画像
    - フリーフォントでキーボード画像作成: 
http://lightbox.on.coocan.jp/html/fontImage_kb.php?w=600&font=hc-poo5&font2=hc-poo5&textColor=000000&rc=FF0000&url1=&url1x=0&url1y=0&url1t=100&check=1

- モンスター画像、アイコン画像
    - アンデットとか好きだから - Enemies: 
http://lud.sakura.ne.jp/battler.php?page=ss

- 攻撃エフェクト画像
    - ぴぽや倉庫: 
http://piposozai.wiki.fc2.com/wiki/%E3%82%A8%E3%83%95%E3%82%A7%E3%82%AF%E3%83%88%E3%82%A2%E3%83%8B%E3%83%A1%28%E6%88%A6%E9%97%98%E7%94%A81%29

- 英文
    - Breaking News English: 
http://www.breakingnewsenglish.com/easy-news-english.html

- 英単語
    - Wiktionary:英語の基本語彙1000: 
https://ja.wiktionary.org/wiki/Wiktionary:%E8%8B%B1%E8%AA%9E%E3%81%AE%E5%9F%BA%E6%9C%AC%E8%AA%9E%E5%BD%991000

- 音楽
    - フリー音楽素材 - 魔王魂
        - パソコン03: 
    http://maoudamashii.jokersounds.com/list/se14.html
        - システム18: 
    http://maoudamashii.jokersounds.com/core.cgi?page=2&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e3%82%b7%e3%82%b9%e3%83%86%e3%83%a0%e9%9f%b3
        - ワンポイント09: 
    http://maoudamashii.jokersounds.com/core.cgi?page=2&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e3%83%af%e3%83%b3%e3%83%9d%e3%82%a4%e3%83%b3%e3%83%88
        - 点火01: 
    http://maoudamashii.jokersounds.com/core.cgi?page=1&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e7%89%a9%e9%9f%b3
        - システム37: 
    http://maoudamashii.jokersounds.com/core.cgi?page=1&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e3%82%b7%e3%82%b9%e3%83%86%e3%83%a0%e9%9f%b3
        - ファンタジー03: 
    http://maoudamashii.jokersounds.com/list/bgm10.html
        - テーマ14: 
    http://maoudamashii.jokersounds.com/list/game5.html
        - アコースティック45: 
    http://maoudamashii.jokersounds.com/list/bgm3.html
        - ピアノ1-1ド: 
    http://maoudamashii.jokersounds.com/core.cgi?page=1&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e6%a5%bd%e5%99%a8
        - ピアノ1-8ド: 
    http://maoudamashii.jokersounds.com/core.cgi?page=0&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e6%a5%bd%e5%99%a8
        - 戦闘17: 
    http://maoudamashii.jokersounds.com/list/se7.html
        - 戦闘18: 
    http://maoudamashii.jokersounds.com/list/se7.html
        - ジングル05: 
    http://maoudamashii.jokersounds.com/list/se3.html
        - ワンポイント06: 
    http://maoudamashii.jokersounds.com/core.cgi?page=2&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e3%83%af%e3%83%b3%e3%83%9d%e3%82%a4%e3%83%b3%e3%83%88
        - システム20: 
    http://maoudamashii.jokersounds.com/core.cgi?page=2&field=%e5%8a%b9%e6%9e%9c%e9%9f%b3%e7%b4%a0%e6%9d%90%3c%3e%e3%82%b7%e3%82%b9%e3%83%86%e3%83%a0%e9%9f%b3
        - ワンポイント23: 
    http://maoudamashii.jokersounds.com/list/se2.html

## 参考文献について

プログラムを作成する上で、以下のサイトを参考にしました。

- プログラミング入門サイト～bituse～ WINAPI入門: 
http://bituse.info/winapi/
- 前田稔(Maeda Minoru)の超初心者のプログラム入門 Windows Program 入門: 
http://www.eonet.ne.jp/~maeda/winc/winc.htm#%E5%85%A5%E9%96%80
- WisdomSoft(旧) 標準 Windows API: 
http://wisdom.sakura.ne.jp/system/winapi/win32/index.html
- プログラミングのメモ帳 ダブル・バッファリングの方法(1): 
http://blog.goo.ne.jp/masaki_goo_2006/e/d3c18365234ffb3383f5c30e32c83cf5
- 時刻を取得する - GetSystemTime, GetLocalTime, GetSystemTimeAsFileTime, GetSystemTimes関数: 
http://d.hatena.ne.jp/prog_pc/20090522/1242977667
- Input Method Editor(IME)概要: 
http://www.geocities.co.jp/NatureLand/2023/reference/Win32API/ime01.html

#include <Novice.h>
#include <cmath>
#include <cstdlib>

const char kWindowTitle[] = "TD1BossBattle";
const float M_PI = 3.14159265358979323846f;

void DrawLightningLine(int startX, int startY, int endX, int endY, unsigned int color) {
    const int segments = 400;  // 分割するセグメント数
    const int maxOffset = 40;  // セグメントごとの最大オフセット値

    int prevX = startX;
    int prevY = startY;

    for (int i = 1; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        int currentX = static_cast<int>(startX + (endX - startX) * t);
        int currentY = static_cast<int>(startY + (endY - startY) * t);

        currentX += (rand() % (2 * maxOffset)) - maxOffset;
        currentY += (rand() % (2 * maxOffset)) - maxOffset;

        Novice::DrawLine(prevX, prevY, currentX, currentY, color);

        prevX = currentX;
        prevY = currentY;
    }
}

void DrawBeams(int* startLineX, int* startLineY, int* goalLineX, int* goalLineY, unsigned int color) {
    for (int i = 0; i < 7; ++i) {
        DrawLightningLine(startLineX[i], startLineY[i], goalLineX[i], goalLineY[i], color);
    }
}

// 斬撃を描画する関数
void DrawSlash(int startX, int startY, int targetX, int targetY, unsigned int color, float length) {
    const float width = 130.0f;   // 斬撃の幅を30に変更

    // 斬撃の角度を計算
    float angle = static_cast<float>(atan2(targetY - startY, targetX - startX));

    // 斬撃の発生位置を自機の中心から100ピクセル離れた位置に計算
    float slashX = startX + cosf(angle) * 100.0f; // 自機から100ピクセルの位置
    float slashY = startY + sinf(angle) * 100.0f; // 自機から100ピクセルの位置

    // 四角形の左上、右上、左下、右下の座標を計算
    float halfWidth = width / 2.0f; // 幅の半分
    float leftTopX = slashX - halfWidth * cosf(angle + M_PI / 2.0f);
    float leftTopY = slashY - halfWidth * sinf(angle + M_PI / 2.0f);
    float rightTopX = slashX + halfWidth * cosf(angle + M_PI / 2.0f);
    float rightTopY = slashY + halfWidth * sinf(angle + M_PI / 2.0f);
    float leftBottomX = leftTopX + length * cosf(angle);
    float leftBottomY = leftTopY + length * sinf(angle);
    float rightBottomX = rightTopX + length * cosf(angle);
    float rightBottomY = rightTopY + length * sinf(angle);

    // ボックスを描画
    Novice::DrawQuad(int(leftTopX), int(leftTopY), int(rightTopX), int(rightTopY), int(leftBottomX), int(leftBottomY), int(rightBottomX), int(rightBottomY), 0, 0, 1, 1, 0, color);
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    // ライブラリの初期化
    Novice::Initialize(kWindowTitle, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), true); // フルスクリーンを有効にする
    // フルスクリーンにする
    HWND hwnd = GetForegroundWindow(); // 現在のウィンドウのハンドルを取得
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW); // ウィンドウのスタイルを変更
    SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW); // ウィンドウ位置とサイズをフルスクリーンに設定
    ShowWindow(hwnd, SW_MAXIMIZE); // ウィンドウを最大化

    // キー入力結果を受け取る箱
    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    bool xBeamFlag = false;
    bool xBeamFlag2 = false;
    bool yBeamFlag = false;

    int startLineX[7] = { 50, 50, 1490, 1490, 200, 750, 1350};
    int startLineY[7] = { 250, 650, 250, 650, 50, 50, 50 };
    int goalLineX[7] = { 50, 50, 1490, 1490, 200, 750, 1350 };
    int goalLineY[7] = { 250, 650, 250, 650, 50, 50, 50 };

    const int initialStartLineX[7] = { 50, 50, 1490, 1490, 200, 750, 1350 };
    const int initialStartLineY[7] = { 250, 650, 250, 650, 50, 50, 50 };
    const int initialGoalLineX[7] = { 50, 50, 1490, 1490, 200, 750, 1350 };
    const int initialGoalLineY[7] = { 250, 650, 250, 650, 50, 50, 50 };

    int posX = 500;
    int posY = 600;
    const int sizeX = 100;
    const int sizeY = 120;
    const int speed = 10;

    // ウィンドウの×ボタンが押されるまでループ
    while (Novice::ProcessMessage() == 0) {
        // フレームの開始
        Novice::BeginFrame();

        // キー入力を受け取る
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ///
        /// 更新処理ここから
        /// 

         // マウスの位置を取得
        int mouseX, mouseY;
        Novice::GetMousePosition(&mouseX, &mouseY);

        if (keys[DIK_W]) posY -= speed;
        if (keys[DIK_S]) posY += speed;
        if (keys[DIK_A]) posX -= speed;
        if (keys[DIK_D]) posX += speed;

        // ビームフラグの管理
        if (keys[DIK_1] && !preKeys[DIK_1]) xBeamFlag = true;
        if (keys[DIK_2] && !preKeys[DIK_2]) xBeamFlag2 = true;
        if (keys[DIK_3] && !preKeys[DIK_3]) yBeamFlag = true;
        if (keys[DIK_4] && !preKeys[DIK_4]) { 
            xBeamFlag = true; xBeamFlag2 = true; yBeamFlag = true; 
        }

        // ビームの進行処理
        if (xBeamFlag) {
            goalLineX[0] += 50; goalLineX[1] += 50;
        }
        // ビームの進行処理
        if (xBeamFlag2) {
            goalLineX[2] -= 50; goalLineX[3] -= 50;
        }
        if (yBeamFlag) {
            goalLineY[4] += 50;
            goalLineY[5] += 50;
            goalLineY[6] += 50;
        }

        // 端の位置制限
        if (goalLineX[0] >= 1600) { goalLineX[0] = 1600; startLineX[0] += 50; }
        if (goalLineX[1] >= 1600) { goalLineX[1] = 1600; startLineX[1] += 50; }
        if (goalLineX[2] <= -120) { goalLineX[2] = -120; startLineX[2] -= 50; }
        if (goalLineX[3] <= -120) { goalLineX[3] = -120; startLineX[3] -= 50; }
        if (goalLineY[4] >= 940) { goalLineY[4] = 940; startLineY[4] += 50; }
        if (goalLineY[5] >= 940) { goalLineY[5] = 940; startLineY[5] += 50; }
        if (goalLineY[6] >= 940) { goalLineY[6] = 940; startLineY[6] += 50; }

        // リセット処理
        if (keys[DIK_R] && !preKeys[DIK_R]) {
            xBeamFlag = false;
            xBeamFlag2 = false;
            yBeamFlag = false;
            memcpy(startLineX, initialStartLineX, sizeof(initialStartLineX));
            memcpy(startLineY, initialStartLineY, sizeof(initialStartLineY));
            memcpy(goalLineX, initialGoalLineX, sizeof(initialGoalLineX));
            memcpy(goalLineY, initialGoalLineY, sizeof(initialGoalLineY));
        }

        ///
        /// 更新処理ここまで
        /// 

        ///
        /// 描画処理ここから
        ///

        //自機
        Novice::DrawBox(posX, posY, sizeX, sizeY, 0.0f, WHITE, kFillModeSolid);
        //ボス
        Novice::DrawBox(600, 100, 200, 200, 0.0f, WHITE, kFillModeSolid);
        DrawBeams(startLineX, startLineY, goalLineX, goalLineY, BLACK);

        // 右クリックで斬撃を描画
        if (Novice::IsPressMouse(0)) {
            DrawSlash(posX + sizeX / 2, posY + sizeY / 2, mouseX, mouseY, BLACK, 60.0f); // 自機の中心から描画
        }
        ///
        /// 描画処理ここまで
        ///

        // フレームの終了
        Novice::EndFrame();

        // ESCキーが押されたらループを抜ける
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }
}
#include <Novice.h>
#include <cmath>
#include <cstdlib>

const char kWindowTitle[] = "TD1BossBattle";
const float M_PI = 3.14159265358979323846f;

enum Scene
{
    TITLE,
    GAME,
    GAME_CLEAR,
    GAME_OVER
};

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

bool CheckCollision(float leftTopX, float leftTopY, float rightTopX, float rightTopY, float leftBottomX, float leftBottomY, float rightBottomX, float rightBottomY, int bossPosX, int bossPosY, int bossSizeX, int bossSizeY) {
    // ボスの矩形領域
    int bossLeft = bossPosX;
    int bossRight = bossPosX + bossSizeX;
    int bossTop = bossPosY;
    int bossBottom = bossPosY + bossSizeY;

    // スラッシュの各頂点がボスの矩形領域内にあるかチェック
    return (leftTopX >= bossLeft && leftTopX <= bossRight && leftTopY >= bossTop && leftTopY <= bossBottom) ||
        (rightTopX >= bossLeft && rightTopX <= bossRight && rightTopY >= bossTop && rightTopY <= bossBottom) ||
        (leftBottomX >= bossLeft && leftBottomX <= bossRight && leftBottomY >= bossTop && leftBottomY <= bossBottom) ||
        (rightBottomX >= bossLeft && rightBottomX <= bossRight && rightBottomY >= bossTop && rightBottomY <= bossBottom);
}

int bossColor = WHITE;
int bossHP = 20;
void DrawSlash(int startX, int startY, int targetX, int targetY, unsigned int color, float length, int bossPosX, int bossPosY, int bossSizeX, int bossSizeY) {
    const float width = 130.0f;

    // 斬撃の角度を計算
    float angle = static_cast<float>(atan2(targetY - startY, targetX - startX));

    // 斬撃の発生位置を自機の中心から130ピクセル離れた位置に計算
    float slashX = startX + cosf(angle) * 130.0f;
    float slashY = startY + sinf(angle) * 130.0f;

    // 四角形の左上、右上、左下、右下の座標を計算
    float halfWidth = width / 2.0f;
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

    // 当たり判定
    if (bossHP > 0) {
        if (CheckCollision(leftTopX, leftTopY, rightTopX, rightTopY, leftBottomX, leftBottomY, rightBottomX, rightBottomY, bossPosX, bossPosY, bossSizeX, bossSizeY)) {
            bossHP -= 1;
            bossColor = BLACK; // 当たった瞬間だけ黒に変更
        }
    }
}

// ビームの進行方向に自機の当たり判定を追加
bool CheckBeamCollisionWithPlayer(int playerX, int playerY, int playerWidth, int playerHeight, int beamStartX, int beamStartY, int beamEndX, int beamEndY) {
    // 自機の矩形領域
    int playerLeft = playerX - 40;
    int playerRight = playerX + playerWidth + 40;
    int playerTop = playerY - 40;
    int playerBottom = playerY + playerHeight + 40;

    // ビームが自機の矩形領域内にあるかチェック
    if ((beamStartX >= playerLeft && beamStartX <= playerRight && beamStartY >= playerTop && beamStartY <= playerBottom) ||
        (beamEndX >= playerLeft && beamEndX <= playerRight && beamEndY >= playerTop && beamEndY <= playerBottom)) {
        return true;  // 衝突した
    }
    return false;  // 衝突しなかった
}

void CheckEnemyAttackRangeAndExecute(int playerX, int playerY, int enemyX, int enemyY, int enemyRadius, int enemySizeX, int enemySizeY) {
    // プレイヤーとの距離を計算
    float distance = (float)sqrt(pow(enemyX - playerX, 2) + pow(enemyY - playerY, 2));

    // プレイヤーが範囲内に入ったら攻撃
    if (distance <= enemyRadius) {
        DrawSlash(enemyX, enemyY, playerX, playerY, RED, 200, enemyX, enemyY, enemySizeX, enemySizeY);
    }
}

int bossAttackCoolTime = 0;
int bossAttackTimeFlag = false;
void ExecuteCloseRangeAttack(int playerPosX, int playerPosY, int playerSizeX, int playerSizeY, int bossPosX, int bossPosY, int bossSizeX, int bossSizeY) {
    // プレイヤーの中心座標を計算
    int playerCenterX = playerPosX + playerSizeX / 2;
    int playerCenterY = playerPosY + playerSizeY / 2;

    // ボスの中心座標を計算
    int bossCenterX = bossPosX + bossSizeX / 2;
    int bossCenterY = bossPosY + bossSizeY / 2;

    // プレイヤーとボスの距離が400ピクセル以内かどうかチェック
    if (std::abs(bossCenterX - playerCenterX) <= 300 && std::abs(bossCenterY - playerCenterY) <= 130) {
        if (playerCenterX < bossCenterX) {
            // プレイヤーがボスの左側にいる場合
            Novice::DrawBox(bossCenterX - bossSizeX / 2 - 120, bossCenterY - bossSizeY / 2, 70, bossSizeY, 0.0f, RED, kFillModeSolid);
            bossAttackTimeFlag = true;
        }
        else {
            // プレイヤーがボスの右側にいる場合
            Novice::DrawBox(bossCenterX + bossSizeX / 2 + 120, bossCenterY - bossSizeY / 2, 70, bossSizeY, 0.0f, BLUE, kFillModeSolid);
            bossAttackTimeFlag = true;
        }
    }
}

const int numOfBullets = 10; // 円の数
float bulletPosX[numOfBullets];
float bulletPosY[numOfBullets];
bool bulletActive[numOfBullets] = { false }; // 弾の発射状態
float bulletSpeed = 5.0f; // 弾の移動速度
int bulletCooldown = 0; // 弾の発射クールタイム

void ShootBullets(int bossPosX, int bossPosY, int playerPosX, int playerPosY, int bossSizeX) {
    // 発射間隔が1秒になるように調整
    if (bulletCooldown <= 0) {
        for (int i = 0; i < numOfBullets; ++i) {
            if (!bulletActive[i]) {
                float angle = (float)atan2(playerPosY - bossPosY, playerPosX - bossPosX);
                bulletPosX[i] = bossPosX + bossSizeX / 2 + cos(angle) * 50; // 発射位置
                bulletPosY[i] = bossPosY + sin(angle) * 50;
                bulletActive[i] = true;
                break; // 一度に1発だけ発射
            }
        }
        bulletCooldown = 60; // 1秒のクールタイム
    }
}

void MoveBullets(int playerPosY,int playerPosX,int playerSizeX,int playerSizeY) {
    for (int i = 0; i < numOfBullets; ++i) {
        if (bulletActive[i]) {
            // 弾がアクティブなら移動
            float angle = atan2(playerPosY + playerSizeY / 2 - bulletPosY[i], playerPosX + playerSizeX / 2 - bulletPosX[i]);
            bulletPosX[i] += cos(angle) * bulletSpeed;
            bulletPosY[i] += sin(angle) * bulletSpeed;

            // 画面外に出たら非アクティブ化
            if (bulletPosX[i] < 0 || bulletPosX[i] > 1600 || bulletPosY[i] < 0 || bulletPosY[i] > 900) {
                bulletActive[i] = false;
            }
        }
    }
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
    int playerHP = 1000;
    int bossPosX = 1000;
    int bossPosY = 500;
    int bossSizeX = 200;
    int bossSizeY = 200;

    int scene = TITLE;
    //ビームの実際の当たり判定
    int boxSizeX[7] = { 75,75,75,75,75,75,75 };
    int boxSizeY[7] = { 75,75,75,75,75,75,75 };
    int boxPosX[7];
    int boxPosY[7];
    // スピードを設定します（1フレームあたりの加算量）
    float beamSpeed[7] = { 40.0f };  // スタート時の速度

    //画像の読み込み
    int playerImage[7] = {
        Novice::LoadTexture("./Resources/move1.png"),
        Novice::LoadTexture("./Resources/move2.png"),
        Novice::LoadTexture("./Resources/move3.png"),
        Novice::LoadTexture("./Resources/move4.png"),
        Novice::LoadTexture("./Resources/move5.png"),
        Novice::LoadTexture("./Resources/move6.png"),
        Novice::LoadTexture("./Resources/move0.png"),
    };
    int bossImage = Novice::LoadTexture("./Resources/darkPhenix01.png");
    int catBossImage = Novice::LoadTexture("./Resources/bossCatAttack01.png");
    int bossGauge = Novice::LoadTexture("./Resources/bossgauge.png");
    int stageBackGround = Novice::LoadTexture("./Resources/haikei0003.png");
    int playerGauge = Novice::LoadTexture("./Resources/playerGauge.png");

    int playerImageFrameCount = 0;
    int isTurnLeft = false;
    int isTurnRight = true;
    int playerColor = WHITE;

    int bossImageChange = false;

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

        switch (scene)
        {
        case TITLE:
            if (keys[DIK_P] && preKeys[DIK_P] == 0) {
                scene = GAME;
            }
            break;
        case GAME:
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
                goalLineX[0] += 40; goalLineX[1] += 40;
            }
            // ビームの進行処理
            if (xBeamFlag2) {
                goalLineX[2] -= 40; goalLineX[3] -= 40;
            }
            if (yBeamFlag) {
                goalLineY[4] += 40;
                goalLineY[5] += 40;
                goalLineY[6] += 40;
            }
            
            // 端の位置制限
            if (goalLineX[0] >= 1600) { goalLineX[0] = 1600; startLineX[0] += 50; }
            if (goalLineX[1] >= 1600) { goalLineX[1] = 1600; startLineX[1] += 50; }
            if (goalLineX[2] <= -120) { goalLineX[2] = -120; startLineX[2] -= 50; }
            if (goalLineX[3] <= -120) { goalLineX[3] = -120; startLineX[3] -= 50; }
            if (goalLineY[4] >= 940) { goalLineY[4] = 940; startLineY[4] += 50; }
            if (goalLineY[5] >= 940) { goalLineY[5] = 940; startLineY[5] += 50; }
            if (goalLineY[6] >= 940) { goalLineY[6] = 940; startLineY[6] += 50; }

            //ボスの近接攻撃のクールタイム
            if (bossAttackTimeFlag) {
                bossAttackCoolTime++;
            }
            if (bossAttackCoolTime > 120) {
                bossAttackTimeFlag = false;
                bossAttackCoolTime = 0;
            }
            if (bossAttackTimeFlag == false) {
                ExecuteCloseRangeAttack(posX, posY, sizeX, sizeY, bossPosX, bossPosY, bossSizeX, bossSizeY);
            }
            bulletCooldown--;
            if (bossHP > 0) {
                ShootBullets(bossPosX, bossPosY, posX, posY,bossSizeX); // ボスから弾を発射
            }
            MoveBullets(posY,posX,sizeX,sizeY); // 弾を移動

            // リセット処理
            if (keys[DIK_R] && !preKeys[DIK_R]) {
                // ビームフラグをリセット
                xBeamFlag = false;
                xBeamFlag2 = false;
                yBeamFlag = false;

                // 位置を初期化
                memcpy(startLineX, initialStartLineX, sizeof(initialStartLineX));
                memcpy(startLineY, initialStartLineY, sizeof(initialStartLineY));
                memcpy(goalLineX, initialGoalLineX, sizeof(initialGoalLineX));
                memcpy(goalLineY, initialGoalLineY, sizeof(initialGoalLineY));

                // スピードを初期化
                beamSpeed[0] = 40.0f;
                beamSpeed[1] = 40.0f;
                beamSpeed[2] = 40.0f;
                beamSpeed[3] = 40.0f;
                beamSpeed[4] = 40.0f;
                beamSpeed[5] = 40.0f;
                beamSpeed[6] = 40.0f;

                playerHP = 1000;
                bossHP = 20;
            }
            
            if(playerHP <= 0){
                scene = GAME_OVER;
            }
            if (bossHP <= 0) {
                scene = GAME_CLEAR;
            }
            break;
        case GAME_CLEAR:
            if (keys[DIK_P] && preKeys[DIK_P] == 0) {
                playerHP = 1000;
                bossHP = 20;
                scene = TITLE;
            }
            break;
        case GAME_OVER:
            if (keys[DIK_P] && preKeys[DIK_P] == 0) {
                playerHP = 1000;
                bossHP = 20;
                scene = TITLE;
            }
            break;
        }

        ///
        /// 更新処理ここまで
        /// 

        ///
        /// 描画処理ここから
        ///

        switch (scene)
        {
        case TITLE:
            break;
        case GAME:
            //背景
            Novice::DrawSprite(0, 0, stageBackGround, 1.2f, 1.2f, 0.0f, WHITE);
            //自機
            // Novice::DrawBox(posX, posY, sizeX, sizeY, 0.0f, GREEN, kFillModeSolid);
            //プレイヤーの画像描画
            if (keys[DIK_A] && !keys[DIK_D]) {
                playerImageFrameCount++;
                if (playerImageFrameCount >= 60) {
                    playerImageFrameCount = 0;
                }
                Novice::DrawSprite(posX, posY, playerImage[playerImageFrameCount / 12], 1.0f, 1.0f, 0.0f, playerColor);
                isTurnLeft = true;
                isTurnRight = false;
            }
            if (keys[DIK_D] && !keys[DIK_A]) {
                playerImageFrameCount++;
                if (playerImageFrameCount >= 60) {
                    playerImageFrameCount = 0;
                }
                Novice::DrawSprite(posX + sizeX, posY, playerImage[playerImageFrameCount / 12], -1.0f, 1.0f, 0.0f, playerColor);
                isTurnLeft = false;
                isTurnRight = true;
            }
            if (!keys[DIK_A] && !keys[DIK_D] && !keys[DIK_W] && !keys[DIK_S]) {
                if (isTurnLeft == 1) {
                    Novice::DrawSprite(posX, posY, playerImage[6], 1.0f, 1.0f, 0.0f, playerColor);
                }       
                if (isTurnRight == 1) {
                    Novice::DrawSprite(posX + sizeX, posY, playerImage[6], -1.0f, 1.0f, 0.0f, playerColor);
                }
            }
            if (keys[DIK_A] && keys[DIK_D] && keys[DIK_W] && keys[DIK_S]) {
                Novice::DrawSprite(posX, posY, playerImage[6], 1.0f, 1.0f, 0.0f, playerColor);
            }
            //上下移動の時も歩くようにする
            if (keys[DIK_W]) {
                if (isTurnLeft == 1) {
                    playerImageFrameCount++;
                    if (playerImageFrameCount >= 60) {
                        playerImageFrameCount = 0;
                    }
                    Novice::DrawSprite(posX, posY, playerImage[playerImageFrameCount / 12], 1.0f, 1.0f, 0.0f, playerColor);
                }
                if (isTurnRight == 1) {
                    playerImageFrameCount++;
                    if (playerImageFrameCount >= 60) {
                        playerImageFrameCount = 0;
                    }
                    Novice::DrawSprite(posX + sizeX, posY, playerImage[playerImageFrameCount / 12], -1.0f, 1.0f, 0.0f, playerColor);
                }
            }
            if (keys[DIK_S]) {
                if (isTurnLeft == 1) {
                    playerImageFrameCount++;
                    if (playerImageFrameCount >= 60) {
                        playerImageFrameCount = 0;
                    }
                    Novice::DrawSprite(posX, posY, playerImage[playerImageFrameCount / 12], 1.0f, 1.0f, 0.0f, playerColor);
                }
                if (isTurnRight == 1) {
                    playerImageFrameCount++;
                    if (playerImageFrameCount >= 60) {
                        playerImageFrameCount = 0;
                    }
                    Novice::DrawSprite(posX + sizeX, posY, playerImage[playerImageFrameCount / 12], -1.0f, 1.0f, 0.0f, playerColor);
                }
            }
            //プレイヤーのHPゲージ(仮置き)
            Novice::DrawSprite(50, 500, playerGauge, 1.0f, 1.0f, 0.0f, WHITE);

            // ゲーム内でのビームと自機の衝突判定
            for (int i = 0; i < 7; ++i) {
                if (CheckBeamCollisionWithPlayer(posX, posY, sizeX, sizeY, startLineX[i], startLineY[i], goalLineX[i], goalLineY[i])) {
                    playerHP -= 10;  // 自機がビームに当たった場合のダメージ
                   // Novice::DrawBox(posX, posY, sizeX, sizeY, 0.0f, RED, kFillModeSolid); // 赤色で自機を描画してダメージを表示
                    Novice::DrawSprite(posX, posY, playerImage[6], 1.0f, 1.0f, 0.0f, RED);
                }
            }
            
            //ボス
            if (bossHP > 0) {
                //Novice::DrawBox(bossPosX, bossPosY, bossSizeX, bossSizeY, 0.0f, bossColor, kFillModeSolid);
                //デバック用のボス画像切り替え(後からボス描画以外消去)
                if (keys[DIK_C] && !preKeys[DIK_C]) {
                    if (bossImageChange == 0) {
                        bossImageChange = true;
                    }  
                }
                if (keys[DIK_V] && !preKeys[DIK_V]) {
                    if (bossImageChange == 1) {
                        bossImageChange = false;
                    }
                }
                if (bossImageChange == 0) {
                    Novice::DrawSprite(bossPosX, bossPosY, bossImage, 1.0f, 1.0f, 0.0f, WHITE);
                }
                if (bossImageChange == 1) {
                    Novice::DrawSprite(bossPosX, bossPosY, catBossImage, 1.0f, 1.0f, 0.0f, WHITE);
                }
                //ボスゲージ（仮置き）
                Novice::DrawSprite(200, 100, bossGauge, 1.0f, 1.0f, 0.0f, WHITE);
            }
            // 敵の弾の描画
            for (int i = 0; i < numOfBullets; ++i) {
                if (bulletActive[i]) {
                    Novice::DrawEllipse(int(bulletPosX[i]), int(bulletPosY[i]), 20, 20,0.0f,RED,kFillModeSolid); // 弾の描画
                }
            }
            for (int i = 0; i < 7; ++i) {
                // ボックスの中心位置を設定
                boxPosX[i] = startLineX[i] - boxSizeX[i] / 2;
                boxPosY[i] = startLineY[i] - boxSizeY[i] / 2;

                float leftTopX = static_cast<float>(boxPosX[i]);
                float leftTopY = static_cast<float>(boxPosY[i]);
                float rightTopX = static_cast<float>(boxPosX[i]) + static_cast<float>(boxSizeX[i]);
                float rightTopY = static_cast<float>(boxPosY[i]);
                float leftBottomX = static_cast<float>(boxPosX[i]);
                float leftBottomY = static_cast<float>(boxPosY[i]) + static_cast<float>(boxSizeY[i]);
                float rightBottomX = static_cast<float>(boxPosX[i]) + static_cast<float>(boxSizeX[i]);
                float rightBottomY = static_cast<float>(boxPosY[i]) + static_cast<float>(boxSizeY[i]);

                if (xBeamFlag == true && i == 0) {
                    // スピードをフレームごとに増加させる
                    rightTopX += beamSpeed[0];
                    rightBottomX += beamSpeed[0];

                    //スピードを増加
                    beamSpeed[0] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (xBeamFlag == true && i == 1) {
                    // スピードをフレームごとに増加させる
                    rightTopX += beamSpeed[1];
                    rightBottomX += beamSpeed[1];

                    //スピードを増加
                    beamSpeed[1] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (xBeamFlag2 == true && i == 2) {
                    // スピードをフレームごとに増加させる
                    leftTopX += beamSpeed[2];
                    leftBottomX += beamSpeed[2];

                    //スピードを増加
                    beamSpeed[2] -= 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (xBeamFlag2 == true && i == 3) {
                    // スピードをフレームごとに増加させる
                    leftTopX += beamSpeed[3];
                    leftBottomX += beamSpeed[3];

                    //スピードを増加
                    beamSpeed[3] -= 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (yBeamFlag == true && i == 4) {
                    // スピードをフレームごとに増加させる
                    rightBottomY += beamSpeed[4];
                    leftBottomY += beamSpeed[4];

                    //スピードを増加
                    beamSpeed[4] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (yBeamFlag == true && i == 5) {
                    // スピードをフレームごとに増加させる
                    rightBottomY += beamSpeed[5];
                    leftBottomY += beamSpeed[5];

                    //スピードを増加
                    beamSpeed[5] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (yBeamFlag == true && i == 6) {
                    // スピードをフレームごとに増加させる
                    rightBottomY += beamSpeed[6];
                    leftBottomY += beamSpeed[6];

                    //スピードを増加
                    beamSpeed[6] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }

                // ボックスを描画
                Novice::DrawQuad(
                    static_cast<int>(leftTopX), static_cast<int>(leftTopY),
                    static_cast<int>(rightTopX), static_cast<int>(rightTopY),
                    static_cast<int>(leftBottomX), static_cast<int>(leftBottomY),
                    static_cast<int>(rightBottomX), static_cast<int>(rightBottomY),
                    0, 0, 1, 1, 0, WHITE);  // 白いボックスを描画
            }

            DrawBeams(startLineX, startLineY, goalLineX, goalLineY, BLACK);
            
            bossColor = WHITE;
            // 左クリックで斬撃を描画し、当たり判定をチェック
            if (Novice::IsTriggerMouse(0)) {
                DrawSlash(posX + sizeX / 2, posY + sizeY / 2, mouseX, mouseY, BLACK, 60.0f, bossPosX, bossPosY, bossSizeX, bossSizeY);
            }

            Novice::ScreenPrintf(20,20,"%d",bossHP);
            Novice::ScreenPrintf(20, 40, "%d", playerHP);
            Novice::ScreenPrintf(20, 60, "%d", bossAttackCoolTime);
            Novice::ScreenPrintf(20, 80, "%d", bossImageChange);
            break;
        case GAME_CLEAR:
            break;
        case GAME_OVER:
            break;
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
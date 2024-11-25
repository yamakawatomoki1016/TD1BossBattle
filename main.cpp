#include <Novice.h>
#include <cmath>
#include <cstdlib>
#include <vector>

const char kWindowTitle[] = "TD1BossBattle";
const float M_PI = 3.14159265358979323846f;

enum Scene
{
    TITLE,
    GAME,
    GAME_CLEAR,
    GAME_OVER
};

struct Circle {
    float x, y;   // 円の位置
    float vx, vy; // 円の速度
    bool active;  // 円が飛んでいるかどうか
    float radius;
};
std::vector<Circle> circles; // 円のリスト
struct Particle {
    float x, y;       // パーティクルの位置
    float vx, vy;     // パーティクルの速度
    float size;       // パーティクルのサイズ
    float alpha;      // パーティクルの透明度
    bool active;      // パーティクルが有効かどうか
};
struct FireParticle {
    float x, y;      // パーティクルの位置
    float size;      // パーティクルのサイズ
    float speedY;    // 上方向の速度
    float alpha;     // 透明度 (0.0 - 1.0)
    bool active;     // アクティブ状態
};
const int kMaxParticles = 100; // パーティクルの最大数
Particle particles[kMaxParticles];

const int kMaxBlackParticles = 200; // パーティクルの最大数
const int kScreenWidth = 1600;  // 画面の幅
const int kScreenHeight = 900;  // 画面の高さ
FireParticle blackParticles[kMaxBlackParticles]; // パーティクル配列

void InitializeBlackParticles() {
    for (int i = 0; i < kMaxBlackParticles; ++i) {
        blackParticles[i].x = static_cast<float>(rand() % kScreenWidth);
        blackParticles[i].y = static_cast<float>(rand() % kScreenHeight);
        blackParticles[i].size = static_cast<float>(rand() % 3 + 1); // サイズ5～15
        blackParticles[i].speedY = static_cast<float>(rand() % 5 + 1); // 上昇速度1～5
        blackParticles[i].alpha = static_cast<float>(rand() % 50 + 50) / 100.0f; // 透明度0.5～1.0
        blackParticles[i].active = true;
    }
}

void UpdateBlackParticles() {
    for (int i = 0; i < kMaxBlackParticles; ++i) {
        if (blackParticles[i].active) {
            blackParticles[i].y -= blackParticles[i].speedY; // 上に移動

            // 画面外に出たらリセット
            if (blackParticles[i].y < 0) {
                blackParticles[i].x = static_cast<float>(rand() % kScreenWidth);
                blackParticles[i].y = static_cast<float>(kScreenHeight);
                blackParticles[i].size = static_cast<float>(rand() % 3 + 1);
                blackParticles[i].speedY = static_cast<float>(rand() % 5 + 1);
                blackParticles[i].alpha = static_cast<float>(rand() % 50 + 50) / 100.0f;
            }
        }
    }
}

void DrawBlackParticles() {
    for (int i = 0; i < kMaxBlackParticles; ++i) {
        if (blackParticles[i].active) {
            Novice::DrawEllipse(
                static_cast<int>(blackParticles[i].x),
                static_cast<int>(blackParticles[i].y),
                static_cast<int>(blackParticles[i].size),
                static_cast<int>(blackParticles[i].size),
                0.0f,
                0x191970,
                kFillModeSolid
            );
        }
    }
}

void InitializeParticles() {
    for (int i = 0; i < kMaxParticles; ++i) {
        particles[i] = { 0, 0, 0, 0, 0, 0, false };
    }
}

void GenerateParticle(float bossX, float bossY) {
    for (int i = 0; i < kMaxParticles; ++i) {
        if (!particles[i].active) {
            particles[i].x = bossX + (rand() % 100 - 50); // ボス周囲にランダム生成
            particles[i].y = bossY + (rand() % 100 - 50);
            particles[i].vx = (rand() % 20 - 10) * 0.1f; // ランダムな速度
            particles[i].vy = (rand() % 20 - 10) * 0.1f;
            particles[i].size = static_cast<float>(rand() % 5 + 3);
            particles[i].alpha = 1.0f;                  // 初期の透明度
            particles[i].active = true;
            break;
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < kMaxParticles; ++i) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].alpha -= 0.01f; // 透明度を徐々に減少

            // 消滅条件
            if (particles[i].alpha <= 0) {
                particles[i].active = false;
            }
        }
    }
}

void DrawParticles() {
    for (int i = 0; i < kMaxParticles; ++i) {
        if (particles[i].active) {
            // パーティクルの描画
            Novice::DrawEllipse(
                static_cast<int>(particles[i].x),
                static_cast<int>(particles[i].y),
                static_cast<int>(particles[i].size),
                static_cast<int>(particles[i].size),
                0.0f,
                BLACK,
                kFillModeSolid
            );
        }
    }
}

// 地面の位置
const int groundHeight = GetSystemMetrics(SM_CYSCREEN) - 50;
int posX = 500;
int posY = groundHeight - 120;
const int sizeX = 100;
const int sizeY = 120;
const int playerSpeed = 10;
int playerColor = WHITE;

// 残像の履歴を保存する構造体
struct Position {
    float x, y;
    float alpha;
};

std::vector<Position> playerTrail;  // 自機の残像を保存するベクター
const int MAX_TRAIL_LENGTH = 10;    // 残像の長さ（過去何フレーム分）

// イージング関数（EaseOut）
float EaseOut(float t) {
    return t * (2.0f - t);  // tが0から1の間で滑らかに減少
}

void LaunchCircles(float startX, float startY) {
    const int circleCount = 100;
    const float speed = 1.0f; // 円が飛ぶ速度

    circles.clear(); // 前の円をリセット

    for (int i = 0; i < circleCount; ++i) {
        float angle = 2.0f * 3.14159265359f * i / circleCount; // 角度を計算
        Circle circle = {
            startX,
            startY,
            speed * cos(angle), // x方向の速度
            speed * sin(angle), // y方向の速度
            true                // アクティブに設定
        };
        circles.push_back(circle);
    }
}

//void DrawLightningLine(int startX, int startY, int endX, int endY, unsigned int color) {
//    const int segments = 400;  // 分割するセグメント数
//    const int maxOffset = 40;  // セグメントごとの最大オフセット値
//
//    int prevX = startX;
//    int prevY = startY;
//
//    for (int i = 1; i <= segments; ++i) {
//        float t = static_cast<float>(i) / segments;
//        int currentX = static_cast<int>(startX + (endX - startX) * t);
//        int currentY = static_cast<int>(startY + (endY - startY) * t);
//
//        currentX += (rand() % (2 * maxOffset)) - maxOffset;
//        currentY += (rand() % (2 * maxOffset)) - maxOffset;
//
//        Novice::DrawLine(prevX, prevY, currentX, currentY, color);
//
//        prevX = currentX;
//        prevY = currentY;
//    }
//}
//
//void DrawBeams(int* startLineX, int* startLineY, int* goalLineX, int* goalLineY, unsigned int color) {
//    for (int i = 0; i < 7; ++i) {
//        DrawLightningLine(startLineX[i], startLineY[i], goalLineX[i], goalLineY[i], color);
//    }
//}

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

    // 斬撃の発生位置を自機の中心から140ピクセル離れた位置に計算
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
    //int slash = Novice::LoadTexture("./Resources/slash.png");
    Novice::DrawQuad(int(leftTopX), int(leftTopY), int(rightTopX), int(rightTopY), int(leftBottomX), int(leftBottomY), int(rightBottomX), int(rightBottomY), 0, 0, 1, 1, 0, color);


    // 当たり判定
    if (bossHP > 0) {
        if (CheckCollision(leftTopX, leftTopY, rightTopX, rightTopY, leftBottomX, leftBottomY, rightBottomX, rightBottomY, bossPosX, bossPosY, bossSizeX, bossSizeY)) {
            bossHP -= 1;
            bossColor = RED; // 当たった瞬間だけ黒に変更
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
        playerColor = RED;
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
int playerHP = 1000;
int bossAttackCoolTime = 0;
int bossAttackTimeFlag = false;
int bossAttackCooldownTime = 0;
int bossAttackDelay = 120;
//敵の近接攻撃
void ExecuteCloseRangeAttack(int playerPosX, int playerPosY, int playerSizeX, int playerSizeY, int bossPosX, int bossPosY, int bossSizeX, int bossSizeY) {
    // プレイヤーの中心座標を計算
    int playerCenterX = playerPosX + playerSizeX / 2;
    int playerCenterY = playerPosY + playerSizeY / 2;

    // ボスの中心座標を計算
    int bossCenterX = bossPosX + bossSizeX / 2;
    int bossCenterY = bossPosY + bossSizeY / 2;

    // プレイヤーとボスの距離が400ピクセル以内かどうかチェック
    if (std::abs(bossCenterX - playerCenterX) <= 300 && std::abs(bossCenterY - playerCenterY) <= 150) {
        if (bossAttackCooldownTime == 0) {
            if (playerCenterX < bossCenterX) {
                // プレイヤーがボスの左側にいる場合、赤い攻撃ボックスを描画
                Novice::DrawBox(bossCenterX - bossSizeX / 2 - 70, bossCenterY - bossSizeY / 2, 70, bossSizeY, 0.0f, RED, kFillModeSolid);
                // プレイヤーがそのボックスと衝突する場合、HPを減らす
                if (playerPosX + playerSizeX >= bossCenterX - bossSizeX / 2 - 70 && playerPosX <= bossCenterX - bossSizeX / 2 - 70 + 70) {
                    if (playerPosY + playerSizeY >= bossCenterY - bossSizeY / 2 && playerPosY <= bossCenterY + bossSizeY / 2) {
                        playerHP -= 10; // ダメージ処理
                        playerColor = RED;
                    }
                }
            }
            else {
                // プレイヤーがボスの右側にいる場合、青い攻撃ボックスを描画
                Novice::DrawBox(bossCenterX + bossSizeX / 2 + 10, bossCenterY - bossSizeY / 2, 70, bossSizeY, 0.0f, BLUE, kFillModeSolid);
                // プレイヤーがそのボックスと衝突する場合、HPを減らす
                if (playerPosX + playerSizeX >= bossCenterX + bossSizeX / 2 + 10 && playerPosX <= bossCenterX + bossSizeX / 2 + 10 + 70) {
                    if (playerPosY + playerSizeY >= bossCenterY - bossSizeY / 2 && playerPosY <= bossCenterY + bossSizeY / 2) {
                        playerHP -= 10; // ダメージ処理
                        playerColor = RED;
                    }
                }
            }

            bossAttackCooldownTime = bossAttackDelay;
        }
    }
}

const int numOfBullets = 10; // 円の数
float bulletPosX[numOfBullets];
float bulletPosY[numOfBullets];
bool bulletActive[numOfBullets] = { false }; // 弾の発射状態
float bulletSpeed = 2.0f; // 弾の移動速度
int bulletCooldown = 0; // 弾の発射クールタイム
int bulletTimer[numOfBullets];

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
        bulletCooldown = 600; //クールタイム
        int homingBulletSounds = Novice::LoadAudio("./Resources/maou_se_sound03.mp3");
        Novice::PlayAudio(homingBulletSounds, 0, 0.5);
    }
}
// 弾の移動処理内
void MoveBullets(int playerPosY, int playerPosX, int playerSizeX, int playerSizeY) {
    for (int i = 0; i < numOfBullets; ++i) {
        if (bulletActive[i]) {
            // 弾がアクティブなら移動
            float angle = atan2(playerPosY + playerSizeY / 2 - bulletPosY[i], playerPosX + playerSizeX / 2 - bulletPosX[i]);
            bulletPosX[i] += cos(angle) * bulletSpeed;
            bulletPosY[i] += sin(angle) * bulletSpeed;

            // 自機との当たり判定
            if (bulletPosX[i] > playerPosX && bulletPosX[i] < playerPosX + playerSizeX &&
                bulletPosY[i] > playerPosY && bulletPosY[i] < playerPosY + playerSizeY) {
                playerHP -= 100; // 自機のHPを減少
                playerColor = RED;
                bulletActive[i] = false; // 弾を消す
                bulletTimer[i] = 0;
            }

            if (bulletActive[i] == true) {
                bulletTimer[i]++;
            }
            if (bulletTimer[i] >= 300) {
                bulletActive[i] = false;
                bulletTimer[i] = 0;
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

    int startLineX[7] = { -50, -50, 1650, 1650, 200, 750, 1350 };
    int startLineY[7] = { 270, 800, 270, 800, -50, -50, -50 };
    int goalLineX[7] = { -50, -50, 1540, 1540, 200, 750, 1350 };
    int goalLineY[7] = { 270, 800, 270, 800, -50, -50, -50 };

    const int initialStartLineX[7] = { -50, -50, 1650, 1650, 200, 750, 1350 };
    const int initialStartLineY[7] = { 270, 800, 270, 800, -50, -50, -50 };
    const int initialGoalLineX[7] = { -50, -50, 1540, 1540, 200, 750, 1350 };
    const int initialGoalLineY[7] = { 270, 800, 270, 800, -50, -50, -50 };
    int bossBeamCooldown = 0; // ビームのクールダウンタイマー
    int randomBeamIndex = -1; // 発射するビームのインデックス
    bool xBeamFlagInProgress = false;  // x方向ビームの進行状態
    bool xBeamFlag2InProgress = false; // 反対方向xビームの進行状態
    bool yBeamFlagInProgress = false;  // y方向ビームの進行状態

    int bossPosX = 1000;
    int bossPosY = 500;
    int bossSizeX = 200;
    int bossSizeY = 200;
    int bossTeleportTimer = 0;
    int randTimer = 0;
    int bossCircularAttackTimer = 0; // タイマー初期化
    bool bossCircularAttackFlag = false; // 最初の発射フラグ
    int bossCircularAttackCooldown = 1400; // 1400フレームのクールダウン
    bool isFirstLaunch = true; // 初回発射かどうかを判定するフラグ
    int blackBall = Novice::LoadTexture("./Resources/blackBall.png");

    int scene = TITLE;
    //ビームの実際の当たり判定
    int boxSizeX[7] = { 75,75,75,75,75,75,75 };
    int boxSizeY[7] = { 75,75,75,75,75,75,75 };
    int boxPosX[7];
    int boxPosY[7];
    float beamSpeed[7] = { 40.0f };  // スタート時の速度
    float bossCenterX = 0;
    float bossCenterY = 0;
    
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
    //int slash = Novice::LoadTexture("./Resources/slash.png");
    int title = Novice::LoadTexture("./Resources/title.png");
    int gameover = Novice::LoadTexture("./Resources/zannnenn.png");
    int homingBullet1 = Novice::LoadTexture("./Resources/tuibidan1.png");
    int homingBullet2 = Novice::LoadTexture("./Resources/tuibidan2.png");
    int homingBullet3 = Novice::LoadTexture("./Resources/tuibidan3.png");

    //音の読み込み
    //int homingBulletSounds = Novice::LoadAudio("./Resources/maou_se_sound03.mp3");
    int slashSounds = Novice::LoadAudio("./Resources/maou_se_battle03.mp3");

    int playerImageFrameCount = 0;
    int isTurnLeft = false;
    int isTurnRight = true;
    int bossImageChange = false;
    int homingBulletTimer = 0;
    // ジャンプ関連の変数
    bool isJumping = false; // ジャンプ中かどうか
    int jumpVelocity = 0; // ジャンプの速度
    const int gravity = 2; // 重力加速度
    const int jumpPower = 35; // ジャンプの初速度
    InitializeBlackParticles();

    int beamPosX[7] = { 50, 50, 1550, 1550, 200, 750, 1350 };
    int beamPosY[7] = { 270, 800, 270, 800, 50, 50, 50 };
    int beamSize = 75;
    int beamTimer[3] = { 0 };

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
            // 横移動
            if (keys[DIK_A]) posX -= playerSpeed;
            if (keys[DIK_D]) posX += playerSpeed;
            
            // ジャンプ処理
            if (!isJumping && keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
                isJumping = true;
                jumpVelocity = -jumpPower; // 上方向の速度をセット
            }

            // ジャンプ中の動き
            if (isJumping) {
                posY += jumpVelocity; // Y座標を更新
                jumpVelocity += gravity; // 重力を適用

                // 地面に着地したら停止
                if (posY >= groundHeight - sizeY) {
                    posY = groundHeight - sizeY;
                    isJumping = false;
                    jumpVelocity = 0;
                }
            }

            // 自機の位置を過去に保存
            playerTrail.push_back(Position{ (float)posX, (float)posY, 1.0f });  // 初期透明度は1.0
            if (playerTrail.size() > MAX_TRAIL_LENGTH) {
                playerTrail.erase(playerTrail.begin());  // 古い位置を削除
            }

            // 透明度をイージングで減少させる
            for (int i = 0; i < playerTrail.size(); ++i) {
                playerTrail[i].alpha = EaseOut(static_cast<float>(i) / static_cast<float>(playerTrail.size()));
            }

            // 透明度をイージングで減少させる
            for (int i = 0; i < playerTrail.size(); ++i) {
                playerTrail[i].alpha = EaseOut(static_cast<float>(i) / static_cast<float>(playerTrail.size()));
            }

            // ENTERキーが押された時の処理
            if (preKeys[DIK_RETURN] == 0 && keys[DIK_RETURN] != 0) {
                // 最初の発射
                LaunchCircles(static_cast<float>(bossPosX) + bossSizeX / 2, static_cast<float>(bossPosY) + bossSizeY / 2);
                bossCircularAttackFlag = true;
                bossCircularAttackTimer = 0; // タイマーリセット
                isFirstLaunch = false; // 初回発射フラグを無効にする
            }

            if (bossCircularAttackFlag == true) {
                bossCircularAttackTimer++; // タイマー進行

                // 最初に弾を発射してから10秒経過したら再度発射
                if (bossCircularAttackTimer >= bossCircularAttackCooldown) {
                    LaunchCircles(static_cast<float>(bossPosX) + bossSizeX / 2, static_cast<float>(bossPosY) + bossSizeY / 2);
                    bossCircularAttackTimer = 0; // タイマーリセット
                }
            }

            if (bossCircularAttackTimer >= 1400) { // 1400フレーム後に攻撃を終了
                bossCircularAttackFlag = false;
                bossCircularAttackTimer = 0;
            }

            // 飛んでいる円の位置を更新
            for (auto& circle : circles) {
                if (circle.active) {
                    circle.x += circle.vx;
                    circle.y += circle.vy;

                    // ウィンドウ外に出たら非アクティブにする
                    if (circle.x < 0 || circle.x > GetSystemMetrics(SM_CXSCREEN) || circle.y < 0 || circle.y > GetSystemMetrics(SM_CYSCREEN)) {
                        circle.active = false;
                    }
                }
            }

            // クールダウンのタイマーを更新
            bossBeamCooldown++;

            if (bossBeamCooldown > 200) {
                // 600フレーム経過したら、ランダムでビームを選択
                randomBeamIndex = rand() % 3;  // 0から6の間でランダムに選択
                bossBeamCooldown = 0; // クールダウンリセット
            }
            // ボスの座標
            bossCenterX = static_cast<float>(bossPosX) + static_cast<float>(bossSizeX) / 2.0f;
            bossCenterY = static_cast<float>(bossPosY) + static_cast<float>(bossSizeY) / 2.0f;
            if (randomBeamIndex != -1) {
                // ランダムで選ばれたビームを発射する処理
                switch (randomBeamIndex) {
                case 0:
                    // ビーム1の発射処理
                    xBeamFlag = true;
                    break;
                case 1:
                    // ビーム2の発射処理
                    xBeamFlag2 = true;
                    break;
                case 2:
                    // ビーム3の発射処理
                    yBeamFlag = true;
                    break;
                }
            }

            if (xBeamFlag) {
                beamTimer[0]++;
            }
            if (xBeamFlag2) {
                beamTimer[1]++;
            }
            if (yBeamFlag) {
                beamTimer[2]++;
            }

            // ビームの進行処理
            if (xBeamFlag && beamTimer[0] > 60) {
                goalLineX[0] += 40; goalLineX[1] += 40;
            }
            // ビームの進行処理
            if (xBeamFlag2 && beamTimer[1] > 60) {
                goalLineX[2] -= 40; goalLineX[3] -= 40;
            }
            if (yBeamFlag && beamTimer[2] > 60) {
                goalLineY[4] += 40;
                goalLineY[5] += 40;
                goalLineY[6] += 40;
            }

            // 端の位置制限
            if (goalLineX[0] >= 800) { goalLineX[0] = 1600; startLineX[0] += 50; }
            if (goalLineX[1] >= 800) { goalLineX[1] = 1600; startLineX[1] += 50; }
            if (goalLineX[2] <= 600) { goalLineX[2] = -120; startLineX[2] -= 50; }
            if (goalLineX[3] <= 600) { goalLineX[3] = -120; startLineX[3] -= 50; }
            if (goalLineY[4] >= 700) { goalLineY[4] = 940; startLineY[4] += 50; }
            if (goalLineY[5] >= 700) { goalLineY[5] = 940; startLineY[5] += 50; }
            if (goalLineY[6] >= 700) { goalLineY[6] = 940; startLineY[6] += 50; }

            if (startLineX[0] >= 1400) {
                startLineX[0] = 1400;
                xBeamFlagInProgress = true;
                randomBeamIndex = -1;
                beamTimer[0] = 0;
            }
            if (startLineX[1] >= 1400) {
                startLineX[1] = 1400;
                xBeamFlagInProgress = true;
                randomBeamIndex = -1;
                beamTimer[0] = 0;
            }
            if (startLineX[2] <= 80) {
                startLineX[2] = 80;
                xBeamFlag2InProgress = true;
                randomBeamIndex = -1;
                beamTimer[1] = 0;
            }
            if (startLineX[3] <= 80) {
                startLineX[3] = 80;
                xBeamFlag2InProgress = true;
                randomBeamIndex = -1;
                beamTimer[1] = 0;
            }
            if (startLineY[4] >= 740) {
                startLineY[4] = 740;
                yBeamFlagInProgress = true;
                randomBeamIndex = -1;
                beamTimer[2] = 0;
            }
            if (startLineY[5] >= 740) {
                startLineY[5] = 740;
                yBeamFlagInProgress = true;
                randomBeamIndex = -1;
                beamTimer[2] = 0;
            }
            if (startLineY[6] >= 740) {
                startLineY[6] = 740;
                yBeamFlagInProgress = true;
                randomBeamIndex = -1;
                beamTimer[2] = 0;
            }
            // パーティクル生成
            if (rand() % 5 == 0) { // 毎フレームではなく間引いて生成
                GenerateParticle(bossCenterX, bossCenterY);
            }

            // パーティクル更新
            UpdateParticles();
            UpdateBlackParticles();
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
                ShootBullets(bossPosX, bossPosY, posX, posY, bossSizeX); // ボスから弾を発射
            }
            MoveBullets(posY, posX, sizeX, sizeY); // 弾を移動
            //敵の近接攻撃のクールダウン
            if (bossAttackCooldownTime > 0) {
                bossAttackCooldownTime--;
            }

            bossTeleportTimer++;
            if (bossTeleportTimer > 600 + randTimer) {
                bossPosX = rand() % (1200 - 300);  // ボスの幅200を考慮して位置を決定
                bossPosY = rand() % (500 - 250); // ボスの高さ200を考慮して位置を決定
                randTimer = rand() % (1000 - 100);
                bossTeleportTimer = 0;
            }

            ///////////////////////////////////////////////////////////
            // リセット処理
            if (xBeamFlagInProgress == true || xBeamFlag2InProgress == true || yBeamFlagInProgress == true) {
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

                xBeamFlagInProgress = false;
                xBeamFlag2InProgress = false;
                yBeamFlagInProgress = false;
            }

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

            if (playerHP <= 0) {
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
            Novice::DrawSprite(0, 0, title, 1.2f, 1.2f, 0.0f, WHITE);
            break;
        case GAME:
            //背景描画
            Novice::DrawSprite(0, 0, stageBackGround, 1.2f, 1.2f, 0.0f, WHITE);
            // パーティクル描画
            DrawParticles();
            DrawBlackParticles();
            // 地面の描画
            Novice::DrawBox(0, 600, 1600, 200, 0.0f, 0xb8860b, kFillModeSolid);
            
            //プレイヤーのHPゲージ(仮置き)
            Novice::DrawSprite(100, 750, playerGauge, 1.0f, 1.0f, 0.0f, WHITE);

            // ゲーム内でのビームと自機の衝突判定
            if (playerColor == RED) {
                playerColor = WHITE;
            }
            for (int i = 0; i < 7; ++i) {
                if (CheckBeamCollisionWithPlayer(posX, posY, sizeX, sizeY, startLineX[i], startLineY[i], goalLineX[i], goalLineY[i])) {
                    playerHP -= 10;  // 自機がビームに当たった場合のダメージ
                    //Novice::DrawBox(posX, posY, sizeX, sizeY, 0.0f, RED, kFillModeSolid); // 赤色で自機を描画してダメージを表示
                    playerColor = RED;
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
                    Novice::DrawSprite(bossPosX, bossPosY, bossImage, 1.0f, 1.0f, 0.0f, bossColor);
                }
                if (bossImageChange == 1) {
                    Novice::DrawSprite(bossPosX, bossPosY, catBossImage, 1.0f, 1.0f, 0.0f, bossColor);
                }
                //ボスゲージ（仮置き）
                Novice::DrawSprite(400, 100, bossGauge, 1.0f, 1.0f, 0.0f, WHITE);
            }

            // 飛んでいる円を描画
            for (const auto& circle : circles) {
                if (circle.active) {
                    Novice::DrawEllipse(static_cast<int>(circle.x),
                        static_cast<int>(circle.y), 16, 16, 0.0f, BLACK, kFillModeSolid);
                    Novice::DrawSprite(static_cast<int>(circle.x) - 16,
                        static_cast<int>(circle.y) - 16, blackBall, 1,1,.0f,WHITE);
                }
            }
            // 敵のホーミング弾の描画
            for (int i = 0; i < numOfBullets; ++i) {
                if (bulletActive[i]) {
                    //Novice::DrawEllipse(int(bulletPosX[i]), int(bulletPosY[i]), 20, 20, 0.0f, RED, kFillModeSolid); // 弾の描画
                    homingBulletTimer++;
                    if (homingBulletTimer <= 20) {
                        Novice::DrawSprite(int(bulletPosX[i]) - 15, int(bulletPosY[i]) - 15, homingBullet1, 1.0f, 1.0f, 0.0f, WHITE);
                    }
                    if (homingBulletTimer > 20 && homingBulletTimer <= 40) {
                        Novice::DrawSprite(int(bulletPosX[i]) - 15, int(bulletPosY[i]) - 15, homingBullet2, 1.0f, 1.0f, 0.0f, WHITE);
                    }
                    if (homingBulletTimer > 40 && homingBulletTimer < 60) {
                        Novice::DrawSprite(int(bulletPosX[i]) - 15, int(bulletPosY[i]) - 15, homingBullet3, 1.0f, 1.0f, 0.0f, WHITE);
                    }
                    if (homingBulletTimer >= 60) {
                        homingBulletTimer = 0;
                    }
                }
            }
            if (xBeamFlag && beamTimer[0] < 60) {
                Novice::DrawBox(beamPosX[0], beamPosY[0], beamSize, beamSize, 0.0f, RED, kFillModeSolid);
                Novice::DrawBox(beamPosX[1], beamPosY[1], beamSize, beamSize, 0.0f, RED, kFillModeSolid);
            }
            if (xBeamFlag2 && beamTimer[1] < 60) {
                Novice::DrawBox(beamPosX[2] - beamSize - beamSize, beamPosY[2] - beamSize / 2, beamSize, beamSize, 0.0f, RED, kFillModeSolid);
                Novice::DrawBox(beamPosX[3] - beamSize - beamSize, beamPosY[3] - beamSize / 2, beamSize, beamSize, 0.0f, RED, kFillModeSolid);
            }
            if (yBeamFlag && beamTimer[2] < 60) {
                Novice::DrawBox(beamPosX[4] - beamSize / 2, beamPosY[4], beamSize, beamSize, 0.0f, RED, kFillModeSolid);
                Novice::DrawBox(beamPosX[5] - beamSize / 2, beamPosY[5], beamSize, beamSize, 0.0f, RED, kFillModeSolid);
                Novice::DrawBox(beamPosX[6] - beamSize / 2, beamPosY[6], beamSize, beamSize, 0.0f, RED, kFillModeSolid);
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

                if (xBeamFlag == true && i == 0 && beamTimer[0] > 60) {
                    // スピードをフレームごとに増加させる
                    rightTopX += beamSpeed[0];
                    rightBottomX += beamSpeed[0];

                    //スピードを増加
                    beamSpeed[0] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (xBeamFlag == true && i == 1 && beamTimer[0] > 60) {
                    // スピードをフレームごとに増加させる
                    rightTopX += beamSpeed[1];
                    rightBottomX += beamSpeed[1];

                    //スピードを増加
                    beamSpeed[1] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (xBeamFlag2 == true && i == 2 && beamTimer[1] > 60) {
                    // スピードをフレームごとに増加させる
                    leftTopX += beamSpeed[2];
                    leftBottomX += beamSpeed[2];

                    //スピードを増加
                    beamSpeed[2] -= 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (xBeamFlag2 == true && i == 3 && beamTimer[1] > 60) {
                    // スピードをフレームごとに増加させる
                    leftTopX += beamSpeed[3];
                    leftBottomX += beamSpeed[3];

                    //スピードを増加
                    beamSpeed[3] -= 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (yBeamFlag == true && i == 4 && beamTimer[2] > 60) {
                    // スピードをフレームごとに増加させる
                    rightBottomY += beamSpeed[4];
                    leftBottomY += beamSpeed[4];

                    //スピードを増加
                    beamSpeed[4] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (yBeamFlag == true && i == 5 && beamTimer[2] > 60) {
                    // スピードをフレームごとに増加させる
                    rightBottomY += beamSpeed[5];
                    leftBottomY += beamSpeed[5];

                    //スピードを増加
                    beamSpeed[5] += 40.0f;  // 速度を増加させる（増加する量は調整可能）
                }
                if (yBeamFlag == true && i == 6 && beamTimer[2] > 60) {
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

            //DrawBeams(startLineX, startLineY, goalLineX, goalLineY, BLACK);

            bossColor = WHITE;

            // 自機の残像を描画（透明度を適用）
            for (int i = 0; i < playerTrail.size(); ++i) {
                Novice::DrawBox((int)playerTrail[i].x, (int)playerTrail[i].y, sizeX, sizeY, 0.0f, 0x98fb98, kFillModeSolid);
            }

            //自機の攻撃
            if (Novice::IsTriggerMouse(0)) {
                DrawSlash(posX + sizeX / 2, posY + sizeY / 2, mouseX, mouseY, WHITE, 60.0f, bossPosX, bossPosY, bossSizeX, bossSizeY);
                Novice::PlayAudio(slashSounds, 0, 0.5f);
            }

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
            if (!keys[DIK_A] && !keys[DIK_D]) {
                if (isTurnLeft == 1) {
                    Novice::DrawSprite(posX, posY, playerImage[6], 1.0f, 1.0f, 0.0f, playerColor);
                }
                if (isTurnRight == 1) {
                    Novice::DrawSprite(posX + sizeX, posY, playerImage[6], -1.0f, 1.0f, 0.0f, playerColor);
                }
            }
            if (keys[DIK_A] && keys[DIK_D]) {
                if (isTurnLeft == 1) {
                    Novice::DrawSprite(posX, posY, playerImage[6], 1.0f, 1.0f, 0.0f, playerColor);
                }
                if (isTurnRight == 1) {
                    Novice::DrawSprite(posX + sizeX, posY, playerImage[6], -1.0f, 1.0f, 0.0f, playerColor);
                }
            }

            Novice::ScreenPrintf(20, 20, "bossHP : %d", bossHP);
            Novice::ScreenPrintf(20, 40, "playerHP : %d", playerHP);
            Novice::ScreenPrintf(20, 100, "beamTimer : %d",bossBeamCooldown);
            Novice::ScreenPrintf(20, 60, "%d", bossAttackCoolTime);
            Novice::ScreenPrintf(20, 80, "%d", bossImageChange);
            break;
        case GAME_CLEAR:
            break;
        case GAME_OVER:
            Novice::DrawSprite(0, 0, gameover, 1.2f, 1.2f, 0.0f, WHITE);
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

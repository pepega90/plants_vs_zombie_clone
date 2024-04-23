#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

const int WIDTH = 840;
const int HEIGHT = 640;
const int col = 6;
const int row = 4;
int grid[row][col];
int rectSize = 120;

enum TypePlant
{
    SHOOTER,
    SUN
};

struct Card
{
    Texture2D img;
    Vector2 pos;
    int price;
    std::string nama;
    Rectangle rect;

    void Update(int &playerMoney)
    {
        rect.x = pos.x;
        rect.y = pos.y;
        rect.width = img.width;
        rect.height = img.height;
        DrawTextureV(img, pos, playerMoney < price ? DARKGRAY : WHITE);
        // DrawRectangleLinesEx(rect, 2.0, YELLOW);
    }
};

struct Bullet
{
    Texture2D img;
    Vector2 pos;
    int radius;
};

struct Sun
{
    Texture2D img;
    Vector2 pos;
    int radius;
    int val;
    bool get = false;
};

struct Zombie
{
    enum Anim
    {
        WALK,
        EAT,
        DEAD
    };

    std::vector<Texture2D> img;
    std::vector<Texture2D> eatImg;
    std::vector<Texture2D> deadImg;
    Vector2 pos;
    Rectangle rect;
    int row;
    float speed;
    int health = 5;
    bool alive = true;
    bool isEating = false;

    // animation
    Anim currentAnimation = Anim::WALK;
    int currentFrame = 0;   // variabel untuk current frame gambar animasi
    float frameDelay;       // variabel untuk delay animasi
    float frameTimer = 0.0; // variabel untuk timer animasi

    void Update()
    {
        rect.x = pos.x + img[currentFrame].width / 2;
        rect.y = pos.y + img[currentFrame].height / 2;
        rect.width = img[currentFrame].width / 2;
        rect.height = img[currentFrame].height / 2;
    }

    void Draw(float &deltaTime, std::vector<Zombie> &listZombies)
    {
        switch (currentAnimation)
        {
        case WALK:
            frameTimer += deltaTime;
            if (frameTimer > 0.05)
            {
                currentFrame = (1 + currentFrame) % img.size();
                frameTimer = 0.0;
            }
            DrawTextureV(img[currentFrame], pos, WHITE);
            break;
        case EAT:
            frameTimer += deltaTime;
            if (frameTimer > 0.05)
            {
                currentFrame = (1 + currentFrame) % eatImg.size();
                frameTimer = 0.0;
            }
            DrawTextureV(eatImg[currentFrame], pos, WHITE);
            break;
        case DEAD:
            frameTimer += deltaTime;
            if (frameTimer > 0.05)
            {
                currentFrame = (1 + currentFrame) % deadImg.size();
                if (!alive && currentFrame == deadImg.size() - 1)
                {
                    auto it = std::find_if(listZombies.begin(), listZombies.end(), [this](const Zombie &z)
                                           { return &z == this; });
                    if (it != listZombies.end())
                    {
                        listZombies.erase(it);
                    }
                }
                frameTimer = 0.0;
            }
            DrawTextureV(deadImg[currentFrame], pos, WHITE);
            break;
        }
        // DrawText(std::to_string(health).c_str(), pos.x + 30, pos.y, 20, WHITE);
        // DrawRectangleLinesEx(rect, 3.0, RED);
    }
};

struct Plant
{
    std::vector<Texture2D> img;
    std::vector<Sun> suns;
    std::vector<Bullet> bullets;
    Vector2 pos;
    int price;
    std::string nama;
    Rectangle rect;
    bool place = false;
    TypePlant tipe;

    // health
    int health = 3;
    float lastHealth = GetTime();
    float countHealth = 3;

    // counter for sun
    int countSun = 10;
    float lastSun = GetTime();

    // counter for shoot
    int countShoot = 3;
    float lastShoot = GetTime();

    // animation
    bool play = false;      // indikasi apakah animasi sedang di play atau tidak
    int currentFrame = 0;   // variabel untuk current frame gambar animasi
    float frameDelay;       // variabel untuk delay animasi
    float frameTimer = 0.0; // variabel untuk timer animasi

    void Update(Texture2D &sunImg, Texture2D &bulletImg, std::vector<Zombie> &listZombie, Vector2 &mousePos, int &playerMoney)
    {
        if (place)
        {
            rect.x = pos.x;
            rect.y = pos.y;
        }
        rect.width = img[currentFrame].width;
        rect.height = img[currentFrame].height;
        if (tipe == SHOOTER)
        {

            bool zombieInFront = false;

            for (auto &zombie : listZombie)
            {
                // if (pos.x < zombie.pos.x && static_cast<int>((pos.y - 110) / rectSize) == zombie.row)
                if (static_cast<int>((pos.y - 110) / rectSize) == zombie.row)
                {
                    zombieInFront = true;
                    break;
                }
            }

            if (zombieInFront && GetTime() - lastShoot > 0.7)
            {
                play = true;
                countShoot -= 1;
                lastShoot = GetTime();
            }

            if (countShoot == 0 && zombieInFront)
            {
                Bullet b;
                b.img = bulletImg;
                b.pos = Vector2(pos.x + img[currentFrame].width / 2, pos.y);
                b.radius = 14;
                bullets.push_back(b);
                play = false;
                countShoot = 3;
            }

            for (int i = 0; i < bullets.size(); i++)
            {
                bullets[i].pos.x += 6;
            }

            for (int i = 0; i < listZombie.size();)
            {
                bool hapus = false;
                for (int j = 0; j < bullets.size(); j++)
                {
                    if (CheckCollisionCircleRec(Vector2(bullets[j].pos.x + bullets[j].radius, bullets[j].pos.y + bullets[j].radius), bullets[j].radius, listZombie[i].rect) && listZombie[i].health > 0)
                    {
                        listZombie[i].health -= 1;
                        bullets.erase(bullets.begin() + j);
                        hapus = true;
                        break;
                    }
                }
                if (!hapus)
                    ++i;
            }
        }
        else
        {
            if (GetTime() - lastSun > 0.7 && suns.size() < 1)
            {
                countSun -= 1;
                lastSun = GetTime();
            }

            if (countSun == 0)
            {
                Sun s;
                s.img = sunImg;
                s.pos = Vector2(pos.x + 10, pos.y + 10);
                s.radius = 20;
                s.val = 50;
                suns.push_back(s);
                countSun = 10;
            }

            for (int i = 0; i < suns.size(); i++)
            {
                if (CheckCollisionPointCircle(mousePos, Vector2(suns[i].pos.x + suns[i].radius * 2, suns[i].pos.y + suns[i].radius * 2), suns[i].radius))
                {
                    suns[i].get = true;
                }
                if (suns[i].get)
                {
                    Vector2 dir = (Vector2(17, 5) - suns[i].pos).Normalize();
                    suns[i].pos += dir * 10;
                }
            }

            for (int i = 0; i < suns.size(); i++)
            {
                if (suns[i].get)
                {
                    if (CheckCollisionPointCircle(Vector2(17, 5), suns[i].pos, suns[i].radius))
                    {
                        playerMoney += suns[i].val;
                        suns.erase(suns.begin() + i);
                    }
                }
            }
        }
    }

    void Draw(float &deltaTime)
    {
        frameTimer += deltaTime;
        if (frameTimer >= frameDelay && play)
        {
            currentFrame = (1 + currentFrame) % img.size();
            frameTimer = 0.0;
        }
        if (tipe == SHOOTER)
        {
            // draw bullet
            for (int i = 0; i < bullets.size(); i++)
            {
                DrawTextureV(bullets[i].img, bullets[i].pos, WHITE);
                // DrawCircleLines(bullets[i].pos.x + bullets[i].radius, bullets[i].pos.y + bullets[i].radius, bullets[i].radius, PURPLE);
            }
            DrawTextureV(img[currentFrame], pos, WHITE);
            // DrawRectangleLinesEx(rect, 3.0, RED);
        }
        else
        {
            // draw sun
            DrawTextureV(img[currentFrame], pos, WHITE);
            // DrawRectangleLinesEx(rect, 3.0, RED);
            for (int i = 0; i < suns.size(); i++)
            {
                DrawTextureV(suns[i].img, suns[i].pos, WHITE);
                // DrawCircleLines(suns[i].pos.x + suns[i].radius * 2, suns[i].pos.y + suns[i].radius * 2, suns[i].radius, RED);
            }
        }
        // DrawText(std::to_string(health).c_str(), pos.x, pos.y - 15, 30, WHITE);
    }
};

void loadVectorImage(int start, int end, std::string path, std::vector<Texture> &textures, float scale = 1);

int main()
{
    // Initialization
    InitWindow(WIDTH, HEIGHT, "Plant V Zombie");
    SetTargetFPS(60);

    // load image
    Texture2D bg = LoadTexture("images/mainBG.png");
    Texture2D sunCard = LoadTexture("images/cards/card_sunflower.png");
    Texture2D peaCard = LoadTexture("images/cards/card_peashooter.png");
    Texture2D peaBullet = LoadTexture("images/pea.png");
    Texture2D sunImg = LoadTexture("images/sun.png");

    // list gambar matahari
    std::vector<Texture2D> sunImgs;
    std::vector<Texture2D> peaImgs;
    float peaScale = 0.8;

    // init vector sun images
    loadVectorImage(7, 42, "images/matahari/", sunImgs);
    // init vector pea images
    loadVectorImage(7, 24, "images/pea/", peaImgs, peaScale);

    // list gambar zombie idle
    std::vector<Texture2D> zombieIdleImage;
    std::vector<Texture2D> zombieEatImage;
    std::vector<Texture2D> zombieDeadImage;
    loadVectorImage(0, 47, "images/zombies/idle/", zombieIdleImage);
    loadVectorImage(0, 40, "images/zombies/eat/", zombieEatImage);
    loadVectorImage(0, 39, "images/zombies/dead/", zombieDeadImage);

    std::vector<Zombie> listZombies;
    for (int i = 0; i < 1; i++)
    {
        Zombie zmb;
        zmb.img = zombieIdleImage;
        zmb.eatImg = zombieEatImage;
        zmb.deadImg = zombieDeadImage;
        // zmb.speed = 1.5;
        // zmb.speed = GetRandomValue(1, 1.5);
        zmb.speed = .4;
        zmb.pos = Vector2(WIDTH - 50, 50 + i * 120);
        zmb.row = 0;
        listZombies.push_back(zmb);
    }

    // init cards untuk setiap plant
    std::vector<Card> cards;
    int lengthCard = 2;
    for (int i = 0; i < lengthCard; i++)
    {
        Card c;
        if (i == 0)
        {
            c.img = sunCard;
            c.price = 50;
            c.nama = "sun";
        }
        else
        {
            c.img = peaCard;
            c.price = 100;
            c.nama = "pea";
        }
        c.pos = Vector2(110 + i * 70, 10);
        c.rect = Rectangle{c.pos.x, c.pos.y, (float)c.img.width, (float)c.img.height};
        cards.push_back(c);
    }

    // list berisikan plant apa saja yang sudah kita pilih
    std::vector<Plant> plants;
    std::vector<Sun> listSun;
    // offset untuk grid yang dibuat agak ke bawah dari tempat card plant
    int xGridOffset = 40;
    int yGridOffset = 110;
    // posisi mouse
    Vector2 mousePos;
    // cell x dan y untuk menaruh plants di grid
    int cellX;
    int cellY;
    int barisZombie[] = {0, 1, 2, 3};
    int playerMoney = 50;
    int countListSun = 3;
    float lastListSun = GetTime();
    int jumlahZombie = 3;

    // Main game loop
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Draw
        ClearBackground(RAYWHITE);

        // mendapatkan current mouse position
        mousePos = GetMousePosition();

        if (IsKeyPressed(KEY_SPACE))
            plants.pop_back();

        // ketika mouse di klik, kita kalkulasi untuk mendapatkan posisi kolom dan baris di grid
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            cellX = (mousePos.x - xGridOffset) / rectSize;
            cellY = (mousePos.y - yGridOffset) / rectSize;
        }

        // check collision mouse dengan card, dan jika kita klik card maka akan push plant ke vector plants
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            for (auto &c : cards)
            {
                if (CheckCollisionPointRec(mousePos, c.rect))
                {
                    if (c.nama == "sun")
                    {
                        Plant p;
                        p.tipe = TypePlant::SUN;
                        p.img = sunImgs;
                        p.price = 50;
                        p.pos = Vector2(mousePos.x, mousePos.y);
                        p.frameDelay = 0.05f;
                        p.rect = Rectangle{p.pos.x, p.pos.y, (float)p.img[p.currentFrame].width, (float)p.img[p.currentFrame].height};
                        plants.push_back(p);
                    }
                    else
                    {
                        Plant p;
                        p.tipe = TypePlant::SHOOTER;
                        p.img = peaImgs;
                        p.price = 100;
                        p.frameDelay = 0.09;
                        p.pos = Vector2(mousePos.x, mousePos.y);
                        p.rect = Rectangle{p.pos.x, p.pos.y, (float)p.img[p.currentFrame].width * peaScale, (float)p.img[p.currentFrame].height * peaScale};
                        plants.push_back(p);
                    }
                    // break;
                }
            }
        }

        // mekanik untuk drag plant yang sudah kita push ke vector plants, disini kita menggunakan method back() dari std::vector untuk mendapatkan value yang paling akhir, value yang di push saat kita pilih card
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && plants.back().place == false)
        {
            // if (plants.size() < 1)
            // {
            plants.back().pos.x = mousePos.x - plants.back().rect.width / 2;
            plants.back().pos.y = mousePos.y - plants.back().rect.height / 2;
            // }
        }

        // saat kita release mouse klik kiri, maka kita cek jika di grid dengan cell x dan y berikut bernilai 0, artinya belum ada plant yang menempati tempat grid tersebut, maka kita buah valuenya menjadi 1, artinya sudah terisi. jika kita menaruh plant di tempat yang sudah terisi 1 di grid, akan kita pop_back() kita hapus
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            if (plants.back().place == false && grid[cellY][cellX] == 0)
            {
                grid[cellY][cellX] = 1;

                if (plants.size() > 0)
                {
                    if (cellX >= 0 && cellX < col && cellY >= 0 && cellY < row)
                    {
                        // jika kita taruh plant, makan mainkan animasi
                        plants.back().place = true;
                        plants.back().play = true;
                        plants.back().pos.x = cellX * rectSize + xGridOffset;
                        plants.back().pos.y = cellY * rectSize + yGridOffset;
                        playerMoney -= plants.back().price;
                    }
                }
            }
            else if (!plants.back().place && grid[cellY][cellX] == 1)
            {
                plants.pop_back();
            }
        }

        if (GetTime() - lastListSun > 0.7 && listSun.size() < 2)
        {
            countListSun -= 1;
            lastListSun = GetTime();
        }

        if (countListSun == 0)
        {
            Sun s;
            s.img = sunImg;
            s.pos = Vector2(GetRandomValue(10, WIDTH - sunImg.width), -sunImg.height);
            s.radius = 20;
            s.val = 25;
            listSun.push_back(s);
            countListSun = 3;
        }

        for (int i = 0; i < listSun.size(); i++)
        {
            if (CheckCollisionPointCircle(mousePos, Vector2(listSun[i].pos.x + listSun[i].radius * 2, listSun[i].pos.y + listSun[i].radius * 2), listSun[i].radius) || listSun[i].pos.y > HEIGHT - listSun[i].img.height)
            {
                listSun[i].get = true;
            }
            if (listSun[i].get)
            {
                Vector2 dir = (Vector2(17, 5) - listSun[i].pos).Normalize();
                listSun[i].pos += dir * 10;
            }
        }

        for (int i = 0; i < listSun.size(); i++)
        {
            if (listSun[i].get)
            {
                if (CheckCollisionPointCircle(Vector2(17, 5), listSun[i].pos, listSun[i].radius))
                {
                    playerMoney += listSun[i].val;
                    listSun.erase(listSun.begin() + i);
                }
            }
        }

        if (playerMoney > 500)
            jumlahZombie = 8;

        std::cout << jumlahZombie << std::endl;

        while (listZombies.size() < jumlahZombie)
        {
            Zombie zmb;
            zmb.img = zombieIdleImage;
            zmb.eatImg = zombieEatImage;
            zmb.deadImg = zombieDeadImage;
            // zmb.speed = 1.5;
            // zmb.speed = GetRandomValue(1, 1.5);
            zmb.speed = .4;
            float randomY = barisZombie[GetRandomValue(0, 3)];
            zmb.row = randomY;
            zmb.pos = Vector2(WIDTH, 50 + randomY * 120);
            listZombies.push_back(zmb);
        }

        for (int i = 0; i < listZombies.size(); i++)
        {
            listZombies[i].Update();

            if (listZombies[i].currentAnimation == Zombie::Anim::WALK)
                listZombies[i].pos.x -= listZombies[i].speed;

            if (listZombies[i].isEating)
                listZombies[i].currentAnimation = Zombie::Anim::EAT;

            if (listZombies[i].health <= 0)
            {
                listZombies[i].currentAnimation = Zombie::Anim::DEAD;
                listZombies[i].alive = false;
            }

            if (listZombies[i].pos.x < -listZombies[i].rect.width - 100)
            {
                listZombies.erase(listZombies.begin() + i);
            }
        }

        // update plants
        for (int i = 0; i < plants.size(); i++)
        {
            plants[i].Update(sunImg, peaBullet, listZombies, mousePos, playerMoney);
        }

        // check if zombie collide plant, change zombie animation to eating
        // if (listZombies.size() > 1 && plants.size() > 1)
        // {
        for (int i = 0; i < listZombies.size(); i++)
        {
            bool isEating = false;
            for (int j = 0; j < plants.size(); j++)
            {
                if (listZombies[i].alive && CheckCollisionRecs(listZombies[i].rect, plants[j].rect))
                {
                    if (GetTime() - plants[j].lastHealth > 0.7)
                    {
                        plants[j].countHealth -= 1;
                        plants[j].lastHealth = GetTime();
                    }
                    if (plants[j].countHealth == 0)
                    {
                        plants[j].health -= 1;
                        plants[j].countHealth = 3;
                    }

                    if (plants[j].health <= 0)
                    {
                        grid[static_cast<int>((plants[j].pos.y - yGridOffset) / rectSize)][static_cast<int>((plants[j].pos.x - xGridOffset) / rectSize)] = 0;
                        plants.erase(plants.begin() + j);
                        listZombies[i].isEating = false;
                        listZombies[i].currentAnimation = Zombie::Anim::WALK;
                        --j; // Adjust index after erasing element
                    }
                    isEating = true;
                    listZombies[i].currentAnimation = Zombie::Anim::EAT;
                    break;
                }
            }

            if (listZombies[i].alive && !isEating)
                listZombies[i].currentAnimation = Zombie::Anim::WALK;
        }
        // }

        // if zombie collide with plants, then decrease the plant health every 1 seconds
        // for (int i = 0; i < listZombies.size(); i++)
        // {
        //     bool isEating = false;
        //     for (int j = 0; j < plants.size(); j++)
        //     {
        //         if (CheckCollisionRecs(listZombies[i].rect, plants[j].rect))
        //         {
        //             listZombies[i].currentAnimation = Zombie::Anim::EAT;
        //             isEating = true;
        //             break;
        //         }
        //     }

        //     if (!isEating)
        //         listZombies[i].currentAnimation = Zombie::Anim::WALK;
        // }

        BeginDrawing();

        // draw background
        DrawTexture(bg, 0, 0, WHITE);

        DrawText("made by aji mustofa @pepega90", 45, HEIGHT - 30, 20, WHITE);

        // draw player money
        DrawText(std::to_string(playerMoney).c_str(), 47, 83, 20, BLACK);

        // draw cout for debug
        // DrawText(std::to_string(countdown).c_str(), 100, 100, 30, WHITE);

        // draw cards
        for (auto c : cards)
            c.Update(playerMoney);

        // draw plants
        for (int i = 0; i < plants.size(); i++)
        {
            plants[i].Draw(dt);
        }

        // for (int i = 0; i < listSun.size(); i++)
        // {
        //     DrawTextureV(listSun[i].img, listSun[i].pos, WHITE);
        // }

        // draw grid
        // for (int y = 0; y < row; y++)
        // {
        //     for (int x = 0; x < col; x++)
        //     {
        //         if (grid[y][x] != 1)
        //         {
        //             DrawRectangleLines(x * rectSize + xGridOffset, y * rectSize + yGridOffset, rectSize, rectSize, YELLOW);
        //         }
        //     }
        // }

        for (int i = 0; i < listZombies.size(); i++)
            listZombies[i].Draw(dt, listZombies);

        for (int i = 0; i < listSun.size(); i++)
        {
            listSun[i].pos.y += 2;
            DrawTextureV(listSun[i].img, listSun[i].pos, WHITE);
        }

        // show mouse position
        // std::string mouseStringPos = "X = " + std::to_string(GetMouseX()) + "\n" + "Y = " + std::to_string(GetMouseY());
        // DrawText(mouseStringPos.c_str(), GetMouseX(), GetMouseY(), 20, BLACK);
        EndDrawing();
    }

    // De-Initialization
    for (auto &sun : sunImgs)
        UnloadTexture(sun);
    for (auto &p : peaImgs)
        UnloadTexture(p);
    for (auto &p : zombieIdleImage)
        UnloadTexture(p);
    for (auto &p : zombieEatImage)
        UnloadTexture(p);
    for (auto &p : zombieDeadImage)
        UnloadTexture(p);
    UnloadTexture(sunImg);
    UnloadTexture(peaCard);
    UnloadTexture(sunCard);
    UnloadTexture(peaBullet);
    UnloadTexture(bg);
    CloseWindow();

    return 0;
}

void loadVectorImage(int start, int end, std::string path, std::vector<Texture> &textures, float scale)
{
    for (int i = start; i < end; i++)
    {
        std::string filename = path + std::to_string(i) + ".png";
        Texture2D z = LoadTexture(filename.c_str());
        z.width *= scale;
        z.height *= scale;
        textures.push_back(z);
    }
}

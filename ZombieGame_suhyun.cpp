#include "pch.h"
#include <stdio.h>
#include "tipsware.h"

#define MAX_IMAGE_COUNT     16
#define KNIGHT              p_app_data->knight
#define ZOMBIE              p_app_data->zombie

typedef struct AnimationImageData
{
    UINT16 image_count, index;
    void* p_image[MAX_IMAGE_COUNT];
} AID;

void SetImagePath(AID* ap_data, const char* ap_path, UINT16 a_count)
{
    ap_data->image_count = a_count;  
    ap_data->index = 0; 

    char temp_path[MAX_PATH];
    for (UINT16 i = 0; i < a_count; i++) 
    {
        sprintf_s(temp_path, MAX_PATH, "%s%02d.png", ap_path, i);
        ap_data->p_image[i] = LoadImageGP(temp_path);
    }
}

void DeleteImageList(AID* ap_data)
{
    for (UINT16 i = 0; i < ap_data->image_count; i++) DeleteImageGP(ap_data->p_image[i]);
}

UINT16 DrawImage(AID* ap_data, int a_x, int a_y, double a_cx_rate, double a_cy_rate)
{
    void* p_image = ap_data->p_image[ap_data->index];
    int cx = (int)(GetWidthGP(p_image) * a_cx_rate), cy = (int)(GetHeightGP(p_image) * a_cy_rate);
    DrawImageGP(p_image, a_x, a_y, cx, cy);

    return ap_data->index = (ap_data->index + 1) % ap_data->image_count;
}

// 기사 데이터
struct KnightData
{
    // 캐릭터의 상태값 (0:대기, 1: 앞으로(왼쪽) 걷기, 2: 뒤로(오른쪽) 걷기), 3:공격, 4: 점프, 5:공격당함)
    UINT8 state;
    short x;
    AID run_image, idle_image, hurt_image, hurt_effect_image, attack_image;
};

// 좀비 데이터
struct ZombieData
{
    // 캐릭터의 상태값 (0: 대기, 1: 앞으로(오른쪽) 걷기, 3: 공격, 5:공격당함)
    UINT8 state;
    short x;
    AID run_image, attack_image, hurt_image, hurt_effect_image;
};

struct AppData
{
    void* p_bk_image;   // 배경 
    KnightData knight;  // 기사 정보
    ZombieData zombie;  // 좀비 정보
};

TIMER RedrawTimer(NOT_USE_TIMER_DATA)
{
    AppData* p_app_data = (AppData*)GetAppData();
    Clear();

    if (KNIGHT.state == 1) {
        if (KNIGHT.x > 80 && (ZOMBIE.x + 110) < KNIGHT.x) KNIGHT.x -= 3;
    }
    else if (KNIGHT.state == 3) {
        if ((ZOMBIE.x + 110) >= KNIGHT.x) ZOMBIE.state = 5;
    }
    else if (KNIGHT.state == 2) {
        if (KNIGHT.x < 920) KNIGHT.x += 3;
    }

    if ((ZOMBIE.x + 110) < KNIGHT.x) 
    {
        if (ZOMBIE.state == 3) 
        {
            ZOMBIE.state = 1;  
            ZOMBIE.run_image.index = 0; 
        }
        ZOMBIE.x++;
    }
    else {
        if (ZOMBIE.state == 1 && KNIGHT.state != 3) 
        {
            ZOMBIE.state = 3;  // 공격 상태
            ZOMBIE.attack_image.index = 0; 
        }
    }

    if (ZOMBIE.state == 3 && KNIGHT.state != 5) 
    {
        KNIGHT.state = 5;  //기사가 피해를 입는 상태
        KNIGHT.hurt_image.index = 0;
    }

    DrawImageGP(p_app_data->p_bk_image, 0, 0); // 배경 이미지

    // 캐릭터의 상태에 따라 이미지 출력
    if (KNIGHT.state == 1 || KNIGHT.state == 2) DrawImage(&KNIGHT.run_image, KNIGHT.x, 320, 0.4, 0.4);
    else if (KNIGHT.state == 0) DrawImage(&KNIGHT.idle_image, KNIGHT.x, 320, 0.4, 0.4);
    else if (KNIGHT.state == 3) DrawImage(&KNIGHT.attack_image, KNIGHT.x, 320, 0.4, 0.4);
    else if (KNIGHT.state == 5) 
    {
        // 기사가 좀비의 공격을 받을때 피해 효과출력
        DrawImage(&KNIGHT.hurt_image, KNIGHT.x, 320, 0.4, 0.4);
        DrawImage(&KNIGHT.hurt_effect_image, KNIGHT.x + 40, 260, 1.0, 1.0);
    }

    // 좀비 상태에 따른 이미지를 출력한다.
    if (ZOMBIE.state == 1) DrawImage(&ZOMBIE.run_image, ZOMBIE.x, 353, 0.4, 0.4);
    else if (ZOMBIE.state == 3) DrawImage(&ZOMBIE.attack_image, ZOMBIE.x, 353, 0.4, 0.4);
    else if (ZOMBIE.state == 5) 
    {
        // 기사의 공격으로 피해를 입고 있다면 효과출력 
        DrawImage(&ZOMBIE.hurt_image, ZOMBIE.x, 353, 0.4, 0.4);
        DrawImage(&ZOMBIE.hurt_effect_image, ZOMBIE.x + 20, 260, 1.0, 1.0);
    }

    ShowDisplay();
}

void OnDestroy()
{
    KillTimer(1);
    AppData* p_app_data = (AppData*)GetAppData();

    // 이미지 모두 제거
    DeleteImageList(&KNIGHT.idle_image);
    DeleteImageList(&KNIGHT.run_image);
    DeleteImageList(&KNIGHT.hurt_image);
    DeleteImageList(&KNIGHT.hurt_effect_image);
    DeleteImageList(&KNIGHT.attack_image);
    DeleteImageList(&ZOMBIE.run_image);
    DeleteImageList(&ZOMBIE.attack_image);
    DeleteImageList(&ZOMBIE.hurt_image);
    DeleteImageList(&ZOMBIE.hurt_effect_image);
    DeleteImageGP(p_app_data->p_bk_image);
}

int WndMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN) 
    {
        if (wParam == VK_LEFT || wParam == VK_RIGHT) 
        {
            AppData* p_app_data = (AppData*)GetAppData();
            if (KNIGHT.state != 1 && KNIGHT.state != 2) 
            {
                if (wParam == VK_LEFT) KNIGHT.state = 1;
                else KNIGHT.state = 2;
                KNIGHT.run_image.index = 0;
            }
            return 1;
        }
        else if (wParam == 0x41) 
        { // A키
            AppData* p_app_data = (AppData*)GetAppData();
            if (KNIGHT.state != 3) 
            {
                KNIGHT.state = 3;
                KNIGHT.attack_image.index = 0;
            }
            return 1;
        }
    }
    else if (uMsg == WM_KEYUP) 
    {
        if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == 0x41) 
        { //걷기 혹은 A키 사용시 전환
            AppData* p_app_data = (AppData*)GetAppData();
            KNIGHT.state = 0;
            KNIGHT.idle_image.index = 0;
            if (wParam == 0x41) ZOMBIE.state = 1;
        }
    }

    return 0;
}


CMD_USER_MESSAGE(NULL, OnDestroy, WndMessage)

int main()
{
    ChangeWorkSize(1200, 540); // 작업영역 설정
    AppData app_data = { NULL, { 0, 920, }, { 1, 80, }, };

    app_data.p_bk_image = LoadImageGP(".\\image\\bk.png");

    // 기사 애니메이션 이미지
    SetImagePath(&app_data.knight.run_image, ".\\image\\run_", 8);
    SetImagePath(&app_data.knight.idle_image, ".\\image\\idle_", 8);
    SetImagePath(&app_data.knight.hurt_image, ".\\image\\hurt_", 8);
    SetImagePath(&app_data.knight.hurt_effect_image, ".\\image\\k_hurt_effect_", 8);
    SetImagePath(&app_data.knight.attack_image, ".\\image\\k_attack_", 8);
    // 좀비 애니메이션 이미지
    SetImagePath(&app_data.zombie.run_image, ".\\image\\z_run_", 8);
    SetImagePath(&app_data.zombie.attack_image, ".\\image\\z_attack_", 12);
    SetImagePath(&app_data.zombie.hurt_image, ".\\image\\z_hurt_", 8);
    SetImagePath(&app_data.zombie.hurt_effect_image, ".\\image\\z_hurt_effect_", 8);

    SetAppData(&app_data, sizeof(app_data));

    SetTimer(1, 100, RedrawTimer);

    ShowDisplay(); 
    return 0;
}
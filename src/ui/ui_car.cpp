#include "lvgl.h"
#include "ui_car.h"
#include <cmath>
#include <cstdio>

extern "C" {
    void onMusicPlay(lv_event_t *e);
    void onMusicPrevious(lv_event_t *e);
    void onMusicNext(lv_event_t *e);
}

#define LED_RADIUS 8
#define LED_SPACING 14
#define ARC_RADIUS 85

static bool navEnabled = true;
static bool shiftLightEnabled = true;
static int shiftLightRpm = 5000;

lv_obj_t *ui_rpmScreen = nullptr;
lv_obj_t *ui_musicScreen = nullptr;
lv_obj_t *ui_gpsScreen = nullptr;
lv_obj_t *ui_settingsScreen = nullptr;

lv_obj_t *ui_rpmLabel = nullptr;
lv_obj_t *ui_rpmArc = nullptr;
lv_obj_t *ui_tachLeds[MAX_LEDS] = {nullptr};
lv_obj_t *ui_shiftOverlay = nullptr;

lv_obj_t *ui_musicPrevBtn = nullptr;
lv_obj_t *ui_musicPlayBtn = nullptr;
lv_obj_t *ui_musicNextBtn = nullptr;

lv_obj_t *ui_gpsClockLabel = nullptr;
lv_obj_t *ui_gpsDirectionIcon = nullptr;
lv_obj_t *ui_gpsExitLabel = nullptr;

lv_obj_t *ui_settingsNavSwitch = nullptr;
lv_obj_t *ui_settingsShiftSwitch = nullptr;
lv_obj_t *ui_settingsShiftRpmDropdown = nullptr;

static lv_obj_t *currentScreen = nullptr;
static int currentScreenIndex = 0;

static const char *rpmOptions = "1000\n1500\n2000\n2500\n3000\n3500\n4000\n4500\n5000\n5500\n6000";

void ui_car_screen_init(void)
{
    ui_rpm_screen_init();
    ui_music_screen_init();
    ui_gps_screen_init();
    ui_car_settings_screen_init();

    currentScreen = ui_rpmScreen;
    lv_screen_load(ui_rpmScreen);
}

void ui_rpm_screen_init(void)
{
    ui_rpmScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_rpmScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_rpmScreen, lv_color_hex(0x000000), LV_PART_MAIN);

    ui_rpmLabel = lv_label_create(ui_rpmScreen);
    lv_label_set_text(ui_rpmLabel, "0");
    lv_obj_set_style_text_font(ui_rpmLabel, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_rpmLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(ui_rpmLabel, LV_ALIGN_CENTER, 0, 40);

    lv_obj_t *rpmUnit = lv_label_create(ui_rpmScreen);
    lv_label_set_text(rpmUnit, "RPM");
    lv_obj_set_style_text_font(rpmUnit, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(rpmUnit, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align(rpmUnit, LV_ALIGN_CENTER, 0, 70);

    ui_rpmArc = lv_arc_create(ui_rpmScreen);
    lv_arc_set_rotation(ui_rpmArc, 135);
    lv_arc_set_bg_angles(ui_rpmArc, 0, 270);
    lv_arc_set_value(ui_rpmArc, 0);
    lv_obj_set_size(ui_rpmArc, ARC_RADIUS * 2 + 20, ARC_RADIUS * 2 + 20);
    lv_obj_align(ui_rpmArc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(ui_rpmArc, LV_OBJ_FLAG_CLICKABLE);

    for (int i = 0; i < MAX_LEDS; i++)
    {
        float angle = 135.0f + (270.0f * i / (MAX_LEDS - 1));
        float rad = angle * 3.14159f / 180.0f;
        int x = 120 + (ARC_RADIUS + 15) * cos(rad);
        int y = 120 + (ARC_RADIUS + 15) * sin(rad);

        ui_tachLeds[i] = lv_obj_create(ui_rpmScreen);
        lv_obj_set_size(ui_tachLeds[i], LED_RADIUS * 2, LED_RADIUS * 2);
        lv_obj_set_pos(ui_tachLeds[i], x - LED_RADIUS, y - LED_RADIUS);
        lv_obj_set_style_radius(ui_tachLeds[i], LED_RADIUS, LV_PART_MAIN);
        lv_obj_set_style_bg_color(ui_tachLeds[i], lv_color_hex(0x111111), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_tachLeds[i], 255, LV_PART_MAIN);
        lv_obj_remove_flag(ui_tachLeds[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    ui_shiftOverlay = lv_obj_create(ui_rpmScreen);
    lv_obj_set_size(ui_shiftOverlay, 240, 240);
    lv_obj_set_pos(ui_shiftOverlay, 0, 0);
    lv_obj_set_style_bg_color(ui_shiftOverlay, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_shiftOverlay, 180, LV_PART_MAIN);
    lv_obj_add_flag(ui_shiftOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_shiftOverlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *tapLabel = lv_label_create(ui_rpmScreen);
    lv_label_set_text(tapLabel, "Tap to switch");
    lv_obj_set_style_text_font(tapLabel, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tapLabel, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_align(tapLabel, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_add_event_cb(ui_rpmScreen, car_ui_event_rpmScreen, LV_EVENT_ALL, NULL);
}

void ui_music_screen_init(void)
{
    ui_musicScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_musicScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_musicScreen, lv_color_hex(0x000000), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(ui_musicScreen);
    lv_label_set_text(title, "Music");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    ui_musicPrevBtn = lv_button_create(ui_musicScreen);
    lv_obj_set_size(ui_musicPrevBtn, 50, 50);
    lv_obj_set_pos(ui_musicPrevBtn, 30, 100);
    lv_obj_set_style_bg_color(ui_musicPrevBtn, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_t *prevIcon = lv_label_create(ui_musicPrevBtn);
    lv_label_set_text(prevIcon, LV_SYMBOL_PREV);
    lv_obj_set_style_text_color(prevIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(prevIcon);

    ui_musicPlayBtn = lv_button_create(ui_musicScreen);
    lv_obj_set_size(ui_musicPlayBtn, 60, 60);
    lv_obj_set_pos(ui_musicPlayBtn, 90, 90);
    lv_obj_set_style_bg_color(ui_musicPlayBtn, lv_color_hex(0x008800), LV_PART_MAIN);
    lv_obj_t *playIcon = lv_label_create(ui_musicPlayBtn);
    lv_label_set_text(playIcon, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(playIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(playIcon);

    ui_musicNextBtn = lv_button_create(ui_musicScreen);
    lv_obj_set_size(ui_musicNextBtn, 50, 50);
    lv_obj_set_pos(ui_musicNextBtn, 160, 100);
    lv_obj_set_style_bg_color(ui_musicNextBtn, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_t *nextIcon = lv_label_create(ui_musicNextBtn);
    lv_label_set_text(nextIcon, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_color(nextIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(nextIcon);

    lv_obj_t *tapLabel = lv_label_create(ui_musicScreen);
    lv_label_set_text(tapLabel, "Tap to switch");
    lv_obj_set_style_text_font(tapLabel, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tapLabel, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_align(tapLabel, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_add_event_cb(ui_musicPrevBtn, car_ui_event_musicPrev, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_musicPlayBtn, car_ui_event_musicPlay, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_musicNextBtn, car_ui_event_musicNext, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_musicScreen, car_ui_event_musicScreen, LV_EVENT_ALL, NULL);
}

void ui_gps_screen_init(void)
{
    ui_gpsScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_gpsScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_gpsScreen, lv_color_hex(0x000000), LV_PART_MAIN);

    ui_gpsClockLabel = lv_label_create(ui_gpsScreen);
    lv_label_set_text(ui_gpsClockLabel, "--:--");
    lv_obj_set_style_text_font(ui_gpsClockLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_gpsClockLabel, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align(ui_gpsClockLabel, LV_ALIGN_TOP_MID, 0, 15);

    ui_gpsDirectionIcon = lv_label_create(ui_gpsScreen);
    lv_label_set_text(ui_gpsDirectionIcon, "^");
    lv_obj_set_style_text_font(ui_gpsDirectionIcon, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_gpsDirectionIcon, lv_color_hex(0x00AAFF), LV_PART_MAIN);
    lv_obj_align(ui_gpsDirectionIcon, LV_ALIGN_CENTER, 0, -20);

    ui_gpsExitLabel = lv_label_create(ui_gpsScreen);
    lv_label_set_text(ui_gpsExitLabel, "");
    lv_obj_set_style_text_font(ui_gpsExitLabel, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_gpsExitLabel, lv_color_hex(0x00AAFF), LV_PART_MAIN);
    lv_obj_align(ui_gpsExitLabel, LV_ALIGN_CENTER, 0, 30);

    lv_obj_t *tapLabel = lv_label_create(ui_gpsScreen);
    lv_label_set_text(tapLabel, "Tap to switch");
    lv_obj_set_style_text_font(tapLabel, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tapLabel, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_align(tapLabel, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_add_event_cb(ui_gpsScreen, car_ui_event_gpsScreen, LV_EVENT_ALL, NULL);
}

void ui_car_settings_screen_init(void)
{
    ui_settingsScreen = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_settingsScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_settingsScreen, lv_color_hex(0x000000), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(ui_settingsScreen);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t *navPanel = lv_obj_create(ui_settingsScreen);
    lv_obj_set_size(navPanel, 200, 40);
    lv_obj_set_pos(navPanel, 20, 50);
    lv_obj_remove_flag(navPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(navPanel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(navPanel, 0, LV_PART_MAIN);

    lv_obj_t *navLabel = lv_label_create(navPanel);
    lv_label_set_text(navLabel, "Navigation");
    lv_obj_set_style_text_color(navLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(navLabel, 20, 10);

    ui_settingsNavSwitch = lv_switch_create(navPanel);
    lv_obj_set_pos(ui_settingsNavSwitch, 150, 8);
    lv_obj_add_state(ui_settingsNavSwitch, LV_STATE_CHECKED);

    lv_obj_t *shiftPanel = lv_obj_create(ui_settingsScreen);
    lv_obj_set_size(shiftPanel, 200, 40);
    lv_obj_set_pos(shiftPanel, 20, 100);
    lv_obj_remove_flag(shiftPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(shiftPanel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(shiftPanel, 0, LV_PART_MAIN);

    lv_obj_t *shiftLabel = lv_label_create(shiftPanel);
    lv_label_set_text(shiftLabel, "Shift Light");
    lv_obj_set_style_text_color(shiftLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(shiftLabel, 20, 10);

    ui_settingsShiftSwitch = lv_switch_create(shiftPanel);
    lv_obj_set_pos(ui_settingsShiftSwitch, 150, 8);
    lv_obj_add_state(ui_settingsShiftSwitch, LV_STATE_CHECKED);

    lv_obj_t *rpmPanel = lv_obj_create(ui_settingsScreen);
    lv_obj_set_size(rpmPanel, 200, 40);
    lv_obj_set_pos(rpmPanel, 20, 150);
    lv_obj_remove_flag(rpmPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(rpmPanel, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(rpmPanel, 0, LV_PART_MAIN);

    lv_obj_t *rpmLabel = lv_label_create(rpmPanel);
    lv_label_set_text(rpmLabel, "Shift RPM");
    lv_obj_set_style_text_color(rpmLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(rpmLabel, 20, 10);

    ui_settingsShiftRpmDropdown = lv_dropdown_create(rpmPanel);
    lv_dropdown_set_options(ui_settingsShiftRpmDropdown, rpmOptions);
    lv_dropdown_set_selected(ui_settingsShiftRpmDropdown, 8);
    lv_obj_set_pos(ui_settingsShiftRpmDropdown, 90, 0);
    lv_obj_set_size(ui_settingsShiftRpmDropdown, 100, 30);

    lv_obj_t *tapLabel = lv_label_create(ui_settingsScreen);
    lv_label_set_text(tapLabel, "Tap to switch");
    lv_obj_set_style_text_font(tapLabel, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tapLabel, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_align(tapLabel, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_add_event_cb(ui_settingsNavSwitch, car_ui_event_navSwitch, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingsShiftSwitch, car_ui_event_shiftSwitch, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingsShiftRpmDropdown, car_ui_event_shiftRpm, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingsScreen, car_ui_event_settingsScreen, LV_EVENT_ALL, NULL);
}

void ui_update_rpm(float rpm)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", (int)rpm);
    lv_label_set_text(ui_rpmLabel, buf);

    int arcValue = (int)((rpm / 8000.0f) * 100);
    if (arcValue > 100) arcValue = 100;
    lv_arc_set_value(ui_rpmArc, arcValue);
}

void ui_update_shift_overlay(bool show)
{
    if (shiftLightEnabled && show)
    {
        lv_obj_remove_flag(ui_shiftOverlay, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ui_shiftOverlay, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_update_tach_leds(int activeLeds)
{
    for (int i = 0; i < MAX_LEDS; i++)
    {
        if (i < activeLeds)
        {
            if (i == 0)
            {
                lv_obj_set_style_bg_color(ui_tachLeds[i], lv_color_hex(0xFF0000), LV_PART_MAIN);
            }
            else
            {
                lv_obj_set_style_bg_color(ui_tachLeds[i], lv_color_hex(0x00FF00), LV_PART_MAIN);
            }
        }
        else
        {
            lv_obj_set_style_bg_color(ui_tachLeds[i], lv_color_hex(0x111111), LV_PART_MAIN);
        }
    }

    if (shiftLightEnabled && activeLeds >= MAX_LEDS)
    {
        ui_update_shift_overlay(true);
    }
    else
    {
        ui_update_shift_overlay(false);
    }
}

void ui_update_clock(const char *time)
{
    if (ui_gpsClockLabel)
    {
        lv_label_set_text(ui_gpsClockLabel, time);
    }
}

void ui_update_gps_direction(int direction, int exit)
{
    const char *dirSymbol = "^";
    switch (direction)
    {
        case 0: dirSymbol = "^"; break;
        case 1: dirSymbol = "<"; break;
        case 2: dirSymbol = ">"; break;
        case 3: dirSymbol = "o"; break;
        case 4: dirSymbol = "e"; break;
        default: dirSymbol = "^"; break;
    }
    lv_label_set_text(ui_gpsDirectionIcon, dirSymbol);

    if (exit > 0)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", exit);
        lv_label_set_text(ui_gpsExitLabel, buf);
    }
    else
    {
        lv_label_set_text(ui_gpsExitLabel, "");
    }
}

void ui_update_music_state(bool playing)
{
    if (ui_musicPlayBtn)
    {
        lv_obj_t *icon = lv_obj_get_child(ui_musicPlayBtn, 0);
        if (icon)
        {
            lv_label_set_text(icon, playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
        }
    }
}

void switch_to_next_screen(void)
{
    currentScreenIndex = (currentScreenIndex + 1) % 4;

    switch (currentScreenIndex)
    {
        case 0:
            currentScreen = ui_rpmScreen;
            break;
        case 1:
            currentScreen = ui_musicScreen;
            break;
        case 2:
            currentScreen = ui_gpsScreen;
            break;
        case 3:
            currentScreen = ui_settingsScreen;
            break;
    }
    lv_screen_load(currentScreen);
}

extern "C" {

void car_ui_event_rpmScreen(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED || lv_event_get_code(e) == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        switch_to_next_screen();
    }
}

void car_ui_event_musicScreen(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED || lv_event_get_code(e) == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        switch_to_next_screen();
    }
}

void car_ui_event_gpsScreen(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED || lv_event_get_code(e) == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        switch_to_next_screen();
    }
}

void car_ui_event_settingsScreen(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED || lv_event_get_code(e) == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        switch_to_next_screen();
    }
}

void car_ui_event_musicPrev(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        onMusicPrevious(e);
    }
}

void car_ui_event_musicPlay(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        onMusicPlay(e);
    }
}

void car_ui_event_musicNext(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        onMusicNext(e);
    }
}

void car_ui_event_navSwitch(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        navEnabled = lv_obj_has_state(ui_settingsNavSwitch, LV_STATE_CHECKED);
        saveCarPrefs();
    }
}

void car_ui_event_shiftSwitch(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        shiftLightEnabled = lv_obj_has_state(ui_settingsShiftSwitch, LV_STATE_CHECKED);
        saveCarPrefs();
    }
}

void car_ui_event_shiftRpm(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        shiftLightRpm = 1000 + lv_dropdown_get_selected(ui_settingsShiftRpmDropdown) * 500;
        saveCarPrefs();
    }
}

}

void saveCarPrefs(void)
{
}

void loadCarPrefs(void)
{
}
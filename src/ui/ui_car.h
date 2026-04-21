#ifndef UI_CAR_H
#define UI_CAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdbool.h>

#define MAX_LEDS 10

extern lv_obj_t *ui_rpmScreen;
extern lv_obj_t *ui_musicScreen;
extern lv_obj_t *ui_gpsScreen;
extern lv_obj_t *ui_settingsScreen;

extern lv_obj_t *ui_rpmLabel;
extern lv_obj_t *ui_rpmArc;
extern lv_obj_t *ui_tachLeds[MAX_LEDS];
extern lv_obj_t *ui_shiftOverlay;

extern lv_obj_t *ui_musicPrevBtn;
extern lv_obj_t *ui_musicPlayBtn;
extern lv_obj_t *ui_musicNextBtn;

extern lv_obj_t *ui_gpsClockLabel;
extern lv_obj_t *ui_gpsDirectionIcon;
extern lv_obj_t *ui_gpsExitLabel;

extern lv_obj_t *ui_settingsNavSwitch;
extern lv_obj_t *ui_settingsShiftSwitch;
extern lv_obj_t *ui_settingsShiftRpmDropdown;

void ui_car_screen_init(void);
void ui_rpm_screen_init(void);
void ui_music_screen_init(void);
void ui_gps_screen_init(void);
void ui_car_settings_screen_init(void);

void ui_update_rpm(float rpm);
void ui_update_shift_overlay(bool show);
void ui_update_tach_leds(int activeLeds);
void ui_update_clock(const char *time);
void ui_update_gps_direction(int direction, int exit);
void ui_update_music_state(bool playing);

void car_ui_event_rpmScreen(lv_event_t *e);
void car_ui_event_musicScreen(lv_event_t *e);
void car_ui_event_gpsScreen(lv_event_t *e);
void car_ui_event_settingsScreen(lv_event_t *e);

void car_ui_event_musicPrev(lv_event_t *e);
void car_ui_event_musicPlay(lv_event_t *e);
void car_ui_event_musicNext(lv_event_t *e);

void car_ui_event_navSwitch(lv_event_t *e);
void car_ui_event_shiftSwitch(lv_event_t *e);
void car_ui_event_shiftRpm(lv_event_t *e);

void saveCarPrefs(void);
void loadCarPrefs(void);

#ifdef __cplusplus
}
#endif

#endif
//CugoSDKライブラリ
#ifndef CUGOCMDMODE_H
#define CUGOCMDMODE_H

//インクルード関連
#include "RPi_Pico_TimerInterrupt.h"
#include "hardware/watchdog.h"

//各種定数
//cugo仕様関連
// CuGo V4のパラメータ
//#define CUGO_WHEEL_RADIUS_L  0.03858d
//#define CUGO_WHEEL_RADIUS_R  0.03858d
//#define CUGO_TREAD  0.380d
//#define CUGO_ENCODER_RESOLUTION  600.0d
//#define CUGO_MAX_MOTOR_RPM  130    //モータの速度上限値
//#define CUGO_NORMAL_MOTOR_RPM  90  //通常速度の設定値
// CuGo V3iのパラメータ

#define CUGO_WHEEL_RADIUS_L 0.03858d
#define CUGO_WHEEL_RADIUS_R 0.03858d
#define CUGO_TREAD 0.376d
#define CUGO_ENCODER_RESOLUTION 360.0d
#define CUGO_MAX_MOTOR_RPM 180    //モータの速度上限値
#define CUGO_NORMAL_MOTOR_RPM 90  //通常速度の設定値

// PID位置制御のゲイン調整
#define CUGO_L_COUNT_KP 50.0f
#define CUGO_L_COUNT_KI 0.5f
#define CUGO_L_COUNT_KD 10.0f
#define CUGO_R_COUNT_KP 50.0f
#define CUGO_R_COUNT_KI 0.5f
#define CUGO_R_COUNT_KD 10.0f
#define CUGO_L_MAX_COUNT_I 120
#define CUGO_R_MAX_COUNT_I 120


// 各種動作モード定義
#define CUGO_RC_MODE 0
#define CUGO_CMD_MODE 1

//オドメトリ定義
#define CUGO_ODO_X 0
#define CUGO_ODO_Y 1
#define CUGO_ODO_THETA 2
#define CUGO_ODO_DEGREE 3

//モーター定義
#define CUGO_MOTOR_LEFT 0
#define CUGO_MOTOR_RIGHT 1

//LD2関連
#define CUGO_LD2_COUNT_MAX 65536
#define NVIC_SYSRESETREQ 2

// グローバル変数宣言
extern int cugo_old_runmode;
extern int cugo_runmode;
extern long int cugo_count_prev_L;
extern long int cugo_count_prev_R;

//カウント関連
extern long int cugo_target_count_L;
extern long int cugo_target_count_R;
extern long int cugo_start_count_L;
extern long int cugo_start_count_R;
extern long int cugo_current_count_L;
extern long int cugo_current_count_R;
extern volatile long cugo_current_encoder_R;
extern volatile long cugo_current_encoder_L;
extern long int cugo_prev_encoder_L;
extern long int cugo_prev_encoder_R;
extern unsigned long long int cugo_calc_odometer_time;
extern float cugo_odometer_theta;
extern float cugo_odometer_x;
extern float cugo_odometer_y;
extern float cugo_odometer_degree;
extern long int cugo_odometer_count_theta;
extern bool cugo_direction_L;
extern bool cugo_direction_R;

//ld2関連
extern volatile long cugo_ld2_id;
extern volatile long cugo_ld2_feedback_hz;
extern volatile long cugo_ld2_feedback_dutation;
extern bool cugo_switching_reset;

//各種関数
//初期設定関数関連
void cugo_init();
bool cugo_timer_handler0(struct repeating_timer* t);
void cugo_init_display();
void cugo_reset_pid_gain();
void cugo_check_mode_change();
void cugo_reset();

//モータ直接制御関連
void cugo_rpm_direct_instructions(float left, float right);


//wait関数
void cugo_wait(unsigned long long int wait_ms);
void cugo_long_wait(unsigned long long int wait_seconds);

//プロポ入力確認関数
int cugo_check_propo_channel_value(int channel_number);



//LD2関連
//便利関数
void ld2_float_to_frame(float data, long int start, unsigned char* index);   //配列indexの4番目からfloat dataを書き込む場合-> FloatTolong int(data, 4, index);
void ld2_frame_to_float(unsigned char* index, long int start, float* data);  //配列indexの3番目からfloat dataに書き込む場合-> ld2_frame_to_float(index, 3, data);
void ld2_frame_to_short(unsigned char* index, long int start, short* data);  //配列indexの3番目からulong int16_t dataに書き込む場合-> ld2_frame_to_float(index, 3, data);
                                                                             //通信関係
void ld2_write_cmd(unsigned char cmd[10]);
void ld2_get_cmd();
//設定
void ld2_set_feedback(unsigned char freq_index, unsigned char kindof_data);  //freq{0:10[hz] 1:50[hz] 2:100[hz]} kindof_data{0b1:Mode 0b10:CMD_RPM 0b100:CurrentRPM 0b1000:AveCurrentRPM 0b10000000:EncorderData}
void ld2_set_control_mode(unsigned char mode);                               //mode{0:RC_mode 1:CMD_Mode}
void ld2_set_encorder(unsigned char frame[12]);
void ld2_encoder_reset();

#endif

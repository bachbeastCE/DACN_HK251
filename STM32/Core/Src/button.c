#include "button.h"

const int MAX_BUTTON = 1;

// Định nghĩa Port và Pin cho 5 nút nhấn để vòng lặp for tự động dò
GPIO_TypeDef* Button_Port[5] = {GPIOC, 0,0,0,0}; // Sửa lại đúng Port phần cứng của bạn
uint16_t Button_Pin[5]       = {GPIO_PIN_13, 0, 0,0,0}; // Sửa lại đúng Pin của bạn

int KeyReg0[5] = {SET, SET, SET, SET, SET};
int KeyReg1[5] = {SET, SET, SET, SET, SET};
int KeyReg2[5] = {SET, SET, SET, SET, SET};
int KeyReg3[5] = {SET, SET, SET, SET, SET};

// MỖI NÚT PHẢI CÓ MỘT BIẾN ĐẾM THỜI GIAN RIÊNG
int TimeOutForKeyPress[5] = {200, 200, 200, 200, 200};

int button_long_pressed[5] = {0, 0, 0, 0, 0};
int button_flag[5]         = {0, 0, 0, 0, 0};

int isButtonPressed(int button_index){
    if(button_flag[button_index] == 1){
        button_flag[button_index] = 0;
        return 1;
    }
    return 0;
}

int isButtonLongPressed(int button_index){
    if(button_long_pressed[button_index] == 1){
        button_long_pressed[button_index] = 0;
        return 1;
    }
    return 0;
}

void getKeyInput(){
    for(int i = 0; i < MAX_BUTTON; i++){
        KeyReg2[i] = KeyReg1[i];
        KeyReg1[i] = KeyReg0[i];

        // Đọc tín hiệu bằng cách gọi mảng Port và Pin tương ứng với index 'i'
        KeyReg0[i] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

        if ((KeyReg1[i] == KeyReg0[i]) && (KeyReg1[i] == KeyReg2[i])){
            if (KeyReg2[i] != KeyReg3[i]){
                KeyReg3[i] = KeyReg2[i];

                if (KeyReg3[i] == PRESSED_STATE){ // PRESSED_STATE thường là 0 (RESET) nếu có Pull-up
                    TimeOutForKeyPress[i] = 200; // Reset lại bộ đếm của nút thứ i
                    button_flag[i] = 1;
                }
            }
            else {
                // Đang giữ phím
                TimeOutForKeyPress[i]--;
                if (TimeOutForKeyPress[i] == 0){
                    TimeOutForKeyPress[i] = 200;
                    if (KeyReg3[i] == PRESSED_STATE){
                        button_long_pressed[i] = 1;
                    }
                }
            }
        }
    }
}

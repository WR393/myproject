#include "btn_app.h"
#include "mydefine.h"
#include "ebtn.h" 

uint8_t mode;


const ebtn_btn_param_t defaul_ebtn_param = EBTN_PARAMS_INIT(
    20,     // time_debounce: ??????? 20ms
    20,     // time_debounce_release: ?????? 20ms
    50,     // time_click_pressed_min: ?????????? 50ms
    500,    // time_click_pressed_max: ?????????? 500ms (??????????)
    300,    // time_click_multi_max: ??¦Å???????? 300ms (???¦Å??????????????????)
    500,    // time_keepalive_period: ??????????? 500ms (???3??? 500ms ??? 500ms ???????)
    2       // max_consecutive: ?????? 5 ????
);

const ebtn_btn_param_t combo_ebtn_param = EBTN_PARAMS_INIT(
    30,     // time_debounce: ??????? 20ms
    0,     // time_debounce_release: ?????? 20ms
    50,     // time_click_pressed_min: ?????????? 50ms
    500,    // time_click_pressed_max: ?????????? 500ms (??????????)
    200,    // time_click_multi_max: ??¦Å???????? 300ms (???¦Å??????????????????)
    0,    // time_keepalive_period: ??????????? 500ms (???3??? 500ms ??? 500ms ???????)
    1       // max_consecutive: ?????? 5 ????
);

typedef enum
{
    USER_BUTTON_0 = 0,
    USER_BUTTON_1,
    USER_BUTTON_2,
    USER_BUTTON_3,
    USER_BUTTON_4,
    USER_BUTTON_5,
    USER_BUTTON_MAX,
	
	  COMBO_BUTTON_1=100,
	  COMBO_BUTTON_2,
	  COMBO_BUTTON_MAX,	
} user_button_t;

static ebtn_btn_t btns[] = 
{
	EBTN_BUTTON_INIT(USER_BUTTON_0, &defaul_ebtn_param),
	EBTN_BUTTON_INIT(USER_BUTTON_1, &defaul_ebtn_param),
	EBTN_BUTTON_INIT(USER_BUTTON_2, &defaul_ebtn_param),
	EBTN_BUTTON_INIT(USER_BUTTON_3, &defaul_ebtn_param),
	EBTN_BUTTON_INIT(USER_BUTTON_4, &defaul_ebtn_param),
	EBTN_BUTTON_INIT(USER_BUTTON_5, &defaul_ebtn_param),
};

static ebtn_btn_combo_t btns_combo[]={
	EBTN_BUTTON_COMBO_INIT_RAW(COMBO_BUTTON_1,&combo_ebtn_param,EBTN_EVT_MASK_ONCLICK),
};

uint8_t prv_btn_get_state(struct ebtn_btn *btn)
{
	switch (btn->key_id)
	{
	case USER_BUTTON_0:
		return !HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15);
	case USER_BUTTON_1:
		return !HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13);
	case USER_BUTTON_2:
		return !HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11);
	case USER_BUTTON_3:
		return !HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_9);
	case USER_BUTTON_4:
		return !HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7);
	case USER_BUTTON_5:
		return !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
	default:
		// ??????????????¦Ä????????? ID?????? 0
		return 0;
	}	
}

// EBTN_EVT_ONPRESS = 0x00, /*!< ??????? - ?????§¹????????? */
// EBTN_EVT_ONRELEASE,      /*!< ?????? - ?????§¹??????????? (????????) */
// EBTN_EVT_ONCLICK,        /*!< ??????? - ??????§¹????o???????????????? */
// EBTN_EVT_KEEPALIVE,      /*!< ???????? - ?????????????????? */

void prv_btn_event(struct ebtn_btn *btn, ebtn_evt_t evt)
{
	// ?????????/??????
	if (evt == EBTN_EVT_ONCLICK)
	{
		uint16_t click_cnt = ebtn_click_get_count(btn);

		switch (btn->key_id)
		{
		// --- ?????????? ---
			
			case COMBO_BUTTON_1:
				if(click_cnt==1)
					ucLed[4]^=1;
			break;
		case USER_BUTTON_0:
			if (click_cnt == 1)
			{
				
				mode++;
				if(mode==3)
					mode=0;
				dac_sin_init();
			} // ????????
			else if (click_cnt == 2)
			{
				ucLed[0] = 0;
			} // ??????
			my_printf(&huart1,"ok\r\n");
			break;
		case USER_BUTTON_1:
			if (click_cnt == 1)
			{
					current_freq_index++;
				if(current_freq_index==4)
					current_freq_index=0;
				Set_DAC_Frequency(frequency_steps[current_freq_index]);
			}
			else if (click_cnt == 2)
			{
				ucLed[1] = 0;
			}
			break;
		case USER_BUTTON_2:
			if (click_cnt == 1)
			{
				ucLed[2] = 1;
			}
			else if (click_cnt == 2)
			{
				ucLed[2] = 0;
			}
			break;
		case USER_BUTTON_3:
			if (click_cnt == 1)
			{
				ucLed[3] = 1;
			}
			else if (click_cnt == 2)
			{
				ucLed[3] = 0;
			}
			break;
		case USER_BUTTON_4:
			if (click_cnt == 1)
			{
				ucLed[4] = 1;
			}
			else if (click_cnt == 2)
			{
				ucLed[4] = 0;
			}
			break;
		case USER_BUTTON_5:
			if (click_cnt == 1)
			{
				ucLed[5] = 1;
			}
			else if (click_cnt == 2)
			{
				ucLed[5] = 0;
			}
			break;
		default:
			// ??????????¦Ä????????????
			break;
		}
	}
}

void app_ebtn_init(void)
{
	ebtn_init(btns, EBTN_ARRAY_SIZE(btns), btns_combo, EBTN_ARRAY_SIZE(btns_combo), prv_btn_get_state, prv_btn_event);
	
	
	int btn0_idx=ebtn_get_btn_index_by_key_id(USER_BUTTON_0);
	int btn1_idx=ebtn_get_btn_index_by_key_id(USER_BUTTON_1);
	int btn2_idx=ebtn_get_btn_index_by_key_id(USER_BUTTON_2);
	int btn3_idx=ebtn_get_btn_index_by_key_id(USER_BUTTON_3);
	
	if(btn0_idx>=0&&btn1_idx>=0)
	{
		ebtn_combo_btn_add_btn_by_idx(&btns_combo[0],btn0_idx);
		ebtn_combo_btn_add_btn_by_idx(&btns_combo[0],btn1_idx);
	}
}

void btn_task(void)
{
	ebtn_process(HAL_GetTick());
}


//????0+????1 ????ucled
//????0+????2 ???ucled
//????0+????3 ????ucled???????????????


#include"menu.h"
#include"menu_1.h"
rt_uint8_t KeyFuncIndex = 0;
void(*current_operation_index)(void);


//�˵��б�
KbdTabStruct	KeyTab[KEY_MAX_MENU_NUM] = 
{
	//����
	{0,0,0,1,0,menu_1_processing},//��½����

	//����
	{1,2,2,3,1,menu_2_processing},
	{2,1,1,4,1,menu_3_processing},

	//����
	{3,4,4,3,1,menu_4_processing},
	{4,3,3,4,1,menu_5_processing},
};


void key_input_processing(void)
{
	rt_err_t result;
	rt_uint8_t KeyValue;
	
	result = gui_key_input(&KeyValue);
	if (result == RT_EOK) 
	{
		rt_kprintf("recv key value :%c\n",KeyValue);
		switch(KeyValue)
		{
			case '#':
			{
				//ȷ��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].SureState;
				break;
			}
			case '*':
			{
				//ȡ��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].BackState;
				break;
			}
			case '2':
			{
				//��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].UpState;
				break;
			}
			case'8':
			{
				//��
				KeyFuncIndex = KeyTab[ KeyFuncIndex].DnState;
				break;
			}
			default:
			{
				break;
			}
		}
		current_operation_index = KeyTab[KeyFuncIndex].CurrentOperate;
		current_operation_index();
	}
}

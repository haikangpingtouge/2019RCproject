/**
	|------------------------------- Copyright ----------------------------------|
	|                                                                            |
	|                       (C) Copyright 2018,����ƽͷ��,                        |
	|          1 Xuefu Rd, Huadu Qu, Guangzhou Shi, Guangdong Sheng, China       |
	|                           All Rights Reserved                              |
	|                                                                            |
	|          By(GCU The wold of team | ����������ѧ����ѧԺ������Ұ�Ƕ�)          |
	|                   https://github.com/GCUWildwolfteam                       |
	|----------------------------------------------------------------------------|
	|--FileName    : motor.c                                                
	|--Version     : v1.0                                                            
	|--Author      : ����ƽͷ��                                                       
	|--Date        : 2018-12-31               
	|--Libsupports : ��׼���HAL��
	|--Description : 1��maxion���+RoboModule���� 
	|								 2��3508���+c610���
	|								 3��6623���
	|                4��6002���
	|								 5��                                                        
	|--FunctionList                                                       
	|-------1. ....                                                       
	|          <version>:                                                       
	|     <modify staff>:                                                       
	|             <data>:                                                       
	|      <description>:                                                        
	|-------2. ...                                                       
	|------------------------------declaration of end----------------------------|
 **/
#include "motor.h" 
/* ==============================common of begin ============================ */
	static int16_t s_max_motor_lines = 8192 ;//���һȦ�������
	/**
	* @Data    2019-01-18 20:14
	* @brief   ����ת��  
	* @param   real ��ʵֵ
	* @param   threshold һȦ��������ķ�ֵ
	* @param   perce //ת������������ǰ�Ƕ�:���ٺ�ĽǶ� = x:1
	* @retval  int16_t ������Ŀ��ֵ
	*/
	int16_t RatiometricConversion(int16_t real,int16_t threshold,int16_t perce)
	{
		static int32_t last_real,tem=0;
		static uint16_t  coefficient=0;
		if (real - last_real < threshold)
		{
			/*�Ƕ�ϵ��ѭ������,����ת36Ȧ����ϵ��,���÷�Χ[0,perce] */
			coefficient =(coefficient+1)%(perce);
		}
		else if(last_real - real < threshold)
		{
			coefficient = (perce-1)-((((perce-1) - coefficient)+1)%(perce));
		}
		last_real = real;//��������ֵ
		tem = real + (s_max_motor_lines* coefficient); //ת���ܽǶ�
		return ((int16_t)(tem/(perce)));//���������תһȦ
	}
	/**
	* @Data    2019-01-18 20:48
	* @brief   ��㴦��
	* @param   real ��ʵֵ
	* @param   threshold һȦ��������ķ�ֵ
	* @retval  int16_t ����֮���Ŀ��ֵ
	*/
	int16_t zeroArgument(int16_t real,int16_t threshold)
	{
		static int16_t last_realdata = 0,counter = 0;
		if(real - last_realdata < threshold) 
			counter = 1;
		if(last_realdata - real <threshold)
			counter = -1;
			else counter = 0;
		last_realdata = real;
		return (real + (s_max_motor_lines * counter));
	}
/* ============================ common of end =============================== */
/* ============================= RM6623 of begin ============================ */
	/**
		* @Data    2019-01-18 19:59
		* @brief   RM6623�ṹ���ʼ��
		* @param   RM6623����ṹ���ַ
		* @retval  void
		*/
	void RM6623StructInit(RM6623Struct *RM6623,CAN_HandleTypeDef *hcanx)
	{
		RM6623->id = 0;
		RM6623->target = 0;				//Ŀ��ֵ
		RM6623->tem_target = 0;		//��ʱĿ��ֵ
		RM6623->real_current = 0; //��ʵ����
		RM6623->real_angle = 0;		//��ʵ�Ƕ�
		RM6623->tem_angle = 0;		//��ʱ�Ƕ�
		RM6623->zero = 0;					//������
		RM6623->Percentage = 0;		//ת������������ǰ�Ƕ�:���ٺ�ĽǶ� = x:1��
		/*���ݶ�ȡʱ�������������ת������p��thresholds = p -s_max_motor_lines */
		RM6623->thresholds = 1200 - s_max_motor_lines;//1200��Ϲ���ģ���ƽ�
	}
		/**
		* @Data    2019-01-18 20:03
		* @brief   6623���ݽ���
		* @param   RM6623����ṹ���ַ
		* @retval  HAL Status
		*/
	void RM6623ParseData(RM6623Struct*RM6623,uint8_t *data)
	{
		int16_t tem_angle = 0;
		RM6623->real_current = ((int16_t)(data[4] << 8) | data[5]);
		tem_angle = ((int16_t)(data[0] << 8) | data[1]);
		tem_angle = RatiometricConversion(tem_angle, RM6623->thresholds,\
																								 RM6623->Percentage);
		RM6623->real_angle = zeroArgument(tem_angle, RM6623->thresholds);
	}

/* ============================= RM6623 of end ============================== */
/* ============================= 3508 of begin ============================== */
	/**
	* @Data    2019-01-19 00:42
	* @brief   3508���ݽ���
	* @param   RM3508Struct 3508�ṹ��ָ��
	* @param   *data  ���յ�����ָ��
	* @retval  void
	*/
	void RM3508ParseData(RM3508Struct *RM3508,uint8_t *data)
	{
		RM3508->real_angle=((int16_t)(data[0]<<8)|data[1]);
		RM3508->real_speed=((int16_t)(data[2]<<8)|data[3]);
		RM3508->real_current=((int16_t)(data[4]<<8)|data[5]);
	}
/* ============================== 3508 of end =============================== */
/* ============================= M2006 of begin ============================= */
	/**
		* @Data    2019-01-26 18:21
		* @brief   M2006���ݽ���
		* @param   M2006struct 2006�ṹ��ָ��
		* @param   *data  ���յ�����ָ��
		* @retval  void
		*/
		void RM2006ParseData(M2006Struct *M2006,uint8_t *data)
		{
			M2006->real_angle  = ((int16_t)(data[0]<<8)|data[1]);
    	M2006->real_speed = ((int16_t)(data[2]<<8)|data[3]);
			M2006->real_current=((int16_t)(data[4]<<8)|data[5]);
		}
/* ============================== M2006 of end ============================== */
/* =======================-===== maxion of begin ============================ */
	/**
	* @Data    2019-01-04 13:27
	* @brief   �ٶ�ģʽ�µ�����ָ��
	* @param   Group   ȡֵ��Χ 0-7
	* @param	 Number  ȡֵ��Χ 0-15������Number==0ʱ��Ϊ�㲥����
	* @param	 temp_pwm��ȡֵ��Χ���£�
							0 ~ +5000����ֵ5000������temp_pwm = 5000ʱ����������ѹΪ��Դ��ѹ
	* @param	 temp_velocity��ȡֵ��Χ���£�
							-32768 ~ +32767����λRPM
	* @retval  HAL_StatusTypeDef
	*/
	/**
		* @Data    2019-02-16 16:17
		* @brief   ��λָ��
		* @param   void
		* @retval  void
		*/
		void ResetMode(CAN_HandleTypeDef *hcanx,uint32_t rx_id)
		{
      uint8_t s[8];
			rx_id |= reset_fun_e;
      DataFilling(s,0x55,8);
			CanTxMsg(hcanx,rx_id,s);
		}
	/**
		* @Data    2019-02-16 16:35
		* @brief   ����ģʽѡ��
		* @param   void
		* @retval  void
		*/
		void ModeSelectionMode(CAN_HandleTypeDef *hcanx,uint32_t rx_id,uint8_t mode)
		{
			uint8_t s[8];
			rx_id |= mode_selection_fun_e;
      DataFilling(s,0x55,8);
			s[0] = mode;
			CanTxMsg(hcanx,rx_id,s);
		}
  /**
	* @Data    2019-02-16 13:15
	* @brief   �ٶ�ģʽָ��
	* @param	 Speed��ȡֵ��Χ���£�
						-32768 ~ +32767����λRPM
	* @retval  void
	*/
	void SpeedLoopMode(int16_t pwm,int16_t Speed,uint8_t *data)
	{
		TwobyteToByle(pwm,(data));
		TwobyteToByle(Speed,(data+2));
		DataFilling((data+4),0x55,8);
	}
	 /**
		* @Data    2019-02-16 13:31
		* @brief   ����ģʽָ��
		* @param   void
		* @retval  void
		*/
		void OpenLoopMode(int16_t pwm,uint8_t *data)
		{
			TwobyteToByle(pwm,(data));
			DataFilling((data+2),0x55,8);
		}
	/**
	* @Data    2019-02-17 17:12
	* @brief   ����ָ��
	* @param   void
	* @retval  void
	*/
	void ConfigMode(CAN_HandleTypeDef *hcanx,uint32_t rx_id,uint8_t Time,\
																													uint8_t Ctl1_Ctl2)
	{
    uint8_t s[8];
		uint32_t id = config_fun_e;
		id |= rx_id;
		if((Ctl1_Ctl2 != 0x00)&&(Ctl1_Ctl2 != 0x01))
    {
        Ctl1_Ctl2 = 0x00;
    }
      DataFilling(s,0x55,8);
		s[0] = Time;
		s[1] = Ctl1_Ctl2;
		CanTxMsg(hcanx,id,s);
	}
	/**
		* @Data    2019-02-16 14:10
		* @brief   ���can����
		* @param   void
		* @retval  void
		*/
		void SendMotorMsg(void)
		{
			
		}
	/**
	* @Data    2019-01-23 00:00
	* @brief   maxioncan���Ľ���
	* @param   void
	* @retval  void
	*/
	void MaxionParseData(maxionStruct*maxion,uint8_t *data)
	{
		maxion->real_current  = (data[0]<<8)|data[1];
		maxion->real_velocity = (data[2]<<8)|data[3];
		maxion->real_position = (data[4]<<24)| (data[5]<<16)| (data[6]<<8)| data[7];
	}
	/**
		* @Data    2019-02-16 16:15
		* @brief   maxion�����ʼ��
		* @param   void
		* @retval  void
		*/
		void MotorInit(CAN_HandleTypeDef *hcanx,uint8_t id,uint8_t mode)
		{
//			ResetMode(hcanx,id);//���͸�λָ��
			osDelay(500);//�ȴ�500ms
//			ModeSelectionMode(hcanx,id,mode);
			osDelay(500);//�ȴ�500ms
		}
/* ============================= maxion of end ============================== */


/*---------------------------------file of end--------------------------------*/
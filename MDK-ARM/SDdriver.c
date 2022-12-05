#include "SDdriver.h"

#include "ff.h"

uint8_t DFF=0xFF;
uint8_t test;
uint8_t SD_TYPE=0x00;

MSD_CARDINFO SD0_CardInfo;

//////////////////////////////////////////////////////////////
//Ƭѡ
//////////////////////////////////////////////////////////////
void SD_CS(uint8_t p){
	if(p==0){	
		HAL_GPIO_WritePin(SD_CS_GPIO_Port,SD_CS_Pin,GPIO_PIN_SET);
		}else{
		HAL_GPIO_WritePin(SD_CS_GPIO_Port,SD_CS_Pin,GPIO_PIN_RESET);
		}
}
///////////////////////////////////////////////////////////////
//������������ͷ�
//////////////////////////////////////////////////////////////
int SD_sendcmd(uint8_t cmd,uint32_t arg,uint8_t crc){
	uint8_t r1;
  uint8_t retry;

  SD_CS(0);
	HAL_Delay(20);
  SD_CS(1);
	do{
		retry=spi_readwrite(DFF);
	}while(retry!=0xFF);

  spi_readwrite(cmd | 0x40);
  spi_readwrite(arg >> 24);
  spi_readwrite(arg >> 16);
  spi_readwrite(arg >> 8);
  spi_readwrite(arg);
  spi_readwrite(crc);
  if(cmd==CMD12)spi_readwrite(DFF);
  do
	{
		r1=spi_readwrite(0xFF);
	}while(r1&0X80);
	
	return r1;
}


/////////////////////////////////////////////////////////////
//SD����ʼ��
////////////////////////////////////////////////////////////
uint8_t SD_init(void)
{
	uint8_t r1;	
	uint8_t buff[6] = {0};
	uint16_t retry; 
	uint8_t i;
	
//	MX_SPI3_Init();
	SPI_setspeed(SPI_BAUDRATEPRESCALER_256);
	SD_CS(0);
	for(retry=0;retry<10;retry++){
			spi_readwrite(DFF);
	}
	
	//SD������IDLE״̬
	do{
		r1 = SD_sendcmd(CMD0 ,0, 0x95);	
	}while(r1!=0x01);
	
	//�鿴SD��������
	SD_TYPE=0;
	r1 = SD_sendcmd(CMD8, 0x1AA, 0x87);
	if(r1==0x01){
		for(i=0;i<4;i++)buff[i]=spi_readwrite(DFF);	//Get trailing return value of R7 resp
		if(buff[2]==0X01&&buff[3]==0XAA)//���Ƿ�֧��2.7~3.6V
		{
			retry=0XFFFE;
			do
			{
				SD_sendcmd(CMD55,0,0X01);	//����CMD55
				r1=SD_sendcmd(CMD41,0x40000000,0X01);//����CMD41
			}while(r1&&retry--);
			if(retry&&SD_sendcmd(CMD58,0,0X01)==0)//����SD2.0���汾��ʼ
			{
				for(i=0;i<4;i++)buff[i]=spi_readwrite(0XFF);//�õ�OCRֵ
				if(buff[0]&0x40){
					SD_TYPE=V2HC;
				}else {
					SD_TYPE=V2;
				}						
			}
		}else{
			SD_sendcmd(CMD55,0,0X01);			//����CMD55
			r1=SD_sendcmd(CMD41,0,0X01);	//����CMD41
			if(r1<=1)
			{		
				SD_TYPE=V1;
				retry=0XFFFE;
				do //�ȴ��˳�IDLEģʽ
				{
					SD_sendcmd(CMD55,0,0X01);	//����CMD55
					r1=SD_sendcmd(CMD41,0,0X01);//����CMD41
				}while(r1&&retry--);
			}else//MMC����֧��CMD55+CMD41ʶ��
			{
				SD_TYPE=MMC;//MMC V3
				retry=0XFFFE;
				do //�ȴ��˳�IDLEģʽ
				{											    
					r1=SD_sendcmd(CMD1,0,0X01);//����CMD1
				}while(r1&&retry--);  
			}
			if(retry==0||SD_sendcmd(CMD16,512,0X01)!=0)SD_TYPE=ERR;//����Ŀ�
		}
	}
	SD_CS(0);
	SPI_setspeed(SPI_BAUDRATEPRESCALER_2);
	if(SD_TYPE)return 0;
	else return 1;
}


//��ȡָ����������
uint8_t SD_ReceiveData(uint8_t *data, uint16_t len)
{

   uint8_t r1;
   SD_CS(1);									   
   do
   { 
      r1 = spi_readwrite(0xFF);	
      HAL_Delay(100);
		}while(r1 != 0xFE);	
  while(len--)
  {
   *data = spi_readwrite(0xFF);
   data++;
  }
  spi_readwrite(0xFF);
  spi_readwrite(0xFF); 										  		
  return 0;
}
//��sd��д��һ�����ݰ������� 512�ֽ�
uint8_t SD_SendBlock(uint8_t*buf,uint8_t cmd)
{	
	uint16_t t;	
uint8_t r1;	
	do{
		r1=spi_readwrite(0xFF);
	}while(r1!=0xFF);
	
	spi_readwrite(cmd);
	if(cmd!=0XFD)//���ǽ���ָ��
	{
		for(t=0;t<512;t++)spi_readwrite(buf[t]);//����ٶ�,���ٺ�������ʱ��
	    spi_readwrite(0xFF);//����crc
	    spi_readwrite(0xFF);
		t=spi_readwrite(0xFF);//������Ӧ
		if((t&0x1F)!=0x05)return 2;//��Ӧ����									  					    
	}						 									  					    
    return 0;//д��ɹ�
}

//��ȡCID��Ϣ
uint8_t SD_GETCID (uint8_t *cid_data)
{
		uint8_t r1;
	  r1=SD_sendcmd(CMD10,0,0x01); //��ȡCID�Ĵ���
		if(r1==0x00){
			r1=SD_ReceiveData(cid_data,16);
		}
		SD_CS(0);
		if(r1)return 1;
		else return 0;
}
//��ȡCSD��Ϣ
uint8_t SD_GETCSD(uint8_t *csd_data){
		uint8_t r1;	 
    r1=SD_sendcmd(CMD9,0,0x01);//��CMD9�����CSD�Ĵ���
    if(r1==0)
	{
    	r1=SD_ReceiveData(csd_data, 16);//����16���ֽڵ����� 
    }
	SD_CS(0);//ȡ��Ƭѡ
	if(r1)return 1;
	else return 0;
}
//��ȡSD������������
uint32_t SD_GetSectorCount(void)
{
    uint8_t csd[16];
    uint32_t Capacity;  
    uint8_t n;
		uint16_t csize;  					    
	//ȡCSD��Ϣ������ڼ��������0
    if(SD_GETCSD(csd)!=0) return 0;	    
    //���ΪSDHC�����������淽ʽ����
    if((csd[0]&0xC0)==0x40)	 //V2.00�Ŀ�
    {	
		csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
		Capacity = (uint32_t)csize << 10;//�õ�������	 		   
    }else//V1.XX�Ŀ�
    {	
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
		Capacity= (uint32_t)csize << (n - 9);//�õ�������   
    }
    return Capacity;
}
int MSD0_GetCardInfo(PMSD_CARDINFO SD0_CardInfo)
{
  uint8_t r1;
  uint8_t CSD_Tab[16];
  uint8_t CID_Tab[16];

  /* Send CMD9, Read CSD */
  r1 = SD_sendcmd(CMD9, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(SD_ReceiveData(CSD_Tab, 16))
  {
	return 1;
  }

  /* Send CMD10, Read CID */
  r1 = SD_sendcmd(CMD10, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(SD_ReceiveData(CID_Tab, 16))
  {
	return 2;
  }  

  /* Byte 0 */
  SD0_CardInfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  SD0_CardInfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  SD0_CardInfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  SD0_CardInfo->CSD.TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  SD0_CardInfo->CSD.NSAC = CSD_Tab[2];
  /* Byte 3 */
  SD0_CardInfo->CSD.MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  SD0_CardInfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  SD0_CardInfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  SD0_CardInfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  SD0_CardInfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  SD0_CardInfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  SD0_CardInfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  SD0_CardInfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  SD0_CardInfo->CSD.Reserved2 = 0; /* Reserved */
  SD0_CardInfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  SD0_CardInfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  SD0_CardInfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  SD0_CardInfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  SD0_CardInfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  SD0_CardInfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  SD0_CardInfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  SD0_CardInfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  SD0_CardInfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  SD0_CardInfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  SD0_CardInfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  SD0_CardInfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  SD0_CardInfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  SD0_CardInfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  SD0_CardInfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  SD0_CardInfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  SD0_CardInfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  SD0_CardInfo->CSD.Reserved3 = 0;
  SD0_CardInfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  SD0_CardInfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  SD0_CardInfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  SD0_CardInfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  SD0_CardInfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  SD0_CardInfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  SD0_CardInfo->CSD.ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  SD0_CardInfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  SD0_CardInfo->CSD.Reserved4 = 1;

  if(SD0_CardInfo->CardType == V2HC)
  {
	 /* Byte 7 */
	 SD0_CardInfo->CSD.DeviceSize = (uint16_t)(CSD_Tab[8]) *256;
	 /* Byte 8 */
	 SD0_CardInfo->CSD.DeviceSize += CSD_Tab[9] ;
  }

  SD0_CardInfo->Capacity = SD0_CardInfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
  SD0_CardInfo->BlockSize = MSD_BLOCKSIZE;

  /* Byte 0 */
  SD0_CardInfo->CID.ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  SD0_CardInfo->CID.OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  SD0_CardInfo->CID.OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  SD0_CardInfo->CID.ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  SD0_CardInfo->CID.ProdName2 = CID_Tab[7];
  /* Byte 8 */
  SD0_CardInfo->CID.ProdRev = CID_Tab[8];
  /* Byte 9 */
  SD0_CardInfo->CID.ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[12];
  /* Byte 13 */
  SD0_CardInfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  SD0_CardInfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  SD0_CardInfo->CID.ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  SD0_CardInfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  SD0_CardInfo->CID.Reserved2 = 1;

  return 0;  
}


//дSD��
//buf:���ݻ�����
//sector:��ʼ����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
uint8_t SD_WriteDisk(uint8_t*buf,uint32_t sector,uint8_t cnt)
{
	uint8_t r1;
	if(SD_TYPE!=V2HC)sector *= 512;//ת��Ϊ�ֽڵ�ַ
	if(cnt==1)
	{
		r1=SD_sendcmd(CMD24,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
			r1=SD_SendBlock(buf,0xFE);//д512���ֽ�	   
		}
	}else
	{
		if(SD_TYPE!=MMC)
		{
			SD_sendcmd(CMD55,0,0X01);	
			SD_sendcmd(CMD23,cnt,0X01);//����ָ��	
		}
 		r1=SD_sendcmd(CMD25,sector,0X01);//����������
		if(r1==0)
		{
			do
			{
				r1=SD_SendBlock(buf,0xFC);//����512���ֽ�	 
				buf+=512;  
			}while(--cnt && r1==0);
			r1=SD_SendBlock(0,0xFD);//����512���ֽ� 
		}
	}   
	SD_CS(0);//ȡ��Ƭѡ
	return r1;//
}	
//��SD��
//buf:���ݻ�����
//sector:����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
uint8_t SD_ReadDisk(uint8_t*buf,uint32_t sector,uint8_t cnt)
{
	uint8_t r1;
	if(SD_TYPE!=V2HC)sector <<= 9;//ת��Ϊ�ֽڵ�ַ
	if(cnt==1)
	{
		r1=SD_sendcmd(CMD17,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
			r1=SD_ReceiveData(buf,512);//����512���ֽ�	   
		}
	}else
	{
		r1=SD_sendcmd(CMD18,sector,0X01);//����������
		do
		{
			r1=SD_ReceiveData(buf,512);//����512���ֽ�	 
			buf+=512;  
		}while(--cnt && r1==0); 	
		SD_sendcmd(CMD12,0,0X01);	//����ֹͣ����
	}   
	SD_CS(0);//ȡ��Ƭѡ
	return r1;//
}




uint8_t spi_readwrite(uint8_t Txdata){
	uint8_t Rxdata;	
	HAL_SPI_TransmitReceive(&hspi1,&Txdata,&Rxdata,1,100);
	return Rxdata;
}
//SPI1����������
void SPI_setspeed(uint8_t speed){
	hspi1.Init.BaudRatePrescaler = speed;
}






///////////////////////////END//////////////////////////////////////


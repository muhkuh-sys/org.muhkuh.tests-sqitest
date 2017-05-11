/***************************************************************************
 *   Copyright (C) 2008 by Hilscher GmbH                                   *
 *   cthelen@hilscher.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "version.h"
#include "netx_consoleapp.h"
#include "rdy_run.h"
#include "uprintf.h"
#include "systime.h"
#include "netx90_sqi_test.h"
#include "sqitest_interface.h"
#include "boot_drv_sqi.h"
/* ------------------------------------- */


/*
   W25Q32BV commands:
   Jedec ID: 0x9f  ef 40 16 / ef 60 16
   Read SFDP: 0x5a, Addr, 1 Dummy byte
*/
#define CMD_ENTER_QPI_MODE (0x38)
#define CMD_EXIT_QPI_MODE (0xff)
#define CMD_READ_JEDEC_ID (0x9f)
#define CMD_READ_SR1 (0x05)
#define CMD_READ_SR2 (0x35)
#define CMD_READ_SR3 (0x15)
#define CMD_WRITE_SR2 (0x31)
#define CMD_WRITE_ENABLE (0x06)
#define CMD_WRITE_ENABLE_VOLATILE_SR (0x50)
#define MSK_SR2_QE (0x02)
#define MSK_SR1_BUSY (0x01)
#define CMD_FAST_READ (0x0b) /* valid in SPI and QPI mode, 2 dummy cycles */


const unsigned char aucJEDEC_ID_SPI[] = {
	0xef, 0x40, 0x16
};

const unsigned char aucJEDEC_ID_QPI[] = {
	0xef, 0x60, 0x16
};


/* ------------------------------------- */


void hexdump_mini(const unsigned char* pucData, size_t sizData);
void hexdump_mini_maxsize(const unsigned char* pucData, size_t sizData, size_t sizMaxSize);
void hexdump1(const unsigned char* pucData, size_t sizData, size_t sizBytesPerLine);
int check_response(unsigned char *pucResponse, const unsigned char *pucDatasheet, size_t sizLen, const char *pcName);

int sqi_init(SPI_CONFIGURATION_T *ptSpiCnf, SPI_CFG_T *ptSqiCfg);

int sqi_test(SPI_CFG_T *ptSqiCfg, SQITEST_PARAM_T *ptParam);

/* generic SQI command exchange:
   Send a number of bytes, a number of dummy bytes and receive a number of bytes. */
void sqi_command(SPI_CFG_T *ptSqiCfg, 
	const unsigned char *pucCmd, size_t sizCmdSize, 
	size_t sizDummyBytes, 
	unsigned char *pucResp, size_t sizResp);

/* Send a command byte, send dummy bytes (optional), receive bytes */
void sqi_cmd_response(SPI_CFG_T *ptSqiCfg, 
	const unsigned char ucCmd, 
	size_t sizDummyBytes, 
	unsigned char *pucResp, size_t sizResp);

/* Send a command byte. */
void sqi_cmd(SPI_CFG_T *ptSqiCfg, const unsigned char ucCmd);

/* Send command to enter QPI mode and setup interface for 4 bit mode. */
int set_1bit(SPI_CFG_T *ptSqiCfg);

/* Send command to exit QPI mode and setup interface for 1 bit mode. */
int set_4bit(SPI_CFG_T *ptSqiCfg);



void hexdump_mini(const unsigned char* pucData, size_t sizData)
{
	unsigned int i;
	for (i=0; i<sizData; i++)
	{
		uprintf("%02x ", pucData[i]);
	}
}

void hexdump_mini_maxsize(const unsigned char* pucData, size_t sizData, size_t sizMaxSize)
{
	unsigned int i;
	for (i=0; i<sizData && i <sizMaxSize; i++)
	{
		uprintf("%02x ", pucData[i]);
	}
	
	if (sizData > sizMaxSize)
	{
		uprintf("<...>");
	}
}

/*
	boot_drv_sqi_init_b   status 0/-1
	pfnSetBusWidth        status 0/-1
	pfnSelect             no return value
	pfnSendData           always returns status 0
	pfnReceiveData        always returns status 0
	pfnSendDummy          always returns status 0
	
	transaction types: 
	command code, response data (RDID, RES)
	command code, no response (QPI enable/disable)
*/

void sqi_command(SPI_CFG_T *ptSqiCfg, 
	const unsigned char *pucCmd, size_t sizCmd, size_t sizDummyBytes, 
	unsigned char *pucResp, size_t sizResp)
{
	ptSqiCfg->pfnSelect(ptSqiCfg, 1);
	if ((pucCmd!=NULL) && (sizCmd>0))
	{
		uprintf("Cmd: ");
		hexdump_mini_maxsize(pucCmd, sizCmd, 64);
		uprintf("  ");
		ptSqiCfg->pfnSendData(ptSqiCfg, pucCmd, sizCmd);
	}
	
	if (sizDummyBytes != 0)
	{
		ptSqiCfg->pfnSendDummy(ptSqiCfg, sizDummyBytes);
		//ptSqiCfg->pfnSendIdleCycles(ptSqiCfg, sizDummyBytes*8);
	}
	
	if ((pucResp!=NULL) && (sizResp>0))
	{
		ptSqiCfg->pfnReceiveData(ptSqiCfg, pucResp, sizResp);
		uprintf("  Response: ");
		hexdump_mini_maxsize(pucResp, sizResp, 64);
	}
	
	ptSqiCfg->pfnSelect(ptSqiCfg, 0);
	uprintf("\n");
}

void sqi_cmd_response(SPI_CFG_T *ptSqiCfg, 
	const unsigned char ucCmd, size_t sizDummyBytes, 
	unsigned char *pucResp, size_t sizResp)
{
	unsigned char aucCmd[1];
	aucCmd[0] = ucCmd;
	sqi_command(ptSqiCfg, &aucCmd[0], sizeof(aucCmd), sizDummyBytes, pucResp, sizResp);
}

void sqi_cmd(SPI_CFG_T *ptSqiCfg, const unsigned char ucCmd)
{
	unsigned char aucCmd;
	aucCmd = ucCmd;
	sqi_command(ptSqiCfg, &aucCmd, sizeof(aucCmd), 0, NULL, 0);
}




int set_4bit(SPI_CFG_T *ptSqiCfg)
{
	int iRes;
	uprintf("Enable QPI (1-bit) ");
	sqi_cmd(ptSqiCfg, CMD_ENTER_QPI_MODE);
	uprintf("Setting bus width to 4 bits\n");
	iRes = ptSqiCfg->pfnSetBusWidth(ptSqiCfg, SPI_BUS_WIDTH_4BIT);
	uprintf("Result: %d\n", iRes);
	return iRes;
}

int set_1bit(SPI_CFG_T *ptSqiCfg)
{
	int iRes;
	uprintf("Disable QPI (4-bit) ");
	sqi_cmd(ptSqiCfg, CMD_EXIT_QPI_MODE);
	uprintf("Setting bus width to 1 bit\n");
	iRes = ptSqiCfg->pfnSetBusWidth(ptSqiCfg, SPI_BUS_WIDTH_1BIT);
	uprintf("Result: %d\n", iRes);
	return iRes;
}

/* Initialize/Configure SQI interface according to the settings in ptSpiCnf.

	ptSpiCnf [in]
	ptSqiCfg [out]
*/
int sqi_init(SPI_CONFIGURATION_T *ptSpiCnf, SPI_CFG_T *ptSqiCfg)
{
	int iRes;
	unsigned int uiUnit;
	unsigned int uiChipSelect;
	BOOT_SPI_CONFIGURATION_T tBootSpiCnf; 
	
	tBootSpiCnf.ulInitialSpeedKhz = ptSpiCnf->ulInitialSpeedKhz;
	tBootSpiCnf.ucDummyByte = 0xffU; /* todo: check */
	tBootSpiCnf.ucMode = (unsigned char) ptSpiCnf->uiMode;
	tBootSpiCnf.ucIdleConfiguration = (unsigned char) ptSpiCnf->uiIdleCfg;
	
	uiUnit = ptSpiCnf->uiUnit;
	uiChipSelect = ptSpiCnf->uiChipSelect;
	
	uprintf("boot_drv_sqi_init_b\n");
	iRes = boot_drv_sqi_init_b(ptSqiCfg, &tBootSpiCnf, uiUnit, uiChipSelect);
	uprintf("Result: %d\n", iRes);
	return iRes;
}


int check_response(unsigned char *pucResponse, const unsigned char *pucDatasheet, size_t sizLen, const char *pcName)
{
	int iRes;
	iRes = memcmp(pucResponse, pucDatasheet, sizLen);
	if (iRes == 0)
	{
		uprintf(pcName);
		uprintf(" OK\n");
	}
	else
	{
		uprintf(pcName);
		uprintf(" NOT OK\n");
	}
	return iRes;
}



int sqi_test(SPI_CFG_T *ptSqiCfg, SQITEST_PARAM_T *ptParam)
{
	int iRes;
	
	//unsigned char aucBuffer[1024]; /* general buffer for responses */
	unsigned char aucJedecID[sizeof(aucJEDEC_ID_QPI)];
	unsigned char ucSR1;
	unsigned char ucSR2;
	unsigned char ucSR3;
	unsigned char aucWriteSR[2];
	
	(void) ptParam;
	iRes = 0;
	
	if (iRes == 0)
	{
		uprintf("Setting bus width to 1 bit\n");
		iRes = ptSqiCfg->pfnSetBusWidth(ptSqiCfg, SPI_BUS_WIDTH_1BIT);
		uprintf("Result: %d\n\n", iRes);
	}
	
	if (iRes == 0)
	{
		uprintf("Read device ID\n");
		sqi_cmd_response(ptSqiCfg, CMD_READ_JEDEC_ID, 0, aucJedecID, sizeof(aucJEDEC_ID_SPI));
		
		if (0 == memcmp(aucJedecID, aucJEDEC_ID_SPI, sizeof(aucJEDEC_ID_SPI)))
		{
			uprintf(" - OK\n\n");
		}
		else
		{
			uprintf(" - NOT OK\n\n");
			iRes = -1;
		}
	}
	
	if (iRes == 0)
	{
		uprintf("Read status register 1  ");
		sqi_cmd_response(ptSqiCfg, CMD_READ_SR1, 0, &ucSR1, 1);
		uprintf("\n");
	}

	if (iRes == 0)
	{
		uprintf("Read status register 2  ");
		sqi_cmd_response(ptSqiCfg, CMD_READ_SR2, 0, &ucSR2, 1);
		uprintf("\n");
	}
	
	if (iRes == 0)
	{
		uprintf("Read status register 3  ");
		sqi_cmd_response(ptSqiCfg, CMD_READ_SR3, 0, &ucSR3, 1);
		uprintf("\n");
	}
	
	if (iRes == 0)
	{
		uprintf("Write enable (volatile SR) ");
		sqi_cmd(ptSqiCfg, CMD_WRITE_ENABLE_VOLATILE_SR);
		uprintf("\n");
	}
	
	if (iRes == 0)
	{
		uprintf("Enable Quad mode ");
		aucWriteSR[0] = CMD_WRITE_SR2;
		aucWriteSR[1] = ucSR2 | MSK_SR2_QE;
		sqi_command(ptSqiCfg, aucWriteSR, sizeof(aucWriteSR), 0, NULL, 0);
	}
	
	if (iRes == 0)
	{
		uprintf("Read status register 2  ");
		sqi_cmd_response(ptSqiCfg, CMD_READ_SR2, 0, &ucSR2, 1);
		uprintf("\n");
		
		if ((ucSR2 & MSK_SR2_QE)== 0)
		{
			uprintf("Failed to set Quad Enable bit!\n");
			iRes = -1;
		}
		uprintf("\n");
	}
	
	if (iRes == 0)
	{
		uprintf("Read device ID (Quad mode enabled, but Enter QPI command not sent)\n");
		sqi_cmd_response(ptSqiCfg, CMD_READ_JEDEC_ID, 0, aucJedecID, sizeof(aucJEDEC_ID_QPI));
		uprintf("\n\n");
	}
	
	if (iRes == 0)
	{
		iRes = set_4bit(ptSqiCfg);
		uprintf("\n");
	}
	
	if (iRes == 0)
	{
		uprintf("Read device ID (QPI)  ");
		sqi_cmd_response(ptSqiCfg, CMD_READ_JEDEC_ID, 0, aucJedecID, sizeof(aucJEDEC_ID_QPI));
		if (0 == memcmp(aucJedecID, aucJEDEC_ID_QPI, sizeof(aucJEDEC_ID_QPI)))
		{
			uprintf(" - OK\n\n");
		}
		else
		{
			uprintf(" - NOT OK\n\n");
			iRes = -1;
		}
	}
	
	if (iRes == 0)
	{
		iRes = set_1bit(ptSqiCfg);
		uprintf("\n");
	}
	
	return iRes;
}


NETX_CONSOLEAPP_RESULT_T netx_consoleapp_main(NETX_CONSOLEAPP_PARAMETER_T *ptTestParam)
{
	NETX_CONSOLEAPP_RESULT_T tResult;
	int iResult;
	tFlasherInputParameter *ptAppParams;
	//OPERATION_MODE_T tOpMode;
	SPI_CFG_T tSqiCfg; /* this is filled in by the driver */

	
	ptAppParams = (tFlasherInputParameter*)ptTestParam->pvInitParams;
	//tOpMode = ptAppParams->tOperationMode; 
	tResult=NETX_CONSOLEAPP_RESULT_ERROR;
	iResult=-1;
	
	/* Switch off the SYS led. */
	rdy_run_setLEDs(RDYRUN_OFF);
	
	/* Configure the systime, used by progress functions. */
	systime_init();  

	/* say hi */
	uprintf(
	"\f\n\n\n\nnetx 90 SQI Test v" FLASHER_VERSION_ALL " " FLASHER_VERSION_VCS "\n\n");
	
	iResult = sqi_init(&(ptAppParams->uParameter.tSqitest.tSpi), &tSqiCfg);
	if (iResult == 0)
	{
		iResult = sqi_test(&tSqiCfg, &(ptAppParams->uParameter.tSqitest.tSqitest_Param));
	}
		
	if( iResult==0 )
	{
		/*  Operation OK! */
		tResult=NETX_CONSOLEAPP_RESULT_OK;
		uprintf("* OK *\n");
		rdy_run_setLEDs(RDYRUN_GREEN);
	}
	else
	{
		tResult=NETX_CONSOLEAPP_RESULT_ERROR;
		/*  Operation failed. */
		rdy_run_setLEDs(RDYRUN_YELLOW);
	}

	return tResult;
}

#ifndef	__BMS_309PROC_H__
#define	__BMS_309PROC_H__


void bms309Proc(void);
void bms309TimerSet(uint8_t on);

extern void Initial(void);
extern void WakeUpProcess(void);
extern void CurProcess(void);
extern void CaliProcess(void);
extern void VolProcess(void);
extern void BalProcess(void);
extern void GaugeManage(void);
extern void ShutDownProcess(void);
extern void IntoIdle(void);
extern void IntoSleep(void);
extern void SystemIntoSleep(void);
extern void ProtectProcess(void);
extern void E2PRomBKCheck(void);
extern void	BatteryInfoManage(void);
extern void	LEDPowerOn(void);
extern void	LEDPowerOff(void);
extern void ResetInit(void);
extern void E2PRomBKProcess(void);
extern void ISPProcess(void);
extern void ResetAFE(void);
extern void UpdataAfeConfig(void);
extern void IntoShutDown(void);
extern void AFECheck(void);
extern void AFERdFlag(void); 
extern void InitClk(void); 
extern uint8_t FlashProcess(void);
extern void BleOnorOff(void);
extern void BleShutDown(void);
extern void BleDisplay(void);
extern void InitSysPara(void);
extern void RamCheckProcess(void);

#endif



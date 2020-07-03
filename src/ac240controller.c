
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "AcqirisD1Import.h"
#include "ac240controller.h"


#define AC_FPGA_READ         0
#define AC_FPGA_WRITE        1
#define AC_REGISTER_INDIRECT_ACCESS_PORT        0
#define AC_REGISTER_INDIRECT_ADDRESS            1
#define AC_REGISTER_BUFFER_IDENTIFIER           2
#define AC_REGISTER_MAIN_CONTROL                3
#define AC_REGISTER_CODE_PROTECTION             4
#define AC_REGISTER_MAIN_STATUS                 6
#define AC_REGISTER_TEMPERATURE                 7
#define AC_REGISTER_DEBUS_CONTROL               8
#define AC_REGISTER_FFT_MAIN_CONTROL           64
#define AC_REGISTER_FFT_STATUS                 65
#define AC_REGISTER_FFT_NUMBER_OF_ACCUMULATION 66
#define AC_REGISTER_FFT_CONFIGURATION          67
#define AC_REGISTER_FRONTPANEL_CONTROL         68
#define AC_REGISTER_FFT_ACCUMULATOR_CLEAR      69
#define AC_REGISTER_FFT_OVERFLOW_STATUS        70
#define AC_REGISTER_FFT_ADC_OVERFLOW_COUNTER   71
#define AC_BUFFER_DE         0x08
#define AC_BUFFER_FFT_OUTPUT 0x81
#define AC_BUFFER_WINDOW     0x82

#define AC_WINDOWFUNCTION_NONE      0
#define AC_WINDOWFUNCTION_HANNING   1
#define AC_WINDOWFUNCTION_HAMMING   2
#define AC_WINDOWFUNCTION_FLATTOP   3
#define AC_WINDOWFUNCTION_BLACKMANN 4


static long error_cnt = 0;
void error_handler(ViSession ac_id, char *msg)
{
  error_cnt += 1;
  
  printf("!!!!\n");
  printf("%s\n", msg);
  printf("**** aborted ****\n\n");
  
  printf("System shuting-down\n");
  printf("-------------------\n");
  
  if(ac_id != -1)
    {
      if(error_cnt == 1)
	{
	  ac240_fpga_stop_fft(ac_id);
	}
      ac240_stop_acquisition(ac_id);
    }
  
  ac240_close();
  
  printf("System finished\n\n");
  exit(1);
}

void ac240_get_number_boards(long *n_boards)
{
  ViStatus ac_stat;
  char emsg[100];
  
  printf("ac240_get_number_boards(*n_boards) ");
  fflush(stdout);
  ac_stat = Acqrs_getNbrInstruments(n_boards);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default: 
      sprintf(emsg, "ERROR: AcqrsD1_getNbrInstruments (%ld)", ac_stat);
      error_handler(-1, emsg);
    }
  
  printf("--> n_boards = %ld\n", *n_boards);
}

void ac240_open(ViString resource, ViSession *ac_id)
{
  ViStatus ac_stat;
  char emsg[100];
  
  printf("ac240_open(\"%s\", *ac_id) ", resource);
  fflush(stdout);
  ac_stat = AcqrsD1_InitWithOptions(resource, VI_FALSE, VI_FALSE, "cal=0,dma=1", ac_id);
  
  switch(ac_stat)
    {
    case 0: 
    case -1074116608: 
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_InitWithOptions (%ld)", ac_stat);
      error_handler(*ac_id, emsg);
    }
  
  printf("--> ac_id = %ld\n", *ac_id);
}

void ac240_close()
{
  ViStatus ac_stat;
  char emsg[100];
  
  printf("ac240_close() ");
  fflush(stdout);
  ac_stat= AcqrsD1_closeAll();

  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_closeAll (%ld)", ac_stat);
      error_handler(-1, emsg);
    }
  
  printf("\n");
}

void ac240_fpga_clear(ViSession ac_id)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_fpga_clear(ac_id=%ld) ", ac_id);
  fflush(stdout);
  ac_stat = AcqrsD1_configLogicDevice(ac_id, "Block1Dev1", NULL, 1);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configLogicDevice (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }

  printf("\n");
}

void ac240_fpga_config(ViSession ac_id, ViString path, ViInt32 flags)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_config_fpga(ac_id=%ld, path=\"%s\", flags=%ld) ", ac_id, path, flags);
  fflush(stdout);
  ac_stat = AcqrsD1_configLogicDevice(ac_id, "Block1Dev1", path, flags);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configLogicDevice (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_config_mode(ViSession ac_id, ViInt32 mode, ViInt32 flags)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_config_mode(ac_id=%ld, mode=%ld, flags=%ld) ", ac_id, mode, flags);
  fflush(stdout);
  ac_stat = AcqrsD1_configMode(ac_id, mode, 0, flags);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configMode (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_config_external_clock(ViSession ac_id, ViInt32 type)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_config_external_clock(ac_id=%ld, type=%ld) ", ac_id, type);
  fflush(stdout);
  ac_stat = AcqrsD1_configExtClock(ac_id, type, 0.0, 0, 0.0, 0.0);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configExtClock (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_config_channel_combination(ViSession ac_id, ViInt32 conv_per_ch, ViInt32 used_ch)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_config_channel_combination(ac_id=%ld, conv_per_ch=%ld, used_ch=%ld) ", ac_id, conv_per_ch, used_ch);
  fflush(stdout);
  ac_stat = AcqrsD1_configChannelCombination(ac_id, conv_per_ch, used_ch);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configChannelCombination (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_config_horizontal(ViSession ac_id, ViReal64 sampling, ViReal64 delay)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_config_horizontal(ac_id=%ld, sampling=%.2e, delay=%.2e) ", ac_id, sampling, delay);
  fflush(stdout);
  ac_stat = AcqrsD1_configHorizontal(ac_id, sampling, delay);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configHorizontal (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_config_vertical(ViSession ac_id, ViReal64 fullscale, ViReal64 offset)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_config_vertical(ac_id=%ld, fullscale=%.2e, offset=%.2e) ", ac_id, fullscale, offset);
  fflush(stdout);
  ac_stat = AcqrsD1_configVertical(ac_id, 1, fullscale, offset, 3, 0);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_configVertical (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_calibrate(ViSession ac_id)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_calibrate(ac_id=%ld) ", ac_id);
  fflush(stdout);
  ac_stat = AcqrsD1_calibrate(ac_id);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_calibrate (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_stop_acquisition(ViSession ac_id)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_stop_aquisition(ac_id=%ld) ", ac_id);
  fflush(stdout);
  ac_stat= AcqrsD1_stopAcquisition(ac_id);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_stopAcquisition (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("\n");
}

void ac240_start_acquisition(ViSession ac_id)
{
  ViStatus ac_stat;
  char emsg[100];

  printf("ac240_start_aquisition(ac_id=%ld) ---->\n", ac_id);
  ac240_stop_acquisition(ac_id);
  ac240_config_mode(ac_id, 1, 0);
  ac_stat= AcqrsD1_acquire(ac_id);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_acquire (%ld)", ac_stat);
      error_handler(ac_id, emsg);
    }
  
  printf("<---- done : ac240_start_aquisition(ac_id=%ld)\n", ac_id);
}

void ac240_get_board_temperature(ViSession ac_id, double *temp)
{
  long tReg;
  AcqrsD1_getInstrumentInfo(ac_id, "Temperature", &tReg);
  *temp = (double)tReg;
}

void FPGA_write(ViSession ac_id, long regID, long nbrValues, long *dataArrayP)
{
  ViInt32 FPGA_WRITE = 1;

  ViStatus ac_stat;
  char emsg[100];
  
  ac_stat = AcqrsD1_logicDeviceIO(ac_id, "Block1Dev1", regID, nbrValues, dataArrayP, FPGA_WRITE, 0);
  
  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_logicDeviceIO (%ld) :: WRITE(%ld)", ac_stat, regID);
      error_handler(ac_id, emsg);
    }
}

void FPGA_read(ViSession ac_id, long regID, long nbrValues, long *dataArrayP)
{
  ViInt32 FPGA_READ = 0;

  ViStatus ac_stat;
  char emsg[100];
  
  ac_stat =  AcqrsD1_logicDeviceIO(ac_id, "Block1Dev1", regID, nbrValues, dataArrayP, FPGA_READ, 0);

  switch(ac_stat)
    {
    case 0:
      break;
    default:
      sprintf(emsg, "ERROR: AcqrsD1_logicDeviceIO (%ld) :: READ(%ld)", ac_stat, regID);
      error_handler(ac_id, emsg);
    }
}

void ac240_fpga_enable_temperature_monitor(ViSession ac_id)
{
  long tReg;
  
  printf("ac240_fpga_enable_temperature_monitor(ac_id=%ld) ", ac_id);
  fflush(stdout);
  FPGA_read(ac_id, AC_REGISTER_TEMPERATURE, 1, &tReg);
  
  if ((tReg & 0x8000) == 0)
    {
      tReg |= 0x8000;
      FPGA_write(ac_id, AC_REGISTER_TEMPERATURE, 1, &tReg);
      FPGA_read(ac_id, AC_REGISTER_TEMPERATURE, 1, &tReg);
    }
  
  printf("--> reg = %ld: val = 0x%x\n", AC_REGISTER_TEMPERATURE, tReg);
}

void ac240_fpga_init(ViSession ac_id)
{
  long tReg;
  
  printf("ac240_fpga_init(ac_id=%ld) ", ac_id);
  fflush(stdout);
  
  tReg = 0x00000000;
  FPGA_write(ac_id, AC_REGISTER_DEBUS_CONTROL, 1, &tReg);
  
  tReg = 0x000C0020;
  FPGA_write(ac_id, AC_REGISTER_FFT_CONFIGURATION, 1, &tReg);

  printf("\n");
}

void ac240_fpga_set_windowfunction(ViSession ac_id, long window_function)
{
  long i;
  long AC_SPECTRALLINES_NUM = 16384;
  double window[AC_SPECTRALLINES_NUM];
  
  printf("ac240_fpga_load_windowfunction(ac_id=%ld) ", ac_id);
  fflush(stdout);

  for (i = 0; i < AC_SPECTRALLINES_NUM; i++)
    {
      double w = 3.1415926535*i/AC_SPECTRALLINES_NUM;
      double cos1 = cos(w);
      double cos2 = cos(2.0*w);
      switch(window_function)
	{
	case AC_WINDOWFUNCTION_NONE:      window[i] = 1.0;                                  break;
	case AC_WINDOWFUNCTION_HANNING:   window[i] = 0.5     - 0.5 *  cos1;                break;
	case AC_WINDOWFUNCTION_HAMMING:   window[i] = 0.54    - 0.46*  cos1;                break;
	case AC_WINDOWFUNCTION_FLATTOP:   window[i] = 0.28106 - 0.5209*cos1 + 0.19804*cos2; break;
	case AC_WINDOWFUNCTION_BLACKMANN: window[i] = 0.42    - 0.5 *  cos1 + 0.08 *  cos2; break;
	}
    }
  
  long windowFunction[AC_SPECTRALLINES_NUM/2];
  for (i = 0; i < AC_SPECTRALLINES_NUM/2; i++)
    {
      long winEven = window[i*2] * 256;
      long winOdd  = window[i*2+1]*256;
      if (winEven < -256) winEven = -256;
      if (winEven >  255) winEven =  255;
      if (winOdd  < -256) winOdd = -256;
      if (winOdd  >  255) winOdd =  255;
      windowFunction[i] = ( (winEven & 0x1ff) << 9) + (winOdd & 0x1ff);
    }
  
  long startAddr  = 0x0;
  long bufAddress = AC_BUFFER_WINDOW;
  FPGA_write(ac_id, AC_REGISTER_INDIRECT_ADDRESS, 1, &startAddr);
  FPGA_write(ac_id, AC_REGISTER_BUFFER_IDENTIFIER, 1, &bufAddress);
  FPGA_write(ac_id, AC_REGISTER_INDIRECT_ACCESS_PORT, AC_SPECTRALLINES_NUM/2, windowFunction);

  printf("\n");
}

void ac240_fpga_set_integtime(ViSession ac_id, double *integ_sec, double fft_sec)
{
  long nbrDoubleBlocks;
  
  printf("ac240_fpga_set_integtime(ac_id=%ld, integ_sec=%.4e) ", ac_id, *integ_sec);
  fflush(stdout);
  
  // nbrDoubleBlocks = (((long)(*integ_sec / 0.000016384)) & 0xfffffffe)/2;
  nbrDoubleBlocks = (((long)(*integ_sec / fft_sec)) & 0xfffffffe)/2;
  FPGA_write(ac_id, AC_REGISTER_FFT_NUMBER_OF_ACCUMULATION, 1, &nbrDoubleBlocks);
  // *integ_sec = nbrDoubleBlocks * 0.000016384 * 2;
  *integ_sec = nbrDoubleBlocks * fft_sec * 2;

  printf("--> fpgainteg = %.4e, count = %ld\n", *integ_sec, nbrDoubleBlocks*2);
}

void ac240_fpga_start_fft(ViSession ac_id)
{
  long tReg;
  
  printf("ac240_fpga_start_fft(ac_id=%ld) ", ac_id);
  fflush(stdout);
  
  tReg = 0x00000001;
  FPGA_write(ac_id, AC_REGISTER_FFT_ACCUMULATOR_CLEAR, 1, &tReg);
  
  usleep(100000);
  
  tReg = 0x80000000;
  FPGA_write(ac_id, AC_REGISTER_DEBUS_CONTROL, 1, &tReg);
  
  tReg = 0x00000000;
  FPGA_write(ac_id, AC_REGISTER_MAIN_CONTROL, 1, &tReg);
  
  tReg  = 0x00000001;
  FPGA_write(ac_id, AC_REGISTER_FFT_MAIN_CONTROL, 1, &tReg);
  
  printf("\n");
}

void ac240_fpga_stop_fft(ViSession ac_id)
{
  long tReg;
  
  printf("ac240_fpga_stop_fft(ac_id=%ld) ", ac_id);
  fflush(stdout);
  
  tReg = 0x00000000;
  FPGA_write(ac_id, AC_REGISTER_FFT_MAIN_CONTROL, 1, &tReg);
  
  usleep(100000);
  
  tReg = 0x00000000;
  FPGA_write(ac_id, AC_REGISTER_DEBUS_CONTROL, 1, &tReg);
  
  tReg = 0x00000000;
  FPGA_write(ac_id, AC_REGISTER_MAIN_CONTROL, 1, &tReg);
  
  printf("\n");
}

void ac240_fpga_init_sepctrum_streaming(ViSession ac_id)
{
  long tReg;
  
  printf("ac240_fpga_init_spectrum_streaming(ac_id=%ld) ", ac_id);
  fflush(stdout);
  
  tReg = 0x0;
  FPGA_write(ac_id, AC_REGISTER_INDIRECT_ADDRESS, 1, &tReg);
  
  tReg = AC_BUFFER_FFT_OUTPUT;
  FPGA_write(ac_id, AC_REGISTER_BUFFER_IDENTIFIER, 1, &tReg);

  printf("\n");  
}

void ac240_fpga_get_spectrum_stream(ViSession ac_id, long integ_count, float spectrum[])
{
  const long NUM_CH = 16384;
  long tReg;
  long raw[NUM_CH*2];

  int i, j;

  for(i=0; i<NUM_CH; i++){ spectrum[i] = 0.0; }
  for(i=0; i<NUM_CH*2; i++){ raw[i] = 0; }

  for(i=0; i<5; i++)
    {
      FPGA_read(ac_id, AC_REGISTER_FFT_STATUS, 1, &tReg);
      if ((tReg & 0x80000000) != 0)
        {
          FPGA_read(ac_id, AC_REGISTER_INDIRECT_ACCESS_PORT, 2*NUM_CH, raw);
        }
    }

  for(i=0; i<integ_count; i++)
    {
      while(1)
	{
	  FPGA_read(ac_id, AC_REGISTER_FFT_STATUS, 1, &tReg);
	  if ((tReg & 0x80000000) != 0)
	    {
	      break;
	    }
          usleep(10);
	}
      
      FPGA_read(ac_id, AC_REGISTER_INDIRECT_ACCESS_PORT, 2*NUM_CH, raw);
      for(j=0; j<NUM_CH; j++)
	{
	  double hi = 16.0*(double)((unsigned long)raw[2*j]);
	  double lo = (double)(raw[2*j + 1]&0xf);
	  spectrum[j] += (float)(hi + lo);
	}
    }
}

void ac240_fpga_get_adc_overflow(ViSession ac_id, long *overflow)
{
  FPGA_read(ac_id, AC_REGISTER_FFT_ADC_OVERFLOW_COUNTER, 1, overflow);
}

void ac240_fpga_get_fpga_overflow(ViSession ac_id, double *overflow)
{
  long tReg = 0x0;
  FPGA_read(ac_id, AC_REGISTER_FFT_OVERFLOW_STATUS, 1, &tReg);
  *overflow = ((tReg & 0x2) == 0x2);
}

void ac240_fpga_get_temperature(ViSession ac_id, double *temp)
{
  long tReg = 0x0;
  FPGA_read(ac_id, AC_REGISTER_TEMPERATURE, 1, &tReg);
  *temp = (double)(tReg & 0x1FFF) * 0.0625;  
}


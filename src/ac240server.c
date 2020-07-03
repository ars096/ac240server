
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include "AcqirisD1Import.h"
#include "ac240controller.h"
#include "ac240socket.h"


extern volatile sig_atomic_t do_finalize = 0;

typedef struct _ac240msg{
  double timestamp;
  float spectrum[16384];
  double total_power;
  double integ_time;
  double temp_board;
  double temp_fpga;
  double overflow_fpga;
  double overflow_ad;
}ac240msg;


double get_unixtime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void parse_args(
  int argc,
  char *argv[],
  double *interval,
  double *ad,
  long *window,
  double *integ,
  double *start,
  double *fpgainteg,
  long *port,
  long *showstatus
)
{
  const char *optstring = "";
  
  const struct option longopts[] = {
    {"bw",         required_argument, 0, 'b'},
    {"ad",         required_argument, 0, 'a'},
    {"window",     required_argument, 0, 'w'},
    {"integ",      required_argument, 0, 'i'},
    {"start",      required_argument, 0, 's'},
    {"fpgainteg",  required_argument, 0, 'f'},
    {"port",       required_argument, 0, 'p'},
    {"showstatus", required_argument, 0, 'd'},
    {0,                            0, 0,   0},
  };

  int longindex = 0;
  int v;
  char *endp;

  while((v = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1)
    {
      if(v == 'b')
	{
	  if(strcmp(optarg, "1000")==0){ *interval = 5e-10; }
	  else if(strcmp(optarg, "500")==0){ *interval = 1e-9; }
	  else if(strcmp(optarg, "250")==0){ *interval = 2e-9; }
	  else if(strcmp(optarg, "200")==0){ *interval = 2.5e-9; }
	  else if(strcmp(optarg, "100")==0){ *interval = 5e-9; }
	  else
	    {
	      printf("ERROR: bad argument: bw = %s\n\n", optarg);
	      exit(1);
	    }
	}
      
      if(v == 'a')
	{
	  if(strcmp(optarg, "5")==0){ *ad = 5; }
	  else if(strcmp(optarg, "2")==0){ *ad = 2; }
	  else if(strcmp(optarg, "1")==0){ *ad = 1; }
	  else if(strcmp(optarg, "0.5")==0){ *ad = 0.5; }
	  else if(strcmp(optarg, "0.2")==0){ *ad = 0.2; }
	  else if(strcmp(optarg, "0.1")==0){ *ad = 0.1; }
	  else if(strcmp(optarg, "0.05")==0){ *ad = 0.05; }
	  else
	    {
	      printf("ERROR: bad argument: ad = %s\n\n", optarg);
	      exit(1);
	    }
	}
      
      if(v == 'w')
	{
	  if(strcmp(optarg, "none")==0){ *window = 0; }
	  else if(strcmp(optarg, "hanning")==0){ *window = 1; }
	  else if(strcmp(optarg, "hamming")==0){ *window = 2; }
	  else if(strcmp(optarg, "flattop")==0){ *window = 3; }
	  else if(strcmp(optarg, "blackmann")==0){ *window = 4; }
	  else
	    {
	      printf("ERROR: bad argument: window = %s\n\n", optarg);
	      exit(1);
	    }
	}
      
      if(v == 'i')
	{
	  double val1;
	  val1 = strtod(optarg, &endp);
	  if(*endp != '\0' || val1 < 0.1 || val1 > 100)
	    {
	      printf("ERROR: bad argument: integ = %s\n\n", optarg);
	      exit(1);
	    }
	  else
	    {
	      *integ = val1;
	    }
	}
      
      if(v == 's')
	{
	  double val2;
	  val2 = strtod(optarg, &endp);
	  if(*endp != '\0' || val2 < 0 || val2 > 0.999)
	    {
	      printf("ERROR:bad argument: start = %s\n\n", optarg);
	      exit(1);
	    }
	  else
	    {
	      *start = val2;
	    }
	}
      
      if(v == 'f')
	{
	  double val4;
	  val4 = strtod(optarg, &endp);
	  if(*endp != '\0' || val4 > 1 || val4 < 0)
	    {
	      printf("ERROR: bad argument: fpgainteg = %s\n\n", optarg);
	      exit(1);
	    }
	  else
	    {
	      *fpgainteg = val4;
	    }
	}
      
      if(v == 'p')
	{
	  long val3;
	  val3 = strtol(optarg, &endp, 0);
	  if(*endp != '\0' || val3 < 1 || val3 > 65535)
	    {
	      printf("ERROR: bad argument: port = %s\n\n", optarg);
	      exit(1);
	    }
	  else
	    {
	      *port = val3;
	    }
	}

      if(v == 'd')
	{
	  long val5;
	  val5 = strtol(optarg, &endp, 0);
	  if(*endp != '\0' || !(val5==0 || val5==1))
	    {
	      printf("ERROR: bad argument: showstatus = %s\n\n", optarg);
	      exit(1);
	    }
	  else
	    {
	      *showstatus = val5;
	    }
	}
    }  
}


void wait_until_start_time(double start, double integ, double fpgainteg)
{
  double now, dt;
  
  while(1)
    {
      now = get_unixtime();
      dt = (((now/integ) - (long long)(now/integ)) * integ) - start;
      
      if(dt > fpgainteg*0.25 && dt <= fpgainteg*1.25)
	{
	  break;
	}
      
      usleep(10);
      continue;
    }
}


void get_integration_start_time(double end_time, double integ_time, double *start_time)
{
  *start_time = end_time - integ_time;
}

void get_adc_overflow_percent(ViSession ac_id, long adc_num_per_integ, double *overflow_percent)
{
  long overflow_count_per_integ;
  
  ac240_fpga_get_adc_overflow(ac_id, &overflow_count_per_integ);
  *overflow_percent = overflow_count_per_integ / adc_num_per_integ;
}

void get_total_power(float spectrum[], double *total_power)
{
  long i;
  *total_power = 0;
  
  for(i=0; i<16384; i++)
    {
      *total_power += spectrum[i];
    }
}

void print_status(ac240msg *msg)
{
  printf("%.5lf dt=%.5lf tp=%.2e overflow: ad=%2.3lf%% fpga=%ld temp: board=%ld fpga=%.1f\n",
	     msg->timestamp,
	     msg->integ_time,
	     msg->total_power,
	     msg->overflow_ad,
             (long) msg->overflow_fpga,
             (long) msg->temp_board,
	     msg->temp_fpga);
}

void sigint_handler(int sig)
{
  printf("\n");
  printf(">>> Interruption signal is detected <<<\n");
  printf("\n");
  do_finalize = 1;
}

int main(int argc, char *argv[])
{
  nice(-20);

  if(signal(SIGINT, sigint_handler) == SIG_ERR)
    {
      printf("Failed to configure signal handler. System aborted.\n");
      exit(1);
    }
  
  printf("...............\n");
  printf("  ac240server  \n");
  printf("...............\n");
  
  printf("\n");
  printf("Parse input arguments\n");
  printf("---------------------\n");

  double interval = 5e-10;
  double ad = 5.0;
  long window = 0;
  double integ = 1.0;
  double start = 0.0;
  double fpgainteg = 0.01111;
  long port = 24000;
  long showstatus = 1;
  
  parse_args(argc, argv, &interval, &ad, &window, &integ, &start, &fpgainteg, &port, &showstatus);

  printf("interval  = %.1e\n", interval);
  printf("ad        = %.2lf\n", ad);
  printf("window    = %ld\n", window);
  printf("integ     = %.3lf\n", integ);
  printf("fpgainteg = %.3e\n", fpgainteg);
  printf("start     = %.3lf\n", start);
  printf("port      = %ld\n", port);

  
  printf("\n");
  printf("Starting spectrometer\n");
  printf("---------------------\n");
  
  long n_boards;
  ViSession ac_id;

  ac240_get_number_boards(&n_boards);
  ac240_open("PCI::INSTR0", &ac_id);
  ac240_fpga_clear(ac_id);
  ac240_fpga_config(ac_id, "../firmware/AC240FFT2GSs.bit", 0);
  ac240_config_mode(ac_id, 0, 0);
  ac240_config_external_clock(ac_id, 0);
  ac240_config_channel_combination(ac_id, 2, 0x1);
  ac240_config_horizontal(ac_id, interval, 0.0);
  ac240_config_vertical(ac_id, ad, 0.0);
  ac240_fpga_enable_temperature_monitor(ac_id);
  ac240_calibrate(ac_id);
  ac240_start_acquisition(ac_id);

  ac240_fpga_init(ac_id);
  ac240_fpga_set_windowfunction(ac_id, window);
  ac240_fpga_set_integtime(ac_id, &fpgainteg, interval*16384*2);
  ac240_fpga_start_fft(ac_id);

  
  printf("\n");
  printf("Starting socket server\n");
  printf("----------------------\n");

  long server;
  long client;
  long sock_stat = 0;
  
  long integ_count;
  
  ac240msg msg;
  long size;
  
  sock_stat = socket_init(&server, port);
  
  if(sock_stat==0)
    {
      printf("\n");
      printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
      printf("ac240server started (TCP port %ld)\n", port);
      printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
      
      printf("waiting for client connection ...\n");
      
      while(!do_finalize)
	{
	  sock_stat = socket_wait_connection(server, &client);
	  if(sock_stat < 0)
	    {
	      continue;
	    }
	  
	  ac240_fpga_init_sepctrum_streaming(ac_id);
	  integ_count = (long)(integ / fpgainteg);
          msg.integ_time = integ_count * fpgainteg;
	  
          if(showstatus==0){ printf("data streaming ...\n"); }
	  
	  while(!do_finalize)
	    {
	      wait_until_start_time(start, integ, fpgainteg);
	      
	      ac240_fpga_get_spectrum_stream(ac_id, integ_count, &msg.spectrum);
	      
              get_integration_start_time(get_unixtime(), msg.integ_time, &msg.timestamp);
              get_adc_overflow_percent(ac_id, fpgainteg/interval, &msg.overflow_ad);
	      ac240_fpga_get_fpga_overflow(ac_id, &msg.overflow_fpga);
              ac240_get_board_temperature(ac_id, &msg.temp_board);
	      ac240_fpga_get_temperature(ac_id, &msg.temp_fpga);
              get_total_power(&msg.spectrum, &msg.total_power);
	      
              if(showstatus==1){ print_status(&msg); }

	      sock_stat = socket_check_connection(client);
	      if(sock_stat == -1)
		{
		  printf("connection closed by client.\n");
		  break;
		}
	      
	      if((size = send(client, &msg, sizeof(msg), 0)) < 0)
		{
		  printf("send failed, (%ld)\n", size);
		  break;
		}
	      else if(size == 0)
		{
		  printf("connection closed by client.\n");
		  break;
		}
	    }
	  
	  printf("waiting for client connection ...\n");	  
	}
    }
  
  
  printf("\n");
  printf("Stopping spectrometer\n");
  printf("---------------------\n");
  
  ac240_fpga_stop_fft(ac_id);
  ac240_stop_acquisition(ac_id);
  ac240_close();
  
  printf("\n");
  printf("ac240server aborted\n");
  printf("bye\n");
  printf("\n");
  
}

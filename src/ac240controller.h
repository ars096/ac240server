
void error_handler(ViSession ac_id, char *msg);
void ac240_get_number_boards(long *n_boards);
void ac240_open(ViString resource, ViSession *ac_id);
void ac240_close();
void ac240_fpga_clear(ViSession ac_id);
void ac240_fpga_config(ViSession ac_id, ViString path, ViInt32 flags);
void ac240_config_mode(ViSession ac_id, ViInt32 mode, ViInt32 flags);
void ac240_config_external_clock(ViSession ac_id, ViInt32 type);
void ac240_config_channel_combination(ViSession ac_id, ViInt32 conv_per_ch, ViInt32 used_ch);
void ac240_config_horizontal(ViSession ac_id, ViReal64 sampling, ViReal64 delay);
void ac240_config_vertical(ViSession ac_id, ViReal64 fullscale, ViReal64 offset);
void ac240_calibrate(ViSession ac_id);
void ac240_stop_acquisition(ViSession ac_id);
void ac240_start_acquisition(ViSession ac_id);
void ac240_get_board_temperature(ViSession ac_id, double *temp);
void FPGA_write(ViSession ac_id, long regID, long nbrValues, long *dataArrayP);
void FPGA_read(ViSession ac_id, long regID, long nbrValues, long *dataArrayP);
void ac240_fpga_init(ViSession ac_id);
void ac240_fpga_enable_temperature_monitor(ViSession ac_id);
void ac240_fpga_set_windowfunction(ViSession ac_id, long window_function);
void ac240_fpga_set_integtime(ViSession ac_id, double *integ_sec, double fft_sec);
void ac240_fpga_start_fft(ViSession ac_id);
void ac240_fpga_stop_fft(ViSession ac_id);
void ac240_fpga_init_sepctrum_streaming(ViSession ac_id);
void ac240_fpga_get_spectrum_stream(ViSession ac_id, long integ_count, float spectrum[]);
void ac240_fpga_get_adc_overflow(ViSession ac_id, long *overflow);
void ac240_fpga_get_fpga_overflow(ViSession ac_id, double *overflow);
void ac240_fpga_get_temperature(ViSession ac_id, double *temp);




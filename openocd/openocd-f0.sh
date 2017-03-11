openocd -f st_nucleo_f0-chibi.cfg -c 'hla_serial "066CFF505256656767223024"' -c 'gdb_port 3334' -c 'tcl_port 3335' -c 'telnet_port 3336'

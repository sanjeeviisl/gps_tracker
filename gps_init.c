

#include<stdio.h>
#include<gpio_lib.h>
#include<gps_init.h>

void Init_GPS_GSM_Module()
{
sunxi_gpio_init();
sunxi_gpio_set_cfgpin(SUNXI_GPA(7), SUNXI_GPIO_OUTPUT);
sunxi_gpio_output(SUNXI_GPA(7), 1);
sleep(4);
sunxi_gpio_output(SUNXI_GPA(7), 0);
printf("GPS ON\n");
sleep(1);
}

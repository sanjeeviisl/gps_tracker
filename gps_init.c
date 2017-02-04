

#include<stdio.h>
#include<gpio_lib.h>
#include<gps_init.h>

void Init_GPS_GSM_Module()
{
sunxi_gpio_init();
printf("gpio init\n");
sunxi_gpio_set_cfgpin(SUNXI_GPA(29), SUNXI_GPIO_OUTPUT);
sunxi_gpio_output(SUNXI_GPA(29), 1);
printf("gpio ON\n");
sleep(4);
sunxi_gpio_output(SUNXI_GPA(29), 0);
printf("gpio OFF\n");
sleep(1);
}

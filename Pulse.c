#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#define FACTOR 30

/**
 * export_pin - export a GPIO pin
 * @pin_no: pin number to be exported
 */
void export_pin(const char *pin_no)
{
    FILE *sysfs_handle = NULL;
    sysfs_handle = fopen("/sys/class/gpio/export", "w");
    if (sysfs_handle != NULL)
    {
        fwrite(pin_no, sizeof(char), (sizeof(char) * (strlen(pin_no) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

void unexport_pin(const char *pin_no)
{
    FILE *sysfs_handle = NULL;
    sysfs_handle = fopen("/sys/class/gpio/unexport", "w");
    if (sysfs_handle != NULL)
    {
        fwrite(pin_no, sizeof(char), (sizeof(char) * (strlen(pin_no) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

/**
 * set_pin_direction - set a GPIO pin's direction (in or out)
 * @pin_no: pin number
 * @direction: pin number's direction
 */
void set_pin_direction(const char *pin_no, const char *direction)
{
    FILE *sysfs_handle = NULL;
    char direction_path[100];
    sprintf(direction_path, "/sys/class/gpio/gpio%s/direction", pin_no);
    sysfs_handle = fopen(direction_path, "w");
    if (sysfs_handle != NULL)
    {
        fwrite(direction, sizeof(char), (sizeof(char) * (strlen(direction) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

/**
 * set_pin_value - set the value of a  GPIO pin
 * @pin_no: pin number
 * @value: pin number's value
 */
void set_pin_value(const char *pin_no, const char *value)
{
    FILE *sysfs_handle = NULL;
    char value_path[100];
    sprintf(value_path, "/sys/class/gpio/gpio%s/value", pin_no);
    sysfs_handle = fopen(value_path, "w");
    if (sysfs_handle != NULL)
    {
        fwrite(value, sizeof(char), (sizeof(char) * (strlen(value) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

/**
 * set_pin_value - set the value of a  GPIO pin
 * @pin_no: pin number
 * Return: first character read from file
 */
char get_pin_value(const char *pin_no)
{
    FILE *sysfs_handle = NULL;
    char value_path[100];
    char val_button[2];
    sprintf(value_path, "/sys/class/gpio/gpio%s/value", pin_no);
    sysfs_handle = fopen(value_path, "r"); // read D_out
    if (sysfs_handle != NULL)
    {
        fgets(val_button, 2, sysfs_handle);
        fclose(sysfs_handle);
    }
    return val_button[0];
}

void init_ports()
{
    // Export and read from Dout(GPIO23)
    export_pin("23");
    set_pin_direction("23", "in");

    export_pin("24"); // CS pin
    set_pin_direction("24", "out");

    // Set CLK_PIN(GPIO24) and CS_PIN(GPIO18) to high
    export_pin("18"); // CLK pin
    set_pin_direction("18", "out");

    export_pin("25"); // Din pin
    set_pin_direction("25", "out");
}

void chose_channel()
{
    int channel = 0;
    int var = 0x04;
    // start
    set_pin_value("18", "1"); // set CLK high
    set_pin_value("18", "0");
    set_pin_value("25", "1"); // set Din high

    // set SGL/DIFF bit to high
    set_pin_value("18", "1"); // set CLK high
    set_pin_value("18", "0");
    set_pin_value("25", "0"); // set Din high
    set_pin_value("18", "1");

    // ask which channel to read
    // printf("Enter the channel to read from (0-7): ");
    // scanf("%d", &channel);
    channel = 0;
    // Set the SGL bit
    while (var > 0)
    {
        char *bit = (var & channel) ? "1" : "0";
        // printf("%s\n", bit);
        set_pin_value("18", "0");
        set_pin_value("25", bit); // set SGL bit
        set_pin_value("18", "1"); // set CLK high
        var = var >> 1;
    }
    set_pin_value("18", "1"); // set CLK high
    set_pin_value("18", "0");
}

double get_final_value()
{
    FILE *sysfs_handle = NULL; // For reading
    char val_button[2];
    double final_value;
    for (int i = 10; i >= 0; i--)
    {
        set_pin_value("18", "1");                                  // set CLK high
        set_pin_value("18", "0");                                  // set clk low
        sysfs_handle = fopen("/sys/class/gpio/gpio23/value", "r"); // read D_out
        if (sysfs_handle != NULL)
        {

            fgets(val_button, 2, sysfs_handle);
            fclose(sysfs_handle);
        }
        // discard null bit
        if (i < 10)
        {
            final_value += (atoi(val_button) * (pow(2, i)));
        }
    }

    return final_value;
}

int main()
{
	// clock_t start, end;
	struct timeval start, end;
	long seconds, useconds;
	double total_time;
	FILE *fp;
	int i = 0;

    init_ports();

    set_pin_value("18", "1"); // clk high
    set_pin_value("24", "1"); // cs high

    // set the CS_PIN to low
    set_pin_value("24", "0"); // cs low
    
    fp = fopen("output.csv", "w");
    fprintf(fp, "times,voltages\n");
    
    // start = clock();
    gettimeofday(&start, NULL);

    while (++i <= 512) {
        chose_channel();
        // read 13 bits from D_out
        double final_value = get_final_value();
        set_pin_value("24", "1"); // set CS high
        // printf("\n%f\n", final_value);
        double real_value = (final_value * 3.3) / 1024;
        // real_value *= FACTOR;
        
        // int spaces = (int)real_value;
        // for (int i = 0; i < spaces; i++) {
		// 	printf("*");
		// }
		// printf("*\n");
        
        set_pin_value("24", "0"); // cs low
        // end = clock();
        gettimeofday(&end, NULL);
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        // time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        total_time = seconds + useconds/1e6;
        fprintf(fp, "%f,%f\n", total_time, real_value);
        usleep(6912);
    }
    fclose(fp);

    return 0;
}


/// updates from 11/20/2024 







///////////////////////////////////////////////////



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#define FACTOR 30
#define RUNTIME 32
#define FREQ 128
#define RUNS (FREQ*RUNTIME)

/**
 * export_pin - export a GPIO pin
 * @pin_no: pin number to be exported
 */
void export_pin(const char *pin_no)
{
    FILE *sysfs_handle = NULL;
    sysfs_handle = fopen("/sys/class/gpio/export", "w");
    if (sysfs_handle != NULL)
    {
        fwrite(pin_no, sizeof(char), (sizeof(char) * (strlen(pin_no) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

void unexport_pin(const char *pin_no)
{
    FILE *sysfs_handle = NULL;
    sysfs_handle = fopen("/sys/class/gpio/unexport", "w");
    if (sysfs_handle != NULL)
    {
        fwrite(pin_no, sizeof(char), (sizeof(char) * (strlen(pin_no) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

/**
 * set_pin_direction - set a GPIO pin's direction (in or out)
 * @pin_no: pin number
 * @direction: pin number's direction
 */
void set_pin_direction(const char *pin_no, const char *direction)
{
    FILE *sysfs_handle = NULL;
    char direction_path[100];
    sprintf(direction_path, "/sys/class/gpio/gpio%s/direction", pin_no);
    sysfs_handle = fopen(direction_path, "w");
    if (sysfs_handle != NULL)
    {
        fwrite(direction, sizeof(char), (sizeof(char) * (strlen(direction) + 1)), sysfs_handle);
        fclose(sysfs_handle);
    }
}

void set_pin_value_h(FILE *handle, const char *value)
{
    if (handle != NULL)
    {
        fwrite(value, sizeof(char), (sizeof(char) * (strlen(value) + 1)), handle);
        fflush(handle);
    }
}

/**
 * set_pin_value - set the value of a  GPIO pin
 * @pin_no: pin number
 * Return: first character read from file
 */
char get_pin_value(const char *pin_no)
{
    FILE *sysfs_handle = NULL;
    char value_path[100];
    char val_button[2];
    sprintf(value_path, "/sys/class/gpio/gpio%s/value", pin_no);
    sysfs_handle = fopen(value_path, "r"); // read D_out
    if (sysfs_handle != NULL)
    {
        fgets(val_button, 2, sysfs_handle);
        fclose(sysfs_handle);
    }
    return val_button[0];
}

void init_ports()
{
    // Export and read from Dout(GPIO23)
    export_pin("23");
    set_pin_direction("23", "in");

    export_pin("24"); // CS pin
    set_pin_direction("24", "out");

    // Set CLK_PIN(GPIO24) and CS_PIN(GPIO18) to high
    export_pin("18"); // CLK pin
    set_pin_direction("18", "out");

    export_pin("25"); // Din pin
    set_pin_direction("25", "out");
}

void chose_channel(FILE *gpio18, FILE *gpio25)
{
    int channel = 0;
    int var = 0x04;
    // start
    set_pin_value_h(gpio18, "1"); // set CLK high
    set_pin_value_h(gpio18, "0");
    set_pin_value_h(gpio25, "1"); // set Din high

    // set SGL/DIFF bit to high
    set_pin_value_h(gpio18, "1"); // set CLK high
    set_pin_value_h(gpio18, "0");
    set_pin_value_h(gpio25, "1"); // set Din high
    set_pin_value_h(gpio18, "1");

    // ask which channel to read
    // printf("Enter the channel to read from (0-7): ");
    // scanf("%d", &channel);
    channel = 0;
    // Set the SGL bit
    while (var > 0)
    {
        char *bit = (var & channel) ? "1" : "0";
        // printf("%s\n", bit);
        set_pin_value_h(gpio18, "0");
        set_pin_value_h(gpio25, bit); // set SGL bit
        set_pin_value_h(gpio18, "1"); // set CLK high
        var = var >> 1;
    }
    set_pin_value_h(gpio18, "1"); // set CLK high
    set_pin_value_h(gpio18, "0");
}

double get_final_value(FILE *gpio18)
{
    FILE *sysfs_handle = NULL; // For reading
    char val_button[2];
    double final_value;
    for (int i = 10; i >= 0; i--)
    {
        set_pin_value_h(gpio18, "1");                                  // set CLK high
        set_pin_value_h(gpio18, "0");                                  // set clk low
        sysfs_handle = fopen("/sys/class/gpio/gpio23/value", "r"); // read D_out
        if (sysfs_handle != NULL)
        {

            fgets(val_button, 2, sysfs_handle);
            fclose(sysfs_handle);
        }
        // discard null bit
        if (i < 10)
        {
            final_value += ((val_button[0] - '0') * (1 << i));
        }
    }

    return final_value;
}


int main()
{
	// clock_t start, end;
	struct timeval start, end;
	long seconds, useconds;
	double total_time;
	double times[RUNS];
	double voltages[RUNS];
	FILE *fp, *gpio18, *gpio24, *gpio25;
	int i = 0;
	double sleep_time = ((1.0 / FREQ) - 0.00065) * 1000000;
	
	gpio18 = fopen("/sys/class/gpio/gpio18/value", "w");
    gpio24 = fopen("/sys/class/gpio/gpio24/value", "w");
    gpio25 = fopen("/sys/class/gpio/gpio25/value", "w");

    init_ports();

    set_pin_value_h(gpio18, "1"); // clk high
    set_pin_value_h(gpio24, "1"); // cs high

    // set the CS_PIN to low
    set_pin_value_h(gpio24, "0"); // cs low  
    
    gettimeofday(&start, NULL);

    while (++i <= RUNS) {
		chose_channel(gpio18, gpio25);
        // read 13 bits from D_out
        double final_value = get_final_value(gpio18);
        set_pin_value_h(gpio24, "1"); // set CS high

        double real_value = (final_value * (3.3)) / 1024;

        
        set_pin_value_h(gpio24, "0"); // cs low

        gettimeofday(&end, NULL);
        seconds = end.tv_sec - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        total_time = seconds + useconds/1e6;

        times[i-1]=total_time;
        voltages[i-1]= real_value;
        usleep(sleep_time);
    }
    fp = fopen("output5.csv", "w");
    fprintf(fp, "times,voltages\n");
    for(int j = 0; j < RUNS; j++){
		
		fprintf(fp, "%f,%f\n", times[j], voltages[j]);
	}
	
	fclose(gpio18);
    fclose(gpio24);
    fclose(gpio25);
	fclose(fp);
    return 0;
}

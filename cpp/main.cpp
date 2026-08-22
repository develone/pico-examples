#include <iostream>
#include "pico/stdlib.h"

// Define the onboard LED pin for the Pico 2 (RP2350)
class PicoBlinker {
private:
    const uint led_pin = 25; // Default onboard pin for standard Pico/Pico 2 layouts

public:
    // Constructor initializes the GPIO pin
    PicoBlinker() {
        gpio_init(led_pin);
        gpio_set_dir(led_pin, GPIO_OUT);
    }

    // Toggle method to handle flash logic
    void flash(uint32_t delay_ms) {
        gpio_put(led_pin, 1);
        std::cout << "LED is ON" << std::endl;
        sleep_ms(delay_ms);

        gpio_put(led_pin, 0);
        std::cout << "LED is OFF" << std::endl;
        sleep_ms(delay_ms);
    }
};

int main() {
    // Initialize standard I/O (enables USB/UART serial communications)
    stdio_init_all();

    // Instantiate our C++ object
    PicoBlinker blinker;

    // Loop forever
    while (true) {
        blinker.flash(500); 
    }
}

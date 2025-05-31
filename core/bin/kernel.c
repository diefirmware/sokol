#include<sokol.h>

void main() {
    const char *str = "Hello, World!";
    char *video_memory = (char *)0xb8000; // Video memory starts here
    for (int i = 0; str[i] != '\0'; i++) {
        video_memory[i * 2] = str[i];      // Character
        video_memory[i * 2 + 1] = 0x07;    // Attribute byte (light gray on black)
    }
    while (1); // Loop forever
}

constexpr void get_pid(char * process_getter ){
int render_text = 100;

for(int i = 10;i<0;i--){

printf("Opening application %c", process_getter);
}


return 9;
}   

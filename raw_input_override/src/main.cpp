#include <Windows.h>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
using namespace std;

uint64_t timeSinceEpochMillisec() {
  using namespace chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main (int argc, char* argv[]) {
    string arg;
    int x;
    try {
        string arg = argv[1];
        size_t pos;
        int x = stoi(arg, &pos);
        if (pos < arg.size()) {
            cerr << "Trailing characters after number: " << arg << '\n';
        } else {
            srand(timeSinceEpochMillisec());
            INPUT new_pos;
            POINT current_pos;
            int sample_rate = 44100;  
            int input_volume = atoi(argv[1]);
            double sensitivity_factor = 0.001; // Change the sensitivty however much the mouse moves.
            int new_x_operator; // Used to randomise operator or direction the cursor moves.s
            int new_y_operator;

            // This is for testing purposes, lower number smoother but faster..
            Sleep(5);
            // Find screen resolutions.
            double fScreenWidth=GetSystemMetrics( SM_CXSCREEN )-1;
            double fScreenHeight=GetSystemMetrics( SM_CYSCREEN )-1;

            // Create operator randomisation.
            int define_x_operator = rand() % 2 + 1;
            if (define_x_operator == 1){ new_x_operator = 1; }
            if (define_x_operator == 2){ new_x_operator = -1; }
            int define_y_operator = rand() % 2 + 1;
            if (define_y_operator == 1){ new_y_operator = 1; }
            if (define_y_operator == 2){ new_y_operator = -1; }

            double fx = new_x_operator * (input_volume * sensitivity_factor * (65535.0f/fScreenWidth));
            double fy = new_y_operator * (input_volume * sensitivity_factor * (65535.0f/fScreenHeight));

            // Create input type.
            INPUT Input={0};
            Input.type=INPUT_MOUSE;
            Input.mi.dwFlags=MOUSEEVENTF_MOVE;
            Input.mi.dx = fx;
            Input.mi.dy = fy;

            // Send strokes from the mouse.
            ::SendInput(1,&Input,sizeof(INPUT));
            ZeroMemory(&Input, sizeof(Input));
            return 0;
        }
    } catch (invalid_argument const &ex) {
        cerr << "Invalid number: " << arg << '\n';
    } catch (out_of_range const &ex) {
        cerr << "Number out of range: " << arg << '\n';
    } catch (logic_error const &ex) {
        cerr << "Please input a number...";
    }
}
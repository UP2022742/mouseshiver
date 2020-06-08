import pyaudio
import audioop
import random
import operator
import win32api as win32api 
import win32con
import yaml

py_audio = pyaudio.PyAudio()

class GetAudio():
    def __init__(self):
        self.mouse_sensitivty = cfg["mouse_adjustments"]["MOUSE_SENSITIVTY"]
        self.device_index = None
        self.stream = None
        self.device_index = self.find_input_device()
        self.HZ = cfg["audio_recorder"]["HZ"]
        self.SAMPLE_RATE = cfg["audio_recorder"]["SAMPLE_RATE"]
        self.INPUT_FRAMES_PER_BLOCK = int(self.HZ*self.SAMPLE_RATE)

    # Find the preferred input device.
    def find_input_device(self):
        device_index = None            
        for i in range( py_audio.get_device_count() ):     
            devinfo = py_audio.get_device_info_by_index(i)   
            print( "Device %d: %s"%(i,devinfo["name"]) )

            for keyword in ["mic","input"]:
                if keyword in devinfo["name"].lower():
                    print( "Sucessfully detected: device %d - %s"%(i,devinfo["name"]) )
                    device_index = i
                    return device_index

        if device_index == None:
            print( "No preferred input found; using default input device." )

        return device_index

    def get_stream_data(self):
        self.stream = py_audio.open(   
            format = pyaudio.paInt16 ,
            channels = 2,
            rate = 44100,
            input = True,
            input_device_index = self.device_index,
            frames_per_buffer = self.INPUT_FRAMES_PER_BLOCK
        )
    def get_volume(self):
        while True:
            self.get_stream_data()            
            volume = audioop.rms(self.stream.read(self.INPUT_FRAMES_PER_BLOCK), 2) 
            
            # Current cursor position
            position_x, position_y = win32api.GetCursorPos()
            new_x_pos = random.randint(0, round(volume / self.mouse_sensitivty))
            new_y_pos = random.randint(0, round(volume / self.mouse_sensitivty))

            # Random operator, either left or right.
            operators = [('+', operator.add), ('-', operator.sub)]
            op, fn = random.choice(operators)

            # Current position plus the randomised new position.
            
            # autopy.mouse.move(fn(position_x, new_x_pos), fn(position_y, new_y_pos))
            # pyautogui.moveTo((fn(position_x, new_x_pos), fn(position_y, new_y_pos)))
            win32api.SetCursorPos((fn(position_x, new_x_pos), fn(position_y, new_y_pos)))

    def close_stream(self):
        # Close gracefully to avoid issues with the microphone.
        self.stream.stop_stream()
        self.stream.close()
        py_audio.terminate()

if __name__ == "__main__":
    with open("config.yml", "r") as ymlfile:
        cfg = yaml.load(ymlfile, Loader=yaml.FullLoader)
    GetAudio().get_volume()
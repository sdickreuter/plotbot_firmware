import time
import sys
import glob
import serial
import struct
from cobs import cobs
import numpy as np
from scipy import signal


# from: https://stackoverflow.com/a/14224477
def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


def generate_sine_movement(t, freq = 0.0001, phase = 0.0,dtmin = 0.0001, dtmax = 0.001):
    y = np.sin(2*np.pi*t*freq+phase)*dtmax
    dt = np.zeros(len(y))
    dt[y < 0] = y[y < 0] + dtmin + dtmax
    dt[y >= 0] = y[y >= 0] - dtmin - dtmax
    return dt


def generate_triangle_movement(t, freq = 0.0001, phase = 0.0,dtmin = 0.0001, dtmax = 0.001):
    y = signal.sawtooth(2*np.pi*t*freq+phase,0.5)*dtmax
    dt = np.zeros(len(y))
    dt[y < 0] = y[y < 0] + dtmin + dtmax
    dt[y >= 0] = y[y >= 0] - dtmin - dtmax
    return dt


class PlotBot(object):

    def __init__(self, comport=None):
        if comport is None:
            serialports = serial_ports()
            self.serial = serial.Serial(serialports[0], timeout=0.5)

        else:
            self.serial = serial.Serial(comport, timeout=0.5)
    
        time.sleep(1) # allow some time for the Arduino to completely reset


    def read(self):
        msg = self.serial.read_until(terminator=b'\x00')
        return cobs.decode(msg[:-1])


    def write(self, msg):
        msg = bytearray(cobs.encode(bytearray(msg)))
        msg.append(0x00)
        self.serial.write(msg)

    #enable motors
    def enable(self):
        self.write(b'e')

    #clear buffers
    def clear(self):
        self.write(b'c')


    def write_buffer(self, timings, actions, axis):
        reply = b''
        reply += b'b'
        reply += axis
        reply += b's'
        reply += bytes(struct.pack('<l',len(timings)))
        for i in range(len(timings)):
            reply += bytes(struct.pack("<f", timings[i]))
            reply += bytes(struct.pack("B", actions[i]))
        self.write(reply)


    def read_bufferlength(self):
        msg = b'l'
        self.write(msg)

        msg = b''
        while len(msg) < 1:
            msg = self.read()
            time.sleep(0.01)

        if len(msg) > 1:
            if msg[0] == ord("l"): 
                xlen = struct.unpack('<l',msg[2:6])[0]
                ylen = struct.unpack('<l',msg[7:11])[0]
                return xlen, ylen
        return None, None


    def read_positions(self):
        msg = b'p'
        self.write(msg)

        msg = b''
        while len(msg) < 1:
            msg = self.read()
            time.sleep(0.01)

        if len(msg) > 1:
            if msg[0] == ord("p"): 
                xpos = struct.unpack('<l',msg[2:6])[0]
                ypos = struct.unpack('<l',msg[7:11])[0]
                return xpos, ypos
        return None, None

    def home(self):
        # home motors
        self.write(b'h')
        homed = False
        while not homed:
            msg = self.read()
            if len(msg) > 1:
                if msg == b'ok':
                    print(msg,"->", "homing finished")
                    homed = True



if __name__ == '__main__':
    bot = PlotBot()
    bot.home()
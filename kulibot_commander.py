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


def read(ser):
    msg = ser.read_until(terminator=b'\x00')
    #print(cobs.decode(msg[:-1]), ' | ',msg)
    return cobs.decode(msg[:-1])


def write(ser, msg):
    msg = bytearray(cobs.encode(bytearray(msg)))
    msg.append(0x00)
    ser.write(msg)


def write_buffer(timings, actions, axis):
    reply = b''
    reply += b'b'
    reply += axis
    reply += b's'
    reply += bytes(struct.pack('<l',len(timings)))
    #print(reply, struct.unpack('<l',reply[-4:]), len(timings))            
    for i in range(len(timings)):
        reply += bytes(struct.pack("<f", timings[i]))
        reply += bytes(struct.pack("B", actions[i]))
    write(ser, reply)
    
    # msg = b''
    # while len(msg) < 1:
    #     msg = read(ser)
        
    #     if len(msg) > 1:
    #         print(msg)


def read_bufferlength():
    msg = b'l'
    write(ser, msg)

    msg = b''
    while len(msg) < 1:
        msg = read(ser)
        time.sleep(0.01)

    if len(msg) > 1:
        if msg[0] == ord("l"): 
            xlen = struct.unpack('<l',msg[2:6])[0]
            ylen = struct.unpack('<l',msg[7:11])[0]
            return xlen, ylen
    return None, None


def read_positions():
    msg = b'p'
    write(ser, msg)

    msg = b''
    while len(msg) < 1:
        msg = read(ser)
        time.sleep(0.01)

    if len(msg) > 1:
        if msg[0] == ord("p"): 
            xpos = struct.unpack('<l',msg[2:6])[0]
            ypos = struct.unpack('<l',msg[7:11])[0]
            return xpos, ypos
    return None, None


def generate_sine_movement(t, freq = 0.0001, phase = 0.0,dtmin = 0.0001, dtmax = 0.001):
    y = np.sin(2*np.pi*t*freq+phase)*dtmax
    dt = np.zeros(len(y))
    dt[y < 0] = y[y < 0] + dtmin + dtmax
    dt[y >= 0] = y[y >= 0] - dtmin - dtmax
    return dt

    # y = (np.sin(2*np.pi*t*freq+phase)*0.5+1)*dtmax
    # y += dtmin
    # return y

def generate_triangle_movement(t, freq = 0.0001, phase = 0.0,dtmin = 0.0001, dtmax = 0.001):
    y = signal.sawtooth(2*np.pi*t*freq+phase,0.5)*dtmax
    dt = np.zeros(len(y))
    dt[y < 0] = y[y < 0] + dtmin + dtmax
    dt[y >= 0] = y[y >= 0] - dtmin - dtmax
    return dt
    # y = (np.sin(2*np.pi*t*freq+phase)*0.5+1)*dtmax
    # y += dtmin
    # return y
    # dt = np.zeros(len(t))
    # for i in range(len(t)):
    #     if t[i]%(1/freq) > (1/(2*freq)):
    #         dt[i] = 0.002
    #     else:
    #         dt[i] = -0.002
    # return dt

if __name__ == '__main__':
    
    # import matplotlib.pyplot as plt

    # timings = generate_sine_movement(np.arange(start=0,stop=10000),phase=np.pi)

    # plt.plot(timings)
    # plt.show()

    # raise RuntimeError


    serialports = serial_ports()
    print(serialports)
    ser = serial.Serial(serialports[0], timeout=0.5)

    time.sleep(2) # allow some time for the Arduino to completely reset

    write(ser, b'c')

    # enable motors
    write(ser, b'e')

    # home motors
    write(ser, b'h')
    homed = False
    while not homed:
        msg = read(ser)
        if len(msg) > 1:
            if msg == b'ok':
                print(msg,"->", "homing finished")
                homed = True

    timings = np.repeat(0.0002, 500)
    write_buffer(timings, np.repeat(1,len(timings)), b'x')
    write_buffer(timings, np.repeat(1,len(timings)), b'y')
    print("buffer length",read_bufferlength())

    time.sleep(0.2)
    #write(ser, b'c')

    write(ser, b'm')


    size = 500

    count = 0
    tx = 0
    ty = 0
    while count < 19:

        print(read_positions())
        xlen, ylen = read_bufferlength()
        #print(count,"buffer length",xlen,ylen)

        if xlen is not None:
            if xlen <= ylen:
                if xlen < (3000-size):
                    #timings = generate_sine_movement(np.arange(start=tx,stop=tx+size))
                    #timings = generate_sine_movement(np.arange(start=tx,stop=tx+size))
                    tx += size
                    write_buffer(timings, np.repeat(1,len(timings)), b'x')
                    
            else:
                if ylen < (3000-size):
                    #timings = generate_sine_movement(np.arange(start=ty,stop=ty+size),phase=np.pi)
                    #timings = generate_sine_movement(np.arange(start=ty,stop=ty+size),phase=np.pi)
                    ty += size
                    write_buffer(timings, np.repeat(1,len(timings)), b'y')
                    count += 1

        time.sleep(0.01)
        #print(count, ' ', link.status, " | rxBuff ",link.rxBuff[0:5])
        #if count % 10 == 0:
        #    print(count)

    for i in range(100):
        print(read_positions())
        time.sleep(0.1)

    write(ser, b'd')

    ser.close()
    
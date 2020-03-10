import time
import sys
import glob
import serial
import struct
from cobs import cobs
import numpy as np

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


def write_timings(timings, axis):
    reply = b''
    reply += b'b'
    reply += axis
    reply += b's'
    reply += bytes(struct.pack('<h',len(timings)))                   
    for i in range(len(timings)):
        reply += bytes(struct.pack("<f", timings[i]))
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
    
    if len(msg) > 1:
        if msg[0] == ord("l"): 
            xlen = struct.unpack('<l',msg[2:6])[0]
            ylen = struct.unpack('<l',msg[7:11])[0]
            return xlen, ylen
    return None, None


if __name__ == '__main__':

    serialports = serial_ports()
    print(serialports)
    ser = serial.Serial(serialports[0], timeout=0.3)

    time.sleep(2) # allow some time for the Arduino to completely reset

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

    timings = np.repeat(0.0002, 2500)
    write_timings(timings,b'x')
    write_timings(timings,b'y')
    print("buffer length",read_bufferlength())

    time.sleep(1)

    write(ser, b'm')

    for i in range(20):
        #print("buffer length",read_bufferlength())
        print(i)
        time.sleep(0.5)

    write(ser, b'd')


    raise RuntimeError

    size = 666

    count = 0
    while count < 200:

        xlen, ylen = read_bufferlength()
        
        if xlen is not None:        
            if xlen <= ylen:
                if xlen < 3000-size:
                    timings = np.repeat(100000, 500)
                    write_timings(timings,b'x')
            else:
                if ylen < 3000-size:
                    timings = np.repeat(100000, 500)
                    write_timings(timings,b'y')



        time.sleep(0.1)
        count += 1
        #print(count, ' ', link.status, " | rxBuff ",link.rxBuff[0:5])
        #if count % 10 == 0:
        #    print(count)

    time.sleep(1)

    write(ser, b'd')

    ser.close()
    
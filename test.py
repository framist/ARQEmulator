import serial
import time


# 显示可用串口
def show_serial() -> str:
    sl = []
    for i in range(256):
        try:
            ser = serial.Serial('COM'+str(i))
            sl.append('COM'+str(i))
            ser.close()
        except serial.SerialException:
            pass
    return sl

# 读取串口数据
def read_serial(ser: serial.Serial) -> str:
    data = b''
    i = 0
    print('RAW: ')
    data += ser.readline()
    if data[-2:] == b'\r\n':
        print(data[:-2])
        return str(data[:-2], encoding='utf-8')




def SendFrame(ser:serial.Serial, frame:bytes):
    ser.write(frame)
    

# 成帧
def MakeFrame(data:str,S_n:int) -> bytes:
    data += str(len(data))
    data += '\r\n'
    data = str(S_n) + data
    return bytes(data, encoding='utf-8')


def StartTimer(S_n:int):
    pass

def corrupted(frame):
    # 先简单写一下
    if frame is None:
        return True
    return  len(frame) != 6 or frame[:3] not in ['ACK','NAK'] or frame[5] not in ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f']

def TimeOut():
    pass

# 选择性重传ARQ发送方算法
def ARQ_send(ser:serial.Serial):
    S_w = 2**4//2
    S_f = 0
    S_n = 0
    frames = ['']*S_w
    framesTime = [0]*S_w

    while True: # Repeat forever
        frame = input('请输入发送(q退出)：')
        if frame == 'q':  break
        # 有包要发送
        if(S_n + 1 == S_f): # 窗口满
            print('窗口满，等待窗口满后再发送')
            continue
        # 发送
        frames[S_n] = frame
        SendFrame(ser,MakeFrame(frame,S_n))
        S_n += 1
        S_n %= S_w
        
        framesTime[S_n%S_w] = time.time()

        # 接收
        
        read_frame = read_serial(ser)
        if corrupted(read_frame):
            print('接收到的数据有误')
            continue
        ackNo = int(read_frame[5],16)
        if read_frame[:3] == 'NAK' and S_f < ackNo < S_n:
            print('接收到NAK，重传')
            SendFrame(ser,MakeFrame(frames[ackNo],ackNo))
            framesTime[ackNo] = time.time()
            continue

        if read_frame[:3] == 'ACK': 
            print('接收到ACK')
            if S_f <= ackNo <= S_n:
                while(ackNo != S_f):
                    frames[S_f%S_w] = '' # purge
                    S_f += 1
                    S_f %= S_w

        # # 超时  暂时不做 因为没有做多线程
        # for i in range(S_w):
        #     if frames[i] != '' and time.time() - framesTime[i] > 2:
        #         print('超时重传')
        #         SendFrame(ser,MakeFrame(frames[i],i))
        #         framesTime[i] = time.time()

        # 打印状态
        print('S_n:',S_n,'S_f:',S_f,'S_w:',S_w)
        for i in range(S_w):
            print(i, '[' + frames[i] + ']',end=' ')
        print()


def main():
    sl = show_serial()
    port = 'COM' + input('找到串口：'+str(sl)+' 请输入串口号：COM') if len(sl) != 1 else sl[0]
    baudrate = 9600
    ser = serial.Serial(port, baudrate, timeout=0.5)


    # SendFrame(ser,MakeFrame('hello',0))
    # print('read',read_serial(ser))
    ARQ_send(ser)
    ser.close()

if __name__ == '__main__':
    main()

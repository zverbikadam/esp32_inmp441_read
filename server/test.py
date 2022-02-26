import os
import socket

PORT = int(os.environ.get('PORT', '8082'))
path = '.\\uploads\\'
idx = 0
records_cout = 600

if __name__ == '__main__':
    if not os.path.exists(path):
        os.makedirs(path)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    server.bind(('0.0.0.0', PORT))
    server.listen(5)
    size = 64 
    print('Server listening on port: {}'.format(PORT))
    timed_out = []
    try:
        while True:
            client, addr = server.accept()
            print('Accepted connection from: {}'.format(addr))
            client.settimeout(2) 
            f = open('{}audio_{}.wav'.format(path, idx), 'wb')
            idx += 1
            try:
                cycles = 0
                while True:
                    data = client.recv(size)
                    cycles += 1
                    if data == b'':
                        break
                    f.write(bytearray(data))

            except Exception as e:
                print(e)
            f.close() 
            client.close() 
            if idx >= records_cout:
                print(f'Finished, recorded {records_cout} records ({(records_cout/60):.1f} minutes)')
                exit()
            
    except KeyboardInterrupt:
        print('Exit, Interrupted...')
        print(f'Recorded {idx} records ({(idx/60):.1f} minutes)')

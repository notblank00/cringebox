# coding=utf-8
import socketserver
import time
import os


def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)
        f.close()


class RequestHandler(socketserver.BaseRequestHandler):
    def handle(self):
        print(time.strftime("[%d.%m.%Y %H:%M:%S]", time.localtime()) + ': Получен запрос от устройства выдачи (' + self.client_address[0] + '):')
        data = self.request.recv(1024)
        if str(data)[2] == '+':
            print('    Аренда...')
            line_prepender('database.txt', time.strftime("-%d.%m.%Y %H:%M:%S ", time.localtime()) + str(data)[3:-3] + '\n')
            self.request.send(bytes('OK\n'.encode('ascii')))
            print('Успешно.')
        else:
            print('    Возврат...')
            file = open('database.txt', 'r')
            lines = file.readlines()
            resp = False
            for cur in reversed(lines):
                if cur.split(' ')[2][:-1] == str(data)[3:-3]:
                    resp = not resp
            file.close()
            if resp:
                resp = '1'
                line_prepender('database.txt', time.strftime("+%d.%m.%Y %H:%M:%S ", time.localtime()) + str(data)[3:-3] + '\n')
                print('Доступ для возврата оборудования разрешен.')
            else:
                resp = '0'
                print('В доступе отказано: оборудование арендовано другим клиентом.')
            self.request.send(bytes((resp + '\n').encode('ascii')))
        return


class DatabaseServer(socketserver.TCPServer):
    def __init__(self, server_address, handler_class=RequestHandler):
        socketserver.TCPServer.__init__(self, server_address, handler_class)
        return


print("SHARING BOX by 444 Tech Team.\nTCP-cервер базы данных для сетевого взаимодействия с устройством выдачи.\nВведите внутрисетевой IPv4-адрес этого устройства.\nОбычно он начинается на 192.168...: ")
local_ip = input()
address = (local_ip, 41)
print("Запуск сервера...")
server = DatabaseServer(address, RequestHandler)
print('Сервер запущен на порту 41')
server.serve_forever()

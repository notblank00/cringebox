import http.server
import socketserver

PORT = 8080
Handler = http.server.SimpleHTTPRequestHandler

print("SHARING BOX by 444 Tech Team.\nHTTP-cервер базы данных для сетевого взаимодействия с пользователями.\nЗапуск сервера...")
server = socketserver.TCPServer(("", PORT), Handler)
print("Cервер запущен на порту 8080")
server.serve_forever()
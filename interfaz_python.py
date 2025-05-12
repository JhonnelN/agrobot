import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import threading
import time
import serial  # Importar pyserial para comunicación con HC-05

class SensorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Agrobot V1.0")  # Cambiar el título de la ventana
        self.root.geometry("800x600")
        
        # Variables para los datos
        self.humedad_ambiente = tk.DoubleVar(value=0.0)
        self.humedad_suelo = tk.DoubleVar(value=0.0)
        self.luminancia = tk.IntVar(value=0)
        self.lluvia = tk.StringVar(value="No")
        self.temperatura = tk.IntVar(value=0)
        
        # Configurar el grid principal
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_columnconfigure(1, weight=1)
        self.root.grid_rowconfigure(0, weight=1)
        self.root.grid_rowconfigure(1, weight=1)
        
        # Panel superior (título)
        title_frame = ttk.Frame(self.root, padding=10)
        title_frame.grid(row=0, column=0, columnspan=2, sticky="nsew")
        ttk.Label(title_frame, text="Agrobot V1.0", font=('Arial', 20, 'bold')).pack(expand=True)

        # Panel izquierdo (sensores)
        left_frame = ttk.Frame(self.root, padding=20)
        left_frame.grid(row=1, column=0, sticky="nsew")
        
        # Panel derecho (cámara)
        right_frame = ttk.Frame(self.root)
        right_frame.grid(row=1, column=1, sticky="nsew")
        
        # Configurar elementos del panel izquierdo
        ttk.Label(left_frame, text="Humedad Ambiente (%):", font=('Arial', 12)).grid(row=0, column=0, sticky="w", pady=5)
        ttk.Label(left_frame, textvariable=self.humedad_ambiente, font=('Arial', 12)).grid(row=0, column=1, sticky="e", pady=5)
        
        ttk.Label(left_frame, text="Humedad Suelo (%):", font=('Arial', 12)).grid(row=1, column=0, sticky="w", pady=5)
        ttk.Label(left_frame, textvariable=self.humedad_suelo, font=('Arial', 12)).grid(row=1, column=1, sticky="e", pady=5)
        
        ttk.Label(left_frame, text="Luminancia (lux):", font=('Arial', 12)).grid(row=2, column=0, sticky="w", pady=5)
        ttk.Label(left_frame, textvariable=self.luminancia, font=('Arial', 12)).grid(row=2, column=1, sticky="e", pady=5)
        
        ttk.Label(left_frame, text="Lluvia:", font=('Arial', 12)).grid(row=3, column=0, sticky="w", pady=5)
        ttk.Label(left_frame, textvariable=self.lluvia, font=('Arial', 12)).grid(row=3, column=1, sticky="e", pady=5)
        
        ttk.Label(left_frame, text="Temperatura (°C):", font=('Arial', 12)).grid(row=4, column=0, sticky="w", pady=5)
        ttk.Label(left_frame, textvariable=self.temperatura, font=('Arial', 12)).grid(row=4, column=1, sticky="e", pady=5)
        
        # Configurar imagen de la cámara
        self.camera_label = ttk.Label(right_frame)
        self.camera_label.pack(expand=True, fill="both")
        
        # Cargar imagen inicial
        self.load_image()
    
    def load_image(self, image_path="cam_image.jpg"):
        try:
            img = Image.open(image_path)
            img = img.resize((400, 500), Image.Resampling.LANCZOS)
            self.camera_image = ImageTk.PhotoImage(img)
            self.camera_label.config(image=self.camera_image)
        except FileNotFoundError:
            # Crear imagen negra si no se encuentra el archivo
            self.camera_image = ImageTk.PhotoImage(Image.new('RGB', (400, 500), (0, 0, 0)))
            self.camera_label.config(image=self.camera_image)
    
    def actualizar_datos(self, hum_amb, hum_suelo, lux, lluvia, temp):
        self.humedad_ambiente.set(hum_amb)
        self.humedad_suelo.set(hum_suelo)
        self.luminancia.set(lux)
        self.lluvia.set("Sí" if lluvia and int(hum_amb)>95 else "No")
        self.temperatura.set(temp)

    def parse_sensor_data(self, line):
        """
        Parsea una línea de datos recibida desde el dispositivo Bluetooth.
        """
        try:
            parts = line.strip().split(',')
            print(f"Partes procesadas: {parts}")
            return {
                'lluvia': bool(int(parts[0])),
                'humedad_suelo': float(parts[1]),
                'humedad_ambiente': float(parts[2]),
                'temperatura': float(parts[3]),
                'luminancia': int(parts[4])
            }
        except (ValueError, IndexError):
            print("Error al procesar los datos recibidos.")
            return None

    def obtener_datos_bluetooth(self, puerto="COM6", baudrate=9600):
        """
        Conecta al dispositivo Bluetooth HC-05 y obtiene los datos de los sensores.
        """
        try:
            with serial.Serial(puerto, baudrate, timeout=1) as bt:
                linea = bt.readline().decode('utf-8').strip()
                print(f"Datos recibidos: {linea}")
                data = self.parse_sensor_data(linea)
                if data:
                    self.actualizar_datos(
                        data['humedad_ambiente'],
                        data['humedad_suelo'],
                        data['luminancia'],
                        data['lluvia'],
                        data['temperatura']
                    )
        except serial.SerialException as e:
            print(f"Error de conexión Bluetooth: {e}")

    def iniciar_actualizacion_datos(self, puerto="COM6", baudrate=9600, intervalo=5):
        """
        Inicia un hilo para actualizar los datos periódicamente desde el dispositivo Bluetooth.
        """
        def actualizar():
            while True:
                self.obtener_datos_bluetooth(puerto, baudrate)
                time.sleep(intervalo)

        hilo = threading.Thread(target=actualizar, daemon=True)
        hilo.start()

if __name__ == "__main__":
    print("Iniciando la aplicación...")
    root = tk.Tk()
    app = SensorApp(root)
    # Iniciar la actualización de datos desde el HC-05
    app.iniciar_actualizacion_datos(puerto="COM6", baudrate=9600, intervalo=5)
    root.mainloop()
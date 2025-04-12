
# Soldador Rotativo Controlado por Arduino

Versión: **v0.96**  
Autor: **Dimmers Jujuy**

---

## 📖 Descripción

Este proyecto implementa un sistema de control para un **motor paso a paso** que actúa como parte de un **mecanismo de soldadura rotativo**. El usuario puede controlar el comportamiento del motor a través de un **display LCD I2C** de 20x4 y un **teclado analógico**, eligiendo entre distintos modos de operación: ejecución continua, por pasos, por ángulo, modo jog y cambio de relación de engranajes.

---

## 🧠 Funcionalidades principales

- Visualización de datos en pantalla LCD I2C.
- Control de dirección y velocidad del motor paso a paso mediante teclas.
- Lectura de potenciómetro para ajustar velocidad.
- Selección de modos de operación desde un menú.
- Cambio de relaciones de engranaje simuladas por software.

---

## 🧰 Componentes utilizados

| Componente              | Detalles                            |
|------------------------|-------------------------------------|
| Arduino                | Uno o compatible                    |
| Motor Paso a Paso      | 1.8° por paso (200 pasos/rev)       |
| Driver de motor        | Compatible con microstepping        |
| Display LCD I2C        | 20x4, dirección 0x3F                |
| Potenciómetro          | Conectado a A1                      |
| Teclado analógico      | Conectado a A0                      |
| Sensor de temperatura  | Opcional (A1, A2)                   |

---

## 🧩 Pines utilizados

| Función              | Pin Arduino |
|---------------------|-------------|
| Paso del motor      | D2          |
| Dirección del motor | D3          |
| Activación motor    | D11         |
| Teclado analógico   | A0          |
| Potenciómetro       | A1          |
| Temp disipador      | A1 (opcional) |
| Temp motor          | A2 (opcional) |

---

## 🔄 Modos de operación

| Modo        | ID   | Descripción                                      |
|-------------|------|--------------------------------------------------|
| Menú        | 0    | Pantalla principal con navegación por modos     |
| Run Mode    | 1    | Movimiento continuo, velocidad regulada         |
| Step Mode   | 2    | Movimiento controlado por pasos                 |
| Angle Mode  | 3    | Movimiento por ángulo, usando relación engranaje |
| Jog Mode    | 4    | (No implementado en el código actual)           |
| Ratio Mode  | 5    | Permite cambiar la relación de engranajes       |

---

## ⚙️ Parámetros configurables

```cpp
#define StepsPerRevolution 200   // Pasos por vuelta completa del motor
#define Microsteps 8             // Microstepping configurado en el driver
#define AngleIncrement 5         // Incremento por pulsación en modo ángulo
#define SpeedDelayIncrement 50   // Incremento de velocidad
#define JogStepsIncrement 1      // Pasos por pulsación en modo jog
#define GearRatioMax 3           // Número de relaciones de engranaje disponibles
```

---

## ⚖️ Relaciones de engranaje

Estas simulan distintas configuraciones mecánicas:

```cpp
gear_ratio_top_array[] = {1, 2, 3};
gear_ratio_bottom_array[] = {1, 1, 1};
```

Lo que da:

- Relación 1:1
- Relación 2:1
- Relación 3:1

---

## 🎮 Controles de teclado

| Tecla          | Acción                           |
|----------------|----------------------------------|
| SELECT         | Entra al modo seleccionado / Salir del modo |
| UP / DOWN      | Navega entre opciones o cambia valores |
| LEFT / RIGHT   | Cambia dirección del motor       |

---

## 🔁 Funciones principales

- `setup()`  
  Inicializa LCD, pines, muestra pantalla de bienvenida.

- `loop()`  
  Escucha teclas y navega por los distintos modos.

- `dorunmode()`  
  Movimiento continuo con velocidad variable.

- `dostepmode()`  
  Movimiento por pasos con control manual.

- `doanglemode()`  
  Movimiento por ángulo, tomando en cuenta la relación de engranajes.

- `doratiomode()`  
  Permite cambiar la relación de engranajes desde el teclado.

---

## 💡 Personalización

Puedes modificar fácilmente:

- Las relaciones de engranajes para simular distintas transmisiones.
- El mapeo del potenciómetro para cambiar el rango de velocidad.
- La lógica de control de temperatura si decides usar sensores en A1 y A2.

---

## 🧪 Pendiente / Futuras mejoras

- Implementar `dojogmode()`.
- Agregar límite físico mediante sensores.
- Integrar sensores de temperatura para seguridad.
- Añadir EEPROM para guardar configuraciones.


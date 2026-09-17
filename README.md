# Analizador Final

**LENGUAJES Y AUTÓMATAS II**
**GRUPO:** 6N1
**DOCENTE:** ECHEVERRIA RODRIGUEZ CESAR OSVALDO
**FECHA:** 08/09/2026

# Nombres de Equipo

* Carlos Daniel Guzmán Morales
* Jaqueline Hernandez Patricio
* MONSERRAT AURORA ABIGAIL VACA QUEROL

# Número de Equipo

**5**

# Analizador Léxico, Sintáctico y Semántico para 8086

## Descripción

Este proyecto consiste en el desarrollo de un analizador léxico, sintáctico y semántico para un conjunto de instrucciones y expresiones utilizadas como entrada para el análisis.

El proyecto cuenta con una implementación principal desarrollada en **C**, además de una interfaz web desarrollada con **HTML, CSS y JavaScript**, que permite introducir código y visualizar los resultados de los diferentes análisis.

La página web busca reflejar el funcionamiento realizado por el programa en C, permitiendo analizar el código de entrada desde una interfaz gráfica y mostrando los resultados correspondientes a cada etapa.

Además de las instrucciones relacionadas con el conjunto reducido del 8086, el programa permite trabajar con **operaciones aritméticas normales**, las cuales son procesadas para construir y mostrar árboles de expresiones.

El proyecto fue desarrollado con el propósito de aplicar los conceptos vistos en la materia de **Lenguajes y Autómatas II**, principalmente relacionados con análisis léxico, sintáctico, semántico, expresiones y árboles de expresión.

---

## Objetivo

El objetivo del proyecto es implementar las principales etapas del análisis de un lenguaje y mostrar sus resultados de manera gráfica.

El programa permite:

* Identificar los elementos que forman el código de entrada.
* Reconocer instrucciones, registros y operadores.
* Comprobar que las instrucciones y expresiones tengan una estructura correcta.
* Detectar errores léxicos.
* Detectar errores sintácticos.
* Realizar comprobaciones semánticas.
* Analizar operaciones aritméticas.
* Construir árboles de expresiones.
* Obtener los recorridos **Preorden, Inorden y Postorden**.
* Mostrar los resultados de cada análisis en la página web.
* Generar archivos HTML con los resultados de los análisis.

---

# Tecnologías utilizadas

* **C**
* **HTML5**
* **CSS3**
* **JavaScript**
* **Node.js**

---

# Instrucciones reconocidas

El analizador trabaja con un conjunto reducido de instrucciones relacionadas con el lenguaje ensamblador 8086:

* `ADD`
* `SUB`
* `INC`
* `DEC`

También se mantienen las instrucciones y opciones que forman parte de la versión anterior del proyecto, utilizadas para realizar diferentes pruebas del analizador.

---

# Registros reconocidos

Se utilizan los registros de 8 bits:

* `AL`
* `AH`
* `BL`
* `BH`
* `CL`
* `CH`
* `DL`
* `DH`

---

# Operaciones aritméticas

Además de las instrucciones del conjunto reducido del 8086, el analizador permite trabajar con operaciones aritméticas normales.

Por ejemplo:

```text
A + B * C
```

```text
A * B - C
```

```text
(A + B) * C
```

```text
A + (B * C - D)
```

Estas expresiones son utilizadas para comprobar su estructura y generar el árbol de expresiones correspondiente.

El análisis respeta la prioridad de los operadores aritméticos:

1. Paréntesis
2. Multiplicación y división
3. Suma y resta

Por ejemplo:

```text
A + B * C
```

se interpreta como:

```text
A + (B * C)
```

---

# Análisis léxico

El análisis léxico es la primera etapa del programa.

Se encarga de leer el código de entrada e identificar los elementos que forman las instrucciones y expresiones.

Entre los elementos que reconoce se encuentran:

* Instrucciones.
* Registros de 8 bits.
* Números enteros.
* Operadores aritméticos.
* Paréntesis.
* Comas.
* Espacios.
* Saltos de línea.
* Elementos o caracteres no reconocidos.

Por ejemplo, para:

```asm
ADD AL, BH
```

el analizador identifica:

* `ADD` como instrucción.
* `AL` como registro.
* `,` como coma.
* `BH` como registro.

Para una operación:

```text
A + B * 3
```

se identifican elementos como:

* `A` como operando.
* `+` como operador.
* `B` como operando.
* `*` como operador.
* `3` como número entero.

La información obtenida durante esta etapa se utiliza posteriormente en los análisis sintáctico y semántico.

---

# Árbol de expresiones en el análisis léxico

Como parte de la actualización del proyecto, el analizador léxico también muestra el **árbol de expresiones** correspondiente a las operaciones encontradas en el código de entrada.

Para una expresión como:

```text
A + B * C
```

se puede obtener un árbol similar a:

```text
       +
      / \
     A   *
        / \
       B   C
```

Además, se muestran los tres recorridos:

### Preorden

```text
+ A * B C
```

### Inorden

```text
A + B * C
```

### Postorden

```text
A B C * +
```

El árbol se genera automáticamente a partir de la expresión introducida por el usuario.

---

# Análisis sintáctico

El análisis sintáctico comprueba que las instrucciones y expresiones tengan una estructura válida.

Para las instrucciones `ADD` y `SUB`, la estructura utilizada es:

```text
INSTRUCCION REGISTRO, REGISTRO
```

Ejemplos:

```asm
ADD AL, BH
SUB AH, BL
```

Para `INC` y `DEC`, la estructura es:

```text
INSTRUCCION REGISTRO
```

Ejemplos:

```asm
INC AL
DEC BH
```

También se comprueba la estructura de las expresiones aritméticas.

Por ejemplo:

```text
A + B * C
```

es una expresión válida, mientras que una entrada como:

```text
A + * B
```

presenta un problema en su estructura.

Si una instrucción o expresión no cumple con la estructura correspondiente, el programa reporta un error sintáctico.

---

# Árbol de expresiones en el análisis sintáctico

El análisis sintáctico construye un árbol de expresión a partir de las operaciones aritméticas.

El árbol representa la estructura de la expresión y permite observar la prioridad de los operadores.

Por ejemplo:

```text
(A + B) * C
```

produce una estructura similar a:

```text
        *
       / \
      +   C
     / \
    A   B
```

Sus recorridos son:

### Preorden

```text
* + A B C
```

### Inorden

```text
A + B * C
```

### Postorden

```text
A B + C *
```

El árbol se muestra directamente en la página web dentro de la sección correspondiente al análisis sintáctico.

---

# Análisis semántico

El análisis semántico se encarga de revisar diferentes condiciones relacionadas con el uso de los registros y las operaciones.

Entre las comprobaciones realizadas se encuentran:

* Compatibilidad entre los registros utilizados.
* Uso del mismo registro como origen y destino.
* Uso de registros que todavía no han sido utilizados.
* Seguimiento de las operaciones de lectura y escritura de los registros.
* Detección de registros modificados que posteriormente no fueron utilizados.
* Validación de determinadas condiciones relacionadas con las operaciones.

Por ejemplo:

```asm
ADD AL, AL
```

puede generar una advertencia debido a que el mismo registro se utiliza como destino y origen de la operación.

También se mantiene información sobre el estado de los registros durante el análisis.

---

# Árbol de expresiones en el análisis semántico

El análisis semántico también incluye la generación del árbol de expresiones.

Esto permite mantener la representación de la operación durante las diferentes etapas del analizador.

Por ejemplo, para:

```text
A - (B + C) * D
```

el árbol puede representarse como:

```text
        -
       / \
      A   *
         / \
        +   D
       / \
      B   C
```

Los recorridos obtenidos son:

### Preorden

```text
- A + B C * D
```

### Inorden

```text
A - (B + C) * D
```

### Postorden

```text
A B C + D * -
```

De esta manera, el árbol de expresiones está disponible en los tres analizadores:

* **Análisis léxico**
* **Análisis sintáctico**
* **Análisis semántico**

---

# Integración entre C y la página web

Una de las modificaciones principales del proyecto consiste en que la implementación realizada en **C** y la interfaz web mantengan el mismo comportamiento esperado para el análisis.

El archivo:

```text
lex8086.c
```

contiene la implementación principal del analizador.

La página web permite introducir el código de entrada y visualizar los resultados de los diferentes análisis.

La intención es que las opciones y validaciones realizadas por el programa en C también estén representadas en la interfaz web.

De esta forma, el proyecto cuenta con:

```text
Código en C
     │
     ▼
Análisis
     │
     ├── Léxico
     ├── Sintáctico
     ├── Semántico
     └── Árbol de expresiones
              │
              ├── Preorden
              ├── Inorden
              └── Postorden
```

---

# Resultados

El programa genera diferentes archivos HTML dependiendo del análisis realizado:

* `resultado_lexico.html`
* `resultado_sintactico.html`
* `resultado_semantico.html`

La página web también presenta los resultados directamente mediante sus diferentes secciones.

---

## Resultado léxico

El resultado léxico muestra:

* Tokens encontrados.
* Instrucciones reconocidas.
* Registros encontrados.
* Operadores.
* Números.
* Comas.
* Espacios.
* Elementos desconocidos.
* Errores léxicos.
* Árbol de expresiones.
* Recorrido Preorden.
* Recorrido Inorden.
* Recorrido Postorden.

También se presentan estadísticas relacionadas con los elementos encontrados durante el análisis.

---

## Resultado sintáctico

El resultado sintáctico muestra:

* Instrucciones procesadas.
* Validación de la estructura.
* Errores sintácticos.
* Reglas de producción.
* Árbol sintáctico.
* Árbol de expresiones.
* Recorrido Preorden.
* Recorrido Inorden.
* Recorrido Postorden.

Los resultados pueden indicar que una instrucción o expresión es correcta o mostrar el error encontrado.

---

## Resultado semántico

El resultado semántico muestra:

* Tabla semántica de símbolos.
* Registros utilizados.
* Comprobaciones realizadas.
* Advertencias.
* Errores semánticos.
* Árbol de expresiones.
* Recorrido Preorden.
* Recorrido Inorden.
* Recorrido Postorden.

---

# Estructura del proyecto

```text
ANALIZADORFINAL/
│
├── .vscode/
│   └── settings.json
│
└── project/
    ├── lex8086.c
    ├── lex8086
    ├── lex8086.exe
    ├── index.js
    ├── package.json
    ├── package-lock.json
    ├── resultado_lexico.html
    ├── resultado_sintactico.html
    └── resultado_semantico.html
```

---

# Archivos principales

### `lex8086.c`

Es el archivo principal del proyecto.

Contiene la implementación del analizador desarrollado en C y las funciones utilizadas para realizar los diferentes análisis.

### `lex8086.exe`

Es el ejecutable del programa para Windows.

### `lex8086`

Es la versión ejecutable generada para sistemas compatibles.

### `index.js`

Archivo relacionado con la parte de Node.js utilizada en el proyecto.

### `package.json`

Contiene la configuración y las dependencias utilizadas por la parte de Node.js.

### `package-lock.json`

Contiene información de las versiones específicas de las dependencias instaladas mediante Node.js.

### `resultado_lexico.html`

Contiene los resultados correspondientes al análisis léxico.

### `resultado_sintactico.html`

Contiene los resultados correspondientes al análisis sintáctico.

### `resultado_semantico.html`

Contiene los resultados correspondientes al análisis semántico.

---

# Ejecución

## Programa en C

Si se utiliza Windows, el programa puede ejecutarse mediante:

```bash
lex8086.exe
```

También es posible ejecutarlo desde la terminal:

```bash
.\lex8086.exe
```

---

# Compilación

Si se desea volver a compilar el código fuente utilizando GCC:

```bash
gcc lex8086.c -o lex8086
```

En Windows:

```bash
gcc lex8086.c -o lex8086.exe
```

Después de compilarlo, se puede ejecutar con:

```bash
.\lex8086.exe
```

---

# Ejecución de la página web

Para ejecutar la parte web del proyecto se utiliza Node.js.

Primero se deben instalar las dependencias:

```bash
npm install
```

Después se puede iniciar el proyecto utilizando el comando configurado en `package.json`.

Por ejemplo:

```bash
npm start
```

o el comando correspondiente definido en el proyecto.

Una vez iniciado el servidor, se abre la página web desde el navegador.

---

# Funcionamiento de la página

Al iniciar la página se muestra el área donde el usuario puede introducir el código que desea analizar.

El proyecto incluye un ejemplo de código de entrada para poder realizar una prueba inmediatamente.

El usuario puede modificar este código y escribir sus propias instrucciones u operaciones.

Después de presionar:

```text
Analizar código
```

el programa procesa la entrada y muestra los resultados correspondientes.

La información se divide en las diferentes etapas:

```text
Código de entrada
       │
       ▼
┌───────────────┐
│ Análisis      │
│ del código    │
└───────┬───────┘
        │
   ┌────┼────┐
   ▼    ▼    ▼
 Léxico Sintáctico Semántico
   │      │       │
   └──────┼───────┘
          ▼
   Árbol de expresiones
          │
    ┌─────┼─────┐
    ▼     ▼     ▼
 Preorden Inorden Postorden
```

---

# Ejemplo de código de entrada

El programa puede utilizar como ejemplo:

```asm
INT 10
LOAD AL, 5
LOAD BL, 3
ADD AL, BL
SUB AL, 2
INC AL
DEC BL
READ AL
PRINT AL
```

También pueden probarse expresiones aritméticas normales como:

```text
A + B * C
```

o:

```text
(A + B) * C
```

o:

```text
A + (B * C - D)
```

Estas operaciones permiten comprobar la generación del árbol de expresiones.

---

# Ejemplo de árbol de expresión

Para la expresión:

```text
A + B * C
```

el árbol generado tiene la siguiente estructura:

```text
       +
      / \
     A   *
        / \
       B   C
```

### Preorden

```text
+ A * B C
```

### Inorden

```text
A + B * C
```

### Postorden

```text
A B C * +
```

El mismo procedimiento de generación y recorrido del árbol se presenta dentro de los resultados de los analizadores léxico, sintáctico y semántico.

---

# Opciones disponibles

El proyecto mantiene las opciones implementadas anteriormente y agrega el procesamiento de expresiones y árboles de expresión.

Entre las opciones principales se encuentran:

* Análisis léxico.
* Análisis sintáctico.
* Análisis semántico.
* Tabla de tokens.
* Tabla de símbolos.
* Detección de errores léxicos.
* Detección de errores sintácticos.
* Detección de errores semánticos.
* Reglas de producción.
* Árbol sintáctico.
* Árbol de expresiones.
* Recorrido Preorden.
* Recorrido Inorden.
* Recorrido Postorden.
* Procesamiento de instrucciones.
* Procesamiento de operaciones aritméticas.

---

# Limitaciones

El proyecto no pretende implementar todas las instrucciones disponibles en el procesador 8086.

Solamente se considera el conjunto de instrucciones y registros definidos para este analizador.

Las instrucciones principales reconocidas son:

```text
ADD
SUB
INC
DEC
```

Los registros disponibles son:

```text
AL AH BL BH CL CH DL DH
```

Además, se permite trabajar con expresiones aritméticas utilizando:

```text
+
-
*
/
(
)
```

y números enteros y operandos utilizados por el analizador.

Por lo tanto, otras instrucciones, registros, operadores o estructuras que no estén contemplados por el programa pueden ser considerados como elementos no reconocidos o generar errores durante el análisis.

---

# Conclusión

El proyecto permite observar de manera práctica cómo funcionan las diferentes etapas del análisis de un lenguaje.

A partir de un código de entrada, el programa identifica sus elementos mediante el análisis léxico, comprueba su estructura mediante el análisis sintáctico y realiza diferentes validaciones mediante el análisis semántico.

Una de las principales mejoras incorporadas es el procesamiento de **operaciones aritméticas y la generación de árboles de expresiones**.

El árbol permite representar gráficamente la estructura de una expresión y obtener sus tres recorridos:

* **Preorden**
* **Inorden**
* **Postorden**

Estos recorridos se encuentran disponibles en los tres analizadores: **léxico, sintáctico y semántico**.

La integración de la implementación en **C** con la página web permite presentar los resultados de una manera más clara e interactiva, conservando las diferentes opciones del proyecto original y agregando el procesamiento de expresiones y árboles de expresión.

De esta manera, el proyecto permite demostrar de forma práctica los conceptos de análisis léxico, sintáctico, semántico, expresiones, reglas de producción y árboles utilizados en el procesamiento de lenguajes.

# Analizador Final Hecho en .C Reflejado en Html/Pagina Web

LENGUAJES Y AUTOMATAS II
GRUPO: 6N1
DOCENTE:ECHEVERRIA RODRIGEZ CESAR OSVALDO
FECHA: 8/09/2026

# Nombres de Equipo:
Carlos Daniel Guzmán Morales
Jaqueline Hernandez Patricio
MONSERRAT AURORA ABIGAIL VACA QUEROL

# Numero de Equipo:
5

# Analizador Léxico, Sintáctico y Semántico para 8086

## Descripción

Este proyecto consiste en un analizador para un conjunto de instrucciones del lenguaje ensamblador 8086. El programa permite realizar un análisis léxico, sintáctico y semántico del código introducido por el usuario.

El proyecto fue desarrollado con el propósito de aplicar los conceptos vistos en la materia de compiladores y analizar cómo se puede procesar un lenguaje desde sus elementos básicos hasta la validación de sus instrucciones.

El programa también genera resultados en archivos HTML para facilitar la visualización de los análisis realizados.

## Objetivo

El objetivo del proyecto es implementar las principales etapas de análisis de un lenguaje:

* Identificar los elementos que forman cada instrucción.
* Comprobar que las instrucciones tengan una estructura correcta.
* Detectar algunas situaciones que pueden generar problemas durante la ejecución de las instrucciones.
* Mostrar los resultados de cada análisis de una forma sencilla.

## Tecnologías utilizadas

* C
* HTML5
* CSS3
* JavaScript
* Node.js

## Instrucciones reconocidas

El analizador trabaja con un conjunto reducido de instrucciones del 8086:

* `ADD`
* `SUB`
* `INC`
* `DEC`

## Registros reconocidos

Se utilizan los registros de 8 bits:

* `AL`
* `AH`
* `BL`
* `BH`
* `CL`
* `CH`
* `DL`
* `DH`

## Análisis léxico

El análisis léxico es la primera etapa del programa. Se encarga de leer el código carácter por carácter e identificar los elementos que forman cada instrucción.

Entre los elementos que reconoce se encuentran:

* Instrucciones.
* Registros de 8 bits.
* Comas.
* Espacios.
* Saltos de línea.
* Caracteres o palabras que no son reconocidos.

Por ejemplo, para la instrucción:

```asm
ADD AL, BH
```

el analizador identifica:

* `ADD` como instrucción.
* `AL` como registro.
* `,` como coma.
* `BH` como registro.

El programa guarda esta información y posteriormente la utiliza en las siguientes etapas del análisis.

## Análisis sintáctico

El análisis sintáctico comprueba que las instrucciones tengan la estructura esperada.

Para las instrucciones `ADD` y `SUB`, la estructura utilizada es:

```text
INSTRUCCION REGISTRO, REGISTRO
```

Por ejemplo:

```asm
ADD AL, BH
SUB AH, BL
```

Para `INC` y `DEC`, la estructura es:

```text
INSTRUCCION REGISTRO
```

Por ejemplo:

```asm
INC AL
DEC BH
```

Si una instrucción no cumple con la estructura correspondiente, el programa reporta un error sintáctico.

Algunos ejemplos son:

```asm
ADD AL
```

porque falta el segundo registro, o:

```asm
ADD AL BH
```

porque falta la coma entre los registros.

## Análisis semántico

El análisis semántico se encarga de revisar algunas condiciones relacionadas con el uso de los registros.

Entre las comprobaciones realizadas se encuentran:

* Compatibilidad entre los registros utilizados.
* Uso del mismo registro como origen y destino.
* Uso de registros que todavía no han sido utilizados.
* Seguimiento de las operaciones de lectura y escritura de los registros.
* Detección de registros que fueron modificados pero posteriormente no fueron utilizados.

Por ejemplo:

```asm
ADD AL, AL
```

puede generar una advertencia debido a que el mismo registro se utiliza como destino y origen de la operación.

También se mantiene información sobre el estado de los registros durante el análisis para poder detectar algunos de estos casos.

## Resultados

El programa genera diferentes archivos HTML dependiendo del análisis realizado:

* `resultado_lexico.html`
* `resultado_sintactico.html`
* `resultado_semantico.html`

### Resultado léxico

El resultado léxico muestra información sobre los tokens encontrados durante el análisis.

También presenta datos como la cantidad de instrucciones, registros, comas, espacios y elementos desconocidos.

### Resultado sintáctico

El resultado sintáctico muestra las instrucciones procesadas y determina si cada una cumple con la estructura establecida.

Los resultados pueden indicar que una instrucción es correcta o mostrar el error encontrado.

### Resultado semántico

El resultado semántico muestra las comprobaciones realizadas sobre las instrucciones y los registros utilizados.

En esta sección pueden aparecer resultados correctos, advertencias o errores dependiendo del código analizado.

## Estructura del proyecto

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

## Archivos principales

### `lex8086.c`

Es el archivo principal del proyecto. Contiene la implementación del analizador léxico, sintáctico y semántico, además de las funciones utilizadas para generar los resultados.

### `lex8086.exe`

Es el ejecutable del programa para Windows.

### `lex8086`

Es la versión ejecutable generada para sistemas compatibles.

### `resultado_lexico.html`

Contiene los resultados correspondientes al análisis léxico.

### `resultado_sintactico.html`

Contiene los resultados correspondientes al análisis sintáctico.

### `resultado_semantico.html`

Contiene los resultados correspondientes al análisis semántico.

### `index.js`

Archivo relacionado con la parte de Node.js incluida en el proyecto.

### `package.json`

Contiene la configuración y dependencias utilizadas por la parte de Node.js del proyecto.

## Ejecución

Si se utiliza Windows, el programa puede ejecutarse mediante:

```bash
lex8086.exe
```

También es posible ejecutarlo desde la terminal:

```bash
.\lex8086.exe
```

## Compilación

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

## Funcionamiento

Al iniciar el programa se muestra un menú con las opciones disponibles para realizar cada uno de los análisis.

El usuario puede seleccionar el tipo de análisis que desea realizar y utilizar un bloque de código de prueba o introducir sus propias instrucciones.

Una vez realizado el análisis, el programa procesa las instrucciones y genera el resultado correspondiente.

## Ejemplo

Entrada:

```asm
ADD AL, BH
SUB AH, BL
INC AL
DEC BH
```

El programa procesa cada instrucción y realiza las tres etapas de análisis, mostrando los resultados correspondientes en los archivos HTML generados.

## Limitaciones

El proyecto no pretende implementar todas las instrucciones disponibles en el procesador 8086. Solamente se considera el conjunto de instrucciones y registros definidos para este analizador.

Las instrucciones reconocidas son:

```text
ADD
SUB
INC
DEC
```

Y los registros disponibles son:

```text
AL AH BL BH CL CH DL DH
```

Por lo tanto, otras instrucciones o registros que no estén contemplados por el programa serán considerados como elementos no reconocidos.

## Conclusión

El proyecto permite observar de manera práctica cómo funcionan las etapas principales del análisis de un lenguaje. A partir de una entrada en ensamblador, el programa identifica sus elementos, comprueba su estructura y realiza diferentes validaciones sobre el uso de los registros.

La generación de los resultados en HTML permite revisar de una manera más clara la información obtenida durante cada etapa del análisis.


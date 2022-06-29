# Validador de DLLs

## Introducción
Programa que importa una DLL y comprueba si cumple los requisitos de Securemirror.
Esta aplicación valida los cifradores y challenges creados para Securemirror y ayuda a verificar si se ejecutan de la manera deseada.

## Instalación
Descargue o clone el repositorio.
```
git clone https://github.com/SecureworldProject/DLL_VALIDATOR-NOKIA
```
La carpeta del proyecto debería quedar así:

<img width="444" alt="git1" src="https://user-images.githubusercontent.com/9071577/176383211-f54a51d8-691c-4a13-b039-5a2846249c25.png">

Ahora ya puede compilar el proyecto. Para evitar problemas, recomendamos hacerlo en modo Realease x64.

## Requisitos
Para hacer funcionar el validador de manera correcta las librerías (archivos .dll) de los cifradores y challenges que desee validar y los archivos .json correspondientes han de estar en el directorio x64/Release o donde se encuentre el ejecutable DLL_validator.exe.

Debe tener instalado Python 3.10 y la librería python310.dll en el directorio x64/Release.
Las posibles rutas de instalación de Python (donde podrá encontrar el archivo python3.dll son):

*- C:\Program Files\Python310*

*- C:\Users\username\AppData\Local\Programs\Python\Python310*

## Uso
Aunque es posible hacer funcionar el validador ejecutándolo desde Visual Studio (o el entorno de programación elegido), recomendamos abrir la consola en la carpeta *x64/Release* y desde la línea de comandos llamar directamente a DLL_validator.exe.

De esta manera evitamos problemas, ya que el programa podría buscar archivos en la carpeta donde se encuentra la solución del proyecto y no donde se encuentra el ejecutable.

<img width="663" alt="git2" src="https://user-images.githubusercontent.com/9071577/176378637-0023ebf9-80e1-41eb-8850-f18813485f61.png">

### Challenges en C
Antes de validar un challenge en C, comprobar que el archivo .dll y todos los archivos necesarios para la ejecución del mismo se encuentran en la misma carpeta que el ejecutable.

Para validar el challenge en C simplemente tendrás que introducir el nombre de la librería en el validador, como se muestra a continuación:

### Challenges en Python
Si tu challenge python usa un módulo instalable, debes copiar la librería python310.dll despues de instalar dicho módulo, ya que por cada instalación la librería python310.dll cambia.

El módulo python (fichero .py) que contiene tu challenge debe estar en el directorio donde se encuentre el mismo ejecutable (DLL_validator.exe) para que el programa validador pueda encontrarlo.

### Cifradores
Antes de validar un cifrador, comprobar que el archivo .dll y los archivos a cifrar/descifrar se encuentran en la misma carpeta que el ejecutable.

